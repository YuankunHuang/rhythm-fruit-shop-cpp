#pragma once

#include <filesystem>

namespace rfs {
	class IAudioPlayer {
	public:
		virtual ~IAudioPlayer() = default;

		virtual bool Load(const std::filesystem::path& path) = 0;
		virtual void Play() = 0;
		virtual void Stop() = 0;
		virtual void Pause() = 0;
		virtual void Resume() = 0;
		virtual bool IsPlaying() const noexcept = 0;
		virtual void SetLooping(bool looping) = 0;
	};
}