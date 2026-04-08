# include "SkyAppLoopInternal.hpp"
# include "MainScene.hpp"

using namespace MainSupport;
using namespace SkyAppSupport;

namespace SkyAppFlow
{
	namespace
	{
        constexpr double MillLaserLifetime = 0.12;
		constexpr double MillLaserThickness = 5.0;
		constexpr Vec3 MillLaserMuzzleOffset{ 0.0, 4.6, 0.0 };
		constexpr Vec3 MillLaserTargetOffset{ 0.0, 1.6, 0.0 };

		void AddResource(ResourceStock& stock, const ResourceType type, const double amount)
		{
			switch (type)
			{
			case ResourceType::Budget:
				stock.budget += amount;
				return;

			case ResourceType::Gunpowder:
				stock.gunpowder += amount;
				return;

			case ResourceType::Mana:
				stock.mana += amount;
				return;

			default:
				return;
			}
		}

		[[nodiscard]] double GetResourceIncomeAmount(const ResourceType type)
		{
			switch (type)
			{
			case ResourceType::Budget:
				return BudgetAreaIncome;

			case ResourceType::Gunpowder:
				return GunpowderAreaIncome;

			case ResourceType::Mana:
				return ManaAreaIncome;

			default:
				return 0.0;
			}
		}

		void ResetResourceState(SkyAppState& state)
		{
			state.playerResources = ResourceStock{ .budget = StartingResources };
			state.enemyResources = {};
			state.resourceAreaStates = Array<ResourceAreaState>(state.mapData.resourceAreas.size());
		}

		void SpawnEnemyReinforcement(SkyAppState& state, const bool moveImmediately)
		{
			static const Array<Vec3> spawnOffsets{
				Vec3{ -2.5, 0, -4.2 },
				Vec3{ 0.0, 0, -3.0 },
				Vec3{ 2.3, 0, -4.0 },
			};

			const Vec3 spawnPosition = state.mapData.enemyBasePosition.movedBy(spawnOffsets[state.enemyReinforcementCount % spawnOffsets.size()]);
			SpawnEnemySapper(state.enemySappers, spawnPosition, 180_deg);

			if (moveImmediately)
			{
				const size_t enemyIndex = (state.enemySappers.size() - 1);
               SetSpawnedSapperTarget(state.enemySappers[enemyIndex], GetSapperPopTargetPosition(state.mapData.playerBasePosition, enemyIndex), state.mapData);
			}

			++state.enemyReinforcementCount;
		}

		void UpdateBaseCombat(Array<SpawnedSapper>& attackers, const Vec3& basePosition, double& baseHitPoints)
		{
			if (baseHitPoints <= 0.0)
			{
				return;
			}

			for (auto& attacker : attackers)
			{
				if (attacker.hitPoints <= 0.0)
				{
					continue;
				}

				const double attackDistance = (attacker.attackRange + BaseCombatRadius);
				const double distanceSq = GetSpawnedSapperBasePosition(attacker).distanceFromSq(basePosition);

				if (Square(attackDistance) < distanceSq)
				{
					continue;
				}

              if ((Scene::Time() - attacker.lastAttackAt) < GetEffectiveSapperAttackInterval(attacker))
				{
					continue;
				}

				attacker.lastAttackAt = Scene::Time();
              baseHitPoints = Max(0.0, (baseHitPoints - GetEffectiveSapperAttackDamage(attacker)));
			}
		}

		void EnsureMillDefenseState(SkyAppState& state)
		{
			if (state.millLastAttackTimes.size() != state.mapData.placedModels.size())
			{
				state.millLastAttackTimes = Array<double>(state.mapData.placedModels.size(), -1000.0);
			}
		}

		void EmitAttackEffect(SkyAppState& state,
			const AttackEffectType type,
			const Vec3& startPosition,
			const Vec3& endPosition,
			const ColorF& color,
			const double lifetime,
			const double thickness)
		{
			state.attackEffects << AttackEffectInstance{
				.type = type,
				.startPosition = startPosition,
				.endPosition = endPosition,
				.color = color,
				.startedAt = Scene::Time(),
				.lifetime = lifetime,
				.thickness = thickness,
			};
		}

