#pragma once
# include <Siv3D.hpp>
# include "../App/AppSceneSharedData.h"
# include "../Data/MusicManager.h"
# include "../Data/MusicPreview.h"
# include "../UI/MusicEditorController.h"
# include "../UI/MusicEditorDraw.h"
# include "../UI/TitleLayoutEditor.h"

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
			UpdateTitleLayoutEditor(data);
			if (data.titleUiEditor.open)
			{
				return;
			}

			UpdateMusicEditor(data);
			if (SimpleButton(BattleButtonRect(data), LocalizedText(data.localizedTexts, data.defaultTexts, U"title.skirmish"), data.uiFont))
			{
				StopMusicPreview(data.musicEditor);
				changeScene(AppSceneState::Battle, 0.4s);
			}
		}

		void draw() const override
		{
			const auto& data = getData();
			if (data.titleImage)
			{
				data.titleImage.draw(6, 6);
			}
			DrawTitleLayoutEditor(data);
			DrawMusicEditor(data);
			if (!data.startupErrorText.isEmpty())
			{
				const RectF errorRect{ 320.0, 760.0, 960.0, 54.0 };
				errorRect.rounded(8.0).draw(ColorF{ 0.32, 0.06, 0.06, 0.92 }).drawFrame(2.0, ColorF{ 1.0, 0.35, 0.30, 0.95 });
				data.uiFont(data.startupErrorText).drawAt(16, errorRect.center(), Palette::White);
			}
		}

	private:
		static RectF BattleButtonRect(const AppSharedData& data)
		{
			return data.titleUiLayout.skirmishButtonRect;
		}

		static bool SimpleButton(const RectF& rect, const String& label, const Font& font)
		{
			const bool hovered = rect.mouseOver();
			if (hovered) Cursor::RequestStyle(CursorStyle::Hand);
			rect.draw(ColorF{ 0.14, 0.18, 0.24, 0.92 }).drawFrame(2.0, hovered ? ColorF{ 1.0, 0.84, 0.0, 0.95 } : ColorF{ 1.0, 1.0, 1.0, 0.22 });
			font(label).drawAt(18, rect.center(), Palette::White);
			return rect.leftClicked();
		}
	};
}
