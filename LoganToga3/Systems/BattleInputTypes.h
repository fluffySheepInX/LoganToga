# pragma once
# include <Siv3D.hpp>
# include "BattleSystems.h"

namespace LT3
{
	enum class BattleInputCommandType
	{
		SelectUnit,
		MoveUnit,
		StartBuildAction,
		StartBuildLineAction,
	};

	struct BattleInputCommand
	{
		BattleInputCommandType type = BattleInputCommandType::SelectUnit;
		UnitId unit = InvalidUnitId;
		Array<UnitId> units;
		Vec2 position{ 0, 0 };
		Vec2 direction{ 0, 0 };
		BuildActionDefId buildAction = InvalidBuildActionDefId;
		Array<Vec2> positions;
		bool useFormation = false;
		bool hasTargetPosition = false;
	};

	struct BattleInputIntent
	{
		Array<BattleInputCommand> commands;
	};
}
