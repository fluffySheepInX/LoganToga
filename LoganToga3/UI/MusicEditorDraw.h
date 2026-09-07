#pragma once
# include <Siv3D.hpp>
# include "MusicEditorController.h"

namespace LT3
{
	// 音楽設定エディタを描画します。
	inline void DrawMusicEditor(const AppSharedData& data)
	{
		if (!data.musicEditor.open) return;
		const RectF panel = MusicEditorPanelRect();
		panel.rounded(8).draw(ColorF{ 0.08, 0.10, 0.14, 0.94 }).drawFrame(2.0, ColorF{ 1.0, 1.0, 1.0, 0.16 });
		data.uiFont(U"Music Editor").draw(panel.x + 20, panel.y + 16, Palette::White);
		data.uiFont(U"Manage BGM for all scenes").draw(panel.x + 180, panel.y + 18, Palette::Lightgray);

		const Array<MusicSceneId> sceneIds = AllMusicSceneIds();
		for (int32 i = 0; i < static_cast<int32>(sceneIds.size()); ++i)
		{
			const RectF row{ panel.x + 20, panel.y + 54 + i * 44.0, 160, 36 };
			const bool selected = sceneIds[i] == data.musicEditor.selectedScene;
			row.draw(selected ? ColorF{ 0.20, 0.28, 0.42, 0.96 } : ColorF{ 0.12, 0.14, 0.20, 0.88 }).drawFrame(2.0, selected ? ColorF{ 1.0, 0.84, 0.0, 0.95 } : ColorF{ 1.0, 1.0, 1.0, 0.16 });
			data.uiFont(ToMusicSceneLabel(sceneIds[i])).drawAt(16, row.center(), Palette::White);
		}

		const MusicTrackSetting& track = GetMusicTrackSetting(data.musicSettings, data.musicEditor.selectedScene);
		const String fileName = track.path.isEmpty() ? U"(none)" : FileSystem::FileName(track.path);
		data.uiFont(U"Selected Scene: {}"_fmt(ToMusicSceneLabel(data.musicEditor.selectedScene))).draw(panel.x + 204, panel.y + 58, Palette::Orange);
		data.uiFont(U"File").draw(panel.x + 204, panel.y + 146, Palette::Lightgray);
		RectF{ panel.x + 204, panel.y + 170, 344, 28 }.draw(ColorF{ 0.05, 0.06, 0.08, 0.92 }).drawFrame(1.0, ColorF{ 1.0, 1.0, 1.0, 0.12 });
		data.uiFont(fileName.isEmpty() ? track.path : fileName).draw(panel.x + 212, panel.y + 175, Palette::White);
		data.uiFont(U"Volume").draw(panel.x + 204, panel.y + 188, Palette::Lightgray);
		const RectF volumeRect{ panel.x + 256, panel.y + 210, 124, 34 };
		volumeRect.draw(ColorF{ 0.05, 0.06, 0.08, 0.92 }).drawFrame(1.0, ColorF{ 1.0, 1.0, 1.0, 0.12 });
		data.uiFont(U"{:.2f}"_fmt(track.volume)).drawAt(16, volumeRect.center(), Palette::White);
		data.uiFont(IsMusicPreviewPlaying(data.musicEditor) ? U"Preview: playing" : U"Preview: stopped").draw(panel.x + 204, panel.y + 266, Palette::Skyblue);
		data.uiFont(data.musicEditor.dirty ? U"Unsaved changes" : U"Saved").draw(panel.x + 204, panel.y + 290, data.musicEditor.dirty ? Palette::Orange : Palette::Lightgreen);
		data.uiFont(data.musicEditor.statusText).draw(panel.x + 20, panel.y + 350, Palette::White);
	}
}
