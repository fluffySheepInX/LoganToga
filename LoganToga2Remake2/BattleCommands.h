#pragma once

#include "BattleState.h"

struct ClearSelectionCommand
{
};

struct SelectUnitsInRectCommand
{
	RectF rect;
	bool additive = false;
};

struct MoveUnitsCommand
{
	Array<int32> unitIds;
	Vec2 destination = Vec2::Zero();
	FormationType formation = FormationType::Line;
	Vec2 facingDirection = Vec2::Zero();
};

struct SetPlayerFormationCommand
{
	FormationType formation = FormationType::Line;
};

struct IssueConstructionOrderCommand
{
	int32 workerUnitId = -1;
	UnitArchetype archetype = UnitArchetype::Barracks;
	Vec2 position = Vec2::Zero();
};

struct AttackUnitCommand
{
	Array<int32> unitIds;
	int32 targetUnitId = -1;
};

struct IssueTurretUpgradeCommand
{
	int32 turretUnitId = -1;
	TurretUpgradeType type = TurretUpgradeType::Power;
};

struct IssueRepairOrderCommand
{
	Array<int32> unitIds;
	int32 targetUnitId = -1;
};

struct IssueGoliathDetonationCommand
{
	Array<int32> unitIds;
};

using BattleCommand = std::variant<ClearSelectionCommand, SelectUnitsInRectCommand, MoveUnitsCommand, AttackUnitCommand, SetPlayerFormationCommand, IssueConstructionOrderCommand, IssueTurretUpgradeCommand, IssueRepairOrderCommand, IssueGoliathDetonationCommand>;
