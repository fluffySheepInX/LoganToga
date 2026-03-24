#include "BalanceEditScene.h"

#include "Localization.h"
#include "RunCardPresentation.h"

BalanceEditScene::HelpText BalanceEditScene::getHoveredHelpText() const
{
	if (getTopButtonRect(0).mouseOver())
	{
     return{ Localization::GetText(U"balance_edit.button.back"), Localization::GetText(U"balance_edit.help.back") };
	}
	if (getTopButtonRect(1).mouseOver())
	{
        return{ Localization::GetText(U"balance_edit.button.apply_overrides"), Localization::GetText(U"balance_edit.help.apply_overrides") };
	}
	if (getTopButtonRect(2).mouseOver())
	{
        return{ Localization::GetText(U"balance_edit.button.save"), Localization::GetText(U"balance_edit.help.save") };
	}
	if (getTopButtonRect(3).mouseOver())
	{
        return{ Localization::GetText(U"balance_edit.button.clear_editor"), Localization::GetText(U"balance_edit.help.clear_editor") };
	}
	if (getTopButtonRect(4).mouseOver())
	{
      return{ Localization::GetText(U"balance_edit.button.clear_all"), Localization::GetText(U"balance_edit.help.clear_all") };
	}
    if ((m_tab == Tab::Core) && getCoreTestStartButtonRect().mouseOver())
	{
		return{ Localization::GetText(U"balance_edit.button.test_start"), Localization::GetText(U"balance_edit.help.test_start") };
	}
	if (getTabButtonRect(Tab::Core).mouseOver())
	{
       return{ Localization::GetText(U"balance_edit.help.core_tab_title"), Localization::GetText(U"balance_edit.help.core_tab_body") };
	}
   if (getTabButtonRect(Tab::Progression).mouseOver())
	{
		return{ Localization::GetText(U"balance_edit.help.progression_tab_title"), Localization::GetText(U"balance_edit.help.progression_tab_body") };
	}
	if (getTabButtonRect(Tab::Units).mouseOver())
	{
       return{ Localization::GetText(U"balance_edit.help.units_tab_title"), Localization::GetText(U"balance_edit.help.units_tab_body") };
	}
	if (getTabButtonRect(Tab::Cards).mouseOver())
	{
        return{ Localization::GetText(U"balance_edit.help.cards_tab_title"), Localization::GetText(U"balance_edit.help.cards_tab_body") };
	}

    if (m_tab == Tab::Progression)
	{
		for (size_t index = 0; index < m_editConfig.enemyProgression.size(); ++index)
		{
			const auto& progression = m_editConfig.enemyProgression[index];
			if (getProgressionButtonRect(static_cast<int32>(index)).mouseOver())
			{
				return{
					Localization::FormatText(U"balance_edit.progression.title", progression.battle),
					Localization::FormatText(U"balance_edit.help.progression_entry", FileSystem::FileName(progression.mapSourcePath), progression.enemyInitialUnits.size())
				};
			}
		}
	}
	else if (m_tab == Tab::Units)
	{
		for (size_t index = 0; index < m_editConfig.unitDefinitions.size(); ++index)
		{
			const auto& unit = m_editConfig.unitDefinitions[index];
			if (getUnitButtonRect(static_cast<int32>(index)).mouseOver())
			{
				return{
					toUnitArchetypeDisplayString(unit.archetype),
                    unit.description.isEmpty() ? Localization::GetText(U"balance_edit.help.unit_fallback") : unit.description
				};
			}
		}
	}
	else if (m_tab == Tab::Cards)
	{
		for (size_t index = 0; index < m_editCards.size(); ++index)
		{
			const auto& card = m_editCards[index];
			if (getCardButtonRect(static_cast<int32>(index)).mouseOver())
			{
				return{
                  GetRewardCardName(card),
					GetRewardCardDescription(card)
				};
			}
		}
	}

	const auto rowHelp = (m_tab == Tab::Core)
		? getHoveredCoreRowHelp()
      : ((m_tab == Tab::Progression)
			? getHoveredProgressionRowHelp()
			: ((m_tab == Tab::Units) ? getHoveredUnitRowHelp() : getHoveredCardRowHelp()));
	if (rowHelp)
	{
		return *rowHelp;
	}

	return{
      Localization::GetText(U"balance_edit.title"),
		Localization::GetText(U"balance_edit.help.default")
	};
}

