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
		void Play() override;
		void Stop() override;
		void Pause() override;
		void Resume() override;
		bool IsPlaying() const noexcept override;
		void SetLooping(bool looping) override;

		std::uint64_t CursorInPcmFrames() const;
		std::int32_t SampleRate() const;
		bool IsSoundLoaded() const noexcept;

	private:
		struct Impl;
		std::unique_ptr<Impl> pimpl_;
	};
}