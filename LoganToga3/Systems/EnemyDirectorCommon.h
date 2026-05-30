# pragma once
# include <Siv3D.hpp>
# include "BattleOrders.h"
# include "BattleUnitState.h"

namespace LT3
{
	inline const AiProfileDef* GetActiveEnemyAiProfile(const BattleWorld& world, const DefinitionStores& defs)
	{
		const AiProfileDefId profileId = world.aiRuntime.profileId;
		if (profileId != InvalidAiProfileDefId && profileId < defs.aiProfiles.size())
		{
			return &defs.aiProfiles[profileId];
		}

		if (!world.aiRuntime.profileTag.isEmpty() && defs.aiProfileByTag.contains(world.aiRuntime.profileTag))
		{
			return &defs.aiProfiles[defs.aiProfileByTag.at(world.aiRuntime.profileTag)];
		}

		return nullptr;
	}

	inline void AdvanceEnemyAiRuntimeClock(BattleWorld& world, double dt)
	{
		world.aiRuntime.phaseTimerSec += dt;
		world.aiRuntime.spawnTimerSec += dt;
		world.aiRuntime.productionTimerSec += dt;
		world.aiRuntime.attackWaveTimerSec += dt;
		world.aiRuntime.tacticalTimerSec += dt;
		world.aiRuntime.nonWaveRoleReassignmentTimerSec += dt;
		world.enemySpawnTimerSec = world.aiRuntime.spawnTimerSec;
	}

	inline void ResetEnemyAiSpawnTimer(BattleWorld& world)
	{
		world.aiRuntime.spawnTimerSec = 0.0;
		world.enemySpawnTimerSec = 0.0;
	}

	inline double ResolveAiOpeningDelaySec(const AiProfileDef* aiProfile)
	{
		return aiProfile ? Max(0.0, aiProfile->openingDelaySec) : 0.0;
	}

	inline bool ShouldEnemyAiIgnoreCombatWhileMoving(const AiProfileDef* aiProfile)
	{
		return aiProfile && aiProfile->contactBehavior.lowercased() == U"ignore";
	}

	inline void IssueEnemyAiMove(BattleWorld& world, UnitId unit, const Vec2& destination, const AiProfileDef* aiProfile)
	{
		IssueMove(world, unit, destination, ShouldEnemyAiIgnoreCombatWhileMoving(aiProfile));
	}

	inline Vec2 ResolveEnemySpawnOrigin(const BattleWorld& world, const DefinitionStores& defs)
	{
		for (UnitId unit = 0; unit < world.units.size(); ++unit)
		{
			if (!IsValidUnit(world, unit) || world.units.faction[unit] != Faction::Enemy)
			{
				continue;
			}
			if (world.units.defId[unit] >= defs.units.size())
			{
				continue;
			}

			const UnitDef& def = defs.units[world.units.defId[unit]];
			if (def.role == UnitRole::Base && def.unit_id.lowercased() == U"home")
			{
				return world.units.position[unit] + Vec2{ -QuarterTileStep * 1.5, 0.0 };
			}
		}

		for (UnitId unit = 0; unit < world.units.size(); ++unit)
		{
			if (!IsValidUnit(world, unit) || world.units.faction[unit] != Faction::Enemy)
			{
				continue;
			}
			if (world.units.defId[unit] >= defs.units.size())
			{
				continue;
			}

			const UnitDef& def = defs.units[world.units.defId[unit]];
			if (def.role == UnitRole::Base)
			{
				return world.units.position[unit] + Vec2{ -QuarterTileStep * 1.5, 0.0 };
			}
		}

		return Vec2{ 1325, 450.0 };
	}

