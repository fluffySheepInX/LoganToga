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
	if (getTabButtonRect(Tab::Core).mouseOver())
	{
       return{ Localization::GetText(U"balance_edit.help.core_tab_title"), Localization::GetText(U"balance_edit.help.core_tab_body") };
	}
	if (getTabButtonRect(Tab::Units).mouseOver())
	{
       return{ Localization::GetText(U"balance_edit.help.units_tab_title"), Localization::GetText(U"balance_edit.help.units_tab_body") };
	}
	if (getTabButtonRect(Tab::Cards).mouseOver())
	{
        return{ Localization::GetText(U"balance_edit.help.cards_tab_title"), Localization::GetText(U"balance_edit.help.cards_tab_body") };
	}

	if (m_tab == Tab::Units)
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
		: ((m_tab == Tab::Units) ? getHoveredUnitRowHelp() : getHoveredCardRowHelp());
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
	for (int32 row = 0; row < 10; ++row)
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
	for (int32 row = 0; row < 3; ++row)
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
		default:
			break;
		}
	}

	return none;
}
