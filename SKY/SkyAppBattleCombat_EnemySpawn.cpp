# include "SkyAppBattleCombatInternal.hpp"
# include "MainScene.hpp"

using namespace MainSupport;
using namespace SkyAppSupport;

namespace SkyAppFlow::BattleDetail
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
}