	inline bool CanEnemyProduceUnit(const DefinitionStores& defs, UnitDefId unitDefId)
	{
		if (!(0 <= unitDefId && unitDefId < defs.units.size()))
		{
			return false;
		}

		const String unitTag = defs.units[unitDefId].unit_id;
		for (const auto& action : defs.buildActions)
		{
			if (!action.enemyCanProduce || action.resultType != BuildActionResultType::Unit)
			{
				continue;
			}

			if (action.spawnUnit == unitDefId)
			{
				return true;
			}

			for (const auto spawnUnit : action.spawnUnits)
			{
				if (spawnUnit == unitDefId)
				{
					return true;
				}
			}

			if ((!action.spawnTag.isEmpty() && action.spawnTag.lowercased() == unitTag.lowercased())
				|| action.spawnTags.any([&](const String& spawnTag) { return !spawnTag.isEmpty() && spawnTag.lowercased() == unitTag.lowercased(); }))
			{
				return true;
			}
		}

		return false;
	}

	inline bool IsEnemySpawnCandidate(const DefinitionStores& defs, UnitDefId unitDefId)
	{
		if (!(0 <= unitDefId && unitDefId < defs.units.size()))
		{
			return false;
		}

		const UnitDef& def = defs.units[unitDefId];
		if (def.role == UnitRole::Base || def.role == UnitRole::Barrier)
		{
			return false;
		}
		if (def.speed <= 0.0)
		{
			return false;
		}

		return CanEnemyProduceUnit(defs, unitDefId);
	}

	inline Array<UnitDefId> CollectEnemySpawnCandidates(const DefinitionStores& defs)
	{
		Array<UnitDefId> candidates;
		for (UnitDefId unitDefId = 0; unitDefId < defs.units.size(); ++unitDefId)
		{
			if (IsEnemySpawnCandidate(defs, unitDefId))
			{
				candidates << unitDefId;
			}
		}

		return candidates;
	}

	inline int32 CountAliveEnemyCombatUnits(const BattleWorld& world, const DefinitionStores& defs)
	{
		int32 count = 0;
		for (UnitId unit = 0; unit < world.units.size(); ++unit)
		{
			if (!IsValidUnit(world, unit) || world.units.faction[unit] != Faction::Enemy)
			{
				continue;
			}
			if (world.units.defId[unit] >= defs.units.size())
			{
				continue;
			}

			if (IsEnemySpawnCandidate(defs, world.units.defId[unit]))
			{
				++count;
			}
		}

		return count;
	}

	inline bool IsEnemyAttackWaveCandidate(const BattleWorld& world, const DefinitionStores& defs, UnitId unit)
	{
		if (!IsValidUnit(world, unit) || world.units.faction[unit] != Faction::Enemy)
		{
			return false;
		}
		if (world.units.defId[unit] >= defs.units.size())
		{
			return false;
		}

		const UnitDef& def = defs.units[world.units.defId[unit]];
		return IsEnemySpawnCandidate(defs, world.units.defId[unit]) && !ResolveUnitSkillIds(def, defs).isEmpty();
	}

	inline Array<UnitId> CollectEnemyAttackWaveCandidates(const BattleWorld& world, const DefinitionStores& defs)
	{
		Array<UnitId> candidates;
		for (UnitId unit = 0; unit < world.units.size(); ++unit)
		{
			if (IsEnemyAttackWaveCandidate(world, defs, unit))
			{
				candidates << unit;
			}
		}

		return candidates;
	}

	inline bool IsEnemyAttackWaveUnit(const BattleWorld& world, UnitId unit)
	{
		return world.aiRuntime.attackWaveUnits.contains(unit);
	}

	inline bool IsEnemyResourceReclaimUnit(const BattleWorld& world, UnitId unit)
	{
		return world.aiRuntime.resourceReclaimUnits.contains(unit);
	}

	inline bool IsEnemyGuardUnit(const BattleWorld& world, UnitId unit)
	{
		return world.aiRuntime.guardUnits.contains(unit);
	}

	inline bool IsEnemySkirmishUnit(const BattleWorld& world, UnitId unit)
	{
		return world.aiRuntime.skirmishUnits.contains(unit);
	}

