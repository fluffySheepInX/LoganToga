#pragma once
# include <Siv3D.hpp>
# include "../App/AppSceneSharedData.h"
# include "../Data/MusicSettings.h"
# include "../Data/MusicManager.h"
# include "../Data/MusicPreview.h"

namespace LT3
{
	class TitleScene : public AppSceneManager::Scene
	{
	public:
		explicit TitleScene(const InitData& init)
			: AppSceneManager::Scene(init)
		{
			auto& data = getData();
			LoadMusicSettingsToml(data.musicSettings, data.musicEditor.statusText);
			data.musicEditor.open = data.musicSettings.editorOpen;
			PlaySceneMusic(data, MusicSceneId::Title);
		}

		void update() override
		{
			auto& data = getData();
				HandleMusicEditorInput(data);
			if (SimpleButton(RectF{ 60, 120, 220, 44 }, U"Start Battle", data.uiFont))
			{
				StopMusicPreview(data.musicEditor);
				changeScene(AppSceneState::Battle, 0.4s);
				return;
			}
		}

		void draw() const override
		{
			const auto& data = getData();
			data.titleFont(U"LoganToga3").draw(60, 40, Palette::White);
			data.uiFont(U"Title Scene").draw(60, 84, Palette::Lightgray);
				DrawMusicEditor(data);
		}

	private:
		static RectF MusicEditorPanelRect()
		{
			return RectF{ 320, 40, 560, 380 };
		}

		static RectF MusicEditorToggleRect()
		{
			return RectF{ 320, 40, 180, 36 };
		}

		static RectF MusicSceneRowRect(const RectF& panelRect, const int32 index)
		{
			return RectF{ panelRect.x + 20, panelRect.y + 54 + index * 44.0, 160, 36 };
		}

		static RectF MusicBrowseButtonRect(const RectF& panelRect)
		{
			return RectF{ panelRect.x + 204, panelRect.y + 98, 116, 34 };
		}

		static RectF MusicPreviewButtonRect(const RectF& panelRect)
		{
			return RectF{ panelRect.x + 330, panelRect.y + 98, 116, 34 };
		}

		static RectF MusicStopButtonRect(const RectF& panelRect)
		{
			return RectF{ panelRect.x + 456, panelRect.y + 98, 92, 34 };
		}

		static RectF MusicClearButtonRect(const RectF& panelRect)
		{
			return RectF{ panelRect.x + 456, panelRect.y + 146, 92, 34 };
		}

		static RectF MusicSaveButtonRect(const RectF& panelRect)
		{
			return RectF{ panelRect.x + 204, panelRect.y + 318, 140, 38 };
		}

		static RectF MusicReloadButtonRect(const RectF& panelRect)
		{
			return RectF{ panelRect.x + 354, panelRect.y + 318, 140, 38 };
		}

		static RectF MusicVolumeDownButtonRect(const RectF& panelRect)
		{
			return RectF{ panelRect.x + 204, panelRect.y + 210, 42, 34 };
		}

		static RectF MusicVolumeUpButtonRect(const RectF& panelRect)
		{
			return RectF{ panelRect.x + 390, panelRect.y + 210, 42, 34 };
		}

		static RectF MusicVolumeValueRect(const RectF& panelRect)
		{
			return RectF{ panelRect.x + 256, panelRect.y + 210, 124, 34 };
		}

		static String ShortMusicPath(const FilePath& path)
		{
			if (path.isEmpty())
			{
				return U"(none)";
			}

			const String fileName = FileSystem::FileName(path);
			return fileName.isEmpty() ? path : fileName;
		}

