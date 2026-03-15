#include "ContinueRunSave.h"

Optional<ContinueRunPreview> LoadContinueRunPreview()
{
	const TOMLReader toml{ GetContinueRunSavePath() };
	if (!toml)
	{
		return none;
	}

	try
	{
		if (toml[U"schemaVersion"].get<int32>() != 1)
		{
			return none;
		}

		if (!toml[U"hasContinue"].get<bool>())
		{
			return none;
		}

		ContinueRunPreview preview;
		preview.resumeScene = ParseContinueResumeScene(toml[U"resumeScene"].get<String>());
		preview.currentBattleIndex = toml[U"runCurrentBattleIndex"].get<int32>();
		preview.totalBattles = toml[U"runTotalBattles"].get<int32>();
		preview.selectedCardCount = static_cast<int32>(ReadTomlStringArray(toml, U"runSelectedCardIds").size());
		preview.pendingRewardCardCount = static_cast<int32>(ReadTomlStringArray(toml, U"runPendingRewardCardIds").size());
		preview.isActive = toml[U"runIsActive"].get<bool>();
		preview.isCleared = toml[U"runIsCleared"].get<bool>();
		preview.isFailed = toml[U"runIsFailed"].get<bool>();
		if (!IsContinueRunPreviewValid(preview))
		{
			return none;
		}
		return preview;
	}
	catch (const std::exception&)
	{
		return none;
	}
}

bool SaveContinueRun(const GameData& data, const ContinueResumeScene resumeScene)
{
	FileSystem::CreateDirectories(FileSystem::ParentPath(GetContinueRunSavePath()));

	String content;
	AppendTomlLine(content, U"schemaVersion", U"1");
	AppendTomlLine(content, U"hasContinue", U"true");
	AppendTomlLine(content, U"resumeScene", QuoteTomlString(GetContinueResumeSceneLabel(resumeScene)));
	AppendTomlLine(content, U"runIsActive", data.runState.isActive ? U"true" : U"false");
	AppendTomlLine(content, U"runUseDebugFullUnlocks", data.runState.useDebugFullUnlocks ? U"true" : U"false");
	AppendTomlLine(content, U"runCurrentBattleIndex", Format(data.runState.currentBattleIndex));
	AppendTomlLine(content, U"runTotalBattles", Format(data.runState.totalBattles));
	AppendTomlLine(content, U"runIsFailed", data.runState.isFailed ? U"true" : U"false");
	AppendTomlLine(content, U"runIsCleared", data.runState.isCleared ? U"true" : U"false");
	AppendTomlIntArrayLine(content, U"runMapProgressionBattles", data.runState.mapProgressionBattles);
	AppendTomlStringArrayLine(content, U"runSelectedCardIds", data.runState.selectedCardIds);
	AppendTomlStringArrayLine(content, U"runPendingRewardCardIds", data.runState.pendingRewardCardIds);
	AppendTomlStringArrayLine(content, U"bonusViewedRoomIds", data.bonusRoomProgress.viewedRoomIds);
	AppendTomlStringArrayLine(content, U"bonusPendingRoomIds", data.bonusRoomProgress.pendingRoomIds);
	AppendTomlLine(content, U"bonusActiveRoomId", QuoteTomlString(data.bonusRoomProgress.activeRoomId));
	AppendTomlLine(content, U"bonusActivePageIndex", Format(data.bonusRoomProgress.activePageIndex));
	AppendTomlLine(content, U"bonusSceneMode", QuoteTomlString(data.bonusRoomProgress.sceneMode == BonusRoomSceneMode::Gallery ? U"Gallery" : U"Selection"));

	TextWriter writer{ GetContinueRunSavePath() };
	if (!writer)
	{
		return false;
	}

	writer.write(content);
	return true;
}

void ClearContinueRunSave()
{
	if (HasContinueRunSave())
	{
		FileSystem::Remove(GetContinueRunSavePath());
	}
}

bool LoadContinueRun(GameData& data, ContinueResumeScene& resumeScene)
{
	const TOMLReader toml{ GetContinueRunSavePath() };
	if (!toml)
	{
		return false;
	}

	try
	{
		if (toml[U"schemaVersion"].get<int32>() != 1)
		{
			return false;
		}

		if (!toml[U"hasContinue"].get<bool>())
		{
			return false;
		}

		RunState runState;
		runState.isActive = toml[U"runIsActive"].get<bool>();
		runState.useDebugFullUnlocks = toml[U"runUseDebugFullUnlocks"].get<bool>();
		runState.currentBattleIndex = toml[U"runCurrentBattleIndex"].get<int32>();
		runState.totalBattles = toml[U"runTotalBattles"].get<int32>();
		runState.isFailed = toml[U"runIsFailed"].get<bool>();
		runState.isCleared = toml[U"runIsCleared"].get<bool>();
		runState.mapProgressionBattles = ReadTomlIntArray(toml, U"runMapProgressionBattles");
		if (runState.mapProgressionBattles.isEmpty())
		{
			runState.mapProgressionBattles = BuildSequentialRunMapProgressionBattles(data.baseBattleConfig, runState.totalBattles);
		}
		runState.selectedCardIds = ReadTomlStringArray(toml, U"runSelectedCardIds");
		runState.pendingRewardCardIds = ReadTomlStringArray(toml, U"runPendingRewardCardIds");

		BonusRoomProgress bonusRoomProgress;
		bonusRoomProgress.viewedRoomIds = ReadTomlStringArray(toml, U"bonusViewedRoomIds");
		bonusRoomProgress.pendingRoomIds = ReadTomlStringArray(toml, U"bonusPendingRoomIds");
		bonusRoomProgress.activeRoomId = toml[U"bonusActiveRoomId"].get<String>();
		bonusRoomProgress.activePageIndex = toml[U"bonusActivePageIndex"].get<int32>();
		bonusRoomProgress.sceneMode = (toml[U"bonusSceneMode"].get<String>() == U"Gallery")
			? BonusRoomSceneMode::Gallery
			: BonusRoomSceneMode::Selection;
		resumeScene = ParseContinueResumeScene(toml[U"resumeScene"].get<String>());
		if (!IsContinueRunStateValid(runState, bonusRoomProgress, resumeScene))
		{
			return false;
		}

		data.runState = runState;
		data.bonusRoomProgress = bonusRoomProgress;
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
}
