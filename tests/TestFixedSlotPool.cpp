#include <doctest/doctest.h>
#include "util/FixedSlotPool.h"
#include <vector>
#include <utility>
#include <cstddef>
#include <algorithm>

namespace {
	struct Dummy {
		int v = 0;
		float age = 0.f;
	};
}

TEST_CASE("FixedSlotPool - empty pool invariants") {
	rfs::FixedSlotPool<Dummy, 4> pool;

	CHECK(pool.Empty());
	CHECK(pool.ActiveCount() == 0);
	CHECK_FALSE(pool.Full());
	CHECK(pool.TryGet(0) == nullptr);
	CHECK(pool.TryGet(99) == nullptr);

	int visited = 0;
	pool.ForEachActive([&visited](const Dummy&) { ++visited; });
	CHECK(visited == 0);
}

TEST_CASE("FixedSlotPool - acquire and then read back") {
	rfs::FixedSlotPool<Dummy, 4> pool;
	auto handle = pool.Acquire();
	CHECK(handle.valid);
	auto ptr = pool.TryGet(handle.index);
	REQUIRE(ptr != nullptr);
	ptr->v = 42;
	ptr = pool.TryGet(handle.index);
	CHECK(ptr->v == 42);
	CHECK(pool.ActiveCount() == 1);
	CHECK(!pool.Empty());
}

TEST_CASE("FixedSlotPool - fill to capacity then reject") {
	rfs::FixedSlotPool<Dummy, 4> pool;
	std::vector<int> uids(4, -1);
	for (size_t i = 0; i < 4; ++i) {
		auto handle = pool.Acquire();
		CHECK(handle.valid);
		uids[i] = static_cast<int>(handle.index);
	}
	CHECK(pool.Full());
	CHECK(pool.ActiveCount() == 4);
	
	CHECK_FALSE(pool.Acquire().valid);
	for (size_t i = 0; i < 4; ++i) {
		auto ptr = pool.TryGet(static_cast<size_t>(uids[i]));
		REQUIRE(ptr != nullptr);
		CHECK(ptr->v == 0);
	}
}

// Core regression: releasing a non-tail slot triggers swap-and-pop internally.
// External uids of the *other* live elements must still resolve to correct data.
TEST_CASE("FixedSlotPool - release middle keeps other handles valid") {
	rfs::FixedSlotPool<Dummy, 4> pool;
	auto a = pool.Acquire();
	auto b = pool.Acquire();
	auto c = pool.Acquire();
	REQUIRE(a.valid);
	REQUIRE(b.valid);
	REQUIRE(c.valid);
	pool.TryGet(a.index)->v = 10;
	pool.TryGet(b.index)->v = 20;
	pool.TryGet(c.index)->v = 30;

	pool.Release(b.index); // b is not the tail -> c gets swapped into b's packed slot

	CHECK(pool.TryGet(b.index) == nullptr);
	REQUIRE(pool.TryGet(a.index) != nullptr);
	REQUIRE(pool.TryGet(c.index) != nullptr);
	CHECK(pool.TryGet(a.index)->v == 10);
	CHECK(pool.TryGet(c.index)->v == 30); // moved internally, but uid still maps correctly
	CHECK(pool.ActiveCount() == 2);
}

TEST_CASE("FixedSlotPool - release then acquire reuses a free slot") {
	rfs::FixedSlotPool<Dummy, 2> pool;
	auto a = pool.Acquire();
	auto b = pool.Acquire();
	REQUIRE(a.valid);
	REQUIRE(b.valid);
	CHECK(pool.Full());

	pool.Release(a.index);
	CHECK_FALSE(pool.Full());
	CHECK(pool.ActiveCount() == 1);

	auto c = pool.Acquire();
	CHECK(c.valid);
	CHECK(pool.ActiveCount() == 2);
	CHECK(pool.Full());
	// b must remain reachable across the churn
	CHECK(pool.TryGet(b.index) != nullptr);
}

