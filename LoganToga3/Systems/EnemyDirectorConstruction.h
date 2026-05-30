# pragma once
# include <Siv3D.hpp>
# include "BattleBuildOrders.h"
# include "EnemyDirectorSpawn.h"

namespace LT3
{
	struct EnemyAiConstructionDecision
	{
		BuildActionDefId actionId = InvalidBuildActionDefId;
		UnitId builder = InvalidUnitId;
		double score = -DBL_MAX;
	};

	inline const AiBuildPriorityDef* FindAiBuildPriority(const AiProfileDef* aiProfile, StringView actionTag)
	{
		if (!aiProfile || actionTag.isEmpty())
		{
			return nullptr;
		}

		const String lowered = String{ actionTag }.lowercased();
		for (const auto& buildPriority : aiProfile->buildPriorities)
		{
			if (buildPriority.actionTag.lowercased() == lowered)
			{
				return &buildPriority;
			}
		}

		return nullptr;
	}

	inline double ResolveEnemyAiConstructionIntervalSec(const AiProfileDef* aiProfile)
	{
		const double economy = aiProfile ? Clamp(aiProfile->economyFocus, 0.0, 1.0) : 0.5;
		const double tech = aiProfile ? Clamp(aiProfile->techFocus, 0.0, 1.0) : 0.3;
		return 12.0 - economy * 4.0 - tech * 3.0;
	}

	inline UnitDefId ResolveEnemyAiBuildResultUnit(const BuildActionDef& action)
	{
		if (action.resultType != BuildActionResultType::Unit)
		{
			return InvalidUnitDefId;
		}
		if (action.spawnUnit != InvalidUnitDefId)
		{
			return action.spawnUnit;
		}
		for (const UnitDefId unit : action.spawnUnits)
		{
			if (unit != InvalidUnitDefId)
			{
				return unit;
			}
		}
		return InvalidUnitDefId;
	}

	inline bool IsEnemyAiConstructionAction(const DefinitionStores& defs, const BuildActionDef& action)
	{
		if (!action.enemyCanProduce || action.resultType != BuildActionResultType::Unit)
		{
			return false;
		}

		const UnitDefId resultUnit = ResolveEnemyAiBuildResultUnit(action);
		if (!(0 <= resultUnit && resultUnit < defs.units.size()))
		{
			return false;
		}

		const UnitDef& resultDef = defs.units[resultUnit];
		return action.isMove
			&& (resultDef.role == UnitRole::Base
				|| resultDef.role == UnitRole::Barrier
				|| resultDef.blocksTileMovement);
	}

	inline int32 CountAliveEnemyUnitsByDef(const BattleWorld& world, UnitDefId defId)
	{
		int32 count = 0;
		for (UnitId unit = 0; unit < world.units.size(); ++unit)
		{
			if (IsValidUnit(world, unit) && world.units.faction[unit] == Faction::Enemy && world.units.defId[unit] == defId)
			{
				++count;
			}
		}
		return count;
	}

	inline int32 CountPendingEnemyAiBuildAction(const BattleWorld& world, BuildActionDefId actionId)
	{
		int32 count = 0;
		for (UnitId unit = 0; unit < world.units.size(); ++unit)
		{
			if (!IsValidUnit(world, unit) || world.units.faction[unit] != Faction::Enemy)
			{
				continue;
			}
			if (unit < world.buildQueues.hasPendingEntry.size()
				&& world.buildQueues.hasPendingEntry[unit]
				&& world.buildQueues.pendingEntry[unit].actionId == actionId)
			{
				++count;
			}
			if (unit < world.buildQueues.entries.size())
			{
				for (const QueuedBuildAction& queued : world.buildQueues.entries[unit])
				{
					if (queued.actionId == actionId)
					{
						++count;
					}
				}
			}
		}
		return count;
	}

	inline bool HasPendingEnemyAiBuildAction(const BattleWorld& world, BuildActionDefId actionId)
	{
		return CountPendingEnemyAiBuildAction(world, actionId) > 0;
	}

	inline int32 CountEnemyAiConstructionUnits(const BattleWorld& world, const DefinitionStores& defs, BuildActionDefId actionId)
	{
		if (actionId >= defs.buildActions.size())
		{
			return 0;
		}

		const BuildActionDef& action = defs.buildActions[actionId];
		const UnitDefId resultUnit = ResolveEnemyAiBuildResultUnit(action);
		return CountAliveEnemyUnitsByDef(world, resultUnit) + CountPendingEnemyAiBuildAction(world, actionId);
	}

