#pragma once
# include <Siv3D.hpp>
# include "../Data/AudioAssetCache.h"
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
        DefinitionStores battleDefinitions;
        BattleRenderAssets battleRenderAssets;
        uint64 battleDefinitionGeneration = 0;
        uint64 battleDefinitionRevision = 0;
        ResourceFlagRuntimeState resourceFlags;
        BattleNotificationRuntimeState notifications;
        double decalAmbientCooldownSec = 0.0;
        AudioAssetCache audioAssets;
        ModContext activeMod;
    };

    // 戦闘が参照する定義と描画アセットを同一世代として更新する。
    inline void PromoteBattleDefinitions(AppRuntimeState& runtime, const AppDefinitionState& definitions, uint64 definitionRevision)
    {
        runtime.battleDefinitions = definitions.defs;
        runtime.battleRenderAssets = definitions.renderAssets;
        ++runtime.battleDefinitionGeneration;
        runtime.battleDefinitionRevision = definitionRevision;
    }

    // Mod 切替時に旧コンテキストの音声アセットを停止・破棄する。
    inline void SetRuntimeActiveMod(AppRuntimeState& runtime, const ModContext& mod)
    {
        if (runtime.activeMod.rootPath != mod.rootPath)
        {
            runtime.audioAssets.clear();
        }
        runtime.activeMod = mod;
    }

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

    inline void ResetBattleRuntimeState(AppRuntimeState& runtime, const DefinitionStores& defs, bool enemyDirectorPaused, const BattleRequest* request = nullptr)
    {
        runtime.world.audioAssets = &runtime.audioAssets;
        runtime.world.audioMod = &runtime.activeMod;
        runtime.world.reset();
        SpawnDefaultBattle(runtime.world, defs, request);
        runtime.world.definitionGeneration = runtime.battleDefinitionGeneration;
        runtime.world.enemyDirectorPaused = enemyDirectorPaused;
        ClearBattleNotifications(runtime);
        SyncResourceFlagRuntimeState(runtime);
        runtime.decalAmbientCooldownSec = 0.0;
        runtime.audioAssets.clear();
    }

    inline void InitializeAppRuntimeState(AppRuntimeState& runtime, const AppDefinitionState& definitions, uint64 definitionRevision)
    {
        PromoteBattleDefinitions(runtime, definitions, definitionRevision);
        ResetBattleRuntimeState(runtime, runtime.battleDefinitions, false);
    }
}
