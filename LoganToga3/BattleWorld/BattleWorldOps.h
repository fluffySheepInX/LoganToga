#pragma once
# include <Siv3D.hpp>
# include "BattleWorldStores.h"
# include "../Data/BattleAssetPaths.h"
# include "../UI/QuarterView.h"

namespace LT3
{
	inline constexpr double BattleSkillSoundZoomThreshold = 1.15;
	inline constexpr double BattleSkillSoundCursorRadius = 168.0;
	inline constexpr double BattleSkillSoundCenterRadius = 220.0;

	struct ActiveBattleSoundEffectEntry
	{
		String name;
		double endTimeSec = 0.0;
	};

	inline Array<ActiveBattleSoundEffectEntry>& ActiveBattleSoundEffects()
	{
		static Array<ActiveBattleSoundEffectEntry> s_entries;
		return s_entries;
	}

	inline void PruneActiveBattleSoundEffects()
	{
		const double nowSec = Scene::Time();
		auto& entries = ActiveBattleSoundEffects();
		entries.remove_if([&](const ActiveBattleSoundEffectEntry& entry)
		{
			return entry.endTimeSec <= nowSec;
		});
	}

	inline void RegisterActiveBattleSoundEffect(StringView name, double durationSec)
	{
		auto& entries = ActiveBattleSoundEffects();
		const double endTimeSec = Scene::Time() + Max(0.08, durationSec);
		for (auto& entry : entries)
		{
			if (entry.name == name)
			{
				entry.endTimeSec = Max(entry.endTimeSec, endTimeSec);
				return;
			}
		}

		entries << ActiveBattleSoundEffectEntry{ String{ name }, endTimeSec };
	}

	inline Array<String> GetActiveBattleSoundEffectNames()
	{
		PruneActiveBattleSoundEffects();
		Array<String> names;
		for (const auto& entry : ActiveBattleSoundEffects())
		{
			names << entry.name;
		}
		return names;
	}

	inline bool ShouldPlayBattleSkillSoundAtScreen(const Vec2& screenPos)
	{
		const double zoom = QuarterViewCamera2D.getScale();
		if (zoom < BattleSkillSoundZoomThreshold)
		{
			return false;
		}

		const Vec2 cursor = Cursor::PosF();
		const Vec2 center{ QuarterLogicalSceneWidth() * 0.5, QuarterLogicalSceneHeight() * 0.5 };
		const bool nearCursor = (screenPos.distanceFrom(cursor) <= BattleSkillSoundCursorRadius);
		const bool nearCenter = (screenPos.distanceFrom(center) <= BattleSkillSoundCenterRadius);
		return nearCursor || nearCenter;
	}

	inline void PlayBattleSkillSoundIfRelevant(const SkillDef& skill, const Vec2& worldPos)
	{
		PruneActiveBattleSoundEffects();

		if (skill.soundEffect.isEmpty())
		{
			return;
		}

		const Vec2 screenPos = ToQuarterViewportScreen(worldPos);
		if (!RectF{ -96.0, -96.0, QuarterLogicalSceneWidth() + 192.0, QuarterLogicalSceneHeight() + 192.0 }.intersects(screenPos))
		{
			return;
		}
		if (!ShouldPlayBattleSkillSoundAtScreen(screenPos))
		{
			return;
		}

		const FilePath soundPath = ResolveSkillSoundEffectPath(skill.soundEffect);
		if (soundPath.isEmpty() || !FileSystem::Exists(soundPath))
		{
			return;
		}

		static HashTable<FilePath, Audio> s_audioCache;
		static HashTable<FilePath, double> s_lastPlayTimeSec;

		auto cacheIt = s_audioCache.find(soundPath);
		if (cacheIt == s_audioCache.end())
		{
			cacheIt = s_audioCache.emplace(soundPath, Audio{ soundPath }).first;
		}
		if (!cacheIt->second)
		{
			return;
		}

		const double nowSec = Scene::Time();
		constexpr double MinReplayGapSec = 0.05;
		if (const auto it = s_lastPlayTimeSec.find(soundPath); it != s_lastPlayTimeSec.end())
		{
			if ((nowSec - it->second) < MinReplayGapSec)
			{
				return;
			}
		}

		cacheIt->second.playOneShot(Clamp(skill.soundEffectVolume, 0.0, 1.0));
		s_lastPlayTimeSec[soundPath] = nowSec;
		RegisterActiveBattleSoundEffect(skill.soundEffect, cacheIt->second.lengthSec());
	}

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