	inline int32 ResolveEnemyAiDesiredConstructionCount(const DefinitionStores& defs, const BuildActionDef& action, const AiProfileDef* aiProfile)
	{
		if (const AiBuildPriorityDef* buildPriority = FindAiBuildPriority(aiProfile, action.tag))
		{
			return Max(0, buildPriority->desiredCount);
		}

		const UnitDefId resultUnit = ResolveEnemyAiBuildResultUnit(action);
		if (!(0 <= resultUnit && resultUnit < defs.units.size()))
		{
			return 0;
		}

		const UnitDef& resultDef = defs.units[resultUnit];
		const double economy = aiProfile ? Clamp(aiProfile->economyFocus, 0.0, 1.0) : 0.5;
		const double defense = aiProfile ? Clamp(aiProfile->defenseFocus, 0.0, 1.0) : 0.5;
		const double tech = aiProfile ? Clamp(aiProfile->techFocus, 0.0, 1.0) : 0.3;

		if (resultDef.role == UnitRole::Barrier || resultDef.blocksTileMovement)
		{
			return 1 + static_cast<int32>(Round(defense * 2.0));
		}
		if (resultDef.role == UnitRole::Base)
		{
			int32 desired = 1;
			desired += static_cast<int32>(economy >= 0.45);
			desired += static_cast<int32>(tech >= 0.65);
			return Clamp(desired, 1, 3);
		}

		return 1 + static_cast<int32>(economy + tech >= 1.2);
	}

	inline double ResolveEnemyAiConstructionCostPenalty(const BattleWorld& world, const DefinitionStores& defs, const BuildActionDef& action)
	{
		const ResourceDefId goldResource = FindResourceDefByKind(defs, ResourceKind::Gold);
		const ResourceDefId trustResource = FindResourceDefByKind(defs, ResourceKind::Trust);
		const ResourceDefId foodResource = FindResourceDefByKind(defs, ResourceKind::Food);
		const double goldStock = static_cast<double>(GetFactionResourceAmount(world, Faction::Enemy, goldResource));
		const double trustStock = static_cast<double>(GetFactionResourceAmount(world, Faction::Enemy, trustResource));
		const double foodStock = static_cast<double>(GetFactionResourceAmount(world, Faction::Enemy, foodResource));
		const double stock = goldStock + trustStock + foodStock;
		const double weightedCost = action.costGold + action.costTrust * 1.15 + action.costFood * 0.85;

		if (weightedCost <= 0.0)
		{
			return 0.0;
		}

		return weightedCost / Max(40.0, stock + 20.0);
	}

	inline double ResolveEnemyAiConstructionScore(const BattleWorld& world, const DefinitionStores& defs, const BuildActionDef& action, BuildActionDefId actionId, const AiProfileDef* aiProfile)
	{
		const UnitDefId resultUnit = ResolveEnemyAiBuildResultUnit(action);
		if (!(0 <= resultUnit && resultUnit < defs.units.size()))
		{
			return -DBL_MAX;
		}

		const UnitDef& resultDef = defs.units[resultUnit];
		const int32 currentCount = CountEnemyAiConstructionUnits(world, defs, actionId);
		const int32 desiredCount = ResolveEnemyAiDesiredConstructionCount(defs, action, aiProfile);
		if (desiredCount <= 0 || currentCount >= desiredCount)
		{
			return -DBL_MAX;
		}

		const double economy = aiProfile ? Clamp(aiProfile->economyFocus, 0.0, 1.0) : 0.5;
		const double defense = aiProfile ? Clamp(aiProfile->defenseFocus, 0.0, 1.0) : 0.5;
		const double tech = aiProfile ? Clamp(aiProfile->techFocus, 0.0, 1.0) : 0.3;
		const double aggression = aiProfile ? Clamp(aiProfile->aggression, 0.0, 1.0) : 0.55;

		double baseScore = 1.0;
		if (resultDef.role == UnitRole::Barrier || resultDef.blocksTileMovement)
		{
			baseScore += 0.45 + defense * 1.50 + aggression * 0.35;
		}
		else if (resultDef.role == UnitRole::Base)
		{
			baseScore += 0.75 + economy * 0.90 + tech * 1.15;
		}
		else
		{
			baseScore += economy * 0.60 + tech * 0.60;
		}

		const double remainingNeed = static_cast<double>(desiredCount - currentCount);
		const double saturation = static_cast<double>(currentCount) / Max(1.0, static_cast<double>(desiredCount));
		const double costPenalty = ResolveEnemyAiConstructionCostPenalty(world, defs, action);
		const AiBuildPriorityDef* buildPriority = FindAiBuildPriority(aiProfile, action.tag);
		const double priorityWeight = buildPriority ? buildPriority->weight : 1.0;
		return baseScore * Max(0.0, priorityWeight) + remainingNeed * 0.85 - saturation * 1.35 - costPenalty;
	}

