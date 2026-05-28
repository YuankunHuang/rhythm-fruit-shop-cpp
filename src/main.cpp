#include "app/Application.h"
#include "platform/sfml/SfmlWindow.h"
#include "platform/sfml/SfmlRenderer.h"
#include "platform/sfml/SfmlInputSource.h"
#include "platform/miniaudio/MiniaudioAudioPlayer.h"
#include "platform/miniaudio/MiniaudioAudioBackendClock.h"
#include "rhythm/SmoothedSongClock.h"
#include "game/PlaySessionConfig.h"

int main() {

	rfs::SfmlWindow window(1280, 720, "Rhythm Fruit Shop");
	rfs::SfmlRenderer renderer(window);
	rfs::SfmlInputSource input(window);

	rfs::MiniaudioAudioPlayer player;
	rfs::MiniaudioAudioPlayer bgm_player;

	rfs::MiniaudioAudioBackendClock backend_clock{player};
	rfs::SmoothedSongClock song_clock{};
	rfs::PlaySessionConfig session{};
	rfs::Application app(window, input, renderer, backend_clock, song_clock, player, bgm_player, session);

	return app.Run() ? 0 : 1;
}
