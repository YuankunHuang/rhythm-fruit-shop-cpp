#include "MiniaudioAudioPlayer.h"
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4267)
#pragma warning(disable : 4456)
#endif
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <algorithm>
#include <atomic>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <utility>

namespace rfs {

	// -------------------------------------------------------------------------
	// Impl
	// -------------------------------------------------------------------------
	// Two sound slots (double-buffering):
	//   sounds_[active_]  — the live, playable sound (main thread only)
	//   sounds_[pending_] — being loaded on load_thread_; promoted to active
	//                       by IsAudioReady() / Load() once the thread is done
	//
	// Rapid song switching without blocking:
	//   If LoadAsync() is called while load_thread_ is still running, the new
	//   path is stored in next_load_path_ and the thread is NOT joined.
	//   IsAudioReady() picks up next_load_path_ once the thread finishes:
	//   it discards the stale result and immediately starts the deferred load.
	// -------------------------------------------------------------------------
	struct MiniaudioAudioPlayer::Impl {
		ma_engine engine_{};
		bool      engine_ready_ = false;

		ma_sound  sounds_[2]{};
		bool      sound_loaded_[2] = { false, false };
		int       active_  = 0;
		int       pending_ = 1;

		std::thread       load_thread_;
		std::atomic<bool> pending_ready_{ false };
		std::string       next_load_path_;   // deferred request; cleared by any Load/LoadAsync in "safe" path

		Impl() {
			if (ma_engine_init(nullptr, &engine_) != MA_SUCCESS) return;
			engine_ready_ = true;
		}

		~Impl() {
			// Must join before touching miniaudio resources.
			if (load_thread_.joinable()) load_thread_.join();
			for (int i = 0; i < 2; ++i) {
				if (sound_loaded_[i]) {
					ma_sound_uninit(&sounds_[i]);
					sound_loaded_[i] = false;
				}
			}
			if (engine_ready_) {
				ma_engine_uninit(&engine_);
				engine_ready_ = false;
			}
		}

		// Uninit active slot, promote pending to active.
		// Call only from the main thread, after load_thread_ has been joined
		// and pending_ready_ is true.
		void CommitPending() {
			if (sound_loaded_[active_]) {
				ma_sound_uninit(&sounds_[active_]);
				sound_loaded_[active_] = false;
			}
			std::swap(active_, pending_);
			pending_ready_ = false;
		}

		// Uninit the pending slot and reset its state.  Safe to call any time
		// from the main thread when no load thread is running.
		void DiscardPending() {
			if (sound_loaded_[pending_]) {
				ma_sound_uninit(&sounds_[pending_]);
				sound_loaded_[pending_] = false;
			}
			pending_ready_ = false;
		}

		// Spin up a background thread to load path_str into the pending slot.
		// Caller must ensure no load thread is currently running.
		void SpawnLoadThread(std::string path_str) {
			const int slot = pending_;
			load_thread_ = std::thread([this, path_str, slot] {
				ma_result r = ma_sound_init_from_file(
					&engine_, path_str.c_str(),
					MA_SOUND_FLAG_DECODE,
					nullptr, nullptr,
					&sounds_[slot]);
				sound_loaded_[slot] = (r == MA_SUCCESS);
				if (r != MA_SUCCESS) {
					std::cerr << "[audio] load failed (" << ma_result_description(r)
						<< "): " << path_str << '\n';
				}
				pending_ready_ = true;
			});
		}
	};

	// -------------------------------------------------------------------------
	MiniaudioAudioPlayer::MiniaudioAudioPlayer() : pimpl_(std::make_unique<Impl>()) {}
	MiniaudioAudioPlayer::~MiniaudioAudioPlayer() = default;

	// -------------------------------------------------------------------------
	// Synchronous load — used for gameplay audio where we need it immediately.
	// Joins any in-flight async thread, discards its result, then loads
	// synchronously into the pending slot and commits.
	// -------------------------------------------------------------------------
	bool MiniaudioAudioPlayer::Load(const std::filesystem::path& path) {
		if (!pimpl_->engine_ready_) return false;

		// Cancel any deferred async request and wait for the in-flight thread.
		pimpl_->next_load_path_.clear();
		if (pimpl_->load_thread_.joinable()) pimpl_->load_thread_.join();
		pimpl_->DiscardPending();

		if (!std::filesystem::exists(path)) {
			std::cerr << "[audio] file not found: " << path << '\n';
			return false;
		}

		ma_result r = ma_sound_init_from_file(
			&pimpl_->engine_,
			path.string().c_str(),
			MA_SOUND_FLAG_DECODE,
			nullptr, nullptr,
			&pimpl_->sounds_[pimpl_->pending_]);

		if (r != MA_SUCCESS) {
			std::cerr << "[audio] Load failed (" << ma_result_description(r)
				<< "): " << path << '\n';
			return false;
		}

		pimpl_->sound_loaded_[pimpl_->pending_] = true;
		pimpl_->pending_ready_ = true;
		pimpl_->CommitPending();
		return true;
	}

