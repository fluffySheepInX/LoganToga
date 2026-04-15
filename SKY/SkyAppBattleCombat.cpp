# include "SkyAppBattleInternal.hpp"
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
		constexpr double SapperScoutingSkillDuration = 5.0;

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
         const double thickness,
			const double radius = 0.0)
		{
			state.attackEffects << AttackEffectInstance{
				.type = type,
				.startPosition = startPosition,
				.endPosition = endPosition,
				.color = color,
				.startedAt = Scene::Time(),
				.lifetime = lifetime,
				.thickness = thickness,
              .radius = radius,
			};
		}

		void ApplyExplosionDamage(Array<SpawnedSapper>& targets, const Vec3& explosionCenter, const double radius, const double damage)
		{
			const double radiusSq = Square(Max(0.1, radius));

			for (auto& target : targets)
			{
				if (target.hitPoints <= 0.0)
				{
					continue;
				}

				if (radiusSq < GetSpawnedSapperBasePosition(target).distanceFromSq(explosionCenter))
				{
					continue;
				}

				target.hitPoints = Max(0.0, (target.hitPoints - damage));
			}
		}

     [[nodiscard]] Array<size_t> FindMillTargetIndices(const Vec3& millPosition,
			const double attackRange,
			const int32 attackTargetCount,
         const Vec3& defendedBasePosition,
			const Array<SpawnedSapper>& targetSappers)
		{
          Array<std::pair<double, size_t>> candidates;
			const double rangeSq = Square(Max(1.0, attackRange));
			const int32 maxTargetCount = Clamp(attackTargetCount, 1, 6);

            for (size_t i = 0; i < targetSappers.size(); ++i)
			{
               if (targetSappers[i].hitPoints <= 0.0)
				{
					continue;
				}

               const Vec3 enemyPosition = GetSpawnedSapperBasePosition(targetSappers[i]);
				if (rangeSq < millPosition.distanceFromSq(enemyPosition))
				{
					continue;
				}

             const double baseDistanceSq = enemyPosition.distanceFromSq(defendedBasePosition);
                candidates << std::pair<double, size_t>{ baseDistanceSq, i };
			}

           std::sort(candidates.begin(), candidates.end(), [](const auto& a, const auto& b)
				{
					return a.first < b.first;
				});

			Array<size_t> targetIndices;
			for (const auto& candidate : candidates)
			{
				targetIndices << candidate.second;
				if (static_cast<int32>(targetIndices.size()) >= maxTargetCount)
				{
					break;
				}
			}

			return targetIndices;
		}
	}

   bool TryUsePlayerSapperScoutingSkill(SkyAppState& state, const size_t selectedSapperIndex)
	{
		if (selectedSapperIndex >= state.spawnedSappers.size())
		{
			return false;
		}

		SpawnedSapper& sapper = state.spawnedSappers[selectedSapperIndex];
		if (sapper.hitPoints <= 0.0)
		{
			state.blacksmithMenuMessage.show(U"兵が行動不能");
			return false;
		}

		sapper.scoutingSkillUntil = Max(sapper.scoutingSkillUntil, Scene::Time()) + SapperScoutingSkillDuration;
		state.blacksmithMenuMessage.show(U"兵が索敵スキルを使用");
		return true;
	}

  bool TryUsePlayerSapperExplosionSkill(SkyAppState& state, const size_t selectedSapperIndex)
	{
		if (selectedSapperIndex >= state.spawnedSappers.size())
		{
			return false;
		}

		SpawnedSapper& sapper = state.spawnedSappers[selectedSapperIndex];
		if (sapper.hitPoints <= 0.0)
		{
			state.blacksmithMenuMessage.show(U"兵が行動不能");
			return false;
		}

        if (not CanUnitUseExplosionSkill(sapper.unitType))
		{
         state.blacksmithMenuMessage.show(GetExplosionSkillDeniedMessage(sapper.unitType));
			return false;
		}

		const ExplosionSkillParameters& explosionSkill = GetExplosionSkillParameters(state.unitEditorSettings, sapper.team, sapper.unitType);
		const double gunpowderCost = Clamp(explosionSkill.gunpowderCost, 0.0, 200.0);
		const double cooldownSeconds = Clamp(explosionSkill.cooldownSeconds, 0.1, 30.0);
		const double radius = Clamp(explosionSkill.radius, 0.5, 12.0);
		const double unitDamage = Clamp(explosionSkill.unitDamage, 0.0, 300.0);
		const double baseDamage = Clamp(explosionSkill.baseDamage, 0.0, 300.0);
		const double effectLifetime = Clamp(explosionSkill.effectLifetime, 0.05, 2.0);
		const double effectThickness = Clamp(explosionSkill.effectThickness, 1.0, 20.0);
		const Vec3 effectOffset{ 0.0, Clamp(explosionSkill.effectOffsetY, 0.0, 4.0), 0.0 };

     if (state.playerResources.gunpowder < gunpowderCost)
		{
			state.blacksmithMenuMessage.show(U"火薬不足");
			return false;
		}

		if (Scene::Time() < sapper.explosionSkillCooldownUntil)
		{
			state.blacksmithMenuMessage.show(U"爆破スキルは準備中");
			return false;
		}

		const Vec3 explosionCenter = GetSpawnedSapperBasePosition(sapper);
        state.playerResources.gunpowder -= gunpowderCost;
		sapper.explosionSkillCooldownUntil = (Scene::Time() + cooldownSeconds);
		sapper.lastAttackAt = Scene::Time();

		EmitAttackEffect(state,
			AttackEffectType::Explosion,
            (explosionCenter + effectOffset),
			(explosionCenter + effectOffset),
			explosionSkill.effectColor,
			effectLifetime,
			effectThickness,
			radius);

        ApplyExplosionDamage(state.enemySappers, explosionCenter, radius, unitDamage);
		if (GetSpawnedSapperBasePosition(sapper).distanceFromSq(state.mapData.enemyBasePosition) <= Square(radius + BaseCombatRadius))
		{
            state.enemyBaseHitPoints = Max(0.0, (state.enemyBaseHitPoints - baseDamage));
		}

		state.blacksmithMenuMessage.show(U"兵が爆破スキルを使用");
		return true;
	}

	bool TryOrderPlayerSapperRetreat(SkyAppState& state, const size_t selectedSapperIndex)
	{
		if (selectedSapperIndex >= state.spawnedSappers.size())
		{
			return false;
		}

		SpawnedSapper& sapper = state.spawnedSappers[selectedSapperIndex];
		if (sapper.hitPoints <= 0.0)
		{
			state.blacksmithMenuMessage.show(U"兵が行動不能");
			return false;
		}

		if (IsSapperRetreatOrdered(sapper))
		{
			state.blacksmithMenuMessage.show(U"すでに撤退中");
			return false;
		}

		OrderSapperRetreat(sapper, state.mapData.sapperRallyPoint);
		state.blacksmithMenuMessage.show(U"撤退命令: 3秒後に離脱");
		state.selectedSapperIndices.clear();
		return true;
	}

	namespace BattleDetail
	{
        void SpawnEnemyUnit(SkyAppState& state, const SapperUnitType unitType, const bool moveImmediately)
		{
			static const Array<Vec3> spawnOffsets{
				Vec3{ -2.5, 0, -4.2 },
				Vec3{ 0.0, 0, -3.0 },
				Vec3{ 2.3, 0, -4.0 },
			};

			const Vec3 spawnPosition = state.mapData.enemyBasePosition.movedBy(spawnOffsets[state.enemyReinforcementCount % spawnOffsets.size()]);
			SpawnEnemySapper(state.enemySappers, spawnPosition, 180_deg, unitType);
			ApplyUnitParameters(state.enemySappers.back(), GetUnitParameters(state.unitEditorSettings, UnitTeam::Enemy, unitType));

			if (moveImmediately)
			{
				const size_t enemyIndex = (state.enemySappers.size() - 1);
                SetSpawnedSapperTarget(state.enemySappers[enemyIndex], GetSapperPopTargetPosition(state.mapData.playerBasePosition, enemyIndex), state.mapData, state.modelHeightSettings);
			}

			++state.enemyReinforcementCount;
		}

		void SpawnEnemyReinforcement(SkyAppState& state, const bool moveImmediately)
		{
          SpawnEnemyUnit(state, SapperUnitType::Infantry, moveImmediately);
		}

		void UpdateBaseCombat(Array<SpawnedSapper>& attackers, const Vec3& basePosition, double& baseHitPoints)
		{
			if (baseHitPoints <= 0.0)
			{
				return;
			}

			for (auto& attacker : attackers)
			{
              if (not IsSpawnedSapperCombatActive(attacker))
				{
					continue;
				}

				if ((attacker.team == UnitTeam::Enemy) && (attacker.aiRole != UnitAiRole::AssaultBase))
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

		void UpdateMillDefense(SkyAppState& state)
		{
           if ((state.playerBaseHitPoints <= 0.0) && (state.enemyBaseHitPoints <= 0.0))
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

              const MillDefenseParameters defenseParameters = GetMillDefenseParameters(placedModel);
				const double attackInterval = Clamp(defenseParameters.attackInterval, 0.2, 5.0);
				if ((currentTime - state.millLastAttackTimes[i]) < attackInterval)
				{
					continue;
				}

				const bool isEnemyMill = (placedModel.ownerTeam == UnitTeam::Enemy);
				Array<SpawnedSapper>& targetSappers = (isEnemyMill ? state.spawnedSappers : state.enemySappers);
				const Vec3& defendedBasePosition = (isEnemyMill ? state.mapData.enemyBasePosition : state.mapData.playerBasePosition);
				const double defendedBaseHitPoints = (isEnemyMill ? state.enemyBaseHitPoints : state.playerBaseHitPoints);
				if (defendedBaseHitPoints <= 0.0)
				{
					continue;
				}

              const Array<size_t> targetIndices = FindMillTargetIndices(placedModel.position,
                    defenseParameters.attackRange,
					defenseParameters.attackTargetCount,
                   defendedBasePosition,
					targetSappers);
				if (targetIndices.isEmpty())
				{
					continue;
				}

				state.millLastAttackTimes[i] = currentTime;
               for (const size_t targetIndex : targetIndices)
				{
                    SpawnedSapper& target = targetSappers[targetIndex];
					EmitAttackEffect(state,
						AttackEffectType::Laser,
						(placedModel.position + MillLaserMuzzleOffset),
						(GetSpawnedSapperRenderPosition(target) + MillLaserTargetOffset),
						ColorF{ 0.40, 0.92, 1.0, 1.0 },
						MillLaserLifetime,
						MillLaserThickness);
                   target.hitPoints = Max(0.0, (target.hitPoints - Clamp(defenseParameters.attackDamage, 1.0, 80.0)));
					ApplySapperSuppression(target,
                      Clamp(defenseParameters.suppressionDuration, 0.2, 10.0),
						Clamp(defenseParameters.suppressionMoveSpeedMultiplier, 0.1, 1.0),
						Clamp(defenseParameters.suppressionAttackDamageMultiplier, 0.1, 1.0),
						Clamp(defenseParameters.suppressionAttackIntervalMultiplier, 1.0, 10.0));
				}
			}
		}
	}
}
