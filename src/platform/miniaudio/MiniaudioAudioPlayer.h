#pragma once
#include "../IAudioPlayer.h"
#include <filesystem>
#include <memory>
#include <cstdint>

namespace rfs {
	class MiniaudioAudioPlayer final : public IAudioPlayer {
	public:
		MiniaudioAudioPlayer();
		~MiniaudioAudioPlayer() override;

		bool Load(const std::filesystem::path& path) override;
		bool LoadAsync(const std::filesystem::path& path) override;
		bool IsAudioReady() override;
		void Play() override;
		void Stop() override;
		void Pause() override;
		void Resume() override;
		bool IsPlaying() const noexcept override;
		void SetLooping(bool looping) override;
		void SetVolume(float vol) override;
		void Seek(float time_ms) override;

		std::uint64_t CursorInPcmFrames() const;
		std::int32_t SampleRate() const;
		bool IsSoundLoaded() const noexcept;

	private:
		struct Impl;
		std::unique_ptr<Impl> pimpl_;
	};
}
