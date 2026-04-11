# include "SkyAppBattleInternal.hpp"
# include "MainScene.hpp"

using namespace MainSupport;
using namespace SkyAppSupport;

namespace SkyAppFlow
{
   namespace
	{
       [[nodiscard]] EnemyBattlePlan DetermineAutoEnemyBattlePlan(const SkyAppState& state, const Optional<SapperUnitType>& producibleUnitType)
		{
			if (producibleUnitType
				&& (GetUnitAiRole(state.unitEditorSettings, UnitTeam::Enemy, *producibleUnitType) == UnitAiRole::AssaultBase))
			{
				return EnemyBattlePlan::AssaultBase;
			}

			return state.enemyTargetResourceAreaIndex
				? EnemyBattlePlan::SecureResources
				: EnemyBattlePlan::AssaultBase;
		}

       [[nodiscard]] double GetEnemyUnitCost(const SkyAppState& state, const SapperUnitType unitType)
		{
           return GetUnitParameters(state.unitEditorSettings, UnitTeam::Enemy, unitType).manaCost;
		}

      [[nodiscard]] Optional<SapperUnitType> GetEnemyProducibleUnitType(const SkyAppState& state)
		{
                Optional<SapperUnitType> bestUnitType;
             int32 bestPriority = std::numeric_limits<int32>::lowest();

				for (const auto& unitDefinition : GetUnitDefinitions())
				{
					if (GetEnemyUnitCost(state, unitDefinition.unitType) > state.enemyResources.mana)
					{
						continue;
					}

					if (bestPriority < unitDefinition.enemyProductionPriority)
					{
						bestPriority = unitDefinition.enemyProductionPriority;
						bestUnitType = unitDefinition.unitType;
					}
				}

				return bestUnitType;
		}

		[[nodiscard]] double GetEnemyResourceAreaPriority(const SkyAppState& state, const size_t areaIndex)
		{
			if (areaIndex >= state.mapData.resourceAreas.size())
			{
				return -Math::Inf;
			}

			const ResourceArea& area = state.mapData.resourceAreas[areaIndex];
			const ResourceAreaState& areaState = state.resourceAreaStates[areaIndex];
			double score = 0.0;

			if (areaState.ownerTeam && (*areaState.ownerTeam == UnitTeam::Enemy))
			{
				return -Math::Inf;
			}

			switch (area.type)
			{
			case ResourceType::Mana:
				score += 90.0;
				break;

			case ResourceType::Budget:
				score += 45.0;
				break;

			case ResourceType::Gunpowder:
			default:
				score += 30.0;
				break;
			}

			if (not areaState.ownerTeam)
			{
				score += 80.0;
			}
			else if (*areaState.ownerTeam == UnitTeam::Player)
			{
				score += 130.0;
			}

			if (areaState.capturingTeam && (*areaState.capturingTeam == UnitTeam::Enemy))
			{
				score += 25.0;
			}

			score -= (0.6 * state.mapData.enemyBasePosition.distanceFrom(area.position));
			return score;
		}

		[[nodiscard]] Optional<size_t> FindPriorityEnemyResourceArea(const SkyAppState& state)
		{
			double bestScore = -Math::Inf;
			Optional<size_t> bestIndex;

			for (size_t i = 0; i < state.mapData.resourceAreas.size(); ++i)
			{
				const double score = GetEnemyResourceAreaPriority(state, i);
				if (bestScore < score)
				{
					bestScore = score;
					bestIndex = i;
				}
			}

			return bestIndex;
		}

		[[nodiscard]] Vec3 GetEnemyAssaultTargetPosition(const SkyAppState& state, const size_t formationIndex)
		{
			return GetSapperPopTargetPosition(state.mapData.playerBasePosition, formationIndex);
		}

		[[nodiscard]] Vec3 GetEnemyResourceCaptureTargetPosition(const SkyAppState& state, const size_t formationIndex)
		{
			if (state.enemyTargetResourceAreaIndex
				&& (*state.enemyTargetResourceAreaIndex < state.mapData.resourceAreas.size()))
			{
				return GetSapperPopTargetPosition(state.mapData.resourceAreas[*state.enemyTargetResourceAreaIndex].position, formationIndex);
			}

			return GetEnemyAssaultTargetPosition(state, formationIndex);
		}

