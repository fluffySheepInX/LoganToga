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

[[nodiscard]] String GetContinueRunSavePath();
[[nodiscard]] bool HasContinueRunSave();
[[nodiscard]] String GetContinueResumeSceneLabel(ContinueResumeScene scene);
[[nodiscard]] String GetContinueResumeSceneName(ContinueResumeScene scene);
[[nodiscard]] ContinueResumeScene ParseContinueResumeScene(const String& label);

[[nodiscard]] String EscapeTomlString(String value);
[[nodiscard]] String QuoteTomlString(const String& value);
[[nodiscard]] String BuildTomlStringArray(const Array<String>& values);
[[nodiscard]] String BuildTomlIntArray(const Array<int32>& values);
void AppendTomlLine(String& content, const String& key, const String& value);
void AppendTomlStringArrayLine(String& content, const String& key, const Array<String>& values);
void AppendTomlIntArrayLine(String& content, const String& key, const Array<int32>& values);
[[nodiscard]] Array<String> ReadTomlStringArray(const TOMLReader& toml, const String& key);
[[nodiscard]] Array<int32> ReadTomlIntArray(const TOMLReader& toml, const String& key);

[[nodiscard]] bool IsContinueRunBattleRangeValid(int32 currentBattleIndex, int32 totalBattles);
[[nodiscard]] bool IsContinueRunPreviewValid(const ContinueRunPreview& preview);
[[nodiscard]] bool IsContinueRunStateValid(const RunState& runState, const BonusRoomProgress& bonusRoomProgress, ContinueResumeScene resumeScene);

[[nodiscard]] Optional<ContinueRunPreview> LoadContinueRunPreview();
[[nodiscard]] bool SaveContinueRun(const GameData& data, ContinueResumeScene resumeScene);
void ClearContinueRunSave();
[[nodiscard]] bool LoadContinueRun(GameData& data, ContinueResumeScene& resumeScene);