		static void HandleMusicEditorInput(AppSharedData& data)
		{
			if (SimpleButton(MusicEditorToggleRect(), data.musicEditor.open ? U"Hide Music Editor" : U"Show Music Editor", data.uiFont))
			{
				data.musicEditor.open = !data.musicEditor.open;
				data.musicSettings.editorOpen = data.musicEditor.open;
				SaveMusicSettingsToml(data.musicSettings, data.musicEditor.statusText);
				if (!data.musicEditor.open)
				{
					StopMusicPreview(data.musicEditor);
				}
				return;
			}

			if (!data.musicEditor.open)
			{
				return;
			}

			const RectF panelRect = MusicEditorPanelRect();
			for (int32 i = 0; i < static_cast<int32>(AllMusicSceneIds().size()); ++i)
			{
				const MusicSceneId sceneId = AllMusicSceneIds()[i];
				const RectF rowRect = MusicSceneRowRect(panelRect, i);
				if (rowRect.mouseOver())
				{
					Cursor::RequestStyle(CursorStyle::Hand);
				}
				if (rowRect.leftClicked())
				{
					data.musicEditor.selectedScene = sceneId;
					data.musicEditor.statusText = U"Selected music scene: {}"_fmt(ToMusicSceneLabel(sceneId));
					return;
				}
			}

			MusicTrackSetting& track = GetMusicTrackSetting(data.musicSettings, data.musicEditor.selectedScene);
			if (SimpleButton(MusicBrowseButtonRect(panelRect), U"Browse", data.uiFont))
			{
				const Array<FileFilter> filters = { FileFilter::AllAudioFiles(), FileFilter::AllFiles() };
				const Optional<FilePath> path = Dialog::OpenFile(filters);
				if (path)
				{
					track.path = *path;
					data.musicEditor.dirty = true;
					data.musicEditor.statusText = U"Music assigned: {} -> {}"_fmt(ToMusicSceneLabel(data.musicEditor.selectedScene), FileSystem::FileName(*path));
				}
				return;
			}

			if (SimpleButton(MusicPreviewButtonRect(panelRect), U"Preview", data.uiFont))
			{
				PlayMusicPreview(data, track);
				return;
			}

			if (SimpleButton(MusicStopButtonRect(panelRect), U"Stop", data.uiFont))
			{
				StopMusicPreview(data.musicEditor);
				data.musicEditor.statusText = U"Preview stopped";
				return;
			}

			if (SimpleButton(MusicClearButtonRect(panelRect), U"Clear", data.uiFont))
			{
				track.path.clear();
				data.musicEditor.dirty = true;
				if (data.musicPlayback.activeScene && *data.musicPlayback.activeScene == data.musicEditor.selectedScene)
				{
					StopSceneMusic(data);
				}
				if (data.musicEditor.previewPath.isEmpty() == false)
				{
					StopMusicPreview(data.musicEditor);
				}
				data.musicEditor.statusText = U"Music cleared: {}"_fmt(ToMusicSceneLabel(data.musicEditor.selectedScene));
				return;
			}

			if (SimpleButton(MusicVolumeDownButtonRect(panelRect), U"-", data.uiFont))
			{
				track.volume = Max(0.0, Math::Round((track.volume - 0.05) * 100.0) / 100.0);
				data.musicEditor.dirty = true;
				if (IsMusicPreviewPlaying(data.musicEditor))
				{
					data.musicEditor.previewAudio.setVolume(track.volume);
				}
				data.musicEditor.statusText = U"Music volume: {} = {:.2f}"_fmt(ToMusicSceneLabel(data.musicEditor.selectedScene), track.volume);
				return;
			}

			if (SimpleButton(MusicVolumeUpButtonRect(panelRect), U"+", data.uiFont))
			{
				track.volume = Min(1.0, Math::Round((track.volume + 0.05) * 100.0) / 100.0);
				data.musicEditor.dirty = true;
				if (IsMusicPreviewPlaying(data.musicEditor))
				{
					data.musicEditor.previewAudio.setVolume(track.volume);
				}
				data.musicEditor.statusText = U"Music volume: {} = {:.2f}"_fmt(ToMusicSceneLabel(data.musicEditor.selectedScene), track.volume);
				return;
			}

			if (SimpleButton(MusicSaveButtonRect(panelRect), U"Save Settings", data.uiFont))
			{
				if (SaveMusicSettingsToml(data.musicSettings, data.musicEditor.statusText))
				{
					data.musicEditor.dirty = false;
				}
				return;
			}

			if (SimpleButton(MusicReloadButtonRect(panelRect), U"Reload", data.uiFont))
			{
				StopMusicPreview(data.musicEditor);
				LoadMusicSettingsToml(data.musicSettings, data.musicEditor.statusText);
				data.musicEditor.dirty = false;
				return;
			}
		}

