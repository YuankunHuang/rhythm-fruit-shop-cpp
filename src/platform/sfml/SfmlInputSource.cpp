#include "SfmlInputSource.h"

using namespace rfs;

struct SfmlInputSource::Impl {
};

SfmlInputSource::SfmlInputSource() {

}

SfmlInputSource::~SfmlInputSource() = default;

std::span<const InputEvent> SfmlInputSource::Poll([[maybe_unused]] HostNanos ns) noexcept {
	return {};
}
