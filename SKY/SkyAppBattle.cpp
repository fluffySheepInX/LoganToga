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
				&& (GetUnitAiRole(state.editor.unitEditorSettings, UnitTeam::Enemy, *producibleUnitType) == UnitAiRole::AssaultBase))
			{
				return EnemyBattlePlan::AssaultBase;
			}

			return state.enemyAi.targetResourceAreaIndex
				? EnemyBattlePlan::SecureResources
				: EnemyBattlePlan::AssaultBase;
		}

       [[nodiscard]] double GetEnemyUnitCost(const SkyAppState& state, const SapperUnitType unitType)
		{
           return GetUnitParameters(state.editor.unitEditorSettings, UnitTeam::Enemy, unitType).manaCost;
		}

      [[nodiscard]] Optional<SapperUnitType> GetEnemyProducibleUnitType(const SkyAppState& state)
		{
                Optional<SapperUnitType> bestUnitType;
             int32 bestPriority = std::numeric_limits<int32>::lowest();

				for (const auto& unitDefinition : GetUnitDefinitions())
				{
					if (GetEnemyUnitCost(state, unitDefinition.unitType) > state.battle.enemyResources.mana)
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
			if (areaIndex >= state.world.mapData.resourceAreas.size())
			{
				return -Math::Inf;
			}

			const ResourceArea& area = state.world.mapData.resourceAreas[areaIndex];
			const ResourceAreaState& areaState = state.battle.resourceAreaStates[areaIndex];
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

			score -= (0.6 * state.world.mapData.enemyBasePosition.distanceFrom(area.position));
			return score;
		}

		[[nodiscard]] Optional<size_t> FindPriorityEnemyResourceArea(const SkyAppState& state)
		{
			double bestScore = -Math::Inf;
			Optional<size_t> bestIndex;

			for (size_t i = 0; i < state.world.mapData.resourceAreas.size(); ++i)
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
			return GetSapperPopTargetPosition(state.world.mapData.playerBasePosition, formationIndex);
		}

		[[nodiscard]] Vec3 GetEnemyResourceCaptureTargetPosition(const SkyAppState& state, const size_t formationIndex)
		{
			if (state.enemyAi.targetResourceAreaIndex
				&& (*state.enemyAi.targetResourceAreaIndex < state.world.mapData.resourceAreas.size()))
			{
				return GetSapperPopTargetPosition(state.world.mapData.resourceAreas[*state.enemyAi.targetResourceAreaIndex].position, formationIndex);
			}

			return GetEnemyAssaultTargetPosition(state, formationIndex);
		}

		[[nodiscard]] Optional<Vec3> FindEnemySupportAnchorPosition(const SkyAppState& state, const size_t supportIndex)
		{
			if (supportIndex >= state.battle.enemySappers.size())
			{
				return none;
			}

			const Vec3 supportPosition = GetSpawnedSapperBasePosition(state.battle.enemySappers[supportIndex]);
			double nearestDistanceSq = Math::Inf;
			Optional<Vec3> anchorPosition;

			for (size_t i = 0; i < state.battle.enemySappers.size(); ++i)
			{
				if ((i == supportIndex) || (not IsSpawnedSapperCombatActive(state.battle.enemySappers[i])))
				{
					continue;
				}

               if (state.battle.enemySappers[i].aiRole == UnitAiRole::Support)
				{
					continue;
				}

				const Vec3 candidatePosition = GetSpawnedSapperBasePosition(state.battle.enemySappers[i]);
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

			const Vec3 strategicObjective = ((state.enemyAi.battlePlan == EnemyBattlePlan::SecureResources)
				? GetEnemyResourceCaptureTargetPosition(state, formationIndex)
				: GetEnemyAssaultTargetPosition(state, formationIndex));
			const Vec3 stagingPosition = state.world.mapData.enemyBasePosition.lerp(strategicObjective, 0.55);
			return GetSapperPopTargetPosition(stagingPosition, formationIndex);
		}

		[[nodiscard]] Vec3 GetEnemyAdvanceTargetPosition(const SkyAppState& state, const size_t formationIndex)
		{
			if (formationIndex >= state.battle.enemySappers.size())
			{
				return GetEnemyAssaultTargetPosition(state, formationIndex);
			}

         switch (state.battle.enemySappers[formationIndex].aiRole)
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
			if (Scene::Time() < state.enemyAi.nextDecisionAt)
			{
				return;
			}

			state.enemyAi.nextDecisionAt = (Scene::Time() + EnemyAiDecisionInterval);
			state.enemyAi.targetResourceAreaIndex = FindPriorityEnemyResourceArea(state);

		 const Optional<SapperUnitType> producibleUnitType = GetEnemyProducibleUnitType(state);
		  switch (state.enemyAi.battlePlanOverride)
			{
		   case EnemyBattlePlanOverride::ForceSecureResources:
				state.enemyAi.battlePlan = EnemyBattlePlan::SecureResources;
				return;

			case EnemyBattlePlanOverride::ForceAssaultBase:
				state.enemyAi.battlePlan = EnemyBattlePlan::AssaultBase;
				return;

			case EnemyBattlePlanOverride::Auto:
			default:
				break;
			}

		 state.enemyAi.battlePlan = DetermineAutoEnemyBattlePlan(state, producibleUnitType);
		}

      void TryProduceEnemyUnit(SkyAppState& state)
		{
           const Optional<SapperUnitType> unitType = GetEnemyProducibleUnitType(state);

				if ((state.battle.enemyBaseHitPoints <= 0.0) || (Scene::Time() < state.enemyAi.nextProductionAt) || (not unitType))
			{
				return;
			}

			 state.battle.enemyResources.mana -= GetEnemyUnitCost(state, *unitType);
			state.enemyAi.nextProductionAt = (Scene::Time() + EnemyStrongUnitProductionCooldown);
         BattleDetail::SpawnEnemyUnit(state, *unitType, false);
		}

		void UpdateEnemyAdvanceTargets(SkyAppState& state)
		{
			for (size_t i = 0; i < state.battle.enemySappers.size(); ++i)
			{
                if (not IsSpawnedSapperCombatActive(state.battle.enemySappers[i]))
				{
					continue;
				}

              const Vec3 desiredTarget = GetEnemyAdvanceTargetPosition(state, i);

				if (EnemyAdvanceStopDistance < GetSpawnedSapperBasePosition(state.battle.enemySappers[i]).distanceFrom(state.world.mapData.playerBasePosition)
					&& 0.25 < state.battle.enemySappers[i].destinationPosition.distanceFrom(desiredTarget))
				{
                    SetSpawnedSapperTarget(state.battle.enemySappers[i], desiredTarget, state.world.mapData, state.editor.modelHeightSettings);
				}
			}
		}

		void UpdateMatchResult(SkyAppState& state)
		{
			if (state.battle.enemyBaseHitPoints <= 0.0)
			{
				state.battle.playerWon = true;
				state.hud.showBlacksmithMenu = false;
				state.battle.selectedSapperIndices.clear();
			}
			else if (state.battle.playerBaseHitPoints <= 0.0)
			{
				state.battle.playerWon = false;
				state.hud.showBlacksmithMenu = false;
				state.battle.selectedSapperIndices.clear();
			}
		}
	}

	void UpdateAttackEffects(SkyAppState& state)
	{
		const double currentTime = Scene::Time();
		state.battle.attackEffects.remove_if([currentTime](const AttackEffectInstance& effect)
			{
				return ((effect.lifetime <= 0.0) || ((effect.startedAt + effect.lifetime) <= currentTime));
			});
	}

	bool IsValidMillIndex(const SkyAppState& state, const Optional<size_t>& millIndex)
	{
		return millIndex
			&& (*millIndex < state.world.mapData.placedModels.size())
			&& (state.world.mapData.placedModels[*millIndex].type == PlaceableModelType::Mill);
	}

	void ResetMatch(SkyAppState& state)
	{
		state.battle.spawnedSappers.clear();
		state.battle.enemySappers.clear();
		state.battle.selectedSapperIndices.clear();
		state.battle.selectedMillIndex.reset();
		state.battle.selectionDragStart.reset();
		state.hud.showBlacksmithMenu = false;
		state.battle.playerBaseHitPoints = BaseMaxHitPoints;
		state.battle.enemyBaseHitPoints = BaseMaxHitPoints;
		state.battle.millLastAttackTimes = Array<double>(state.world.mapData.placedModels.size(), -1000.0);
      state.battle.attackEffects.clear();
		BattleDetail::ResetResourceState(state);
		state.battle.playerTier = 1;
     state.battle.battleCommandSelectedSlotIndex = 0;
		state.battle.battleCommandUnlockedSlotCount = 1;
     state.battle.moveOrderIndicator.reset();
	  state.enemyAi.battlePlan = EnemyBattlePlan::SecureResources;
		state.enemyAi.targetResourceAreaIndex.reset();
		state.enemyAi.nextDecisionAt = 0.0;
		state.enemyAi.nextProductionAt = 0.0;
		state.enemyAi.nextReinforcementAt = 0.0;
		state.enemyAi.reinforcementCount = 0;
		state.battle.playerWon.reset();
       ResetFogOfWar(state.env.fogOfWar);
      ResetCameraToPlayerBase(state);
	}

	String ReloadMapAndResetMatch(SkyAppState& state)
	{
		const MapDataLoadResult loadResult = LoadMapDataWithStatus(MapDataPath);
		state.world.mapData = loadResult.mapData;
		state.editor.mapEditor.hoveredGroundPosition.reset();
		ResetMatch(state);
		return loadResult.message;
	}

	void UpdateBattleState(SkyAppState& state)
	{
     state.battle.playerResources.mana += (ManaIncomePerSecond * Scene::DeltaTime());
		 state.battle.enemyResources.mana += (ManaIncomePerSecond * Scene::DeltaTime());

     UpdateEnemyBattlePlan(state);
       TryProduceEnemyUnit(state);
      UpdateEnemyAdvanceTargets(state);

     ResolveSapperSpacingAgainstUnits(state.battle.spawnedSappers, state.battle.enemySappers, state.editor.modelHeightSettings);
		ResolveSapperSpacingAgainstUnits(state.battle.enemySappers, state.battle.spawnedSappers, state.editor.modelHeightSettings);
		ResolveSapperSpacingAgainstBase(state.battle.spawnedSappers, state.world.mapData.enemyBasePosition, state.editor.modelHeightSettings);
		ResolveSapperSpacingAgainstBase(state.battle.enemySappers, state.world.mapData.playerBasePosition, state.editor.modelHeightSettings);
        ResolveSapperSpacingAgainstObstacles(state.battle.spawnedSappers, state.world.mapData, state.editor.modelHeightSettings);
		ResolveSapperSpacingAgainstObstacles(state.battle.enemySappers, state.world.mapData, state.editor.modelHeightSettings);
     BattleDetail::UpdateResourceAreas(state);
		BattleDetail::UpdateMillDefense(state);

     UpdateAutoCombat(state.battle.spawnedSappers, state.battle.enemySappers, state.editor.modelHeightSettings);
		UpdateAutoCombat(state.battle.enemySappers, state.battle.spawnedSappers, state.editor.modelHeightSettings);
      BattleDetail::UpdateBaseCombat(state.battle.spawnedSappers, state.world.mapData.enemyBasePosition, state.battle.enemyBaseHitPoints);
		BattleDetail::UpdateBaseCombat(state.battle.enemySappers, state.world.mapData.playerBasePosition, state.battle.playerBaseHitPoints);
		RemoveDefeatedSappers(state.battle.spawnedSappers);
		RemoveDefeatedSappers(state.battle.enemySappers);

        UpdateMatchResult(state);
	}
}
