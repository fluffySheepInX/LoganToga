# include "SkyAppBattleCombatInternal.hpp"
# include "MainScene.hpp"

using namespace MainSupport;
using namespace SkyAppSupport;

namespace SkyAppFlow::BattleDetail
{
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
}
