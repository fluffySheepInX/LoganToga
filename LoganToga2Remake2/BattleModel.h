#pragma once

#include "Remake2Common.h"

enum class Owner
{
	Player,
	Enemy
};

enum class UnitArchetype
{
	Base,
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

struct UnitOrder
{
	UnitOrderType type = UnitOrderType::Idle;
	Vec2 targetPoint = Vec2::Zero();
	Optional<int32> targetUnitId;
};

struct UnitState
{
	int32 id = -1;
	Owner owner = Owner::Player;
	UnitArchetype archetype = UnitArchetype::Soldier;
	Vec2 position = Vec2::Zero();
	Vec2 moveTarget = Vec2::Zero();
	double radius = 12.0;
	double moveSpeed = 80.0;
	double attackRange = 24.0;
	double attackCooldown = 0.7;
	double attackCooldownRemaining = 0.0;
	int32 attackPower = 8;
	int32 hp = 40;
	int32 maxHp = 40;
	bool canMove = true;
	bool isSelected = false;
	bool isAlive = true;
	UnitOrder order;
};

struct ProductionQueueItem
{
	UnitArchetype archetype = UnitArchetype::Soldier;
	double remainingTime = 0.0;
	double totalTime = 0.0;
};

struct BuildingState
{
	int32 unitId = -1;
	Array<ProductionQueueItem> productionQueue;
};

struct BattleState
{
	RectF worldBounds{ 0, 0, 1280, 720 };
	Array<UnitState> units;
	Array<BuildingState> buildings;
	bool isSelecting = false;
	Vec2 selectionStart = Vec2::Zero();
	RectF selectionRect{ 0, 0, 0, 0 };
	int32 nextUnitId = 1;
	int32 playerGold = 200;
	int32 enemyGold = 200;
	double playerIncomeTimer = 0.0;
	double enemyIncomeTimer = 0.0;
	double enemySpawnTimer = 0.0;
	Optional<Owner> winner;

	[[nodiscard]] UnitState* findUnit(const int32 id)
	{
		for (auto& unit : units)
		{
			if (unit.id == id)
			{
				return &unit;
			}
		}

		return nullptr;
	}

	[[nodiscard]] const UnitState* findUnit(const int32 id) const
	{
		for (const auto& unit : units)
		{
			if (unit.id == id)
			{
				return &unit;
			}
		}

		return nullptr;
	}

	[[nodiscard]] BuildingState* findBuildingByUnitId(const int32 unitId)
	{
		for (auto& building : buildings)
		{
			if (building.unitId == unitId)
			{
				return &building;
			}
		}

		return nullptr;
	}

	[[nodiscard]] const BuildingState* findBuildingByUnitId(const int32 unitId) const
	{
		for (const auto& building : buildings)
		{
			if (building.unitId == unitId)
			{
				return &building;
			}
		}

		return nullptr;
	}
};

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
};

struct AttackUnitCommand
{
	Array<int32> unitIds;
	int32 targetUnitId = -1;
};

using BattleCommand = std::variant<ClearSelectionCommand, SelectUnitsInRectCommand, MoveUnitsCommand, AttackUnitCommand>;

[[nodiscard]] inline Vec2 ClampToWorld(const RectF& bounds, const Vec2& position, const double radius)
{
	return {
		Clamp(position.x, bounds.leftX() + radius, bounds.rightX() - radius),
		Clamp(position.y, bounds.topY() + radius, bounds.bottomY() - radius)
	};
}

[[nodiscard]] inline bool IsEnemy(const UnitState& lhs, const UnitState& rhs)
{
	return (lhs.owner != rhs.owner);
}

[[nodiscard]] inline ColorF GetOwnerColor(const Owner owner)
{
	return (owner == Owner::Player) ? ColorF{ 0.25, 0.75, 1.0 } : ColorF{ 1.0, 0.38, 0.32 };
}

[[nodiscard]] inline String GetArchetypeLabel(const UnitArchetype archetype)
{
	switch (archetype)
	{
	case UnitArchetype::Base:
		return U"BASE";
	case UnitArchetype::Worker:
		return U"WORKER";
	case UnitArchetype::Soldier:
		return U"SOLDIER";
	case UnitArchetype::Archer:
		return U"ARCHER";
	default:
		return U"UNIT";
	}
}
