# pragma once
# include <Siv3D.hpp>
# include "EnemyDirectorAttackWave.h"

namespace LT3
{
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

		return weight;
	}

	inline UnitDefId ChooseEnemySpawnUnitByProfile(const DefinitionStores& defs, const Array<UnitDefId>& candidates, const AiProfileDef* aiProfile)
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
			const double weight = ResolveAiUnitSpawnWeight(aiProfile, defs.units[candidate]);
			weights << weight;
			totalWeight += weight;
		}

		if (totalWeight <= 0.0)
		{
			return candidates.choice();
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
		const Array<UnitDefId> enemySpawnCandidates = CollectEnemySpawnCandidates(defs);
		const UnitDefId spawn = ChooseEnemySpawnUnitByProfile(defs, enemySpawnCandidates, aiProfile);
		if (spawn == InvalidUnitDefId)
		{
			return;
		}

		const Vec2 enemySpawnOrigin = ResolveEnemySpawnOrigin(world, defs);
		AddUnitToBattleWorld(world, spawn, Faction::Enemy, enemySpawnOrigin + Vec2{ 0.0, Random(-100.0, 100.0) }, defs);
	}
}
