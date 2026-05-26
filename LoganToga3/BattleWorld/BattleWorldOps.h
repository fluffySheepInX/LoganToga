#pragma once
# include <Siv3D.hpp>
# include "BattleWorldStores.h"
# include "../Data/BattleAssetPaths.h"

namespace LT3
{
	inline void PlayUnitSpawnVoiceOnce(const UnitDef& def, Faction faction)
	{
		if (def.spawnVoice.isEmpty())
		{
			return;
		}
		if (faction == Faction::Enemy && !def.spawnVoiceForEnemy)
		{
			return;
		}

		const FilePath voicePath = ResolveUnitVoicePath(def.spawnVoice);
		if (voicePath.isEmpty() || !FileSystem::Exists(voicePath))
		{
			return;
		}

		static HashTable<FilePath, Audio> s_audioCache;
		static HashTable<FilePath, double> s_lastPlayTimeSec;

		const double nowSec = Scene::Time();
		const double cooldownSec = Max(0.0, def.spawnVoiceCooldownSec);
		if (const auto it = s_lastPlayTimeSec.find(voicePath); it != s_lastPlayTimeSec.end())
		{
			if ((nowSec - it->second) < cooldownSec)
			{
				return;
			}
		}

		auto cacheIt = s_audioCache.find(voicePath);
		if (cacheIt == s_audioCache.end())
		{
			cacheIt = s_audioCache.emplace(voicePath, Audio{ voicePath }).first;
		}

		if (!cacheIt->second)
		{
			return;
		}

		cacheIt->second.playOneShot(Clamp(def.spawnVoiceVolume, 0.0, 1.0));
		s_lastPlayTimeSec[voicePath] = nowSec;
	}

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
		if (unitDef >= defs.units.size())
		{
			return InvalidUnitId;
		}

		const UnitId id = world.units.add(unitDef, faction, pos, defs);
		world.cooldowns.addUnit();
		world.buildQueues.addUnit();
		world.carriers.addUnit();
		world.pathing.addUnit(pos);
		if (id < world.units.iconOverride.size())
		{
			world.units.iconOverride[id] = iconOverride;
		}
		PlayUnitSpawnVoiceOnce(defs.units[unitDef], faction);
		return id;
	}

	inline void AddPlacedObjectToBattleWorld(BattleWorld& world, const Vec2& pos, const String& objectTag, const String& iconName)
	{
		world.placedObjects.add(pos, objectTag, iconName);
	}
}
