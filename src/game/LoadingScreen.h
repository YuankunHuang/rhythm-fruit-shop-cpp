#pragma once

#include "../app/IScreen.h"
#include "../rhythm/FrozenChart.h"
#include "../rhythm/ChartLoader.h"
#include "GameContext.h"
#include "GameConfig.h"
#include <future>
#include <string>
#include <optional>

namespace rfs {

	class LoadingScreen : public IScreen {
	public:
		LoadingScreen(GameContext ctx, std::string chart_path, std::string difficulty, std::string audio_path, std::string cover_path);

		bool IsReady() const noexcept override { return ready_; }

		void Update(const FrameContext& ctx) override;
		void Render() override;
		void HandleInput(const InputEvent& evt) override;

	private:
		struct LoadResult {
			std::optional<FrozenChart> chart;
			std::string title;
			std::string detail;
			bool ok = false;
		};

		static LoadResult DoLoad(std::string path, std::string difficulty);

		GameContext ctx_;
		GameConfig::UiLayout ui_{};

		std::string audio_path_;
		std::string chart_path_;
		std::string cover_path_;

		bool ready_ = false;
		bool load_ok_ = false;
		std::string title_;
		std::string detail_;
		std::optional<FrozenChart> chart_;

		std::future<LoadResult> future_;
		float spin_ms_ = 0.f;
	};

}
