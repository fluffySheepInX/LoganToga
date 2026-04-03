# include "MainUi.hpp"
# include "MainSettings.hpp"

namespace MainSupport
{
	bool DrawTextButton(const Rect& rect, StringView label)
	{
		static const Font buttonFont{ 18, Typeface::Bold };
		const bool hovered = rect.mouseOver();
		rect.draw(hovered ? ColorF{ 0.82 } : ColorF{ 0.72 })
			.drawFrame(1, 0, ColorF{ 0.35 });
		buttonFont(label).drawAt(rect.center(), ColorF{ 0.15 });
		return hovered && MouseL.down();
	}

	void DrawAnimationClipSelector(BirdModel& model, StringView title, const int32 x, const int32 y, const int32 width)
	{
		if (not model.hasAnimations())
		{
			return;
		}

		const auto& clips = model.animations();
		SimpleGUI::GetFont()(U"{} ({})"_fmt(title, clips.size())).draw(x, y, ColorF{ 0.11 });

		for (size_t i = 0; i < clips.size(); ++i)
		{
			const int32 buttonY = (y + 28 + static_cast<int32>(i) * 34);
			const bool isCurrent = (i == model.currentClipIndex());
			const Rect clipButton{ x, buttonY, width, 30 };
			const String label = U"[{}] {}"_fmt(i, clips[i].name);
			const bool hovered = clipButton.mouseOver();
			clipButton.draw(isCurrent ? ColorF{ 0.55, 0.75, 0.95 } : (hovered ? ColorF{ 0.82 } : ColorF{ 0.72 }))
				.drawFrame(1, 0, isCurrent ? ColorF{ 0.2, 0.4, 0.7 } : ColorF{ 0.35 });
			SimpleGUI::GetFont()(label).draw((clipButton.x + 8), (clipButton.y + 4), ColorF{ 0.15 });

			if (hovered && MouseL.down())
			{
				model.setClipIndex(i);
			}
		}
	}

	void DrawModelHeightEditor(ModelHeightSettings& modelHeightSettings,
		String& modelHeightMessage,
		double& modelHeightMessageUntil,
		const Rect& panelRect,
		const Vec3& birdRenderPosition,
		const Vec3& ashigaruRenderPosition)
	{
		panelRect.draw(ColorF{ 1.0, 0.92 });
		panelRect.drawFrame(2, 0, ColorF{ 0.25 });
		SimpleGUI::GetFont()(U"Model Height Editor").draw((panelRect.x + 16), (panelRect.y + 12), ColorF{ 0.12 });
		SimpleGUI::Slider(U"bird offset Y: {:.3f}"_fmt(modelHeightSettings.birdOffsetY), modelHeightSettings.birdOffsetY, ModelHeightOffsetMin, ModelHeightOffsetMax, Vec2{ panelRect.x + 16.0, panelRect.y + 44.0 }, 180, 360);
		SimpleGUI::Slider(U"ashigaru offset Y: {:.3f}"_fmt(modelHeightSettings.ashigaruOffsetY), modelHeightSettings.ashigaruOffsetY, ModelHeightOffsetMin, ModelHeightOffsetMax, Vec2{ panelRect.x + 16.0, panelRect.y + 84.0 }, 180, 360);

		if (DrawTextButton(Rect{ panelRect.x + 16, panelRect.y + 124, 88, 30 }, U"bird -0.1"))
		{
			modelHeightSettings.birdOffsetY = Max(ModelHeightOffsetMin, (modelHeightSettings.birdOffsetY - 0.1));
		}

		if (DrawTextButton(Rect{ panelRect.x + 112, panelRect.y + 124, 88, 30 }, U"bird +0.1"))
		{
			modelHeightSettings.birdOffsetY = Min(ModelHeightOffsetMax, (modelHeightSettings.birdOffsetY + 0.1));
		}

		if (DrawTextButton(Rect{ panelRect.x + 208, panelRect.y + 124, 88, 30 }, U"bird -0.01"))
		{
			modelHeightSettings.birdOffsetY = Max(ModelHeightOffsetMin, (modelHeightSettings.birdOffsetY - 0.01));
		}

		if (DrawTextButton(Rect{ panelRect.x + 304, panelRect.y + 124, 88, 30 }, U"bird +0.01"))
		{
			modelHeightSettings.birdOffsetY = Min(ModelHeightOffsetMax, (modelHeightSettings.birdOffsetY + 0.01));
		}

		if (DrawTextButton(Rect{ panelRect.x + 16, panelRect.y + 160, 88, 30 }, U"ashi -0.1"))
		{
			modelHeightSettings.ashigaruOffsetY = Max(ModelHeightOffsetMin, (modelHeightSettings.ashigaruOffsetY - 0.1));
		}

		if (DrawTextButton(Rect{ panelRect.x + 112, panelRect.y + 160, 88, 30 }, U"ashi +0.1"))
		{
			modelHeightSettings.ashigaruOffsetY = Min(ModelHeightOffsetMax, (modelHeightSettings.ashigaruOffsetY + 0.1));
		}

		if (DrawTextButton(Rect{ panelRect.x + 208, panelRect.y + 160, 88, 30 }, U"ashi -0.01"))
		{
			modelHeightSettings.ashigaruOffsetY = Max(ModelHeightOffsetMin, (modelHeightSettings.ashigaruOffsetY - 0.01));
		}

		if (DrawTextButton(Rect{ panelRect.x + 304, panelRect.y + 160, 88, 30 }, U"ashi +0.01"))
		{
			modelHeightSettings.ashigaruOffsetY = Min(ModelHeightOffsetMax, (modelHeightSettings.ashigaruOffsetY + 0.01));
		}

		if (DrawTextButton(Rect{ panelRect.x + 16, panelRect.y + 200, 120, 30 }, U"Save"))
		{
			modelHeightMessage = SaveModelHeightSettings(modelHeightSettings)
				? U"Saved: {}"_fmt(ModelHeightSettingsPath)
				: U"Save failed";
			modelHeightMessageUntil = (Scene::Time() + 2.0);
		}

		if (DrawTextButton(Rect{ panelRect.x + 144, panelRect.y + 200, 120, 30 }, U"Reload"))
		{
			modelHeightSettings = LoadModelHeightSettings();
			modelHeightMessage = U"Reloaded";
			modelHeightMessageUntil = (Scene::Time() + 2.0);
		}

		if (DrawTextButton(Rect{ panelRect.x + 272, panelRect.y + 200, 120, 30 }, U"Reset"))
		{
			modelHeightSettings = {};
			modelHeightMessage = U"Offsets reset";
			modelHeightMessageUntil = (Scene::Time() + 2.0);
		}

		if (Scene::Time() < modelHeightMessageUntil)
		{
			SimpleGUI::GetFont()(modelHeightMessage).draw((panelRect.x + 16), (panelRect.y + 238), ColorF{ 0.12 });
		}

		SimpleGUI::GetFont()(U"bird world Y: {:.3f}"_fmt(birdRenderPosition.y)).draw((panelRect.x + 16), (panelRect.y + 262), ColorF{ 0.12 });
		SimpleGUI::GetFont()(U"ashigaru world Y: {:.3f}"_fmt(ashigaruRenderPosition.y)).draw((panelRect.x + 208), (panelRect.y + 262), ColorF{ 0.12 });
	}
}
