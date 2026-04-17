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
            if (state.millLastAttackTimes.size() != state.mapData.placedModels.size())
            {
                const size_t previousSize = state.millLastAttackTimes.size();
                state.millLastAttackTimes.resize(state.mapData.placedModels.size(), -1000.0);
                for (size_t i = previousSize; i < state.millLastAttackTimes.size(); ++i)
                {
                    state.millLastAttackTimes[i] = -1000.0;
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
                    ApplyMillAttackToTarget(state, placedModel.position, defenseParameters, targetSappers[targetIndex]);
                }
            }
        }
    }
}
