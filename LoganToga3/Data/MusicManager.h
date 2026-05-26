#pragma once
# include <Siv3D.hpp>
# include "MusicSettings.h"

namespace LT3
{
	inline void StopSceneMusic(AppSharedData& data)
	{
		if (data.musicPlayback.audio)
		{
			data.musicPlayback.audio.stop();
		}
		data.musicPlayback.activeScene = none;
		data.musicPlayback.activePath.clear();
	}

	inline bool PlaySceneMusic(AppSharedData& data, const MusicSceneId sceneId)
	{
		const MusicTrackSetting& track = GetMusicTrackSetting(data.musicSettings, sceneId);
		if (track.path.isEmpty())
		{
			StopSceneMusic(data);
			data.musicEditor.statusText = U"Scene music cleared: {}"_fmt(ToMusicSceneLabel(sceneId));
			return false;
		}

		if (!FileSystem::Exists(track.path))
		{
			StopSceneMusic(data);
			data.musicEditor.statusText = U"Scene music file not found: {}"_fmt(track.path);
			return false;
		}

		const double volume = Clamp(track.volume, 0.0, 1.0);
		if (data.musicPlayback.audio
			&& data.musicPlayback.activeScene
			&& *data.musicPlayback.activeScene == sceneId
			&& data.musicPlayback.activePath == track.path)
		{
			data.musicPlayback.audio.setVolume(volume);
			if (!data.musicPlayback.audio.isPlaying())
			{
				data.musicPlayback.audio.play();
			}
			return true;
		}

		StopSceneMusic(data);
		data.musicPlayback.audio = Audio{ track.path, Loop::Yes };
		if (!data.musicPlayback.audio)
		{
			data.musicEditor.statusText = U"Scene music load failed: {}"_fmt(track.path);
			return false;
		}

		data.musicPlayback.audio.setVolume(volume);
		data.musicPlayback.audio.play();
		data.musicPlayback.activeScene = sceneId;
		data.musicPlayback.activePath = track.path;
		return true;
	}
}
