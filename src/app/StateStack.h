#pragma once

#include "IGameState.h"
#include <memory>
#include <vector>

namespace rfs {
	/// <summary>
	/// Serves as a UI/Frame stack manager
	/// </summary>
	class StateStack {
	public:
		void Push(std::unique_ptr<IGameState> state);
		void Pop();
		IGameState& Top();
	private:
		std::vector<std::unique_ptr<IGameState>> stack_;
	};
}