	inline bool IsEnemyAssignedNonWaveRoleUnit(const BattleWorld& world, UnitId unit)
	{
		return IsEnemyResourceReclaimUnit(world, unit)
			|| IsEnemyGuardUnit(world, unit)
			|| IsEnemySkirmishUnit(world, unit);
	}

	inline void ClearEnemyAiNonWaveRoleAssignments(BattleWorld& world)
	{
		world.aiRuntime.resourceReclaimUnits.clear();
		world.aiRuntime.guardUnits.clear();
		world.aiRuntime.skirmishUnits.clear();
		world.aiRuntime.hasResourceTargetPosition = false;
		world.aiRuntime.hasGuardAnchorPosition = false;
		world.aiRuntime.hasSkirmishAnchorPosition = false;
	}

	inline void RemoveInvalidEnemyAiNonWaveRoleUnits(BattleWorld& world, const DefinitionStores& defs)
	{
		auto isInvalid = [&](UnitId unit)
		{
			return !IsEnemyAttackWaveCandidate(world, defs, unit) || IsEnemyAttackWaveUnit(world, unit);
		};

		world.aiRuntime.resourceReclaimUnits.remove_if(isInvalid);
		world.aiRuntime.guardUnits.remove_if(isInvalid);
		world.aiRuntime.skirmishUnits.remove_if(isInvalid);
	}

	inline double ResolveEnemyAiNonWaveRoleReassignmentIntervalSec(const AiProfileDef* aiProfile)
	{
		const double aggression = aiProfile ? Clamp(aiProfile->aggression, 0.0, 1.0) : 0.55;
		return 7.5 - aggression * 2.5;
	}

	inline bool CanAssignEnemyAiNonWaveRoleUnit(const BattleWorld& world, UnitId unit)
	{
		return !IsEnemyAttackWaveUnit(world, unit) && !IsEnemyAssignedNonWaveRoleUnit(world, unit);
	}

	inline void FillEnemyAiNonWaveRoleUnits(Array<UnitId>& destination, const Array<UnitId>& candidates, const BattleWorld& world, int32 desiredCount)
	{
		if (desiredCount <= 0)
		{
			return;
		}

		for (const UnitId unit : candidates)
		{
			if (static_cast<int32>(destination.size()) >= desiredCount)
			{
				break;
			}
			if (!CanAssignEnemyAiNonWaveRoleUnit(world, unit))
			{
				continue;
			}

			destination << unit;
		}
	}

	inline Vec2 ResolveEnemyAiGuardAnchor(const BattleWorld& world, const DefinitionStores& defs)
	{
		Optional<Vec2> resourceAnchor;
		for (size_t node = 0; node < world.resourceNodes.position.size(); ++node)
		{
			if (node < world.resourceNodes.owner.size()
				&& node < world.resourceNodes.amount.size()
				&& world.resourceNodes.owner[node] == Faction::Enemy
				&& world.resourceNodes.amount[node] > 0)
			{
				resourceAnchor = world.resourceNodes.position[node];
				break;
			}
		}

		if (resourceAnchor)
		{
			return *resourceAnchor;
		}

		return ResolveEnemySpawnOrigin(world, defs);
	}

	inline Vec2 ResolveEnemyAiSkirmishAnchor(const BattleWorld& world, const DefinitionStores& defs)
	{
		const Vec2 home = ResolveEnemySpawnOrigin(world, defs);
		if (world.aiRuntime.hasResourceTargetPosition)
		{
			return home.lerp(world.aiRuntime.resourceTargetPosition, 0.45);
		}

		return home;
	}

	inline double ResolveEnemyAiSkirmishLeashRadius(const AiProfileDef* aiProfile)
	{
		const double aggression = aiProfile ? Clamp(aiProfile->aggression, 0.0, 1.0) : 0.55;
		return 220.0 + aggression * 180.0;
	}
}
