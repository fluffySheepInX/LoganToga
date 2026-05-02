# pragma once
# include <Siv3D.hpp>
# include "../BattleWorld/BattleWorld.h"
# include "../UI/MapEditor.h"

namespace LT3
{
    inline bool HandleEditorInput(MapEditorState& mapEditor, BattleWorld& world, const DefinitionStores& defs, UnitCatalog& unitCatalog, const Vec2& screenMouse)
    {
        const bool consumed = ProcessMapEditorInput(mapEditor, world, defs, unitCatalog, screenMouse);
        world.mapWidth = mapEditor.mapWidth;
        world.mapHeight = mapEditor.mapHeight;
        return consumed;
    }
}