Optional<BalanceEditScene::HelpText> BalanceEditScene::getHoveredCoreRowHelp() const
{
    for (int32 row = 0; row < 20; ++row)
	{
		if (!getEditorRowRect(row).mouseOver())
		{
			continue;
		}

		switch (row)
		{
		case 0:
            return HelpText{ Localization::GetText(U"balance_edit.row.player_gold"), Localization::GetText(U"balance_edit.help.player_gold") };
		case 1:
          return HelpText{ Localization::GetText(U"balance_edit.row.enemy_gold"), Localization::GetText(U"balance_edit.help.enemy_gold") };
		case 2:
            return HelpText{ Localization::GetText(U"balance_edit.row.income_interval"), Localization::GetText(U"balance_edit.help.income_interval") };
		case 3:
         return HelpText{ Localization::GetText(U"balance_edit.row.player_income"), Localization::GetText(U"balance_edit.help.player_income") };
		case 4:
           return HelpText{ Localization::GetText(U"balance_edit.row.enemy_income"), Localization::GetText(U"balance_edit.help.enemy_income") };
		case 5:
          return HelpText{ Localization::GetText(U"balance_edit.row.spawn_interval"), Localization::GetText(U"balance_edit.help.spawn_interval") };
		case 6:
            return HelpText{ Localization::GetText(U"balance_edit.row.advanced_spawn_rate"), Localization::GetText(U"balance_edit.help.advanced_spawn_rate") };
		case 7:
          return HelpText{ Localization::GetText(U"balance_edit.row.spawn_random_y"), Localization::GetText(U"balance_edit.help.spawn_random_y") };
		case 8:
          return HelpText{ Localization::GetText(U"balance_edit.row.assault_threshold"), Localization::GetText(U"balance_edit.help.assault_threshold") };
		case 9:
         return HelpText{ Localization::GetText(U"balance_edit.row.staging_min_units"), Localization::GetText(U"balance_edit.help.staging_min_units") };
        case 10:
			return HelpText{ Localization::GetText(U"balance_edit.row.decision_interval"), Localization::GetText(U"balance_edit.help.decision_interval") };
		case 11:
			return HelpText{ Localization::GetText(U"balance_edit.row.defense_radius"), Localization::GetText(U"balance_edit.help.defense_radius") };
		case 12:
			return HelpText{ Localization::GetText(U"balance_edit.row.rally_distance"), Localization::GetText(U"balance_edit.help.rally_distance") };
		case 13:
			return HelpText{ Localization::GetText(U"balance_edit.row.base_assault_lock_radius"), Localization::GetText(U"balance_edit.help.base_assault_lock_radius") };
		case 14:
			return HelpText{ Localization::GetText(U"balance_edit.row.staging_gather_radius"), Localization::GetText(U"balance_edit.help.staging_gather_radius") };
		case 15:
			return HelpText{ Localization::GetText(U"balance_edit.row.staging_max_wait"), Localization::GetText(U"balance_edit.help.staging_max_wait") };
		case 16:
			return HelpText{ Localization::GetText(U"balance_edit.row.staging_commit_time"), Localization::GetText(U"balance_edit.help.staging_commit_time") };
		case 17:
			return HelpText{ Localization::GetText(U"balance_edit.row.attack_target_pathfinding"), Localization::GetText(U"balance_edit.help.attack_target_pathfinding") };
        case 18:
			return HelpText{ Localization::GetText(U"balance_edit.row.test_battle_number"), Localization::GetText(U"balance_edit.help.test_battle_number") };
        case 19:
			return HelpText{ Localization::GetText(U"balance_edit.row.test_owned_cards"), Localization::GetText(U"balance_edit.help.test_owned_cards") };
		default:
			break;
		}
	}

	return none;
}

Optional<BalanceEditScene::HelpText> BalanceEditScene::getHoveredProgressionRowHelp() const
{
	for (int32 row = 0; row < 13; ++row)
	{
		if (!getEditorRowRect(row).mouseOver())
		{
			continue;
		}

		switch (row)
		{
		case 0:
			return HelpText{ Localization::GetText(U"balance_edit.row.gold_bonus"), Localization::GetText(U"balance_edit.help.gold_bonus") };
		case 1:
			return HelpText{ Localization::GetText(U"balance_edit.row.income_bonus"), Localization::GetText(U"balance_edit.help.income_bonus") };
		case 2:
			return HelpText{ Localization::GetText(U"balance_edit.row.spawn_interval"), Localization::GetText(U"balance_edit.help.progression_spawn_interval") };
		case 3:
			return HelpText{ Localization::GetText(U"balance_edit.row.assault_threshold"), Localization::GetText(U"balance_edit.help.progression_assault_threshold") };
		case 4:
			return HelpText{ Localization::GetText(U"balance_edit.row.ai_mode_override"), Localization::GetText(U"balance_edit.help.ai_mode_override") };
		case 5:
			return HelpText{ Localization::GetText(U"balance_edit.row.ai_mode"), Localization::GetText(U"balance_edit.help.ai_mode") };
		case 6:
			return HelpText{ Localization::GetText(U"balance_edit.row.staging_min_units"), Localization::GetText(U"balance_edit.help.staging_min_units") };
		case 7:
			return HelpText{ Localization::GetText(U"balance_edit.row.staging_gather_radius"), Localization::GetText(U"balance_edit.help.staging_gather_radius") };
		case 8:
			return HelpText{ Localization::GetText(U"balance_edit.row.staging_max_wait"), Localization::GetText(U"balance_edit.help.staging_max_wait") };
		case 9:
			return HelpText{ Localization::GetText(U"balance_edit.row.staging_commit_time"), Localization::GetText(U"balance_edit.help.staging_commit_time") };
		case 10:
			return HelpText{ Localization::GetText(U"balance_edit.row.extra_basic_units"), Localization::GetText(U"balance_edit.help.extra_basic_units") };
		case 11:
			return HelpText{ Localization::GetText(U"balance_edit.row.extra_advanced_units"), Localization::GetText(U"balance_edit.help.extra_advanced_units") };
		case 12:
			return HelpText{ Localization::GetText(U"balance_edit.row.replace_enemy_initial_units"), Localization::GetText(U"balance_edit.help.replace_enemy_initial_units") };
		default:
			break;
		}
	}

	return none;
}