		[[nodiscard]] Optional<size_t> FindMillTargetIndex(const Vec3& millPosition, const double attackRange, const Vec3& playerBasePosition, const Array<SpawnedSapper>& enemySappers)
		{
			double bestBaseDistanceSq = Math::Inf;
			Optional<size_t> bestIndex;
			const double rangeSq = Square(Max(1.0, attackRange));

			for (size_t i = 0; i < enemySappers.size(); ++i)
			{
				if (enemySappers[i].hitPoints <= 0.0)
				{
					continue;
				}

				const Vec3 enemyPosition = GetSpawnedSapperBasePosition(enemySappers[i]);
				if (rangeSq < millPosition.distanceFromSq(enemyPosition))
				{
					continue;
				}

				const double baseDistanceSq = enemyPosition.distanceFromSq(playerBasePosition);
				if (baseDistanceSq < bestBaseDistanceSq)
				{
					bestBaseDistanceSq = baseDistanceSq;
					bestIndex = i;
				}
			}

			return bestIndex;
		}

		void UpdateMillDefense(SkyAppState& state)
		{
			if (state.playerBaseHitPoints <= 0.0)
			{
				return;
			}

			EnsureMillDefenseState(state);
			const double currentTime = Scene::Time();

			for (size_t i = 0; i < state.mapData.placedModels.size(); ++i)
			{
				const PlacedModel& placedModel = state.mapData.placedModels[i];
				if (placedModel.type != PlaceableModelType::Mill)
				{
					continue;
				}

				const double attackInterval = Clamp(placedModel.attackInterval, 0.2, 5.0);
				if ((currentTime - state.millLastAttackTimes[i]) < attackInterval)
				{
					continue;
				}

				const Optional<size_t> targetIndex = FindMillTargetIndex(placedModel.position, placedModel.attackRange, state.mapData.playerBasePosition, state.enemySappers);
				if (not targetIndex)
				{
					continue;
				}

				state.millLastAttackTimes[i] = currentTime;
				SpawnedSapper& target = state.enemySappers[*targetIndex];
               EmitAttackEffect(state,
					AttackEffectType::Laser,
					(placedModel.position + MillLaserMuzzleOffset),
					(GetSpawnedSapperRenderPosition(target) + MillLaserTargetOffset),
					ColorF{ 0.40, 0.92, 1.0, 1.0 },
					MillLaserLifetime,
					MillLaserThickness);
				target.hitPoints = Max(0.0, (target.hitPoints - Clamp(placedModel.attackDamage, 1.0, 80.0)));
               ApplySapperSuppression(target,
                    Clamp(placedModel.suppressionDuration, 0.2, 10.0),
					Clamp(placedModel.suppressionMoveSpeedMultiplier, 0.1, 1.0),
					Clamp(placedModel.suppressionAttackDamageMultiplier, 0.1, 1.0),
					Clamp(placedModel.suppressionAttackIntervalMultiplier, 1.0, 10.0));
			}
		}

