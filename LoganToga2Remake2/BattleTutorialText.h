#pragma once

#include "BattleConfigTypes.h"
#include "Localization.h"

namespace BattleTutorialText
{
	[[nodiscard]] inline String GetHudTitle(const BattleConfigData& config)
	{
		const String english = config.hud.title.isEmpty() ? U"LoganToga2 Tutorial" : config.hud.title;
        return Localization::Legacy::GetText(U"tutorial.hud.title", U"LoganToga2 チュートリアル", english);
	}

	[[nodiscard]] inline String GetModeLabel()
	{
       return Localization::GetText(U"tutorial.mode_label");
	}

	[[nodiscard]] inline String GetPanelTitle()
	{
     return Localization::GetText(U"tutorial.panel.title");
	}

	[[nodiscard]] inline String GetPhaseLabel(const TutorialPhase phase)
	{
		switch (phase)
		{
		case TutorialPhase::MoveUnit:
           return Localization::GetText(U"tutorial.phase.move");
		case TutorialPhase::BuildStructure:
          return Localization::GetText(U"tutorial.phase.build");
		case TutorialPhase::PrepareDefense:
         return Localization::GetText(U"tutorial.phase.prepare");
		case TutorialPhase::ProduceUnit:
            return Localization::GetText(U"tutorial.phase.produce");
		case TutorialPhase::DefendWave:
         return Localization::GetText(U"tutorial.phase.defend");
		case TutorialPhase::Completed:
          return Localization::GetText(U"tutorial.phase.completed");
		case TutorialPhase::None:
		default:
          return Localization::GetText(U"tutorial.phase.none");
		}
	}

	[[nodiscard]] inline String GetObjective(const BattleConfigData& config, const TutorialPhase phase)
	{
		switch (phase)
		{
		case TutorialPhase::MoveUnit:
            return Localization::Legacy::GetText(U"tutorial.objective.move", U"手順 1: Worker を選択し、前方のエリアまで移動させましょう。", config.tutorial.objectiveMove);
		case TutorialPhase::BuildStructure:
            return Localization::Legacy::GetText(U"tutorial.objective.build", U"手順 2: 拠点の近くに Barracks を建てましょう。", config.tutorial.objectiveBuild);
		case TutorialPhase::PrepareDefense:
            return Localization::Legacy::GetText(U"tutorial.objective.prepare", U"敵の斥候を確認しました。防衛線を整えましょう。", config.tutorial.objectivePrepare);
		case TutorialPhase::ProduceUnit:
            return Localization::Legacy::GetText(U"tutorial.objective.produce", U"手順 3: Barracks で Soldier を生産し、敵が来るまで持ちこたえましょう。", config.tutorial.objectiveProduce);
		case TutorialPhase::DefendWave:
            return Localization::Legacy::GetText(U"tutorial.objective.defend", U"手順 4: 接近してくる敵の波を撃退しましょう。", config.tutorial.objectiveDefend);
		case TutorialPhase::Completed:
            return Localization::Legacy::GetText(U"tutorial.objective.completed", U"チュートリアル完了。Enter でタイトルへ戻れます。", config.tutorial.objectiveComplete);
		case TutorialPhase::None:
		default:
			return U"";
		}
	}

	[[nodiscard]] inline String GetEnemyArrivalLabel(const int32 seconds)
	{
       return Localization::FormatText(U"tutorial.enemy_arrival", seconds);
	}

	[[nodiscard]] inline String GetStatusBuild()
	{
        return Localization::GetText(U"tutorial.status.build");
	}

	[[nodiscard]] inline String GetStatusEnemySpotted()
	{
      return Localization::GetText(U"tutorial.status.enemy_spotted");
	}

	[[nodiscard]] inline String GetStatusProduce()
	{
        return Localization::GetText(U"tutorial.status.produce");
	}

	[[nodiscard]] inline String GetStatusProductionComplete()
	{
     return Localization::GetText(U"tutorial.status.production_complete");
	}

	[[nodiscard]] inline String GetStatusEnemyIncoming()
	{
      return Localization::GetText(U"tutorial.status.enemy_incoming");
	}

	[[nodiscard]] inline String GetResultTitle(const bool playerWon)
	{
		return playerWon
          ? Localization::GetText(U"tutorial.result.complete_title")
			: Localization::GetText(U"tutorial.result.failed_title");
	}

	[[nodiscard]] inline String GetResultSubtitle(const bool playerWon)
	{
		return playerWon
            ? Localization::GetText(U"tutorial.result.complete_subtitle")
			: Localization::GetText(U"tutorial.result.failed_subtitle");
	}

	[[nodiscard]] inline String GetResultEnterAction()
	{
        return Localization::GetText(U"tutorial.result.enter_action");
	}

	[[nodiscard]] inline String GetResultRetryAction()
	{
      return Localization::GetText(U"tutorial.result.retry_action");
	}

	[[nodiscard]] inline String GetResultFooter()
	{
      return Localization::GetText(U"tutorial.result.footer");
	}
}
