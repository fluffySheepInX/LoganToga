# include "MainUi.hpp"
# include "MainSettings.hpp"

namespace MainSupport
{
 namespace
	{
		[[nodiscard]] StringView ToModelHeightTargetLabel(const ModelHeightTarget target)
		{
			switch (target)
			{
			case ModelHeightTarget::Ashigaru:
				return U"ashigaru";

			case ModelHeightTarget::SugoiCar:
				return U"sugoiCar";

			case ModelHeightTarget::Bird:
			default:
				return U"bird";
			}
		}

		[[nodiscard]] double& GetModelHeightOffset(ModelHeightSettings& modelHeightSettings, const ModelHeightTarget target)
		{
			switch (target)
			{
			case ModelHeightTarget::Ashigaru:
				return modelHeightSettings.ashigaruOffsetY;

			case ModelHeightTarget::SugoiCar:
				return modelHeightSettings.sugoiCarOffsetY;

			case ModelHeightTarget::Bird:
			default:
				return modelHeightSettings.birdOffsetY;
			}
		}

		[[nodiscard]] double GetModelHeightWorldY(const ModelHeightTarget target,
			const Vec3& birdRenderPosition,
			const Vec3& ashigaruRenderPosition,
			const Vec3& sugoiCarRenderPosition)
		{
			switch (target)
			{
			case ModelHeightTarget::Ashigaru:
				return ashigaruRenderPosition.y;

			case ModelHeightTarget::SugoiCar:
				return sugoiCarRenderPosition.y;

			case ModelHeightTarget::Bird:
			default:
				return birdRenderPosition.y;
			}
		}
	}

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
     ModelHeightTarget& activeTarget,
		String& modelHeightMessage,
		double& modelHeightMessageUntil,
		const Rect& panelRect,
		const Vec3& birdRenderPosition,
     const Vec3& ashigaruRenderPosition,
		const Vec3& sugoiCarRenderPosition)
	{
        const Rect listPanel{ panelRect.x, panelRect.y, 156, panelRect.h };
		const Rect detailPanel{ (panelRect.x + 164), panelRect.y, (panelRect.w - 164), panelRect.h };
		const Array<ModelHeightTarget> targets{
			ModelHeightTarget::Bird,
			ModelHeightTarget::Ashigaru,
			ModelHeightTarget::SugoiCar,
		};
		double& activeOffset = GetModelHeightOffset(modelHeightSettings, activeTarget);
		activeOffset = Clamp(activeOffset, ModelHeightOffsetMin, ModelHeightOffsetMax);

		panelRect.draw(ColorF{ 1.0, 0.92 });
		panelRect.drawFrame(2, 0, ColorF{ 0.25 });
		SimpleGUI::GetFont()(U"Model Height Editor").draw((panelRect.x + 16), (panelRect.y + 12), ColorF{ 0.12 });
        Rect{ (listPanel.rightX() + 3), (panelRect.y + 8), 1, (panelRect.h - 16) }.draw(ColorF{ 0.72, 0.72, 0.74 });
		SimpleGUI::GetFont()(U"Targets").draw((listPanel.x + 16), (listPanel.y + 38), ColorF{ 0.18 });

		for (size_t i = 0; i < targets.size(); ++i)
		{
			const ModelHeightTarget target = targets[i];
			const Rect buttonRect{ (listPanel.x + 12), (listPanel.y + 64 + static_cast<int32>(i) * 58), 132, 48 };
			const bool selected = (activeTarget == target);
			const bool hovered = buttonRect.mouseOver();
			buttonRect.draw(selected ? ColorF{ 0.33, 0.53, 0.82 } : (hovered ? ColorF{ 0.94, 0.95, 0.98 } : ColorF{ 0.98, 0.97, 0.95 }))
				.drawFrame(1, 0, selected ? ColorF{ 0.20, 0.32, 0.52 } : ColorF{ 0.58, 0.56, 0.52 });
			SimpleGUI::GetFont()(ToModelHeightTargetLabel(target)).draw((buttonRect.x + 10), (buttonRect.y + 6), selected ? ColorF{ 0.98 } : ColorF{ 0.14 });
			SimpleGUI::GetFont()(U"Y {:.3f}"_fmt(GetModelHeightOffset(modelHeightSettings, target))).draw((buttonRect.x + 10), (buttonRect.y + 26), selected ? ColorF{ 0.96 } : ColorF{ 0.28 });

			if (hovered && MouseL.down())
			{
				activeTarget = target;
			}
		}

		SimpleGUI::GetFont()(U"Target: {}"_fmt(ToModelHeightTargetLabel(activeTarget))).draw((detailPanel.x + 16), (detailPanel.y + 38), ColorF{ 0.14 });
		SimpleGUI::Slider(U"offset Y: {:.3f}"_fmt(activeOffset), activeOffset, ModelHeightOffsetMin, ModelHeightOffsetMax, Vec2{ detailPanel.x + 16.0, detailPanel.y + 70.0 }, 180, 260);

      if (DrawTextButton(Rect{ detailPanel.x + 16, detailPanel.y + 112, 56, 28 }, U"-1.0"))
		{
           activeOffset = Max(ModelHeightOffsetMin, (activeOffset - 1.0));
		}

     if (DrawTextButton(Rect{ detailPanel.x + 80, detailPanel.y + 112, 56, 28 }, U"-0.1"))
		{
           activeOffset = Max(ModelHeightOffsetMin, (activeOffset - 0.1));
		}

        if (DrawTextButton(Rect{ detailPanel.x + 144, detailPanel.y + 112, 56, 28 }, U"-0.01"))
		{
          activeOffset = Max(ModelHeightOffsetMin, (activeOffset - 0.01));
		}

        if (DrawTextButton(Rect{ detailPanel.x + 208, detailPanel.y + 112, 56, 28 }, U"+0.01"))
		{
          activeOffset = Min(ModelHeightOffsetMax, (activeOffset + 0.01));
		}

      if (DrawTextButton(Rect{ detailPanel.x + 272, detailPanel.y + 112, 56, 28 }, U"+0.1"))
		{
           activeOffset = Min(ModelHeightOffsetMax, (activeOffset + 0.1));
		}

     if (DrawTextButton(Rect{ detailPanel.x + 336, detailPanel.y + 112, 56, 28 }, U"+1.0"))
		{
           activeOffset = Min(ModelHeightOffsetMax, (activeOffset + 1.0));
		}

        if (DrawTextButton(Rect{ detailPanel.x + 16, detailPanel.y + 152, 118, 30 }, U"Reset Target"))
		{
          activeOffset = 0.0;
		}

        SimpleGUI::GetFont()(U"world Y: {:.3f}"_fmt(GetModelHeightWorldY(activeTarget, birdRenderPosition, ashigaruRenderPosition, sugoiCarRenderPosition))).draw((detailPanel.x + 16), (detailPanel.y + 196), ColorF{ 0.12 });
		SimpleGUI::GetFont()(U"range: [{:.1f}, {:.1f}]"_fmt(ModelHeightOffsetMin, ModelHeightOffsetMax)).draw((detailPanel.x + 16), (detailPanel.y + 220), ColorF{ 0.12 });

      if (DrawTextButton(Rect{ detailPanel.x + 16, detailPanel.y + 252, 92, 30 }, U"Save"))
		{
			modelHeightMessage = SaveModelHeightSettings(modelHeightSettings)
				? U"Saved: {}"_fmt(ModelHeightSettingsPath)
				: U"Save failed";
			modelHeightMessageUntil = (Scene::Time() + 2.0);
		}

       if (DrawTextButton(Rect{ detailPanel.x + 116, detailPanel.y + 252, 92, 30 }, U"Reload"))
		{
			modelHeightSettings = LoadModelHeightSettings();
			modelHeightMessage = U"Reloaded";
			modelHeightMessageUntil = (Scene::Time() + 2.0);
		}

        if (DrawTextButton(Rect{ detailPanel.x + 216, detailPanel.y + 252, 92, 30 }, U"Reset All"))
		{
			modelHeightSettings = {};
			modelHeightMessage = U"Offsets reset";
			modelHeightMessageUntil = (Scene::Time() + 2.0);
		}

		if (Scene::Time() < modelHeightMessageUntil)
		{
         SimpleGUI::GetFont()(modelHeightMessage).draw((detailPanel.x + 16), (detailPanel.y + 290), ColorF{ 0.12 });
		}
	}
}
