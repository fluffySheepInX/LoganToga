#pragma once
# include <Siv3D.hpp>
# include "../App/AppSceneTypes.h"

namespace LT3
{
	struct MusicTrackSetting
	{
		FilePath path;
		double volume = 0.2;
	};

	inline MusicTrackSetting DefaultMusicTrackSetting(const MusicSceneId sceneId)
	{
		MusicTrackSetting setting;
		switch (sceneId)
		{
		case MusicSceneId::Title: setting.volume = 0.18; break;
		case MusicSceneId::Battle: setting.volume = 0.22; break;
		case MusicSceneId::Options: setting.volume = 0.16; break;
		case MusicSceneId::Achievements: setting.volume = 0.18; break;
		default: break;
		}
		return setting;
	}

	struct MusicEditorState
	{
		MusicSceneId selectedScene = MusicSceneId::Title;
		String statusText = U"Music settings not loaded";
		bool dirty = false;
		bool open = true;
		Audio previewAudio;
		FilePath previewPath;
	};

	struct MusicPlaybackState
	{
		Audio audio;
		Optional<MusicSceneId> activeScene;
		FilePath activePath;
	};

	struct MusicSettings
	{
		HashTable<MusicSceneId, MusicTrackSetting> tracks;
		FilePath sourcePath;
		bool editorOpen = true;
	};

	inline MusicSettings CreateDefaultMusicSettings()
	{
		MusicSettings settings;
		for (const auto sceneId : AllMusicSceneIds())
		{
			settings.tracks.emplace(sceneId, DefaultMusicTrackSetting(sceneId));
		}
		return settings;
	}

	inline MusicTrackSetting& GetMusicTrackSetting(MusicSettings& settings, const MusicSceneId sceneId)
	{
		if (const auto it = settings.tracks.find(sceneId); it != settings.tracks.end())
		{
			return it->second;
		}
		return settings.tracks.emplace(sceneId, DefaultMusicTrackSetting(sceneId)).first->second;
	}

	inline const MusicTrackSetting& GetMusicTrackSetting(const MusicSettings& settings, const MusicSceneId sceneId)
	{
		if (const auto it = settings.tracks.find(sceneId); it != settings.tracks.end())
		{
			return it->second;
		}
		static const MusicTrackSetting defaultSetting;
		return defaultSetting;
	}
}
