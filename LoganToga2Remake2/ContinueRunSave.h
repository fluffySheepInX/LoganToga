#pragma once

#include "GameData.h"

enum class ContinueResumeScene
{
	Battle,
	Reward,
	BonusRoom,
};

struct ContinueRunPreview
{
	ContinueResumeScene resumeScene = ContinueResumeScene::Battle;
	int32 currentBattleIndex = 0;
	int32 totalBattles = 3;
	int32 selectedCardCount = 0;
	int32 pendingRewardCardCount = 0;
	bool isActive = false;
	bool isCleared = false;
	bool isFailed = false;
};

[[nodiscard]] inline String GetContinueRunSavePath()
{
	return U"save/continue_run.toml";
}

[[nodiscard]] inline bool HasContinueRunSave()
{
	return FileSystem::Exists(GetContinueRunSavePath());
}

[[nodiscard]] inline String GetContinueResumeSceneLabel(const ContinueResumeScene scene)
{
	switch (scene)
	{
	case ContinueResumeScene::Reward:
		return U"Reward";
	case ContinueResumeScene::BonusRoom:
		return U"BonusRoom";
	case ContinueResumeScene::Battle:
	default:
		return U"Battle";
	}
}

[[nodiscard]] inline String GetContinueResumeSceneName(const ContinueResumeScene scene)
{
	return GetContinueResumeSceneLabel(scene);
}

[[nodiscard]] inline ContinueResumeScene ParseContinueResumeScene(const String& label)
{
	if (label == U"Reward")
	{
		return ContinueResumeScene::Reward;
	}

	if (label == U"BonusRoom")
	{
		return ContinueResumeScene::BonusRoom;
	}

	return ContinueResumeScene::Battle;
}

[[nodiscard]] inline String EscapeTomlString(String value)
{
	value.replace(U"\\", U"\\\\");
	value.replace(U"\"", U"\\\"");
	value.replace(U"\n", U"\\n");
	return value;
}

[[nodiscard]] inline String QuoteTomlString(const String& value)
{
	return U"\"" + EscapeTomlString(value) + U"\"";
}

[[nodiscard]] inline String BuildTomlStringArray(const Array<String>& values)
{
	String result = U"[";
	for (size_t index = 0; index < values.size(); ++index)
	{
		if (index > 0)
		{
			result += U", ";
		}

		result += QuoteTomlString(values[index]);
	}

	result += U"]";
	return result;
}

[[nodiscard]] inline String BuildTomlIntArray(const Array<int32>& values)
{
	String result = U"[";
	for (size_t index = 0; index < values.size(); ++index)
	{
		if (index > 0)
		{
			result += U", ";
		}

		result += Format(values[index]);
	}

	result += U"]";
	return result;
}

inline void AppendTomlLine(String& content, const String& key, const String& value)
{
	content += key + U" = " + value + U"\n";
}

inline void AppendTomlStringArrayLine(String& content, const String& key, const Array<String>& values)
{
	AppendTomlLine(content, key, BuildTomlStringArray(values));
}

inline void AppendTomlIntArrayLine(String& content, const String& key, const Array<int32>& values)
{
	AppendTomlLine(content, key, BuildTomlIntArray(values));
}

[[nodiscard]] inline Array<String> ReadTomlStringArray(const TOMLReader& toml, const String& key)
{
	Array<String> values;
	for (const auto& value : toml[key].arrayView())
	{
		values << value.get<String>();
	}

	return values;
}

[[nodiscard]] inline Array<int32> ReadTomlIntArray(const TOMLReader& toml, const String& key)
{
	Array<int32> values;
	for (const auto& value : toml[key].arrayView())
	{
		values << value.get<int32>();
	}

	return values;
}

[[nodiscard]] inline bool IsContinueRunBattleRangeValid(const int32 currentBattleIndex, const int32 totalBattles)
{
	return (totalBattles >= 1) && (currentBattleIndex >= 0) && (currentBattleIndex < totalBattles);
}

[[nodiscard]] inline bool IsContinueRunPreviewValid(const ContinueRunPreview& preview)
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

[[nodiscard]] inline bool IsContinueRunStateValid(const RunState& runState, const BonusRoomProgress& bonusRoomProgress, const ContinueResumeScene resumeScene)
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

[[nodiscard]] inline Optional<ContinueRunPreview> LoadContinueRunPreview()
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

inline bool SaveContinueRun(const GameData& data, const ContinueResumeScene resumeScene)
{
	FileSystem::CreateDirectories(U"save");

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

inline void ClearContinueRunSave()
{
	if (HasContinueRunSave())
	{
		FileSystem::Remove(GetContinueRunSavePath());
	}
}

inline bool LoadContinueRun(GameData& data, ContinueResumeScene& resumeScene)
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
