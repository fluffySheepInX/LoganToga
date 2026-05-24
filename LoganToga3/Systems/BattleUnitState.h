# pragma once
# include <Siv3D.hpp>
# include "../BattleWorld/BattleWorld.h"

namespace LT3
{
	inline bool UnitSlotExists(const BattleWorld& world, UnitId unit)
	{
		return unit != InvalidUnitId && unit < world.units.size();
	}

	inline void SetUnitTask(BattleWorld& world, UnitId unit, UnitTask task)
	{
		if (!UnitSlotExists(world, unit))
		{
			return;
		}

		world.units.task[unit] = task;
	}

	inline void SetUnitTargetPosition(BattleWorld& world, UnitId unit, const Vec2& position)
	{
		if (!UnitSlotExists(world, unit))
		{
			return;
		}

		world.units.targetPosition[unit] = position;
	}

	inline void ClearUnitAttackTarget(BattleWorld& world, UnitId unit)
	{
		if (!UnitSlotExists(world, unit))
		{
			return;
		}

		world.units.attackTarget[unit] = InvalidUnitId;
	}

	inline void SetUnitAttackTarget(BattleWorld& world, UnitId unit, UnitId target)
	{
		if (!UnitSlotExists(world, unit))
		{
			return;
		}

		world.units.attackTarget[unit] = target;
	}

	inline void ClearUnitResourceTarget(BattleWorld& world, UnitId unit)
	{
		if (!UnitSlotExists(world, unit))
		{
			return;
		}

		world.units.resourceTargetNode[unit] = -1;
	}

	inline void SetUnitIgnoreCombatWhileMoving(BattleWorld& world, UnitId unit, bool ignoreCombat)
	{
		if (!UnitSlotExists(world, unit) || unit >= world.units.ignoreCombatWhileMoving.size())
		{
			return;
		}

		world.units.ignoreCombatWhileMoving[unit] = ignoreCombat;
	}

	inline bool HasUnitFormationFinalTarget(const BattleWorld& world, UnitId unit)
	{
		return UnitSlotExists(world, unit)
			&& unit < world.units.hasFormationFinalTarget.size()
			&& world.units.hasFormationFinalTarget[unit];
	}

	inline Vec2 GetUnitFormationFinalTarget(const BattleWorld& world, UnitId unit)
	{
		return HasUnitFormationFinalTarget(world, unit) ? world.units.formationFinalTarget[unit] : world.units.targetPosition[unit];
	}

	inline void SetUnitFormationFinalTarget(BattleWorld& world, UnitId unit, const Vec2& target)
	{
		if (!UnitSlotExists(world, unit) || unit >= world.units.formationFinalTarget.size() || unit >= world.units.hasFormationFinalTarget.size())
		{
			return;
		}

		world.units.formationFinalTarget[unit] = target;
		world.units.hasFormationFinalTarget[unit] = true;
	}

	inline void ClearUnitFormationFinalTarget(BattleWorld& world, UnitId unit)
	{
		if (!UnitSlotExists(world, unit) || unit >= world.units.hasFormationFinalTarget.size())
		{
			return;
		}

		world.units.hasFormationFinalTarget[unit] = false;
	}

	inline void SetUnitResourceTarget(BattleWorld& world, UnitId unit, int32 resourceNode)
	{
		if (!UnitSlotExists(world, unit))
		{
			return;
		}

		world.units.resourceTargetNode[unit] = resourceNode;
	}

	inline void SetUnitAlive(BattleWorld& world, UnitId unit, bool alive)
	{
		if (!UnitSlotExists(world, unit))
		{
			return;
		}

		world.units.alive[unit] = alive;
	}

	inline void ClearUnitPath(BattleWorld& world, UnitId unit)
	{
		if (!UnitSlotExists(world, unit))
		{
			return;
		}

		world.pathing.clearUnitPath(unit);
	}

	inline void SetBuildQueueLocked(BattleWorld& world, UnitId unit, bool locked)
	{
		if (!UnitSlotExists(world, unit) || unit >= world.buildQueues.locked.size())
		{
			return;
		}

		world.buildQueues.locked[unit] = locked;
	}

	inline void ResetBuildQueueProgress(BattleWorld& world, UnitId unit)
	{
		if (!UnitSlotExists(world, unit) || unit >= world.buildQueues.progressSec.size())
		{
			return;
		}

		world.buildQueues.progressSec[unit] = 0.0;
	}

	inline void SetUnitIdle(BattleWorld& world, UnitId unit)
	{
		ClearUnitFormationFinalTarget(world, unit);
		SetUnitTask(world, unit, UnitTask::Idle);
	}

	inline void SetUnitMoving(BattleWorld& world, UnitId unit, const Vec2& destination, bool clearResourceTarget, bool ignoreCombatWhileMoving = false)
	{
		if (!UnitSlotExists(world, unit))
		{
			return;
		}

		SetUnitTargetPosition(world, unit, destination);
		ClearUnitFormationFinalTarget(world, unit);
		ClearUnitAttackTarget(world, unit);
		if (clearResourceTarget)
		{
			ClearUnitResourceTarget(world, unit);
		}
		SetUnitIgnoreCombatWhileMoving(world, unit, ignoreCombatWhileMoving);
		SetUnitTask(world, unit, UnitTask::Moving);
	}

	inline void SetUnitBuilding(BattleWorld& world, UnitId unit)
	{
		SetUnitTask(world, unit, UnitTask::Building);
	}

	inline void SetUnitGathering(BattleWorld& world, UnitId unit)
	{
		ClearUnitFormationFinalTarget(world, unit);
		SetUnitTask(world, unit, UnitTask::Gathering);
	}

	inline void SetUnitAttacking(BattleWorld& world, UnitId unit)
	{
		ClearUnitFormationFinalTarget(world, unit);
		SetUnitIgnoreCombatWhileMoving(world, unit, false);
		SetUnitTask(world, unit, UnitTask::Attacking);
	}

	inline void ResetCompletedBuildQueueState(BattleWorld& world, UnitId unit)
	{
		SetUnitIdle(world, unit);
		SetBuildQueueLocked(world, unit, false);
	}
}
