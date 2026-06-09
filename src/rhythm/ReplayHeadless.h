#pragma once

#include "GameplaySession.h"
#include "ReplayRecord.h"
#include "GameResult.h"

namespace rfs {
	GameResult ReplayHeadless(const FrozenChart& chart, const ReplayRecord& record);
}