		static void DrawMusicEditor(const AppSharedData& data)
		{
			if (!data.musicEditor.open)
			{
				return;
			}

			const RectF panelRect = MusicEditorPanelRect();
			panelRect.rounded(8).draw(ColorF{ 0.08, 0.10, 0.14, 0.94 }).drawFrame(2.0, ColorF{ 1.0, 1.0, 1.0, 0.16 });
			data.uiFont(U"Music Editor").draw(panelRect.x + 20, panelRect.y + 16, Palette::White);
			data.uiFont(U"Manage BGM for all scenes").draw(panelRect.x + 180, panelRect.y + 18, Palette::Lightgray);

			const Array<MusicSceneId> sceneIds = AllMusicSceneIds();
			for (int32 i = 0; i < static_cast<int32>(sceneIds.size()); ++i)
			{
				const MusicSceneId sceneId = sceneIds[i];
				const RectF rowRect = MusicSceneRowRect(panelRect, i);
				const bool selected = (sceneId == data.musicEditor.selectedScene);
				rowRect.draw(selected ? ColorF{ 0.20, 0.28, 0.42, 0.96 } : ColorF{ 0.12, 0.14, 0.20, 0.88 })
					.drawFrame(2.0, selected ? ColorF{ 1.0, 0.84, 0.0, 0.95 } : ColorF{ 1.0, 1.0, 1.0, 0.16 });
				data.uiFont(ToMusicSceneLabel(sceneId)).drawAt(16, rowRect.center(), Palette::White);
			}

			const MusicTrackSetting& track = GetMusicTrackSetting(data.musicSettings, data.musicEditor.selectedScene);
			data.uiFont(U"Selected Scene: {}"_fmt(ToMusicSceneLabel(data.musicEditor.selectedScene))).draw(panelRect.x + 204, panelRect.y + 58, Palette::Orange);
			data.uiFont(U"File").draw(panelRect.x + 204, panelRect.y + 146, Palette::Lightgray);
			RectF{ panelRect.x + 204, panelRect.y + 170, 344, 28 }.draw(ColorF{ 0.05, 0.06, 0.08, 0.92 }).drawFrame(1.0, ColorF{ 1.0, 1.0, 1.0, 0.12 });
			data.uiFont(ShortMusicPath(track.path)).draw(panelRect.x + 212, panelRect.y + 175, Palette::White);

			data.uiFont(U"Volume").draw(panelRect.x + 204, panelRect.y + 188, Palette::Lightgray);
			const RectF volumeRect = MusicVolumeValueRect(panelRect);
			volumeRect.draw(ColorF{ 0.05, 0.06, 0.08, 0.92 }).drawFrame(1.0, ColorF{ 1.0, 1.0, 1.0, 0.12 });
			data.uiFont(U"{:.2f}"_fmt(track.volume)).drawAt(16, volumeRect.center(), Palette::White);

			const String previewLabel = IsMusicPreviewPlaying(data.musicEditor) ? U"Preview: playing" : U"Preview: stopped";
			data.uiFont(previewLabel).draw(panelRect.x + 204, panelRect.y + 266, Palette::Skyblue);
			data.uiFont(data.musicEditor.dirty ? U"Unsaved changes" : U"Saved" ).draw(panelRect.x + 204, panelRect.y + 290, data.musicEditor.dirty ? Palette::Orange : Palette::Lightgreen);
			data.uiFont(data.musicEditor.statusText).draw(panelRect.x + 20, panelRect.y + 350, Palette::White);
		}

		static bool SimpleButton(const RectF& rect, const String& label, const Font& font)
		{
			const bool hovered = rect.mouseOver();
			if (hovered)
			{
				Cursor::RequestStyle(CursorStyle::Hand);
			}
			rect.draw(ColorF{ 0.14, 0.18, 0.24, 0.92 }).drawFrame(2.0, hovered ? ColorF{ 1.0, 0.84, 0.0, 0.95 } : ColorF{ 1.0, 1.0, 1.0, 0.22 });
			font(label).drawAt(18, rect.center(), Palette::White);
			return rect.leftClicked();
		}
	};
}
