#pragma once

#include "BattleConfigTypes.h"
#include "Localization.h"

namespace BattleTutorialText
{
 [[nodiscard]] inline String GetHudTitle(const BattleConfigData&)
	{
      return Localization::GetText(U"tutorial.hud.title");
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

 [[nodiscard]] inline String GetObjective(const BattleConfigData&, const TutorialPhase phase)
	{
		switch (phase)
		{
		case TutorialPhase::MoveUnit:
            return Localization::GetText(U"tutorial.objective.move");
		case TutorialPhase::BuildStructure:
           return Localization::GetText(U"tutorial.objective.build");
		case TutorialPhase::PrepareDefense:
          return Localization::GetText(U"tutorial.objective.prepare");
		case TutorialPhase::ProduceUnit:
           return Localization::GetText(U"tutorial.objective.produce");
		case TutorialPhase::DefendWave:
           return Localization::GetText(U"tutorial.objective.defend");
		case TutorialPhase::Completed:
           return Localization::GetText(U"tutorial.objective.completed");
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
