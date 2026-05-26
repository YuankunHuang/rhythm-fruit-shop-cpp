#include "MiniaudioAudioPlayer.h"
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4244)  // ma_uint64 -> ma_uint32, etc. -> miniaudio's issue, but we don't want to see these warnings since we know they're safe in this context
#pragma warning(disable : 4267)  // size_t conversion -> potentially present in miniaudio
#pragma warning(disable : 4456)  // hide local variable -> miniaudio's issue (func param' name same as local var)
#endif
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <filesystem>
#include <iostream>

namespace rfs {
	struct MiniaudioAudioPlayer::Impl {
		ma_engine engine_{};
		ma_sound sound_{};
		bool engine_ready_ = false;
		bool sound_loaded_ = false;

		Impl() {
			if (ma_engine_init(nullptr, &engine_) != MA_SUCCESS) {
				return;
			}

			engine_ready_ = true;
		}
		
		~Impl() {
			if (sound_loaded_) {
				ma_sound_uninit(&sound_);
				sound_loaded_ = false;
			}
			if (engine_ready_) {
				ma_engine_uninit(&engine_);
				engine_ready_ = false;
			}
		}
	};

	MiniaudioAudioPlayer::MiniaudioAudioPlayer() : pimpl_(std::make_unique<Impl>()) {}
	MiniaudioAudioPlayer::~MiniaudioAudioPlayer() = default;

	bool MiniaudioAudioPlayer::Load(const std::filesystem::path& path) {
		if (!pimpl_->engine_ready_) {
			return false;
		}

		// if we already have a sound loaded, unload it first
		if (pimpl_->sound_loaded_) {
			ma_sound_uninit(&pimpl_->sound_);
			pimpl_->sound_ = {};
			pimpl_->sound_loaded_ = false;
		}

		if (!std::filesystem::exists(path)) {
			std::cerr << "Audio file does not exist: " << path << std::endl;
			return false;
		}

		if (!pimpl_->engine_ready_) {
			std::cerr << "ma_engine_init failed\n";
			return false;
		}

		const ma_result result = ma_sound_init_from_file(
			&pimpl_->engine_,
			path.string().c_str(),
			MA_SOUND_FLAG_DECODE,
			nullptr,
			nullptr,
			&pimpl_->sound_
			);
		if (result != MA_SUCCESS) {
			std::cerr << "ma_sound_init_from_file failed: "
				<< ma_result_description(result)
				<< " (code=" << static_cast<int>(result) << ")\n";
			return false;
		}

		pimpl_->sound_loaded_ = true;
		return true;
	}
	void MiniaudioAudioPlayer::Play() {
		if (!pimpl_->sound_loaded_) {
			return;
		}
		ma_sound_start(&pimpl_->sound_);
	}
	void MiniaudioAudioPlayer::Stop() {
		ma_sound_stop(&pimpl_->sound_);
	}
	void MiniaudioAudioPlayer::Pause() {
		if (pimpl_->engine_ready_) {
			ma_engine_stop(&pimpl_->engine_);
		}
	}
	void MiniaudioAudioPlayer::Resume() {
		if (pimpl_->engine_ready_) {
			ma_engine_start(&pimpl_->engine_);
		}
	}
	bool MiniaudioAudioPlayer::IsPlaying() const noexcept {
		return ma_sound_is_playing(&pimpl_->sound_) != 0;
	}

	void MiniaudioAudioPlayer::SetLooping(bool looping) {
		if (pimpl_->sound_loaded_) {
			ma_sound_set_looping(&pimpl_->sound_, looping ? MA_TRUE : MA_FALSE);
		}
	}

	std::uint64_t MiniaudioAudioPlayer::CursorInPcmFrames() const {
		ma_uint64 cursor = 0;
		ma_sound_get_cursor_in_pcm_frames(&pimpl_->sound_, &cursor);
		return static_cast<std::uint64_t>(cursor);
	}
	std::int32_t MiniaudioAudioPlayer::SampleRate() const {
		return static_cast<std::int32_t>(ma_engine_get_sample_rate(&pimpl_->engine_));
	}
	bool MiniaudioAudioPlayer::IsSoundLoaded() const noexcept {
		return pimpl_->sound_loaded_;
	}
}