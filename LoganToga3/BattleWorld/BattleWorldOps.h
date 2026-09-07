#pragma once
# include <Siv3D.hpp>
# include "BattleWorldStores.h"
# include "../Data/AudioAssetCache.h"
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

	inline void PlayBattleSkillSoundIfRelevant(BattleWorld& world, const SkillDef& skill, const Vec2& worldPos)
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

		if (!world.audioAssets)
		{
			return;
		}

		static HashTable<FilePath, double> s_lastPlayTimeSec;
		const FilePath soundPath = ResolveSkillSoundEffectPath(world.audioMod, skill.soundEffect);
		Audio* const audio = world.audioAssets->findOrLoad(soundPath);
		if (!audio)
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

		audio->playOneShot(Clamp(skill.soundEffectVolume, 0.0, 1.0));
		s_lastPlayTimeSec[soundPath] = nowSec;
		RegisterActiveBattleSoundEffect(skill.soundEffect, audio->lengthSec());
	}

	inline void PlayUnitSpawnVoiceOnce(BattleWorld& world, const UnitDef& def, Faction faction)
	{
		if (def.spawnVoice.isEmpty())
		{
			return;
		}
		if (faction == Faction::Enemy && !def.spawnVoiceForEnemy)
		{
			return;
		}

		if (!world.audioAssets)
		{
			return;
		}

		static HashTable<FilePath, double> s_lastPlayTimeSec;
		const FilePath voicePath = ResolveUnitVoicePath(world.audioMod, def.spawnVoice);

		const double nowSec = Scene::Time();
		const double cooldownSec = Max(0.0, def.spawnVoiceCooldownSec);
		if (const auto it = s_lastPlayTimeSec.find(voicePath); it != s_lastPlayTimeSec.end())
		{
			if ((nowSec - it->second) < cooldownSec)
			{
				return;
			}
		}

		Audio* const audio = world.audioAssets->findOrLoad(voicePath);
		if (!audio)
		{
			return;
		}

		audio->playOneShot(Clamp(def.spawnVoiceVolume, 0.0, 1.0));
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
		const UnitId id = world.addUnit(unitDef, faction, pos, defs, iconOverride);
		if (id == InvalidUnitId)
		{
			return InvalidUnitId;
		}

		PlayUnitSpawnVoiceOnce(world, defs.units[unitDef], faction);
		return id;
	}

	inline void AddPlacedObjectToBattleWorld(BattleWorld& world, const Vec2& pos, const String& objectTag, const String& iconName)
	{
		world.placedObjects.add(pos, objectTag, iconName);
	}
}