Optional<BalanceEditScene::HelpText> BalanceEditScene::getHoveredUnitRowHelp() const
{
	const bool hasProductionSlot = (getSelectedProductionSlot() != nullptr);
	const int32 rowCount = hasProductionSlot ? 10 : 8;
	for (int32 row = 0; row < rowCount; ++row)
	{
		if (!getEditorRowRect(row).mouseOver())
		{
			continue;
		}

		if (row == 0)
		{
            return HelpText{ U"HP", Localization::GetText(U"balance_edit.help.hp") };
		}
		if (row == 1)
		{
         return HelpText{ Localization::GetText(U"balance_edit.row.attack"), Localization::GetText(U"balance_edit.help.attack") };
		}
		if (row == 2)
		{
           return HelpText{ Localization::GetText(U"balance_edit.row.unit_cost"), Localization::GetText(U"balance_edit.help.unit_cost") };
		}

		if (hasProductionSlot)
		{
			switch (row)
			{
			case 3:
             return HelpText{ Localization::GetText(U"balance_edit.row.queue_cost"), Localization::GetText(U"balance_edit.help.queue_cost") };
			case 4:
               return HelpText{ Localization::GetText(U"balance_edit.row.batch_count"), Localization::GetText(U"balance_edit.help.batch_count") };
			case 5:
                return HelpText{ Localization::GetText(U"balance_edit.row.production_time"), Localization::GetText(U"balance_edit.help.production_time") };
			case 6:
              return HelpText{ Localization::GetText(U"balance_edit.row.move_speed"), Localization::GetText(U"balance_edit.help.move_speed") };
			case 7:
              return HelpText{ Localization::GetText(U"balance_edit.row.attack_range"), Localization::GetText(U"balance_edit.help.attack_range") };
			case 8:
                return HelpText{ Localization::GetText(U"balance_edit.row.attack_cooldown"), Localization::GetText(U"balance_edit.help.attack_cooldown") };
			case 9:
               return HelpText{ Localization::GetText(U"balance_edit.row.aggro_range"), Localization::GetText(U"balance_edit.help.aggro_range") };
			default:
				break;
			}
		}

		switch (row)
		{
		case 3:
            return HelpText{ Localization::GetText(U"balance_edit.row.production_time"), Localization::GetText(U"balance_edit.help.production_time") };
		case 4:
          return HelpText{ Localization::GetText(U"balance_edit.row.move_speed"), Localization::GetText(U"balance_edit.help.move_speed") };
		case 5:
          return HelpText{ Localization::GetText(U"balance_edit.row.attack_range"), Localization::GetText(U"balance_edit.help.attack_range") };
		case 6:
            return HelpText{ Localization::GetText(U"balance_edit.row.attack_cooldown"), Localization::GetText(U"balance_edit.help.attack_cooldown") };
		case 7:
           return HelpText{ Localization::GetText(U"balance_edit.row.aggro_range"), Localization::GetText(U"balance_edit.help.aggro_range") };
		default:
			break;
		}
	}

	return none;
}

Optional<BalanceEditScene::HelpText> BalanceEditScene::getHoveredCardRowHelp() const
{
 for (int32 row = 0; row < 4; ++row)
	{
		if (!getEditorRowRect(row).mouseOver())
		{
			continue;
		}

		switch (row)
		{
		case 0:
          return HelpText{ Localization::GetText(U"balance_edit.row.value"), Localization::GetText(U"balance_edit.help.value") };
		case 1:
          return HelpText{ Localization::GetText(U"balance_edit.row.rarity"), Localization::GetText(U"balance_edit.help.rarity") };
		case 2:
            return HelpText{ Localization::GetText(U"balance_edit.row.repeatable"), Localization::GetText(U"balance_edit.help.repeatable") };
        case 3:
			return HelpText{ Localization::GetText(U"balance_edit.row.test_owned"), Localization::GetText(U"balance_edit.help.test_owned") };
		default:
			break;
		}
	}

	return none;
}
