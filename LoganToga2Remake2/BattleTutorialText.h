#pragma once

#include "BattleConfigTypes.h"
#include "Localization.h"

namespace BattleTutorialText
{
	[[nodiscard]] inline String GetHudTitle(const BattleConfigData& config)
	{
		const String english = config.hud.title.isEmpty() ? U"LoganToga2 Tutorial" : config.hud.title;
		return Localization::GetText(U"tutorial.hud.title", U"LoganToga2 チュートリアル", english);
	}

	[[nodiscard]] inline String GetModeLabel()
	{
		return Localization::GetText(U"tutorial.mode_label", U"モード: チュートリアル", U"Mode: tutorial");
	}

	[[nodiscard]] inline String GetPanelTitle()
	{
		return Localization::GetText(U"tutorial.panel.title", U"チュートリアル", U"TUTORIAL");
	}

	[[nodiscard]] inline String GetPhaseLabel(const TutorialPhase phase)
	{
		switch (phase)
		{
		case TutorialPhase::MoveUnit:
			return Localization::GetText(U"tutorial.phase.move", U"手順 1", U"STEP 1");
		case TutorialPhase::BuildStructure:
			return Localization::GetText(U"tutorial.phase.build", U"手順 2", U"STEP 2");
		case TutorialPhase::PrepareDefense:
			return Localization::GetText(U"tutorial.phase.prepare", U"警告", U"WARNING");
		case TutorialPhase::ProduceUnit:
			return Localization::GetText(U"tutorial.phase.produce", U"手順 3", U"STEP 3");
		case TutorialPhase::DefendWave:
			return Localization::GetText(U"tutorial.phase.defend", U"手順 4", U"STEP 4");
		case TutorialPhase::Completed:
			return Localization::GetText(U"tutorial.phase.completed", U"完了", U"DONE");
		case TutorialPhase::None:
		default:
			return Localization::GetText(U"tutorial.phase.none", U"チュートリアル", U"TUTORIAL");
		}
	}

	[[nodiscard]] inline String GetObjective(const BattleConfigData& config, const TutorialPhase phase)
	{
		switch (phase)
		{
		case TutorialPhase::MoveUnit:
			return Localization::GetText(U"tutorial.objective.move", U"手順 1: Worker を選択し、前方のエリアまで移動させましょう。", config.tutorial.objectiveMove);
		case TutorialPhase::BuildStructure:
			return Localization::GetText(U"tutorial.objective.build", U"手順 2: 拠点の近くに Barracks を建てましょう。", config.tutorial.objectiveBuild);
		case TutorialPhase::PrepareDefense:
			return Localization::GetText(U"tutorial.objective.prepare", U"敵の斥候を確認しました。防衛線を整えましょう。", config.tutorial.objectivePrepare);
		case TutorialPhase::ProduceUnit:
			return Localization::GetText(U"tutorial.objective.produce", U"手順 3: Barracks で Soldier を生産し、敵が来るまで持ちこたえましょう。", config.tutorial.objectiveProduce);
		case TutorialPhase::DefendWave:
			return Localization::GetText(U"tutorial.objective.defend", U"手順 4: 接近してくる敵の波を撃退しましょう。", config.tutorial.objectiveDefend);
		case TutorialPhase::Completed:
			return Localization::GetText(U"tutorial.objective.completed", U"チュートリアル完了。Enter でタイトルへ戻れます。", config.tutorial.objectiveComplete);
		case TutorialPhase::None:
		default:
			return U"";
		}
	}

	[[nodiscard]] inline String GetEnemyArrivalLabel(const int32 seconds)
	{
		return Localization::FormatText(U"tutorial.enemy_arrival", U"敵到着まで: {0} 秒", U"Enemy arrival: {0}s", seconds);
	}

	[[nodiscard]] inline String GetStatusBuild()
	{
		return Localization::GetText(U"tutorial.status.build", U"次は Barracks を建てましょう", U"Now build a Barracks");
	}

	[[nodiscard]] inline String GetStatusEnemySpotted()
	{
		return Localization::GetText(U"tutorial.status.enemy_spotted", U"敵影を確認。防衛準備をしてください", U"Enemy forces spotted");
	}

	[[nodiscard]] inline String GetStatusProduce()
	{
		return Localization::GetText(U"tutorial.status.produce", U"敵が来る前に Soldier を生産しましょう", U"Produce a Soldier before the enemy arrives");
	}

	[[nodiscard]] inline String GetStatusProductionComplete()
	{
		return Localization::GetText(U"tutorial.status.production_complete", U"生産完了。迎撃に備えましょう", U"Production complete. Prepare to intercept.");
	}

	[[nodiscard]] inline String GetStatusEnemyIncoming()
	{
		return Localization::GetText(U"tutorial.status.enemy_incoming", U"敵部隊が接近中です", U"Enemy wave incoming");
	}

	[[nodiscard]] inline String GetResultTitle(const bool playerWon)
	{
		return playerWon
			? Localization::GetText(U"tutorial.result.complete_title", U"チュートリアル完了", U"Tutorial Complete")
			: Localization::GetText(U"tutorial.result.failed_title", U"チュートリアル失敗", U"Tutorial Failed");
	}

	[[nodiscard]] inline String GetResultSubtitle(const bool playerWon)
	{
		return playerWon
			? Localization::GetText(U"tutorial.result.complete_subtitle", U"移動 / 建築 / 生産 / 防衛まで完了しました", U"Move / Build / Produce / Defend complete")
			: Localization::GetText(U"tutorial.result.failed_subtitle", U"もう一度チュートリアル戦を遊んで基本操作を練習しましょう", U"Retry the tutorial battle to practice the basics");
	}

	[[nodiscard]] inline String GetResultEnterAction()
	{
		return Localization::GetText(U"tutorial.result.enter_action", U"Enter: タイトルへ戻る", U"Enter: Return to Title");
	}

	[[nodiscard]] inline String GetResultRetryAction()
	{
		return Localization::GetText(U"tutorial.result.retry_action", U"R: チュートリアル再挑戦", U"R: Retry Tutorial");
	}

	[[nodiscard]] inline String GetResultFooter()
	{
		return Localization::GetText(U"tutorial.result.footer", U"チュートリアルはいつでもタイトルメニューから再開できます", U"You can start the tutorial again from the title menu at any time");
	}
}
