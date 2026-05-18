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

    inline void SyncBattleMapPassabilityFromEditor(const MapEditorState& mapEditor, BattleWorld& world)
    {
        EnsureBattleWorldMapSize(world, mapEditor.mapWidth, mapEditor.mapHeight);

        const size_t cellCount = static_cast<size_t>(world.map.width * world.map.height);
        if (world.map.flags.size() != cellCount)
        {
            world.map.flags.assign(cellCount, 1u);
        }

        bool changed = false;
        for (size_t i = 0; i < cellCount; ++i)
        {
            uint32 newFlags = world.map.flags[i] | 1u;
            if (i < mapEditor.cells.size())
            {
                const MapEditorCell& cell = mapEditor.cells[i];
                if (0 <= cell.objectAsset && cell.objectAsset < static_cast<int32>(mapEditor.assets.size()))
                {
                    const MapEditorAsset& asset = mapEditor.assets[cell.objectAsset];
                    if (IsMapEditorDecalAsset(mapEditor, cell.objectAsset) && asset.decalBlocksPassage)
                    {
                        newFlags &= ~1u;
                    }
                }
                for (const MapEditorDecalPlacement& placement : cell.decals)
                {
                    if (0 <= placement.assetIndex && placement.assetIndex < static_cast<int32>(mapEditor.assets.size()))
                    {
                        const MapEditorAsset& decalAsset = mapEditor.assets[placement.assetIndex];
                        if (decalAsset.decalBlocksPassage)
                        {
                            newFlags &= ~1u;
                        }
                    }
                }
            }

            if (newFlags != world.map.flags[i])
            {
                world.map.flags[i] = newFlags;
                changed = true;
            }
        }

        if (changed)
        {
            ++world.map.revision;
        }
    }

    inline void SyncBattleWorldMapFromEditor(const MapEditorState& mapEditor, BattleWorld& world, const DefinitionStores& defs)
    {
        EnsureBattleWorldMapSize(world, mapEditor.mapWidth, mapEditor.mapHeight);
        SyncBattleMapPassabilityFromEditor(mapEditor, world);
        SyncBattleResourceNodesFromEditor(mapEditor, world, defs);
    }

    inline bool HandleEditorInput(MapEditorState& mapEditor, BattleWorld& world, DefinitionStores& defs, UnitCatalog& unitCatalog, const Vec2& screenMouse)
    {
        const bool wasEditorEnabled = mapEditor.enabled;
        const bool consumed = ProcessMapEditorInput(mapEditor, world, defs, unitCatalog, screenMouse);
        EnsureBattleWorldMapSize(world, mapEditor.mapWidth, mapEditor.mapHeight);
        if (wasEditorEnabled || mapEditor.enabled)
        {
            SyncBattleWorldMapFromEditor(mapEditor, world, defs);
        }
        return consumed;
    }
}
