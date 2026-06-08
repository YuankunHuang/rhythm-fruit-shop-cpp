#pragma once
#include <cstddef>

namespace rfs::test {
	class ScopedAllocGuard {
	public:
		explicit ScopedAllocGuard(bool trap = false) noexcept;
		~ScopedAllocGuard() noexcept;
		std::size_t allocations() const noexcept; // incremental allocations since construction

	private:
		std::size_t start_allocs_;
		bool prev_trap_;
	};
}