	inline UnitId FindEnemyAiBuilderForAction(const BattleWorld& world, const DefinitionStores& defs, BuildActionDefId actionId)
	{
		if (actionId >= defs.buildActions.size())
		{
			return InvalidUnitId;
		}

		const BuildActionDef& action = defs.buildActions[actionId];
		for (UnitId unit = 0; unit < world.units.size(); ++unit)
		{
			if (!IsValidUnit(world, unit) || IsBuilderBusyWithBuildQueue(world, unit))
			{
				continue;
			}
			if (CanUseBuildActionForFaction(world, defs, unit, action, Faction::Enemy))
			{
				return unit;
			}
		}

		return InvalidUnitId;
	}

	inline bool IsEnemyAiDefensiveConstruction(const DefinitionStores& defs, const BuildActionDef& action)
	{
		const UnitDefId resultUnit = ResolveEnemyAiBuildResultUnit(action);
		return (0 <= resultUnit && resultUnit < defs.units.size())
			&& (defs.units[resultUnit].role == UnitRole::Barrier || defs.units[resultUnit].blocksTileMovement);
	}

	inline bool IsEnemyAiBaseConstruction(const DefinitionStores& defs, const BuildActionDef& action)
	{
		const UnitDefId resultUnit = ResolveEnemyAiBuildResultUnit(action);
		return (0 <= resultUnit && resultUnit < defs.units.size())
			&& defs.units[resultUnit].role == UnitRole::Base;
	}

	inline Vec2 ResolveEnemyAiHomeConstructionAnchor(const BattleWorld& world, const DefinitionStores& defs)
	{
		for (UnitId unit = 0; unit < world.units.size(); ++unit)
		{
			if (!IsValidUnit(world, unit) || world.units.faction[unit] != Faction::Enemy || world.units.defId[unit] >= defs.units.size())
			{
				continue;
			}

			const UnitDef& def = defs.units[world.units.defId[unit]];
			if (def.role == UnitRole::Base && def.unit_id.lowercased() == U"home")
			{
				return world.units.position[unit];
			}
		}

		return ResolveEnemySpawnOrigin(world, defs);
	}

	inline Optional<Vec2> ResolveEnemyAiFrontlineConstructionAnchor(const BattleWorld& world, const DefinitionStores& defs)
	{
		const Vec2 home = ResolveEnemyAiHomeConstructionAnchor(world, defs);
		if (const Optional<EnemyAiAttackWaveTarget> playerBase = ResolveEnemyAiPlayerBaseAttackWaveTarget(world, defs))
		{
			return home.lerp(playerBase->position, 0.52);
		}
		if (const Optional<Vec2> resourceTarget = ResolveEnemyAiResourceTarget(world))
		{
			return home.lerp(*resourceTarget, 0.48);
		}

		return none;
	}

	inline Optional<Vec2> FindEnemyAiBuildPositionAroundAnchor(const BattleWorld& world, const DefinitionStores& defs, const Vec2& anchor)
	{
		const Point anchorCell = WorldToBattleCell(world, anchor);
		for (int32 radius = 2; radius <= 6; ++radius)
		{
			for (int32 y = -radius; y <= radius; ++y)
			{
				for (int32 x = -radius; x <= radius; ++x)
				{
					if (Max(Abs(x), Abs(y)) != radius)
					{
						continue;
					}

					const Point cell{ anchorCell.x - radius + x, anchorCell.y + y };
					if (!world.map.inBounds(cell.y, cell.x))
					{
						continue;
					}

					const Vec2 position = BattleCellToWorldPosition(cell);
					if (EvaluateBuildPlacementCell(world, defs, position) == BuildPlacementCellState::Allowed)
					{
						return position;
					}
				}
			}
		}

		return none;
	}

	inline Optional<Vec2> FindEnemyAiBuildPositionForAnchors(const BattleWorld& world, const DefinitionStores& defs, const Array<Vec2>& anchors)
	{
		for (const Vec2& anchor : anchors)
		{
			if (const Optional<Vec2> position = FindEnemyAiBuildPositionAroundAnchor(world, defs, anchor))
			{
				return position;
			}
		}

		return none;
	}

