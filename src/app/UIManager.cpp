#include "UIManager.h"
#include <stdexcept>

namespace rfs {

	void UIManager::NavigateTo(std::unique_ptr<IScreen> screen) {
		if (!screens_.empty()) {
			screens_.back()->OnPause();
		}
		screens_.push_back(std::move(screen));
		screens_.back()->OnEnter();
	}

	void UIManager::GoBack() {
		pending_pops_ += 1;
	}

	void UIManager::GoBackToRoot() {
		// Keep 1 (the root), pop everything above it
		if (static_cast<int>(screens_.size()) > 1) {
			pending_pops_ += static_cast<int>(screens_.size()) - 1;
		}
	}

	void UIManager::ReplaceTop(std::unique_ptr<IScreen> screen) {
		pending_replace_ = std::move(screen);
	}

	void UIManager::FlushPending() {
		// Phase 1: commit a pending replace.
		// The old top gets OnPause() (not OnExit yet); new screen is pushed on top.
		// The old screen stays alive at [size-2] until the new one becomes ready.
		if (pending_replace_) {
			if (!screens_.empty()) {
				screens_.back()->OnPause();
			}
			screens_.push_back(std::move(pending_replace_));
			screens_.back()->OnEnter();
			replacing_ = true;
		}

		// Phase 2: process deferred pops (GoBack / GoBackToRoot).
		while (pending_pops_ > 0 && !screens_.empty()) {
			screens_.back()->OnExit();
			screens_.pop_back();
			--pending_pops_;
			if (!screens_.empty()) {
				screens_.back()->OnResume();
			}
		}
		pending_pops_ = 0;

		// Phase 3: if a replace is in progress and the new top is ready,
		// silently destroy the old screen that was waiting below it.
		if (replacing_ && screens_.size() >= 2 && screens_.back()->IsReady()) {
			auto it = screens_.end() - 2;
			(*it)->OnExit();
			screens_.erase(it);
			replacing_ = false;
		}
	}

	IScreen& UIManager::Top() {
		if (screens_.empty()) {
			throw std::runtime_error("UIManager::Top() called on empty stack");
		}
		return *screens_.back();
	}

	bool UIManager::IsEmpty() const {
		return screens_.empty();
	}

	int UIManager::Depth() const {
		return static_cast<int>(screens_.size());
	}
}
