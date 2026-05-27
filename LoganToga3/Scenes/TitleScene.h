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
			LoadTitleUiLayoutToml(data.titleUiLayout);
			LoadMusicSettingsToml(data.musicSettings, data.musicEditor.statusText);
			data.musicEditor.open = false;
			PlaySceneMusic(data, MusicSceneId::Title);
		}

		void update() override
		{
			auto& data = getData();
			HandleTitleUiEditorInput(data);
			if (data.titleUiEditor.open)
			{
				return;
			}

			HandleMusicEditorInput(data);
			if (SimpleButton(BattleButtonRect(data), U"スカーミッシュ", data.uiFont))
			{
				StopMusicPreview(data.musicEditor);
				changeScene(AppSceneState::Battle, 0.4s);
				return;
			}
		}

		void draw() const override
		{
			const auto& data = getData();
			if (data.titleImage)
			{
				data.titleImage.draw(6, 6);
			}
			DrawTitleUiEditorHotspot(data);
			DrawTitleUiEditor(data);
				DrawMusicEditor(data);
		}

	private:
		static RectF BattleButtonRect(const AppSharedData& data)
		{
			return data.titleUiLayout.skirmishButtonRect;
		}

		static RectF MusicEditorPanelRect()
		{
			return RectF{ 96, 96, 560, 380 };
		}

		static RectF MusicEditorToggleRect(const AppSharedData& data)
		{
			return data.titleUiLayout.musicEditorToggleRect;
		}

		static RectF TitleUiEditorHotspotRect()
		{
			return RectF{
				Scene::Width() - TitleEditorHotspotSize,
				Scene::Height() - TitleUnderBarHeight - TitleEditorHotspotSize,
				TitleEditorHotspotSize,
				TitleEditorHotspotSize,
			};
		}

		static void DrawTitleUiEditorHotspot(const AppSharedData& data)
		{
			const RectF hotspotRect = TitleUiEditorHotspotRect();
			if (!hotspotRect.mouseOver())
			{
				return;
			}

			hotspotRect.rounded(10).draw(ColorF{ 0.25, 0.55, 0.90, 0.18 }).drawFrame(2.0, ColorF{ 0.70, 0.90, 1.0, 0.65 });
			data.uiFont(U"UI").drawAt(18, hotspotRect.center(), Palette::White);
		}

		static void DrawTitleUiEditor(const AppSharedData& data)
		{
			if (!data.titleUiEditor.open)
			{
				return;
			}

			const RectF panelRect = TitleUiEditorPanelRect();
			panelRect.rounded(8).draw(ColorF{ 0.10, 0.12, 0.16, 0.96 }).drawFrame(2.0, ColorF{ 1.0, 1.0, 1.0, 0.18 });
			data.uiFont(U"Title UI Editor").draw(panelRect.x + 16, panelRect.y + 12, Palette::White);
			data.uiFont(U"Grid").draw(panelRect.x + 64, panelRect.y + 50, Palette::Lightgray);
			data.uiFont(U"{} px"_fmt(data.titleUiLayout.gridSize)).draw(panelRect.x + 112, panelRect.y + 50, Palette::Orange);

			const Array<TitleUiEditableElement> elements = { TitleUiEditableElement::SkirmishButton, TitleUiEditableElement::MusicEditorToggle };
			for (int32 i = 0; i < static_cast<int32>(elements.size()); ++i)
			{
				const RectF rowRect = TitleUiEditorElementRect(panelRect, i);
				const bool selected = (data.titleUiEditor.selectedElement && *data.titleUiEditor.selectedElement == elements[i]);
				rowRect.draw(selected ? ColorF{ 0.20, 0.28, 0.42, 0.96 } : ColorF{ 0.12, 0.14, 0.20, 0.88 })
					.drawFrame(2.0, selected ? ColorF{ 1.0, 0.84, 0.0, 0.95 } : ColorF{ 1.0, 1.0, 1.0, 0.16 });
				data.uiFont(EditableTitleLabel(elements[i])).draw(rowRect.x + 10, rowRect.y + 5, Palette::White);
			}

			data.uiFont(U"Drag button body to move").draw(panelRect.x + 16, panelRect.y + 180, Palette::Skyblue);
			data.uiFont(U"Drag right edge to resize width").draw(panelRect.x + 16, panelRect.y + 200, Palette::Skyblue);
			data.uiFont(data.titleUiEditor.statusText).draw(panelRect.x + 16, panelRect.y + 168, Palette::White);

			for (const auto element : elements)
			{
				const RectF& targetRect = EditableTitleRect(data, element);
				const bool selected = (data.titleUiEditor.selectedElement && *data.titleUiEditor.selectedElement == element);
				targetRect.drawFrame(2.0, selected ? ColorF{ 1.0, 0.84, 0.0, 0.95 } : ColorF{ 0.60, 0.80, 1.0, 0.55 });
				TitleUiEditorResizeHandleRect(targetRect).draw(selected ? ColorF{ 1.0, 0.84, 0.0, 0.60 } : ColorF{ 0.60, 0.80, 1.0, 0.30 });
			}
		}

		static RectF TitleUiEditorPanelRect()
		{
			return RectF{ TitleEditorPanelX, TitleEditorPanelY, 320, 220 };
		}

		static RectF TitleUiEditorGridDownRect(const RectF& panelRect)
		{
			return RectF{ panelRect.x + 16, panelRect.y + 44, 36, 32 };
		}

		static RectF TitleUiEditorGridUpRect(const RectF& panelRect)
		{
			return RectF{ panelRect.x + 188, panelRect.y + 44, 36, 32 };
		}

		static RectF TitleUiEditorElementRect(const RectF& panelRect, const int32 index)
		{
			return RectF{ panelRect.x + 16, panelRect.y + 92 + index * 40.0, 208, 30 };
		}

		static RectF TitleUiEditorResizeHandleRect(const RectF& targetRect)
		{
			return RectF{ targetRect.x + targetRect.w - 12, targetRect.y, 12, targetRect.h };
		}

		static RectF& EditableTitleRect(AppSharedData& data, const TitleUiEditableElement element)
		{
			switch (element)
			{
			case TitleUiEditableElement::MusicEditorToggle:
				return data.titleUiLayout.musicEditorToggleRect;
			case TitleUiEditableElement::SkirmishButton:
			default:
				return data.titleUiLayout.skirmishButtonRect;
			}
		}

		static const RectF& EditableTitleRect(const AppSharedData& data, const TitleUiEditableElement element)
		{
			switch (element)
			{
			case TitleUiEditableElement::MusicEditorToggle:
				return data.titleUiLayout.musicEditorToggleRect;
			case TitleUiEditableElement::SkirmishButton:
			default:
				return data.titleUiLayout.skirmishButtonRect;
			}
		}

		static StringView EditableTitleLabel(const TitleUiEditableElement element)
		{
			switch (element)
			{
			case TitleUiEditableElement::MusicEditorToggle:
				return U"Music Toggle";
			case TitleUiEditableElement::SkirmishButton:
			default:
				return U"Skirmish";
			}
		}

		static void HandleTitleUiEditorInput(AppSharedData& data)
		{
			TitleUiEditorState& editor = data.titleUiEditor;
			RepairTitleUiLayout(data.titleUiLayout);
			const RectF hotspotRect = TitleUiEditorHotspotRect();
			if (hotspotRect.mouseOver())
			{
				Cursor::RequestStyle(CursorStyle::Hand);
			}
			if (hotspotRect.leftClicked())
			{
				editor.open = !editor.open;
				editor.statusText = editor.open ? U"Title UI Editor opened" : U"Title UI Editor closed";
				return;
			}

			if (!editor.open)
			{
				return;
			}

			const RectF panelRect = TitleUiEditorPanelRect();
			if (SimpleButton(TitleUiEditorGridDownRect(panelRect), U"-", data.uiFont))
			{
				data.titleUiLayout.gridSize = Clamp(data.titleUiLayout.gridSize - 8, 8, 160);
				SaveTitleUiLayoutToml(data.titleUiLayout);
				editor.statusText = U"Grid: {}"_fmt(data.titleUiLayout.gridSize);
				return;
			}
			if (SimpleButton(TitleUiEditorGridUpRect(panelRect), U"+", data.uiFont))
			{
				data.titleUiLayout.gridSize = Clamp(data.titleUiLayout.gridSize + 8, 8, 160);
				SaveTitleUiLayoutToml(data.titleUiLayout);
				editor.statusText = U"Grid: {}"_fmt(data.titleUiLayout.gridSize);
				return;
			}

			const Array<TitleUiEditableElement> elements = { TitleUiEditableElement::SkirmishButton, TitleUiEditableElement::MusicEditorToggle };
			for (int32 i = 0; i < static_cast<int32>(elements.size()); ++i)
			{
				const RectF rowRect = TitleUiEditorElementRect(panelRect, i);
				if (rowRect.leftClicked())
				{
					editor.selectedElement = elements[i];
					editor.statusText = U"Selected: {}"_fmt(EditableTitleLabel(elements[i]));
					return;
				}
			}

			if (!editor.selectedElement)
			{
				return;
			}

			RectF& targetRect = EditableTitleRect(data, *editor.selectedElement);
			const RectF resizeHandleRect = TitleUiEditorResizeHandleRect(targetRect);
			if (!editor.dragOffset && !editor.resizing && MouseL.down() && resizeHandleRect.mouseOver())
			{
				editor.resizing = true;
				editor.resizeAnchorLeft = targetRect.x;
				editor.resizeAnchorY = targetRect.y;
				return;
			}
			if (!editor.resizing && !editor.dragOffset && MouseL.down() && targetRect.mouseOver())
			{
				editor.dragOffset = Cursor::PosF() - targetRect.pos;
				return;
			}

			if (editor.resizing && MouseL.pressed())
			{
				targetRect.w = Max(TitleUiEditorMinButtonWidth, SnapTitleUiScalar(Cursor::PosF().x - editor.resizeAnchorLeft, data.titleUiLayout.gridSize));
				SaveTitleUiLayoutToml(data.titleUiLayout);
				editor.statusText = U"Resized: {}"_fmt(EditableTitleLabel(*editor.selectedElement));
			}
			else if (editor.dragOffset && MouseL.pressed())
			{
				const Vec2 snapped = SnapTitleUiPosition(Cursor::PosF() - *editor.dragOffset, data.titleUiLayout.gridSize);
				targetRect.x = Clamp(snapped.x, 0.0, Max(0.0, Scene::Width() - targetRect.w));
				targetRect.y = Clamp(snapped.y, 0.0, Max(0.0, Scene::Height() - TitleUnderBarHeight - targetRect.h));
				SaveTitleUiLayoutToml(data.titleUiLayout);
				editor.statusText = U"Moved: {}"_fmt(EditableTitleLabel(*editor.selectedElement));
			}

			if (MouseL.up())
			{
				if (editor.dragOffset || editor.resizing)
				{
					RepairTitleUiLayout(data.titleUiLayout);
					SaveTitleUiLayoutToml(data.titleUiLayout);
				}
				editor.dragOffset.reset();
				editor.resizing = false;
			}
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
			if (SimpleButton(MusicEditorToggleRect(data), data.musicEditor.open ? U"Hide Music Editor" : U"Show Music Editor", data.uiFont))
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