	// -------------------------------------------------------------------------
	// Asynchronous load — used for preview audio.
	// Never blocks: if a load thread is already running, stores the new path
	// in next_load_path_ and returns immediately.  IsAudioReady() will start
	// the deferred load once the current thread finishes.
	// -------------------------------------------------------------------------
	bool MiniaudioAudioPlayer::LoadAsync(const std::filesystem::path& path) {
		if (!pimpl_->engine_ready_) return false;

		if (pimpl_->load_thread_.joinable() && !pimpl_->pending_ready_.load()) {
			// Thread still running — record the new path and return without blocking.
			pimpl_->next_load_path_ = path.string();
			return true;
		}

		// Thread is done (or was never started) — safe to join and reuse slot.
		pimpl_->next_load_path_.clear();
		if (pimpl_->load_thread_.joinable()) pimpl_->load_thread_.join();
		pimpl_->DiscardPending();

		if (!std::filesystem::exists(path)) {
			std::cerr << "[audio] file not found: " << path << '\n';
			return false;
		}

		pimpl_->SpawnLoadThread(path.string());
		return true;
	}

	// -------------------------------------------------------------------------
	// Poll from the main thread each frame while in Loading state.
	// Returns true exactly once when the latest requested audio is ready and
	// has been promoted to the active slot.
	// -------------------------------------------------------------------------
	bool MiniaudioAudioPlayer::IsAudioReady() {
		if (!pimpl_->pending_ready_.load()) return false;

		// Thread is done — join now (non-blocking since pending_ready_ is set).
		if (pimpl_->load_thread_.joinable()) pimpl_->load_thread_.join();

		// A newer path arrived while the thread was running.
		// Discard the stale result and immediately load the latest request.
		if (!pimpl_->next_load_path_.empty()) {
			std::string next = std::exchange(pimpl_->next_load_path_, {});
			pimpl_->DiscardPending();
			pimpl_->SpawnLoadThread(std::move(next));
			return false; // still loading
		}

		if (!pimpl_->sound_loaded_[pimpl_->pending_]) {
			// Load failed — clear state, stay in Loading so caller can retry or give up.
			pimpl_->pending_ready_ = false;
			return false;
		}

		pimpl_->CommitPending();
		return true;
	}

	// -------------------------------------------------------------------------
	// Playback controls — all operate on the active slot only.
	// -------------------------------------------------------------------------

	void MiniaudioAudioPlayer::Play() {
		if (pimpl_->sound_loaded_[pimpl_->active_])
			ma_sound_start(&pimpl_->sounds_[pimpl_->active_]);
	}

	void MiniaudioAudioPlayer::Stop() {
		if (pimpl_->sound_loaded_[pimpl_->active_])
			ma_sound_stop(&pimpl_->sounds_[pimpl_->active_]);
	}

	// Sound-level stop/start keeps the engine running at all times.
	// (Engine-level stop caused IsPlaying() to return stale values, preventing
	// BGM from resuming when returning to ChartSelectScreen.)
	void MiniaudioAudioPlayer::Pause() {
		if (pimpl_->sound_loaded_[pimpl_->active_])
			ma_sound_stop(&pimpl_->sounds_[pimpl_->active_]);
	}

	void MiniaudioAudioPlayer::Resume() {
		if (pimpl_->sound_loaded_[pimpl_->active_])
			ma_sound_start(&pimpl_->sounds_[pimpl_->active_]);
	}

	bool MiniaudioAudioPlayer::IsPlaying() const noexcept {
		if (!pimpl_->sound_loaded_[pimpl_->active_]) return false;
		return ma_sound_is_playing(&pimpl_->sounds_[pimpl_->active_]) != 0;
	}

	void MiniaudioAudioPlayer::SetLooping(bool looping) {
		if (pimpl_->sound_loaded_[pimpl_->active_])
			ma_sound_set_looping(&pimpl_->sounds_[pimpl_->active_],
				looping ? MA_TRUE : MA_FALSE);
	}

	void MiniaudioAudioPlayer::SetVolume(float vol) {
		if (pimpl_->sound_loaded_[pimpl_->active_])
			ma_sound_set_volume(&pimpl_->sounds_[pimpl_->active_],
				std::clamp(vol, 0.f, 1.f));
	}

	void MiniaudioAudioPlayer::Seek(float time_ms) {
		if (pimpl_->sound_loaded_[pimpl_->active_])
			ma_sound_seek_to_second(&pimpl_->sounds_[pimpl_->active_], time_ms / 1000.f);
	}

	std::uint64_t MiniaudioAudioPlayer::CursorInPcmFrames() const {
		if (!pimpl_->sound_loaded_[pimpl_->active_]) return 0;
		ma_uint64 cursor = 0;
		ma_sound_get_cursor_in_pcm_frames(&pimpl_->sounds_[pimpl_->active_], &cursor);
		return static_cast<std::uint64_t>(cursor);
	}

	std::int32_t MiniaudioAudioPlayer::SampleRate() const {
		return static_cast<std::int32_t>(ma_engine_get_sample_rate(&pimpl_->engine_));
	}

	bool MiniaudioAudioPlayer::IsSoundLoaded() const noexcept {
		return pimpl_->sound_loaded_[pimpl_->active_];
	}

}
