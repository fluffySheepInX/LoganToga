#pragma once
# include <Siv3D.hpp>
# include "../App/AppSceneSharedData.h"
# include "../Data/MusicPreview.h"

namespace LT3
{
	inline RectF MusicEditorPanelRect()
	{
		return RectF{ 96, 96, 560, 380 };
	}

	inline RectF MusicEditorToggleRect(const AppSharedData& data)
	{
		return data.titleUiLayout.musicEditorToggleRect;
	}

	inline bool HandleMusicEditorButton(const RectF& rect, const String& label, const Font& font)
	{
		const bool hovered = rect.mouseOver();
		if (hovered) Cursor::RequestStyle(CursorStyle::Hand);
		rect.draw(ColorF{ 0.14, 0.18, 0.24, 0.92 }).drawFrame(2.0, hovered ? ColorF{ 1.0, 0.84, 0.0, 0.95 } : ColorF{ 1.0, 1.0, 1.0, 0.22 });
		font(label).drawAt(18, rect.center(), Palette::White);
		return rect.leftClicked();
	}

	inline void UpdateMusicEditor(AppSharedData& data)
	{
		if (HandleMusicEditorButton(MusicEditorToggleRect(data), data.musicEditor.open ? U"Hide Music Editor" : U"Show Music Editor", data.uiFont))
		{
			const bool previousOpen = data.musicEditor.open;
			data.musicEditor.open = !previousOpen;
			data.musicSettings.editorOpen = data.musicEditor.open;
			if (!SaveMusicSettingsToml(data.musicSettings, data.musicEditor.statusText))
			{
				data.musicEditor.open = previousOpen;
				data.musicSettings.editorOpen = previousOpen;
			}
			else if (!data.musicEditor.open) StopMusicPreview(data.musicEditor);
			return;
		}
		if (!data.musicEditor.open) return;

		const RectF panel = MusicEditorPanelRect();
		const Array<MusicSceneId> sceneIds = AllMusicSceneIds();
		for (int32 i = 0; i < static_cast<int32>(sceneIds.size()); ++i)
		{
			const RectF row{ panel.x + 20, panel.y + 54 + i * 44.0, 160, 36 };
			if (row.mouseOver()) Cursor::RequestStyle(CursorStyle::Hand);
			if (row.leftClicked()) { data.musicEditor.selectedScene = sceneIds[i]; data.musicEditor.statusText = U"Selected music scene: {}"_fmt(ToMusicSceneLabel(sceneIds[i])); return; }
		}

		MusicTrackSetting& track = GetMusicTrackSetting(data.musicSettings, data.musicEditor.selectedScene);
		if (HandleMusicEditorButton(RectF{ panel.x + 204, panel.y + 98, 116, 34 }, U"Browse", data.uiFont))
		{
			const Optional<FilePath> path = Dialog::OpenFile({ FileFilter::AllAudioFiles(), FileFilter::AllFiles() });
			if (path) { track.path = *path; data.musicEditor.dirty = true; data.musicEditor.statusText = U"Music assigned: {} -> {}"_fmt(ToMusicSceneLabel(data.musicEditor.selectedScene), FileSystem::FileName(*path)); }
			return;
		}
		if (HandleMusicEditorButton(RectF{ panel.x + 330, panel.y + 98, 116, 34 }, U"Preview", data.uiFont)) { PlayMusicPreview(data, track); return; }
		if (HandleMusicEditorButton(RectF{ panel.x + 456, panel.y + 98, 92, 34 }, U"Stop", data.uiFont)) { StopMusicPreview(data.musicEditor); data.musicEditor.statusText = U"Preview stopped"; return; }
		if (HandleMusicEditorButton(RectF{ panel.x + 456, panel.y + 146, 92, 34 }, U"Clear", data.uiFont))
		{
			track.path.clear(); data.musicEditor.dirty = true;
			if (data.musicPlayback.activeScene && *data.musicPlayback.activeScene == data.musicEditor.selectedScene) StopSceneMusic(data);
			if (!data.musicEditor.previewPath.isEmpty()) StopMusicPreview(data.musicEditor);
			data.musicEditor.statusText = U"Music cleared: {}"_fmt(ToMusicSceneLabel(data.musicEditor.selectedScene)); return;
		}
		const auto adjustVolume = [&](double delta)
		{
			track.volume = Clamp(Math::Round((track.volume + delta) * 100.0) / 100.0, 0.0, 1.0);
			data.musicEditor.dirty = true;
			if (IsMusicPreviewPlaying(data.musicEditor)) data.musicEditor.previewAudio.setVolume(track.volume);
			data.musicEditor.statusText = U"Music volume: {} = {:.2f}"_fmt(ToMusicSceneLabel(data.musicEditor.selectedScene), track.volume);
		};
		if (HandleMusicEditorButton(RectF{ panel.x + 204, panel.y + 210, 42, 34 }, U"-", data.uiFont)) { adjustVolume(-0.05); return; }
		if (HandleMusicEditorButton(RectF{ panel.x + 390, panel.y + 210, 42, 34 }, U"+", data.uiFont)) { adjustVolume(0.05); return; }
		if (HandleMusicEditorButton(RectF{ panel.x + 204, panel.y + 318, 140, 38 }, U"Save Settings", data.uiFont)) { if (SaveMusicSettingsToml(data.musicSettings, data.musicEditor.statusText)) data.musicEditor.dirty = false; return; }
		if (HandleMusicEditorButton(RectF{ panel.x + 354, panel.y + 318, 140, 38 }, U"Reload", data.uiFont)) { StopMusicPreview(data.musicEditor); if (LoadMusicSettingsToml(data.musicSettings, data.musicEditor.statusText)) data.musicEditor.dirty = false; return; }
	}
}
