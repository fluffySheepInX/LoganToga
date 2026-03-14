#include "ContinueRunSave.h"

bool IsContinueRunBattleRangeValid(const int32 currentBattleIndex, const int32 totalBattles)
{
	return (totalBattles >= 1) && (currentBattleIndex >= 0) && (currentBattleIndex < totalBattles);
}

bool IsContinueRunPreviewValid(const ContinueRunPreview& preview)
{
	if (!IsContinueRunBattleRangeValid(preview.currentBattleIndex, preview.totalBattles))
	{
		return false;
	}

	if ((preview.selectedCardCount < 0) || (preview.pendingRewardCardCount < 0))
	{
		return false;
	}

	if (preview.isCleared && preview.isFailed)
	{
		return false;
	}

	if ((preview.resumeScene == ContinueResumeScene::Reward) && (preview.pendingRewardCardCount <= 0))
	{
		return false;
	}

	return true;
}

bool IsContinueRunStateValid(const RunState& runState, const BonusRoomProgress& bonusRoomProgress, const ContinueResumeScene resumeScene)
{
	if (!IsContinueRunBattleRangeValid(runState.currentBattleIndex, runState.totalBattles))
	{
		return false;
	}

	if (runState.isCleared && runState.isFailed)
	{
		return false;
	}

	if (bonusRoomProgress.activePageIndex < 0)
	{
		return false;
	}

	if ((resumeScene == ContinueResumeScene::Reward) && runState.pendingRewardCardIds.isEmpty())
	{
		return false;
	}

	if (resumeScene == ContinueResumeScene::BonusRoom)
	{
		return runState.isCleared && !runState.isFailed;
	}

	return runState.isActive && !runState.isCleared && !runState.isFailed;
}
