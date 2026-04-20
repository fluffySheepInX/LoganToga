# include "SkyAppBattleCombatInternal.hpp"
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

        struct MillTargetCandidate
        {
            double baseDistanceSq;
            size_t index;
        };

        void EnsureMillDefenseState(SkyAppState& state)
        {
            if (state.battle.millLastAttackTimes.size() != state.world.mapData.placedModels.size())
            {
                const size_t previousSize = state.battle.millLastAttackTimes.size();
                state.battle.millLastAttackTimes.resize(state.world.mapData.placedModels.size(), -1000.0);
                for (size_t i = previousSize; i < state.battle.millLastAttackTimes.size(); ++i)
                {
                    state.battle.millLastAttackTimes[i] = -1000.0;
                }
            }
        }

        [[nodiscard]] Array<size_t> FindMillTargetIndices(const Vec3& millPosition,
            const double attackRange,
            const int32 attackTargetCount,
            const Vec3& defendedBasePosition,
            const Array<SpawnedSapper>& targetSappers)
        {
            Array<MillTargetCandidate> candidates;
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

                candidates << MillTargetCandidate{
                    .baseDistanceSq = enemyPosition.distanceFromSq(defendedBasePosition),
                    .index = i,
                };
            }

            std::sort(candidates.begin(), candidates.end(), [](const MillTargetCandidate& a, const MillTargetCandidate& b)
                {
                    return a.baseDistanceSq < b.baseDistanceSq;
                });

            Array<size_t> targetIndices;
            for (const auto& candidate : candidates)
            {
                targetIndices << candidate.index;
                if (static_cast<int32>(targetIndices.size()) >= maxTargetCount)
                {
                    break;
                }
            }

            return targetIndices;
        }

        void ApplyMillAttackToTarget(SkyAppState& state, const Vec3& millPosition, const MillDefenseParameters& defenseParameters, SpawnedSapper& target)
        {
            EmitAttackEffect(state,
                AttackEffectType::Laser,
                (millPosition + MillLaserMuzzleOffset),
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

    namespace BattleDetail
    {
        void UpdateMillDefense(SkyAppState& state)
        {
            if ((state.battle.playerBaseHitPoints <= 0.0) && (state.battle.enemyBaseHitPoints <= 0.0))
            {
                return;
            }

            EnsureMillDefenseState(state);
            const double currentTime = Scene::Time();
            const MillDefenseParameters defenseParameters = GetMillDefenseParameters(state.world.mapData);

            for (size_t i = 0; i < state.world.mapData.placedModels.size(); ++i)
            {
                const PlacedModel& placedModel = state.world.mapData.placedModels[i];
                if (placedModel.type != PlaceableModelType::Mill)
                {
                    continue;
                }

                const double attackInterval = Clamp(defenseParameters.attackInterval, 0.2, 5.0);
                if ((currentTime - state.battle.millLastAttackTimes[i]) < attackInterval)
                {
                    continue;
                }

                const bool isEnemyMill = (placedModel.ownerTeam == UnitTeam::Enemy);
                Array<SpawnedSapper>& targetSappers = (isEnemyMill ? state.battle.spawnedSappers : state.battle.enemySappers);
                const Vec3& defendedBasePosition = (isEnemyMill ? state.world.mapData.enemyBasePosition : state.world.mapData.playerBasePosition);
                const double defendedBaseHitPoints = (isEnemyMill ? state.battle.enemyBaseHitPoints : state.battle.playerBaseHitPoints);
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

                state.battle.millLastAttackTimes[i] = currentTime;
                for (const size_t targetIndex : targetIndices)
                {
                    ApplyMillAttackToTarget(state, placedModel.position, defenseParameters, targetSappers[targetIndex]);
                }
            }
        }
    }
}
