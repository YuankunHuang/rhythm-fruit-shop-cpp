#include "app/Application.h"
#include "platform/sfml/SfmlWindow.h"
#include "platform/sfml/SfmlRenderer.h"
#include "platform/sfml/SfmlInputSource.h"

int main() {

	rfs::SfmlWindow window(800, 600, "Rhythm Fruit Shop");
	rfs::SfmlRenderer renderer(window);
	rfs::SfmlInputSource input{};
	rfs::Application app(window, input, renderer);

	return app.Run() ? 0 : 1;
}