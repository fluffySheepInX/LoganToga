# include "SkyAppUiSettingsInternal.hpp"
# include "MainScene.hpp"
# include "MainUi.hpp"

namespace SkyAppSupport::UiSettingsDetail
{
	Array<String> WrapTooltipText(const StringView text, const double maxWidth)
	{
		const Font& font = SimpleGUI::GetFont();
		Array<String> lines;
		String current;

		for (const char32 ch : text)
		{
			if (ch == U'\n')
			{
				lines << current;
				current.clear();
				continue;
			}

			const String candidate = (current + ch);
			if ((not current.isEmpty()) && (font(candidate).region().w > maxWidth))
			{
				lines << current;
				current = String{ ch };
			}
			else
			{
				current = candidate;
			}
		}

		if (not current.isEmpty())
		{
			lines << current;
		}

		if (lines.isEmpty())
		{
			lines << U"";
		}

		return lines;
	}

	double GetSliderMin(const double value, const double defaultMin)
	{
		return Min(defaultMin, (value - 1.0));
	}

	double GetSliderMax(const double value, const double defaultMax)
	{
		return Max(defaultMax, (value + 1.0));
	}

	void DrawSettingsPanelFrame(const Rect& panelRect, const StringView title)
	{
		UiInternal::DrawNinePatchPanelFrame(panelRect, title, ColorF{ 1.0, 0.92 });
	}

	void DrawCameraSettingsPanelFrame(const Rect& panelRect)
	{
		DrawSettingsPanelFrame(panelRect, U"Camera Settings");
	}

	void DrawSkySettingsPanelFrame(const Rect& panelRect, const StringView title)
	{
		DrawSettingsPanelFrame(panelRect, title);
	}

	void DrawTerrainVisualSettingsPanelFrame(const Rect& panelRect)
	{
		DrawSettingsPanelFrame(panelRect, U"Terrain Surface");
	}

	void DrawHoverTooltip(const RectF& anchorRect, const StringView title, const StringView description)
	{
		const int32 tooltipWidth = 332;
		const int32 tooltipGap = 12;
		const Array<String> tooltipLines = WrapTooltipText(description, (tooltipWidth - 24));
		const int32 tooltipHeight = Max(88, (44 + static_cast<int32>(tooltipLines.size()) * 20));
		int32 tooltipX = (static_cast<int32>(anchorRect.x) - tooltipWidth - tooltipGap);
		if (tooltipX < 8)
		{
			tooltipX = (static_cast<int32>(anchorRect.rightX()) + tooltipGap);
		}
		const int32 tooltipY = Clamp((static_cast<int32>(anchorRect.y + anchorRect.h * 0.5) - tooltipHeight / 2), 8, (Scene::Height() - tooltipHeight - 8));
		const Rect tooltipRect{ tooltipX, tooltipY, tooltipWidth, tooltipHeight };
		tooltipRect.draw(ColorF{ 0.97, 0.98, 1.0, 0.97 })
			.drawFrame(2, 0, ColorF{ 0.70, 0.78, 0.88, 0.95 });
		SimpleGUI::GetFont()(title).draw((tooltipRect.x + 12), (tooltipRect.y + 8), UiInternal::EditorTextOnCardSecondaryColor());
		for (size_t i = 0; i < tooltipLines.size(); ++i)
		{
			SimpleGUI::GetFont()(tooltipLines[i]).draw((tooltipRect.x + 12), (tooltipRect.y + 32 + static_cast<int32>(i) * 20), UiInternal::EditorTextOnCardPrimaryColor());
		}
	}

	bool DrawTerrainPageButton(const Rect& rect, const StringView label, const bool selected)
	{
		const bool hovered = rect.mouseOver();
		const ColorF fillColor = selected
			? (hovered ? ColorF{ 0.52, 0.75, 0.95 } : ColorF{ 0.43, 0.67, 0.90 })
			: (hovered ? ColorF{ 0.84 } : ColorF{ 0.74 });
		rect.rounded(6).draw(fillColor).drawFrame(1, 0, ColorF{ 0.3 });
		SimpleGUI::GetFont()(label).drawAt(rect.center(), selected ? UiInternal::EditorTextOnSelectedPrimaryColor() : UiInternal::EditorTextOnCardPrimaryColor());
		return hovered && MouseL.down();
	}

