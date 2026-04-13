# pragma once
# include "SkyAppUiEditorTextColorsInternal.hpp"
# include "SkyAppUi.hpp"

namespace SkyAppSupport
{
	namespace UiInternal
	{
		[[nodiscard]] inline const Optional<NinePatch>& GetDefaultPanelNinePatch()
		{
			static bool initialized = false;
			static Optional<NinePatch> ninePatch;

			if (not initialized)
			{
				initialized = true;

				if (FileSystem::Exists(DefaultPanelNinePatchPath))
				{
					ninePatch = NinePatch{ Texture{ DefaultPanelNinePatchPath }, DefaultPanelNinePatchPatchSize };
				}
			}

			return ninePatch;
		}

		inline void DrawNinePatchPanelFrame(const Rect& panelRect,
			StringView title = U"",
			const ColorF& backgroundColor = DefaultPanelBackgroundColor,
			const ColorF& frameColor = DefaultPanelFrameColor,
			const ColorF& titleColor = DefaultPanelTitleColor)
		{
			if (const auto& ninePatch = GetDefaultPanelNinePatch())
			{
				const ColorF resolvedTitleColor = ResolvePanelTitleColor(titleColor);
				ninePatch->draw(RectF{ panelRect });
				panelRect.drawFrame(2, 0, frameColor);

				if (not title.isEmpty())
				{
					SimpleGUI::GetFont()(title).draw((panelRect.x + 16), (panelRect.y + 12), resolvedTitleColor);
				}

				return;
			}

			const ColorF resolvedTitleColor = ResolvePanelTitleColor(titleColor);
			panelRect.draw(backgroundColor);
			panelRect.drawFrame(2, 0, frameColor);

			if (not title.isEmpty())
			{
				SimpleGUI::GetFont()(title).draw((panelRect.x + 16), (panelRect.y + 12), resolvedTitleColor);
			}
		}

		inline void DrawPanelFrame(const Rect& panelRect,
			StringView title = U"",
			const ColorF& backgroundColor = DefaultPanelBackgroundColor,
			const ColorF& frameColor = DefaultPanelFrameColor,
			const ColorF& titleColor = DefaultPanelTitleColor)
		{
			const ColorF resolvedTitleColor = ResolvePanelTitleColor(titleColor);
			panelRect.draw(backgroundColor);
			panelRect.drawFrame(2, 0, frameColor);

			if (not title.isEmpty())
			{
				SimpleGUI::GetFont()(title).draw((panelRect.x + 16), (panelRect.y + 12), resolvedTitleColor);
			}
		}

		[[nodiscard]] inline bool DrawAccordionHeader(const Rect& panelRect,
			StringView title,
			const bool isExpanded,
			const ColorF& backgroundColor = DefaultPanelBackgroundColor,
			const ColorF& frameColor = DefaultPanelFrameColor,
			const ColorF& titleColor = DefaultPanelTitleColor)
		{
			const ColorF resolvedTitleColor = ResolvePanelTitleColor(titleColor);
			const Rect headerRect = SkyAppUiLayout::AccordionHeaderRect(panelRect);
			const bool hovered = headerRect.mouseOver();
			headerRect.draw(hovered ? backgroundColor.lerp(ColorF{ 0.92 }, 0.25) : backgroundColor);
			headerRect.drawFrame(2, 0, frameColor);
			SimpleGUI::GetFont()(isExpanded ? U"▼" : U"▶").draw((headerRect.x + 12), (headerRect.y + 7), resolvedTitleColor);
			SimpleGUI::GetFont()(title).draw((headerRect.x + 34), (headerRect.y + 7), resolvedTitleColor);
			return hovered && MouseL.down();
		}
	}
}