TEST_CASE("FixedSlotPool - ForEachActive visits exactly the live set") {
	rfs::FixedSlotPool<Dummy, 8> pool;
	std::vector<rfs::FixedSlotPool<Dummy, 8>::SlotHandle> handles;
	for (int i = 0; i < 5; ++i) {
		auto h = pool.Acquire();
		REQUIRE(h.valid);
		pool.TryGet(h.index)->v = i * 100;
		handles.push_back(h);
	}

	pool.Release(handles[1].index); // remove v=100
	pool.Release(handles[3].index); // remove v=300

	std::vector<int> seen;
	pool.ForEachActive([&seen](const Dummy& d) { seen.push_back(d.v); });

	CHECK(seen.size() == pool.ActiveCount());
	CHECK(seen.size() == 3);

	auto contains = [&seen](int val) {
		return std::find(seen.begin(), seen.end(), val) != seen.end();
	};
	CHECK(contains(0));
	CHECK(contains(200));
	CHECK(contains(400));
	CHECK_FALSE(contains(100)); // released
	CHECK_FALSE(contains(300)); // released
}

// while+swap removal must not skip elements swapped into the current index.
TEST_CASE("FixedSlotPool - ReleaseIf batch removal") {
	rfs::FixedSlotPool<Dummy, 8> pool;
	for (int i = 0; i < 6; ++i) {
		auto h = pool.Acquire();
		REQUIRE(h.valid);
		// Interleave doomed/surviving so removal targets are non-contiguous.
		pool.TryGet(h.index)->age = (i % 2 == 0) ? 999.f : 0.f;
	}

	pool.ReleaseIf([](const Dummy& d) { return d.age >= 500.f; });

	CHECK(pool.ActiveCount() == 3);
	bool all_survivors_low = true;
	pool.ForEachActive([&all_survivors_low](const Dummy& d) {
		if (d.age >= 500.f) all_survivors_low = false;
	});
	CHECK(all_survivors_low);
}

TEST_CASE("FixedSlotPool - release is idempotent and bounds-safe") {
	rfs::FixedSlotPool<Dummy, 4> pool;
	auto a = pool.Acquire();
	REQUIRE(a.valid);
	CHECK(pool.ActiveCount() == 1);

	pool.Release(a.index);
	CHECK(pool.ActiveCount() == 0);
	pool.Release(a.index);   // double release -> no-op
	CHECK(pool.ActiveCount() == 0);

	pool.Release(2);         // never-acquired uid -> no-op
	pool.Release(9999);      // out-of-range uid -> no-op
	CHECK(pool.ActiveCount() == 0);
	CHECK(pool.Empty());
}

TEST_CASE("FixedSlotPool - clear resets to empty") {
	rfs::FixedSlotPool<Dummy, 4> pool;
	auto a = pool.Acquire();
	auto b = pool.Acquire();
	REQUIRE(a.valid);
	REQUIRE(b.valid);

	pool.Clear();
	CHECK(pool.Empty());
	CHECK(pool.ActiveCount() == 0);
	CHECK(pool.TryGet(a.index) == nullptr);
	CHECK(pool.TryGet(b.index) == nullptr);

	// Pool is reusable after Clear.
	auto c = pool.Acquire();
	CHECK(c.valid);
	CHECK(pool.ActiveCount() == 1);
}

// Mixed acquire/release churn: every recorded uid must keep resolving to its
// own value, and the live count must stay consistent.
TEST_CASE("FixedSlotPool - churn keeps uid -> data mapping intact") {
	constexpr std::size_t kCap = 8;
	rfs::FixedSlotPool<Dummy, kCap> pool;
	std::vector<std::pair<std::size_t, int>> live; // {uid, expected v}
	int tag = 1;

	auto verify_all = [&]() {
		for (const auto& [uid, expected] : live) {
			auto* p = pool.TryGet(uid);
			REQUIRE(p != nullptr);
			CHECK(p->v == expected);
		}
		CHECK(pool.ActiveCount() == live.size());
	};

	// Deterministic churn pattern: grow, then alternately drop-front and add.
	for (int round = 0; round < 30; ++round) {
		const bool should_release = (round % 3 == 0) && !live.empty();
		if (should_release) {
			// Release the oldest tracked uid.
			pool.Release(live.front().first);
			live.erase(live.begin());
		}
		if (!pool.Full()) {
			auto h = pool.Acquire();
			REQUIRE(h.valid);
			pool.TryGet(h.index)->v = tag;
			live.emplace_back(h.index, tag);
			++tag;
		}
		verify_all();
	}
}