		[[nodiscard]] Optional<Vec3> FindEnemySupportAnchorPosition(const SkyAppState& state, const size_t supportIndex)
		{
			if (supportIndex >= state.enemySappers.size())
			{
				return none;
			}

			const Vec3 supportPosition = GetSpawnedSapperBasePosition(state.enemySappers[supportIndex]);
			double nearestDistanceSq = Math::Inf;
			Optional<Vec3> anchorPosition;

			for (size_t i = 0; i < state.enemySappers.size(); ++i)
			{
				if ((i == supportIndex) || (not IsSpawnedSapperCombatActive(state.enemySappers[i])))
				{
					continue;
				}

               if (state.enemySappers[i].aiRole == UnitAiRole::Support)
				{
					continue;
				}

				const Vec3 candidatePosition = GetSpawnedSapperBasePosition(state.enemySappers[i]);
				const double distanceSq = supportPosition.distanceFromSq(candidatePosition);
				if (distanceSq < nearestDistanceSq)
				{
					nearestDistanceSq = distanceSq;
					anchorPosition = candidatePosition;
				}
			}

			return anchorPosition;
		}

		[[nodiscard]] Vec3 GetEnemySupportTargetPosition(const SkyAppState& state, const size_t formationIndex)
		{
			if (const Optional<Vec3> supportAnchor = FindEnemySupportAnchorPosition(state, formationIndex))
			{
				return GetSapperPopTargetPosition(*supportAnchor, formationIndex);
			}

			const Vec3 strategicObjective = ((state.enemyBattlePlan == EnemyBattlePlan::SecureResources)
				? GetEnemyResourceCaptureTargetPosition(state, formationIndex)
				: GetEnemyAssaultTargetPosition(state, formationIndex));
			const Vec3 stagingPosition = state.mapData.enemyBasePosition.lerp(strategicObjective, 0.55);
			return GetSapperPopTargetPosition(stagingPosition, formationIndex);
		}

		[[nodiscard]] Vec3 GetEnemyAdvanceTargetPosition(const SkyAppState& state, const size_t formationIndex)
		{
			if (formationIndex >= state.enemySappers.size())
			{
				return GetEnemyAssaultTargetPosition(state, formationIndex);
			}

         switch (state.enemySappers[formationIndex].aiRole)
			{
			case UnitAiRole::Support:
				return GetEnemySupportTargetPosition(state, formationIndex);

			case UnitAiRole::AssaultBase:
				return GetEnemyAssaultTargetPosition(state, formationIndex);

			case UnitAiRole::SecureResources:
			default:
				return GetEnemyResourceCaptureTargetPosition(state, formationIndex);
			}
		}

		void UpdateEnemyBattlePlan(SkyAppState& state)
		{
			if (Scene::Time() < state.nextEnemyAiDecisionAt)
			{
				return;
			}

			state.nextEnemyAiDecisionAt = (Scene::Time() + EnemyAiDecisionInterval);
			state.enemyTargetResourceAreaIndex = FindPriorityEnemyResourceArea(state);

         const Optional<SapperUnitType> producibleUnitType = GetEnemyProducibleUnitType(state);
          switch (state.enemyBattlePlanOverride)
			{
           case EnemyBattlePlanOverride::ForceSecureResources:
				state.enemyBattlePlan = EnemyBattlePlan::SecureResources;
				return;

			case EnemyBattlePlanOverride::ForceAssaultBase:
				state.enemyBattlePlan = EnemyBattlePlan::AssaultBase;
				return;

			case EnemyBattlePlanOverride::Auto:
			default:
				break;
			}

         state.enemyBattlePlan = DetermineAutoEnemyBattlePlan(state, producibleUnitType);
		}

      void TryProduceEnemyUnit(SkyAppState& state)
		{
           const Optional<SapperUnitType> unitType = GetEnemyProducibleUnitType(state);

				if ((state.enemyBaseHitPoints <= 0.0) || (Scene::Time() < state.nextEnemyProductionAt) || (not unitType))
			{
				return;
			}

             state.enemyResources.mana -= GetEnemyUnitCost(state, *unitType);
			state.nextEnemyProductionAt = (Scene::Time() + EnemyStrongUnitProductionCooldown);
         BattleDetail::SpawnEnemyUnit(state, *unitType, false);
		}

		void UpdateEnemyAdvanceTargets(SkyAppState& state)
		{
			for (size_t i = 0; i < state.enemySappers.size(); ++i)
			{
                if (not IsSpawnedSapperCombatActive(state.enemySappers[i]))
				{
					continue;
				}

              const Vec3 desiredTarget = GetEnemyAdvanceTargetPosition(state, i);

				if (EnemyAdvanceStopDistance < GetSpawnedSapperBasePosition(state.enemySappers[i]).distanceFrom(state.mapData.playerBasePosition)
					&& 0.25 < state.enemySappers[i].destinationPosition.distanceFrom(desiredTarget))
				{
                    SetSpawnedSapperTarget(state.enemySappers[i], desiredTarget, state.mapData, state.modelHeightSettings);
				}
			}
		}