	bool DrawEditorSlider(const int32 sliderId,
		const StringView label,
		double& value,
		const double minValue,
		const double maxValue,
		const Vec2& pos,
		const double labelWidth,
		const double trackWidth)
	{
		static Optional<int32> activeSliderId;

		const double clampedMin = Min(minValue, maxValue);
		const double clampedMax = Max(minValue, maxValue);
		value = Clamp(value, clampedMin, clampedMax);

		const RectF labelRect{ pos.x, pos.y, labelWidth, EditorSliderRowHeight };
		const RectF trackRect{ (pos.x + labelWidth + 12.0), (pos.y + 12.0), trackWidth, EditorSliderTrackHeight };
		const double ratio = Math::Saturate((value - clampedMin) / Max(0.0001, (clampedMax - clampedMin)));
		const RectF knobRect{ Arg::center = Vec2{ (trackRect.x + trackRect.w * ratio), trackRect.centerY() }, EditorSliderKnobWidth, EditorSliderKnobHeight };
		const bool hovered = trackRect.stretched(0, 10).mouseOver() || knobRect.mouseOver();

		if (MouseL.down() && hovered)
		{
			activeSliderId = sliderId;
		}

		bool changed = false;
		if (activeSliderId && (*activeSliderId == sliderId))
		{
			if (MouseL.pressed())
			{
				const double cursorRatio = Math::Saturate((Cursor::PosF().x - trackRect.x) / Max(1.0, trackRect.w));
				const double updatedValue = Clamp((clampedMin + (clampedMax - clampedMin) * cursorRatio), clampedMin, clampedMax);

				if (updatedValue != value)
				{
					value = updatedValue;
					changed = true;
				}
			}
			else
			{
				activeSliderId.reset();
			}
		}

		SimpleGUI::GetFont()(label).draw(labelRect.pos.movedBy(0, 2), UiInternal::EditorTextOnLightPrimaryColor());
		trackRect.rounded(4).draw(ColorF{ 0.12, 0.14, 0.17, 0.92 });
		RectF{ trackRect.pos, (trackRect.w * ratio), trackRect.h }.rounded(4).draw(ColorF{ 0.38, 0.70, 0.96, 0.95 });
		trackRect.rounded(4).drawFrame(1.0, ColorF{ 0.82, 0.88, 0.95, 0.65 });
		knobRect.rounded(4).draw((activeSliderId && (*activeSliderId == sliderId)) ? ColorF{ 0.96, 0.98, 1.0 } : ColorF{ 0.90, 0.94, 0.98 })
			.drawFrame(1.0, 0.0, ColorF{ 0.25, 0.34, 0.50, 0.95 });
		return changed;
	}

	void DrawEditorCheckBox(bool& checked,
		const StringView label,
		const Vec2& pos,
		const double width,
		const bool enabled)
	{
		const RectF rowRect{ pos.x, pos.y, width, EditorCheckBoxRowHeight };
		const RectF boxRect{ pos.x, (pos.y + 2.0), EditorCheckBoxSize, EditorCheckBoxSize };
		const bool hovered = enabled && rowRect.mouseOver();

		if (hovered && MouseL.down())
		{
			checked = not checked;
		}

		boxRect.rounded(4).draw(hovered ? ColorF{ 0.90, 0.93, 0.98 } : ColorF{ 0.82, 0.86, 0.92 })
			.drawFrame(1.0, 0.0, ColorF{ 0.32, 0.38, 0.46 });

		if (checked)
		{
			boxRect.stretched(-4).rounded(3).draw(ColorF{ 0.33, 0.53, 0.82 });
			SimpleGUI::GetFont()(U"✓").drawAt(boxRect.center(), ColorF{ 0.98 });
		}

		SimpleGUI::GetFont()(label).draw(Vec2{ (pos.x + EditorCheckBoxSize + 8.0), pos.y + 1.0 }, enabled ? UiInternal::EditorTextOnLightPrimaryColor() : UiInternal::EditorTextOnLightSecondaryColor());
	}
}
