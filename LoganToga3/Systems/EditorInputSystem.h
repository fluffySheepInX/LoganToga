# pragma once
# include <Siv3D.hpp>
# include "../BattleWorld/BattleWorld.h"
# include "../UI/MapEditor.h"

namespace LT3
{
    inline ResourceDefId FindResourceDefIdByKind(const DefinitionStores& defs, ResourceKind kind)
    {
        const String tag = ResourceKindToTag(kind);
        if (defs.resourceByTag.contains(tag))
        {
            return defs.resourceByTag.at(tag);
        }

        return InvalidResourceDefId;
    }

    inline void SyncBattleResourceNodesFromEditor(const MapEditorState& mapEditor, BattleWorld& world, const DefinitionStores& defs)
    {
        world.resourceNodes.defId.clear();
        world.resourceNodes.position.clear();
        world.resourceNodes.amount.clear();
        world.resourceNodes.incomePerSec.clear();
        world.resourceNodes.owner.clear();
        world.resourceNodes.captureProgress.clear();

        for (const auto& node : mapEditor.resourceNodes)
        {
            const ResourceDefId resourceDefId = FindResourceDefIdByKind(defs, node.kind);
            if (resourceDefId == InvalidResourceDefId)
            {
                continue;
            }

            world.resourceNodes.add(
                resourceDefId,
                MapEditorCellCenter(node.cell.x, node.cell.y),
                Max(0, node.amount),
                Max(0, node.incomePerSec));
        }
    }

    inline void SyncBattleWorldMapFromEditor(const MapEditorState& mapEditor, BattleWorld& world, const DefinitionStores& defs)
    {
        EnsureBattleWorldMapSize(world, mapEditor.mapWidth, mapEditor.mapHeight);
        SyncBattleResourceNodesFromEditor(mapEditor, world, defs);
    }

    inline bool HandleEditorInput(MapEditorState& mapEditor, BattleWorld& world, const DefinitionStores& defs, UnitCatalog& unitCatalog, const Vec2& screenMouse)
    {
        const bool wasEditorEnabled = mapEditor.enabled;
        const bool consumed = ProcessMapEditorInput(mapEditor, world, defs, unitCatalog, screenMouse);
        EnsureBattleWorldMapSize(world, mapEditor.mapWidth, mapEditor.mapHeight);
        if (wasEditorEnabled || mapEditor.enabled)
        {
            SyncBattleResourceNodesFromEditor(mapEditor, world, defs);
        }
        return consumed;
    }
}
