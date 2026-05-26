#pragma once
# include <Siv3D.hpp>
# include "MusicSettings.h"
# include "MusicManager.h"

namespace LT3
{
	inline void StopMusicPreview(MusicEditorState& editor)
	{
		if (editor.previewAudio)
		{
			editor.previewAudio.stop();
		}
	}

	inline bool PlayMusicPreview(AppSharedData& data, const MusicTrackSetting& track)
	{
		StopSceneMusic(data);
		MusicEditorState& editor = data.musicEditor;
		StopMusicPreview(editor);
		editor.previewPath.clear();
		if (track.path.isEmpty())
		{
			editor.statusText = U"Preview skipped: no music file selected";
			return false;
		}
		if (!FileSystem::Exists(track.path))
		{
			editor.statusText = U"Preview failed: file not found: {}"_fmt(track.path);
			return false;
		}

		editor.previewAudio = Audio{ track.path, Loop::Yes };
		if (!editor.previewAudio)
		{
			editor.statusText = U"Preview failed: could not load {}"_fmt(track.path);
			return false;
		}

		editor.previewAudio.setVolume(Clamp(track.volume, 0.0, 1.0));
		editor.previewAudio.play();
		editor.previewPath = track.path;
		editor.statusText = U"Previewing: {}"_fmt(FileSystem::FileName(track.path));
		return true;
	}

	inline bool IsMusicPreviewPlaying(const MusicEditorState& editor)
	{
		return editor.previewAudio && editor.previewAudio.isPlaying();
	}
}
