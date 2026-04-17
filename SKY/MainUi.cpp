# include "MainUi.hpp"
# include "SkyAppUiInternal.hpp"

namespace MainSupport
{

	bool DrawTextButton(const Rect& rect, StringView label)
	{
		static const Font buttonFont{ 18, Typeface::Bold };
		const bool hovered = rect.mouseOver();
		rect.draw(hovered ? ColorF{ 0.82 } : ColorF{ 0.72 })
			.drawFrame(1, 0, ColorF{ 0.35 });
        buttonFont(label).drawAt(rect.center(), SkyAppSupport::UiInternal::EditorTextOnCardPrimaryColor());
		return hovered && MouseL.down();
	}

	bool DrawCheckBox(const Rect& rect, bool& checked, StringView label, const bool enabled)
	{
		const bool hovered = enabled && rect.mouseOver();
		const RectF rowRect{ rect };
		const RectF boxRect{ (rect.x + 8), (rect.y + (rect.h - 22) / 2.0), 22, 22 };
		const Vec2 labelPos{ (boxRect.rightX() + 8.0), (rect.y + (rect.h - 22) / 2.0 - 1.0) };

		if (hovered && MouseL.down())
		{
			checked = not checked;
		}

		rowRect.rounded(6).draw(hovered ? ColorF{ 0.90, 0.93, 0.98, 0.36 } : ColorF{ 0.78, 0.84, 0.92, 0.18 })
			.drawFrame(1.0, 0.0, hovered ? ColorF{ 0.44, 0.56, 0.74, 0.78 } : ColorF{ 0.42, 0.48, 0.56, 0.56 });
		boxRect.rounded(4).draw(hovered ? ColorF{ 0.90, 0.93, 0.98 } : ColorF{ 0.82, 0.86, 0.92 })
			.drawFrame(1.0, 0.0, ColorF{ 0.32, 0.38, 0.46 });

		if (checked)
		{
			boxRect.stretched(-4).rounded(3).draw(ColorF{ 0.33, 0.53, 0.82 });
			SimpleGUI::GetFont()(U"✓").drawAt(boxRect.center(), ColorF{ 0.98 });
		}

		SimpleGUI::GetFont()(label).draw(labelPos, enabled ? SkyAppSupport::UiInternal::EditorTextOnLightPrimaryColor() : SkyAppSupport::UiInternal::EditorTextOnLightSecondaryColor());
		return hovered && MouseL.down();
	}

 void DrawAnimationClipSelector(UnitModel& model, StringView title, const int32 x, const int32 y, const int32 width)
	{
		if (not model.hasAnimations())
		{
			return;
		}

		const auto& clips = model.animations();
       SimpleGUI::GetFont()(U"{} ({})"_fmt(title, clips.size())).draw(x, y, SkyAppSupport::UiInternal::EditorTextOnPanelPrimaryColor());

		for (size_t i = 0; i < clips.size(); ++i)
		{
			const int32 buttonY = (y + 28 + static_cast<int32>(i) * 34);
			const bool isCurrent = (i == model.currentClipIndex());
			const Rect clipButton{ x, buttonY, width, 30 };
			const String label = U"[{}] {}"_fmt(i, clips[i].name);
			const bool hovered = clipButton.mouseOver();
			clipButton.draw(isCurrent ? ColorF{ 0.55, 0.75, 0.95 } : (hovered ? ColorF{ 0.82 } : ColorF{ 0.72 }))
				.drawFrame(1, 0, isCurrent ? ColorF{ 0.2, 0.4, 0.7 } : ColorF{ 0.35 });
           SimpleGUI::GetFont()(label).draw((clipButton.x + 8), (clipButton.y + 4), isCurrent ? SkyAppSupport::UiInternal::EditorTextOnSelectedPrimaryColor() : SkyAppSupport::UiInternal::EditorTextOnCardPrimaryColor());

			if (hovered && MouseL.down())
			{
				model.setClipIndex(i);
			}
		}
	}
}
