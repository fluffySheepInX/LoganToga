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
	Turret,
	Worker,
	Soldier,
	Archer
};

enum class UnitOrderType
{
	Idle,
	Move,
	AttackTarget
};

enum class FormationType
{
	Line,
	Column,
	Square
};

struct UnitOrder
{
	UnitOrderType type = UnitOrderType::Idle;
	Vec2 targetPoint = Vec2::Zero();
	Optional<int32> targetUnitId;
};
