#pragma once
# include <Siv3D.hpp>
# include "AppDefinitionState.h"
# include "BattleNotificationState.h"
# include "../Systems/BattleSystems.h"

namespace LT3
{
    struct ResourceNodeFlagState
    {
        Faction displayFaction = Faction::Neutral;
        Faction lastOwner = Faction::Neutral;
        double raiseTimerSec = 0.0;
        bool raising = false;
        bool visible = false;
    };

    struct ResourceFlagRuntimeState
    {
        Array<ResourceNodeFlagState> nodes;
    };

    struct AppRuntimeState
    {
        BattleWorld world;
        ResourceFlagRuntimeState resourceFlags;
        BattleNotificationRuntimeState notifications;
        double decalAmbientCooldownSec = 0.0;
        HashTable<FilePath, Audio> decalAmbientAudioCache;
    };

    inline void ClearBattleNotifications(AppRuntimeState& runtime)
    {
        ClearBattleNotifications(runtime.notifications);
    }

    inline void PushBattleNotification(AppRuntimeState& runtime, const String& message, BattleNotificationType type = BattleNotificationType::Normal)
    {
        PushBattleNotification(runtime.notifications, message, type);
    }

    inline void SyncResourceFlagRuntimeState(AppRuntimeState& runtime)
    {
        const size_t nodeCount = runtime.world.resourceNodes.position.size();
        const size_t oldCount = runtime.resourceFlags.nodes.size();
        runtime.resourceFlags.nodes.resize(nodeCount);

        for (size_t i = oldCount; i < nodeCount; ++i)
        {
            auto& flagState = runtime.resourceFlags.nodes[i];
            const Faction owner = (i < runtime.world.resourceNodes.owner.size())
                ? runtime.world.resourceNodes.owner[i]
                : Faction::Neutral;
            const bool visible = (owner != Faction::Neutral)
                && (i < runtime.world.resourceNodes.captureProgress.size())
                && (runtime.world.resourceNodes.captureProgress[i] >= 1.0);

            flagState.displayFaction = owner;
            flagState.lastOwner = owner;
            flagState.raiseTimerSec = 0.0;
            flagState.raising = false;
            flagState.visible = visible;
        }
    }

    inline void ResetBattleRuntimeState(AppRuntimeState& runtime, const DefinitionStores& defs, bool enemyDirectorPaused)
    {
        runtime.world = BattleWorld{};
        SpawnDefaultBattle(runtime.world, defs);
        runtime.world.enemyDirectorPaused = enemyDirectorPaused;
        ClearBattleNotifications(runtime);
        SyncResourceFlagRuntimeState(runtime);
        runtime.decalAmbientCooldownSec = 0.0;
        runtime.decalAmbientAudioCache.clear();
    }

    inline void InitializeAppRuntimeState(AppRuntimeState& runtime, const AppDefinitionState& definitions)
    {
        ResetBattleRuntimeState(runtime, definitions.defs, false);
    }
}
