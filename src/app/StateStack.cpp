#include "StateStack.h"
#include <stdexcept>

namespace rfs {
	void StateStack::Push(std::unique_ptr<IGameState> state) {
		stack_.push_back(std::move(state));
	}
	void StateStack::Pop() {
		if (!stack_.empty()) {
			stack_.pop_back();
		}
	}
	IGameState& StateStack::Top() {
		if (stack_.empty()) {
			throw std::runtime_error("StateStack::Top() callled on empty stack");
		}
		return *stack_.back();
	}
}