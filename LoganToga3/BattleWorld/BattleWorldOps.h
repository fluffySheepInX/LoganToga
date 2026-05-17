#pragma once
# include <Siv3D.hpp>
# include "BattleWorldStores.h"

namespace LT3
{
	inline void EnsureBattleWorldMapSize(BattleWorld& world, int32 width, int32 height)
	{
		width = Max(1, width);
		height = Max(1, height);
		world.mapWidth = width;
		world.mapHeight = height;
		ResizeBattleMapStore(world.map, width, height);
	}

	inline UnitId AddUnitToBattleWorld(BattleWorld& world, UnitDefId unitDef, Faction faction, const Vec2& pos, const DefinitionStores& defs, const String& iconOverride = U"")
	{
		const UnitId id = world.units.add(unitDef, faction, pos, defs);
		world.cooldowns.addUnit();
		world.buildQueues.addUnit();
		world.carriers.addUnit();
		world.pathing.addUnit(pos);
		if (id < world.units.iconOverride.size())
		{
			world.units.iconOverride[id] = iconOverride;
		}
		return id;
	}

	inline void AddPlacedObjectToBattleWorld(BattleWorld& world, const Vec2& pos, const String& objectTag, const String& iconName)
	{
		world.placedObjects.add(pos, objectTag, iconName);
	}
}