	inline Optional<Vec2> ResolveEnemyAiResourceConstructionAnchor(const BattleWorld& world, const DefinitionStores& defs)
	{
		const Vec2 home = ResolveEnemyAiHomeConstructionAnchor(world, defs);
		if (const Optional<Vec2> resourceTarget = ResolveEnemyAiResourceTarget(world))
		{
			return home.lerp(*resourceTarget, 0.68);
		}

		return none;
	}

	inline Array<Vec2> ResolveEnemyAiConstructionAnchors(const BattleWorld& world, const DefinitionStores& defs, const BuildActionDef& action)
	{
		Array<Vec2> anchors;
		const Vec2 home = ResolveEnemyAiHomeConstructionAnchor(world, defs);
		const Optional<Vec2> frontline = ResolveEnemyAiFrontlineConstructionAnchor(world, defs);
		const Optional<Vec2> resource = ResolveEnemyAiResourceConstructionAnchor(world, defs);

		auto pushUnique = [&](const Optional<Vec2>& candidate)
		{
			if (!candidate)
			{
				return;
			}
			if (anchors.any([&](const Vec2& existing) { return existing.distanceFromSq(*candidate) < Square(4.0); }))
			{
				return;
			}
			anchors << *candidate;
		};

		if (IsEnemyAiDefensiveConstruction(defs, action))
		{
			pushUnique(frontline);
			pushUnique(resource);
			anchors << home;
			return anchors;
		}

		if (IsEnemyAiBaseConstruction(defs, action))
		{
			anchors << home;
			pushUnique(resource);
			pushUnique(frontline);
			return anchors;
		}

		pushUnique(resource);
		anchors << home;
		pushUnique(frontline);
		return anchors;
	}

	inline Vec2 ResolveEnemyAiConstructionAnchor(const BattleWorld& world, const DefinitionStores& defs)
	{
		return ResolveEnemyAiHomeConstructionAnchor(world, defs);
	}

	inline Optional<EnemyAiConstructionDecision> ChooseEnemyAiConstructionAction(const BattleWorld& world, const DefinitionStores& defs, const AiProfileDef* aiProfile)
	{
		EnemyAiConstructionDecision bestDecision;
		for (BuildActionDefId actionId = 0; actionId < defs.buildActions.size(); ++actionId)
		{
			const BuildActionDef& action = defs.buildActions[actionId];
			if (!IsEnemyAiConstructionAction(defs, action))
			{
				continue;
			}

			const UnitId builder = FindEnemyAiBuilderForAction(world, defs, actionId);
			if (builder == InvalidUnitId)
			{
				continue;
			}
			if (!CanAffordBuildActionForFaction(world, defs, action, Faction::Enemy))
			{
				continue;
			}

			const double score = ResolveEnemyAiConstructionScore(world, defs, action, actionId, aiProfile);
			if (score <= -DBL_MAX / 2.0)
			{
				continue;
			}
			if (score > bestDecision.score)
			{
				bestDecision.actionId = actionId;
				bestDecision.builder = builder;
				bestDecision.score = score;
			}
		}

		if (bestDecision.actionId == InvalidBuildActionDefId)
		{
			return none;
		}

		return bestDecision;
	}

	inline void UpdateEnemyAiConstruction(BattleWorld& world, const DefinitionStores& defs, const AiProfileDef* aiProfile)
	{
		if (world.elapsedSec < ResolveAiOpeningDelaySec(aiProfile))
		{
			return;
		}
		if (world.aiRuntime.productionTimerSec < ResolveEnemyAiConstructionIntervalSec(aiProfile))
		{
			return;
		}

		const Optional<EnemyAiConstructionDecision> decision = ChooseEnemyAiConstructionAction(world, defs, aiProfile);
		if (!decision)
		{
			return;
		}

		const UnitId builder = decision->builder;
		if (builder == InvalidUnitId)
		{
			return;
		}

		const BuildActionDef& action = defs.buildActions[decision->actionId];
		const Array<Vec2> anchors = ResolveEnemyAiConstructionAnchors(world, defs, action);
		const Optional<Vec2> buildPosition = FindEnemyAiBuildPositionForAnchors(world, defs, anchors);
		if (!buildPosition)
		{
			return;
		}

		if (TryStartBuildForFaction(world, defs, builder, decision->actionId, Faction::Enemy, *buildPosition))
		{
			world.aiRuntime.productionTimerSec = 0.0;
		}
	}
}
