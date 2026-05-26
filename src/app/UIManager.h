#pragma once

#include "IScreen.h"
#include <memory>
#include <vector>

namespace rfs {

	/// Navigation controller for all IScreen instances.
	///
	/// Navigation is stack-based:
	///   NavigateTo  — push a new screen on top
	///   GoBack      — pop the top screen (deferred, safe to call from HandleInput)
	///   GoBackToRoot — pop all screens except the root (deferred)
	///
	/// Deferred pops (GoBack/GoBackToRoot) are executed by FlushPending(),
	/// which Application::Run() calls after all HandleInput events have been
	/// dispatched. This prevents "delete-this" crashes.
	///
	/// Lifecycle hooks are called automatically:
	///   NavigateTo  → old Top.OnPause() → new Top.OnEnter()
	///   FlushPending (after GoBack) → popped.OnExit() → new Top.OnResume()
	class UIManager {
	public:
		void NavigateTo(std::unique_ptr<IScreen> screen);
		void GoBack();
		void GoBackToRoot();
		// Replaces the current top screen with a new one (deferred, like GoBack).
		// Used for Loading→Gameplay and Gameplay→Result transitions to keep the stack clean.
		void ReplaceTop(std::unique_ptr<IScreen> screen);
		void FlushPending();

		IScreen& Top();
		bool IsEmpty() const;
		int Depth() const;

		// Read-only access to the full screen list (used by Application for rendering).
		const std::vector<std::unique_ptr<IScreen>>& Screens() const { return screens_; }

	private:
		std::vector<std::unique_ptr<IScreen>> screens_;
		int pending_pops_ = 0;
		std::unique_ptr<IScreen> pending_replace_;
		// True while we are waiting for the new top screen to become ready.
		// The screen just below the top (the old one) is kept alive and gets
		// OnExit() only after the new screen reports IsReady() == true.
		bool replacing_ = false;
	};
}