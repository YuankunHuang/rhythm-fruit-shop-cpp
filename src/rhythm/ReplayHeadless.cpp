#include "ReplayHeadless.h"

namespace rfs {
	GameResult ReplayHeadless(const FrozenChart& chart, const ReplayRecord& record) {
		GameplaySession session(chart, record.config);
		for (const auto& evt : record.events) {
			switch (evt.kind) {
			case ReplayEventKind::Tap:
				session.HandleLaneTap(evt.lane, evt.input_ms);
				break;
			case ReplayEventKind::Update:
				session.Update(evt.input_ms);
				break;
			}
		}
		return session.Summary();
	}
}