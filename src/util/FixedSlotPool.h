#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace rfs {

	// Fixed-capacity, heap-free object pool with DOD-packed storage.
	//
	// Layout (classic sparse set, packed variant):
	//   data_[i]   : the i-th live T, packed in [0, active_count_)
	//   uid_[i]    : stable uid that owns data_[i]  (parallel to data_)
	//   sparse_[u] : uid -> packed index i, or kInvalid when uid u is free
	// Invariant: sparse_[uid_[i]] == i for all i < active_count_.
	//
	// Live elements stay contiguous in data_, so ForEachActive is a flat,
	// cache-friendly scan. Acquire returns a stable uid (survives other slots'
	// Release via swap-and-pop). The pool requires no fields on T; when full,
	// Acquire fails (valid == false) and the caller decides what to do.
	template<typename T, std::size_t Capacity>
	class FixedSlotPool {
	public:
		static_assert(Capacity > 0, "FixedSlotPool capacity must be positive");
		static_assert(Capacity <= 0xFFFEu, "FixedSlotPool capacity must fit uint16_t sentinel");

		struct SlotHandle {
			std::size_t index = 0; // stable uid
			bool valid = false;
		};

		FixedSlotPool() {
			sparse_.fill(kInvalid);
		}

		std::size_t ActiveCount() const noexcept { return active_count_; }
		bool Empty() const noexcept { return active_count_ == 0; }
		bool Full() const noexcept { return active_count_ >= Capacity; }

		// Reserve a slot. Returns a stable uid handle, or { valid = false } when full.
		SlotHandle Acquire() {
			if (Full()) {
				return {};
			}
			const std::uint16_t uid = FindFreeUid();
			const std::uint16_t pos = static_cast<std::uint16_t>(active_count_);
			uid_[pos] = uid;
			sparse_[uid] = pos;
			++active_count_;
			return { static_cast<std::size_t>(uid), true };
		}

		// Release the slot owning uid (O(1) swap-and-pop). No-op if uid is not live.
		void Release(std::size_t uid) {
			if (uid >= Capacity) return;
			const std::uint16_t pos = sparse_[uid];
			if (pos == kInvalid) return;

			const std::uint16_t last = static_cast<std::uint16_t>(active_count_ - 1);
			if (pos != last) {
				data_[pos] = data_[last];
				uid_[pos] = uid_[last];
				sparse_[uid_[pos]] = pos;
			}
			sparse_[uid] = kInvalid;
			--active_count_;
		}

		// Live element by uid, or nullptr if uid is free.
		T* TryGet(std::size_t uid) {
			if (uid >= Capacity) return nullptr;
			const std::uint16_t pos = sparse_[uid];
			if (pos == kInvalid) return nullptr;
			return &data_[pos];
		}

		const T* TryGet(std::size_t uid) const {
			if (uid >= Capacity) return nullptr;
			const std::uint16_t pos = sparse_[uid];
			if (pos == kInvalid) return nullptr;
			return &data_[pos];
		}

		// Visit every live T (mutable). Safe to mutate fields; do NOT Acquire/Release here.
		template<typename Fn>
		void ForEachActive(Fn&& fn) {
			for (std::size_t i = 0; i < active_count_; ++i) {
				fn(data_[i]);
			}
		}

		template<typename Fn>
		void ForEachActive(Fn&& fn) const {
			for (std::size_t i = 0; i < active_count_; ++i) {
				fn(data_[i]);
			}
		}

		// Release every live slot whose predicate returns true. O(active), swap-and-pop safe.
		template<typename Pred>
		void ReleaseIf(Pred&& pred) {
			std::size_t i = 0;
			while (i < active_count_) {
				if (pred(data_[i])) {
					Release(uid_[i]); // swaps last into i; re-check same index
				} else {
					++i;
				}
			}
		}

		void Clear() {
			sparse_.fill(kInvalid);
			active_count_ = 0;
		}

	private:
		static constexpr std::uint16_t kInvalid = 0xFFFFu;

		std::uint16_t FindFreeUid() const {
			for (std::uint16_t u = 0; u < Capacity; ++u) {
				if (sparse_[u] == kInvalid) return u;
			}
			return kInvalid; // unreachable when !Full()
		}

		std::array<T, Capacity> data_{};
		std::array<std::uint16_t, Capacity> uid_{};
		std::array<std::uint16_t, Capacity> sparse_{};
		std::size_t active_count_ = 0;
	};

}