		void UpdateResourceAreas(SkyAppState& state)
		{
			const double deltaTime = Scene::DeltaTime();

			if (state.resourceAreaStates.size() != state.mapData.resourceAreas.size())
			{
				state.resourceAreaStates = Array<ResourceAreaState>(state.mapData.resourceAreas.size());
			}

			for (size_t i = 0; i < state.mapData.resourceAreas.size(); ++i)
			{
				const ResourceArea& area = state.mapData.resourceAreas[i];
				ResourceAreaState& areaState = state.resourceAreaStates[i];
				int32 playerInside = 0;
				int32 enemyInside = 0;
				const double radiusSq = Square(area.radius);

				for (const auto& sapper : state.spawnedSappers)
				{
					if ((sapper.hitPoints > 0.0) && (GetSpawnedSapperBasePosition(sapper).distanceFromSq(area.position) <= radiusSq))
					{
						++playerInside;
					}
				}

				for (const auto& sapper : state.enemySappers)
				{
					if ((sapper.hitPoints > 0.0) && (GetSpawnedSapperBasePosition(sapper).distanceFromSq(area.position) <= radiusSq))
					{
						++enemyInside;
					}
				}

				Optional<UnitTeam> occupyingTeam;
				if ((0 < playerInside) && (enemyInside == 0))
				{
					occupyingTeam = UnitTeam::Player;
				}
				else if ((0 < enemyInside) && (playerInside == 0))
				{
					occupyingTeam = UnitTeam::Enemy;
				}

				if (occupyingTeam)
				{
					if (areaState.ownerTeam && (*areaState.ownerTeam == *occupyingTeam))
					{
						areaState.capturingTeam.reset();
						areaState.captureProgress = ResourceAreaCaptureSeconds;
					}
					else
					{
						if ((not areaState.capturingTeam) || (*areaState.capturingTeam != *occupyingTeam))
						{
							areaState.capturingTeam = *occupyingTeam;
							areaState.captureProgress = 0.0;
						}

						areaState.captureProgress += deltaTime;

						if (ResourceAreaCaptureSeconds <= areaState.captureProgress)
						{
							areaState.ownerTeam = *occupyingTeam;
							areaState.capturingTeam.reset();
							areaState.captureProgress = ResourceAreaCaptureSeconds;
							areaState.incomeProgress = 0.0;
						}
					}
				}
				else if (areaState.capturingTeam)
				{
					areaState.captureProgress = Max(0.0, (areaState.captureProgress - deltaTime));
					if (areaState.captureProgress <= 0.0)
					{
						areaState.capturingTeam.reset();
					}
				}

				if (areaState.ownerTeam)
				{
					areaState.incomeProgress += deltaTime;

					while (ResourceAreaIncomeIntervalSeconds <= areaState.incomeProgress)
					{
						areaState.incomeProgress -= ResourceAreaIncomeIntervalSeconds;
						if (*areaState.ownerTeam == UnitTeam::Player)
						{
							AddResource(state.playerResources, area.type, GetResourceIncomeAmount(area.type));
						}
						else
						{
							AddResource(state.enemyResources, area.type, GetResourceIncomeAmount(area.type));
						}
					}
				}
				else
				{
					areaState.incomeProgress = 0.0;
				}
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
		ResetResourceState(state);
      state.playerTier = 1;
		state.nextEnemyReinforcementAt = (Scene::Time() + EnemyReinforcementInterval);
		state.enemyReinforcementCount = 0;
		state.playerWon.reset();
       ResetCameraToPlayerBase(state);
		SpawnEnemyReinforcement(state, true);
		SpawnEnemyReinforcement(state, true);
		SpawnEnemyReinforcement(state, true);
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

		if ((state.enemyBaseHitPoints > 0.0) && (state.nextEnemyReinforcementAt <= Scene::Time()))
		{
			SpawnEnemyReinforcement(state, false);
			state.nextEnemyReinforcementAt = (Scene::Time() + EnemyReinforcementInterval);
		}

		for (size_t i = 0; i < state.enemySappers.size(); ++i)
		{
			const Vec3 desiredTarget = GetSapperPopTargetPosition(state.mapData.playerBasePosition, i);

			if (EnemyAdvanceStopDistance < GetSpawnedSapperBasePosition(state.enemySappers[i]).distanceFrom(state.mapData.playerBasePosition)
             && 0.25 < state.enemySappers[i].destinationPosition.distanceFrom(desiredTarget))
			{
               SetSpawnedSapperTarget(state.enemySappers[i], desiredTarget, state.mapData);
			}
		}

		ResolveSapperSpacingAgainstUnits(state.spawnedSappers, state.enemySappers);
		ResolveSapperSpacingAgainstUnits(state.enemySappers, state.spawnedSappers);
		ResolveSapperSpacingAgainstBase(state.spawnedSappers, state.mapData.enemyBasePosition);
		ResolveSapperSpacingAgainstBase(state.enemySappers, state.mapData.playerBasePosition);
        ResolveSapperSpacingAgainstObstacles(state.spawnedSappers, state.mapData);
		ResolveSapperSpacingAgainstObstacles(state.enemySappers, state.mapData);
		UpdateResourceAreas(state);
		UpdateMillDefense(state);

		UpdateAutoCombat(state.spawnedSappers, state.enemySappers);
		UpdateAutoCombat(state.enemySappers, state.spawnedSappers);
		UpdateBaseCombat(state.spawnedSappers, state.mapData.enemyBasePosition, state.enemyBaseHitPoints);
		UpdateBaseCombat(state.enemySappers, state.mapData.playerBasePosition, state.playerBaseHitPoints);
		RemoveDefeatedSappers(state.spawnedSappers);
		RemoveDefeatedSappers(state.enemySappers);

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