		void UpdateMatchResult(SkyAppState& state)
		{
			if (state.enemyBaseHitPoints <= 0.0)
			{
				state.playerWon = true;
				state.showBlacksmithMenu = false;
				state.selectedSapperIndices.clear();
			}
			else if (state.playerBaseHitPoints <= 0.0)
			{
				state.playerWon = false;
				state.showBlacksmithMenu = false;
				state.selectedSapperIndices.clear();
			}
		}
	}

	void UpdateAttackEffects(SkyAppState& state)
	{
		const double currentTime = Scene::Time();
		state.attackEffects.remove_if([currentTime](const AttackEffectInstance& effect)
			{
				return ((effect.lifetime <= 0.0) || ((effect.startedAt + effect.lifetime) <= currentTime));
			});
	}

	bool IsValidMillIndex(const SkyAppState& state, const Optional<size_t>& millIndex)
	{
		return millIndex
			&& (*millIndex < state.mapData.placedModels.size())
			&& (state.mapData.placedModels[*millIndex].type == PlaceableModelType::Mill);
	}

	void ResetMatch(SkyAppState& state)
	{
		state.spawnedSappers.clear();
		state.enemySappers.clear();
		state.selectedSapperIndices.clear();
		state.selectedMillIndex.reset();
		state.selectionDragStart.reset();
		state.showBlacksmithMenu = false;
		state.playerBaseHitPoints = BaseMaxHitPoints;
		state.enemyBaseHitPoints = BaseMaxHitPoints;
		state.millLastAttackTimes = Array<double>(state.mapData.placedModels.size(), -1000.0);
      state.attackEffects.clear();
		BattleDetail::ResetResourceState(state);
		state.playerTier = 1;
      state.enemyBattlePlan = EnemyBattlePlan::SecureResources;
		state.enemyTargetResourceAreaIndex.reset();
		state.nextEnemyAiDecisionAt = 0.0;
		state.nextEnemyProductionAt = 0.0;
		state.enemyReinforcementCount = 0;
		state.playerWon.reset();
      ResetCameraToPlayerBase(state);
	}

	String ReloadMapAndResetMatch(SkyAppState& state)
	{
		const MapDataLoadResult loadResult = LoadMapDataWithStatus(MapDataPath);
		state.mapData = loadResult.mapData;
		state.mapEditor.hoveredGroundPosition.reset();
		ResetMatch(state);
		return loadResult.message;
	}

	void UpdateBattleState(SkyAppState& state)
	{
     state.playerResources.mana += (ManaIncomePerSecond * Scene::DeltaTime());
		 state.enemyResources.mana += (ManaIncomePerSecond * Scene::DeltaTime());

     UpdateEnemyBattlePlan(state);
       TryProduceEnemyUnit(state);
      UpdateEnemyAdvanceTargets(state);

     ResolveSapperSpacingAgainstUnits(state.spawnedSappers, state.enemySappers, state.modelHeightSettings);
		ResolveSapperSpacingAgainstUnits(state.enemySappers, state.spawnedSappers, state.modelHeightSettings);
		ResolveSapperSpacingAgainstBase(state.spawnedSappers, state.mapData.enemyBasePosition, state.modelHeightSettings);
		ResolveSapperSpacingAgainstBase(state.enemySappers, state.mapData.playerBasePosition, state.modelHeightSettings);
        ResolveSapperSpacingAgainstObstacles(state.spawnedSappers, state.mapData, state.modelHeightSettings);
		ResolveSapperSpacingAgainstObstacles(state.enemySappers, state.mapData, state.modelHeightSettings);
     BattleDetail::UpdateResourceAreas(state);
		BattleDetail::UpdateMillDefense(state);

     UpdateAutoCombat(state.spawnedSappers, state.enemySappers, state.modelHeightSettings);
		UpdateAutoCombat(state.enemySappers, state.spawnedSappers, state.modelHeightSettings);
      BattleDetail::UpdateBaseCombat(state.spawnedSappers, state.mapData.enemyBasePosition, state.enemyBaseHitPoints);
		BattleDetail::UpdateBaseCombat(state.enemySappers, state.mapData.playerBasePosition, state.playerBaseHitPoints);
		RemoveDefeatedSappers(state.spawnedSappers);
		RemoveDefeatedSappers(state.enemySappers);

        UpdateMatchResult(state);
	}
}
