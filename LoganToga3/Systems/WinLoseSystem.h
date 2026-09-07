# pragma once
# include <Siv3D.hpp>
# include "BattleQueries.h"

namespace LT3
{
    // 指定陣営の終局条件が満たされているかを判定する。
    inline bool IsBattleEndConditionMet(BattleEndCondition condition, bool opposingBasesAlive, bool opposingUnitsAlive)
    {
        switch (condition)
        {
        case BattleEndCondition::DestroyEnemyBases:
            return !opposingBasesAlive;
        case BattleEndCondition::EliminateEnemyUnits:
            return !opposingUnitsAlive;
        default:
            return false;
        }
    }

    // 両陣営の状態を同一スナップショットから評価し、終局結果を一度だけ確定する。
    inline void UpdateWinLose(BattleWorld& world, const DefinitionStores& defs)
    {
        if (IsTerminalBattleOutcome(world.outcome))
        {
            return;
        }

        bool playerBaseAlive = false;
        bool enemyBaseAlive = false;
        bool playerUnitAlive = false;
        bool enemyUnitAlive = false;

        for (const UnitId unit : GetLiveBattleWorldUnits(world))
        {
            if (!IsValidUnit(world, unit)) continue;
            const Faction faction = world.units.faction[unit];
            playerUnitAlive |= faction == Faction::Player;
            enemyUnitAlive |= faction == Faction::Enemy;
            if (defs.units[world.units.defId[unit]].role == UnitRole::Base)
            {
                playerBaseAlive |= faction == Faction::Player;
                enemyBaseAlive |= faction == Faction::Enemy;
            }
        }

        const bool victory = IsBattleEndConditionMet(world.outcomeRules.victoryCondition, enemyBaseAlive, enemyUnitAlive);
        const bool defeat = IsBattleEndConditionMet(world.outcomeRules.defeatCondition, playerBaseAlive, playerUnitAlive);
        if (victory || defeat)
        {
            world.outcome = victory && defeat
                ? BattleOutcome::Draw
                : (victory ? BattleOutcome::Victory : BattleOutcome::Defeat);
            return;
        }

        if (world.outcomeRules.timeLimitSec > 0.0 && world.elapsedSec >= world.outcomeRules.timeLimitSec)
        {
            world.outcome = world.outcomeRules.timeoutOutcome;
        }
    }
}
