#include "BattleCommandUi.h"

#include "BattleUiText.h"

Array<CommandIconEntry> CollectTurretUpgradeCommands(const BattleState& state, const BattleConfigData& config)
{
	const auto turretId = FindSingleSelectedPlayerTurretId(state);
	if (!turretId)
	{
		return {};
	}

	const auto* building = state.findBuildingByUnitId(*turretId);
	if (!building)
	{
		return {};
	}

	Array<CommandIconEntry> commands;
	for (const auto& definition : config.turretUpgradeDefinitions)
	{
		const bool isUnlocked = ContainsTurretUpgradeType(config.playerAvailableTurretUpgrades, definition.type);
		const bool alreadyUpgraded = building->turretUpgrade.has_value();
		const bool hasEnoughGold = (state.playerGold >= definition.cost);
		const bool isEnabled = (!state.winner) && isUnlocked && !alreadyUpgraded && (definition.cost > 0) && hasEnoughGold;
       String statusText = BattleUiText::GetCommandStatusReady();
		if (state.winner)
		{
           statusText = BattleUiText::GetCommandStatusBattleEnded();
		}
		else if (!isUnlocked)
		{
         statusText = BattleUiText::GetCommandStatusLocked();
		}
		else if (alreadyUpgraded)
		{
           statusText = BattleUiText::GetCommandStatusUpgraded();
		}
		else if (definition.cost <= 0)
		{
            statusText = BattleUiText::GetCommandStatusUnavailable();
		}
		else if (!hasEnoughGold)
		{
            statusText = BattleUiText::GetCommandStatusNotEnoughGold();
		}

		commands << CommandIconEntry{
			CommandKind::Upgrade,
			definition.slot,
			UnitArchetype::Turret,
			UnitArchetype::Turret,
			definition.cost,
			isEnabled,
			statusText,
           BattleUiText::GetLocalizedTurretUpgradeLabel(definition.type),
			definition.glyph,
         BattleUiText::GetLocalizedTurretUpgradeDescription(definition.type),
			BattleUiText::GetLocalizedTurretUpgradeFlavorText(definition.type),
			definition.type
		};
	}

	return commands;
}
