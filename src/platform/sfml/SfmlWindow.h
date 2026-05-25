#pragma once

#include "../IWindow.h"
#include <memory>

// forward declaration :D we do not want to leak SFML to the external
namespace sf { class RenderWindow; }

namespace rfs {
	class SfmlWindow final : public IWindow {
	public:
		explicit SfmlWindow(unsigned width, unsigned height, const char* title);
		~SfmlWindow();

		bool IsOpen() const override;
		void Close() override;
		sf::RenderWindow& RenderTarget();

	private:
		struct Impl; // use pimple pattern, to avoid leaking SFML in header
		std::unique_ptr<Impl> pimpl;
	};
}