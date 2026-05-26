#pragma once

#include "FrameContext.h"
#include "../platform/InputEvent.h"

namespace rfs {

	/// Base class for all full-screen views and overlay panels.
	///
	/// Lifecycle (called by UIManager):
	///   NavigateTo  →  OnEnter()               — pushed for the first time
	///   GoBack      →  OnExit()                 — popped (just before destruction)
	///   Covered     →  OnPause()                — another screen pushed on top
	///   Uncovered   →  OnResume()               — screen on top was popped
	///
	/// Async loading pattern:
	///   Start loading in OnEnter(), return IsReady()==false while in progress.
	///   UIManager renders the screen below until IsReady() returns true.
	///
	/// Overlay pattern:
	///   Override IsOverlay() to return true. UIManager will render through
	///   to the non-overlay screen below.
	class IScreen {
	public:
		virtual ~IScreen() = default;

		// Lifecycle hooks — all have empty default implementations
		virtual void OnEnter() {}
		virtual void OnExit() {}
		virtual void OnPause() {}
		virtual void OnResume() {}

		// Returns false while async loading is in progress.
		// UIManager skips rendering this screen (and renders the one below) until ready.
		virtual bool IsReady() const noexcept { return true; }

		// Returns true for overlay screens (e.g. Pause). UIManager renders through.
		virtual bool IsOverlay() const noexcept { return false; }

		// Core loop — driven by UIManager every frame
		virtual void Update(const FrameContext& ctx) = 0;
		virtual void Render() = 0;
		virtual void HandleInput(const InputEvent& evt) = 0;
	};

}