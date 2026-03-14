#include "BattleCommandUi.h"

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
		String statusText = U"READY";
		if (state.winner)
		{
			statusText = U"BATTLE ENDED";
		}
		else if (!isUnlocked)
		{
			statusText = U"LOCKED";
		}
		else if (alreadyUpgraded)
		{
			statusText = U"UPGRADED";
		}
		else if (definition.cost <= 0)
		{
			statusText = U"UNAVAILABLE";
		}
		else if (!hasEnoughGold)
		{
			statusText = U"NOT ENOUGH GOLD";
		}

		commands << CommandIconEntry{
			CommandKind::Upgrade,
			definition.slot,
			UnitArchetype::Turret,
			UnitArchetype::Turret,
			definition.cost,
			isEnabled,
			statusText,
			definition.label,
			definition.glyph,
			definition.description,
			definition.flavorText,
			definition.type
		};
	}

	return commands;
}
