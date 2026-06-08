#include "AllocationGuard.h"
#include <cstdlib>
#include <new>
#include <cassert>

namespace {
	thread_local std::size_t g_allocs = 0;
	thread_local int g_armed = 0; // depth
	thread_local bool g_trap = false;
}

void* operator new(std::size_t n) {
	++g_allocs;
	if (g_trap && g_armed > 0) {
		assert(!"alloc in hot path!");
	}
	if (void* p = std::malloc(n ? n : 1)) {
		return p;
	}
	throw std::bad_alloc{};
}
void operator delete(void* p) noexcept {
	std::free(p);
}
void* operator new[](std::size_t n) {
	++g_allocs;
	if (g_trap && g_armed > 0) {
		assert(!"alloc in hot path!");
	}
	if (void* p = std::malloc(n ? n : 1)) {
		return p;
	}
	throw std::bad_alloc{};
}
void operator delete[](void* p) noexcept {
	std::free(p);
}

namespace rfs::test {
	ScopedAllocGuard::ScopedAllocGuard(bool trap) noexcept
		: start_allocs_(g_allocs), prev_trap_(g_trap) {
		g_trap = trap;
		++g_armed;
	}

	ScopedAllocGuard::~ScopedAllocGuard() noexcept {
		--g_armed;
		g_trap = prev_trap_;
	}

	std::size_t ScopedAllocGuard::allocations() const noexcept {
		return g_allocs - start_allocs_;
	}
}