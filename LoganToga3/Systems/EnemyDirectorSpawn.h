# pragma once
# include <Siv3D.hpp>
# include "EnemyDirectorAttackWave.h"

namespace LT3
{
	inline const AiUnitWeightDef* FindAiUnitPriority(const AiProfileDef* aiProfile, StringView unitTag)
	{
		if (!aiProfile || unitTag.isEmpty())
		{
			return nullptr;
		}

		const String lowered = String{ unitTag }.lowercased();
		for (const auto& unitWeight : aiProfile->unitWeights)
		{
			if (unitWeight.unitTag.lowercased() == lowered)
			{
				return &unitWeight;
			}
		}

		return nullptr;
	}

	inline int32 CountAliveEnemySpawnUnitsByDef(const BattleWorld& world, UnitDefId unitDefId)
	{
		int32 count = 0;
		for (UnitId unit = 0; unit < world.units.size(); ++unit)
		{
			if (IsValidUnit(world, unit) && world.units.faction[unit] == Faction::Enemy && world.units.defId[unit] == unitDefId)
			{
				++count;
			}
		}

		return count;
	}

	inline double ResolveAiUnitSpawnWeight(const AiProfileDef* aiProfile, const UnitDef& def)
	{
		if (!aiProfile)
		{
			return 1.0;
		}

		const String unitTag = def.unit_id.lowercased();
		double weight = 0.0;
		for (const auto& unitWeight : aiProfile->unitWeights)
		{
			if (unitWeight.unitTag.lowercased() == unitTag)
			{
				weight += Max(0.0, unitWeight.weight);
			}
		}
		if (weight <= 0.0 && !aiProfile->initialUnits.isEmpty()
			&& aiProfile->initialUnits.any([&](const String& initialUnit) { return initialUnit.lowercased() == unitTag; }))
		{
			return 1.0;
		}

		return weight;
	}

	inline int32 ResolveAiUnitDesiredCount(const AiProfileDef* aiProfile, const UnitDef& def)
	{
		if (const AiUnitWeightDef* unitPriority = FindAiUnitPriority(aiProfile, def.unit_id))
		{
			return Max(0, unitPriority->desiredCount);
		}

		return 0;
	}

	inline double ResolveAiUnitSpawnScore(const BattleWorld& world, const DefinitionStores& defs, UnitDefId unitDefId, const AiProfileDef* aiProfile)
	{
		if (!(0 <= unitDefId && unitDefId < defs.units.size()))
		{
			return 0.0;
		}

		const UnitDef& def = defs.units[unitDefId];
		const double weight = ResolveAiUnitSpawnWeight(aiProfile, def);
		const int32 desiredCount = ResolveAiUnitDesiredCount(aiProfile, def);
		const int32 currentCount = CountAliveEnemySpawnUnitsByDef(world, unitDefId);
		const double shortage = Max(0, desiredCount - currentCount);
		const double saturation = (desiredCount > 0)
			? (static_cast<double>(currentCount) / Max(1.0, static_cast<double>(desiredCount)))
			: 0.0;

		double score = weight;
		if (desiredCount > 0)
		{
			score += shortage * 0.85;
			score -= saturation * 0.65;
		}

		return Max(0.0, score);
	}

	inline bool IsAiInitialUnitAllowed(const AiProfileDef* aiProfile, const UnitDef& def)
	{
		if (!aiProfile || aiProfile->initialUnits.isEmpty())
		{
			return true;
		}

		const String unitTag = def.unit_id.lowercased();
		return aiProfile->initialUnits.any([&](const String& initialUnit)
		{
			return initialUnit.lowercased() == unitTag;
		});
	}

	inline Array<UnitDefId> CollectEnemySpawnCandidatesForProfile(const DefinitionStores& defs, const AiProfileDef* aiProfile)
	{
		Array<UnitDefId> candidates;
		for (UnitDefId unitDefId = 0; unitDefId < defs.units.size(); ++unitDefId)
		{
			if (!IsEnemySpawnCandidate(defs, unitDefId))
			{
				continue;
			}
			if (!IsAiInitialUnitAllowed(aiProfile, defs.units[unitDefId]))
			{
				continue;
			}

			candidates << unitDefId;
		}

		if (candidates.isEmpty() && aiProfile && !aiProfile->initialUnits.isEmpty())
		{
			return CollectEnemySpawnCandidates(defs);
		}

		return candidates;
	}

	inline UnitDefId ChooseEnemySpawnUnitByProfile(const BattleWorld& world, const DefinitionStores& defs, const Array<UnitDefId>& candidates, const AiProfileDef* aiProfile)
	{
		if (candidates.isEmpty())
		{
			return InvalidUnitDefId;
		}

		double totalWeight = 0.0;
		Array<double> weights;
		weights.reserve(candidates.size());
		for (const UnitDefId candidate : candidates)
		{
			const double weight = ResolveAiUnitSpawnScore(world, defs, candidate, aiProfile);
			weights << weight;
			totalWeight += weight;
		}

		if (totalWeight <= 0.0)
		{
			return InvalidUnitDefId;
		}

		double roll = Random(0.0, totalWeight);
		for (size_t i = 0; i < candidates.size(); ++i)
		{
			roll -= weights[i];
			if (roll <= 0.0)
			{
				return candidates[i];
			}
		}

		return candidates.back();
	}

	inline double ResolveAiSpawnIntervalSec(const AiProfileDef* aiProfile)
	{
		return aiProfile ? Max(0.25, aiProfile->spawnIntervalSec) : 8.0;
	}

	inline int32 ResolveAiMaxArmySize(const AiProfileDef* aiProfile)
	{
		return aiProfile ? Max(1, aiProfile->maxArmySize) : INT32_MAX;
	}

	inline bool ShouldUseAiFreeSpawn(const AiProfileDef* aiProfile)
	{
		return !aiProfile || aiProfile->freeSpawnEnabled;
	}

	inline void UpdateEnemyAiSpawning(BattleWorld& world, const DefinitionStores& defs, const AiProfileDef* aiProfile)
	{
		if (defs.units.isEmpty())
		{
			return;
		}
		if (!ShouldUseAiFreeSpawn(aiProfile))
		{
			return;
		}
		if (world.elapsedSec < ResolveAiOpeningDelaySec(aiProfile))
		{
			return;
		}
		if (CountAliveEnemyCombatUnits(world, defs) >= ResolveAiMaxArmySize(aiProfile))
		{
			return;
		}
		if (world.aiRuntime.spawnTimerSec < ResolveAiSpawnIntervalSec(aiProfile))
		{
			return;
		}

		ResetEnemyAiSpawnTimer(world);
		const Array<UnitDefId> enemySpawnCandidates = CollectEnemySpawnCandidatesForProfile(defs, aiProfile);
		const UnitDefId spawn = ChooseEnemySpawnUnitByProfile(world, defs, enemySpawnCandidates, aiProfile);
		if (spawn == InvalidUnitDefId)
		{
			return;
		}

		const Vec2 enemySpawnOrigin = ResolveEnemySpawnOrigin(world, defs);
		AddUnitToBattleWorld(world, spawn, Faction::Enemy, enemySpawnOrigin + Vec2{ 0.0, Random(-100.0, 100.0) }, defs);
	}
}
