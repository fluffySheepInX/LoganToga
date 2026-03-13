#pragma once

#include "Remake2Common.h"

enum class Owner
{
	Neutral,
	Player,
	Enemy
};

enum class UnitArchetype
{
	Base,
	Barracks,
	Stable,
	Turret,
	Worker,
	Soldier,
	Archer,
		Sniper,
		Katyusha,
	MachineGun,
	Healer,
	Spinner
};

enum class UnitOrderType
{
	Idle,
	Move,
	AttackTarget,
	RepairTarget
};

enum class FormationType
{
	Line,
	Column,
	Square
};

enum class TurretUpgradeType
{
	Power,
	Rapid,
	Dual
};

struct UnitOrder
{
	UnitOrderType type = UnitOrderType::Idle;
	Vec2 targetPoint = Vec2::Zero();
	Optional<int32> targetUnitId;
};
