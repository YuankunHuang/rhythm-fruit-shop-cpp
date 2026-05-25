#include "app/Application.h"
#include "platform/sfml/SfmlWindow.h"
#include "platform/sfml/SfmlRenderer.h"
#include "platform/sfml/SfmlInputSource.h"
#include "platform/miniaudio/MiniaudioAudioPlayer.h"
#include "platform/miniaudio/MiniaudioAudioBackendClock.h"
#include "rhythm/SmoothedSongClock.h"

int main() {

	rfs::SfmlWindow window(800, 600, "Rhythm Fruit Shop");
	rfs::SfmlRenderer renderer(window);
	rfs::SfmlInputSource input(window);
	rfs::MiniaudioAudioPlayer player;

	// test the player
	if (!player.Load("assets/audio/service/lemon_water_light.mp3")) {
		return 1; // failed to load audio, cannot continue
	}
	player.Play();

	rfs::MiniaudioAudioBackendClock backend_clock{player};
	rfs::SmoothedSongClock song_clock{};
	rfs::Application app(window, input, renderer, backend_clock, song_clock);

	return app.Run() ? 0 : 1;
}