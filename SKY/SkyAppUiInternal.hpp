# pragma once
# include "SkyAppUi.hpp"

namespace SkyAppSupport
{
	namespace UiInternal
	{
		inline constexpr ColorF DefaultPanelBackgroundColor{ 0.98, 0.95 };
		inline constexpr ColorF DefaultPanelFrameColor{ 0.25 };
		inline constexpr ColorF DefaultPanelTitleColor{ 0.12 };

		inline void DrawPanelFrame(const Rect& panelRect,
			StringView title = U"",
			const ColorF& backgroundColor = DefaultPanelBackgroundColor,
			const ColorF& frameColor = DefaultPanelFrameColor,
			const ColorF& titleColor = DefaultPanelTitleColor)
		{
			panelRect.draw(backgroundColor);
			panelRect.drawFrame(2, 0, frameColor);

			if (not title.isEmpty())
			{
				SimpleGUI::GetFont()(title).draw((panelRect.x + 16), (panelRect.y + 12), titleColor);
			}
		}

		[[nodiscard]] inline bool DrawAccordionHeader(const Rect& panelRect,
			StringView title,
			const bool isExpanded,
			const ColorF& backgroundColor = DefaultPanelBackgroundColor,
			const ColorF& frameColor = DefaultPanelFrameColor,
			const ColorF& titleColor = DefaultPanelTitleColor)
		{
			const Rect headerRect = SkyAppUiLayout::AccordionHeaderRect(panelRect);
			const bool hovered = headerRect.mouseOver();
			headerRect.draw(hovered ? backgroundColor.lerp(ColorF{ 0.92 }, 0.25) : backgroundColor);
			headerRect.drawFrame(2, 0, frameColor);
			SimpleGUI::GetFont()(isExpanded ? U"▼" : U"▶").draw((headerRect.x + 12), (headerRect.y + 7), titleColor);
			SimpleGUI::GetFont()(title).draw((headerRect.x + 34), (headerRect.y + 7), titleColor);
			return hovered && MouseL.down();
		}
	}
}
