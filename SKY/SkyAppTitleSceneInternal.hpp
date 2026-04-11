# pragma once
# include "SkyAppInternal.hpp"

namespace SkyAppInternal::TitleSceneDetail
{
	enum class CampaignActionIcon
	{
		Edit,
		Play,
		Continue,
		Delete,
	};

	inline constexpr bool ShowDebugCampaignButtons =
	#if _DEBUG
		true;
	#else
		false;
	#endif

	inline void DrawCampaignActionButton(const RectF& rect, const CampaignActionIcon icon, const bool enabled)
	{
		const bool hovered = enabled && rect.mouseOver();
		const ColorF fillColor = enabled
			? (hovered ? ColorF{ 0.22, 0.36, 0.56 } : ColorF{ 0.16, 0.25, 0.40 })
			: ColorF{ 0.14, 0.16, 0.20, 0.72 };
		const ColorF frameColor = enabled ? ColorF{ 0.78, 0.88, 1.0, 0.88 } : ColorF{ 0.46, 0.52, 0.60, 0.72 };
		const ColorF iconColor = enabled ? ColorF{ 0.98, 0.99, 1.0, 0.98 } : ColorF{ 0.72, 0.76, 0.82, 1.0 };

		rect.rounded(10).draw(fillColor);
		rect.rounded(10).drawFrame(2, 0, frameColor);

		switch (icon)
		{
		case CampaignActionIcon::Edit:
			Line{ rect.x + 12, rect.y + 28, rect.x + 27, rect.y + 13 }.draw(3.0, iconColor);
			Line{ rect.x + 24, rect.y + 10, rect.x + 30, rect.y + 16 }.draw(3.0, iconColor);
			Line{ rect.x + 11, rect.y + 30, rect.x + 16, rect.y + 25 }.draw(3.0, iconColor);
			break;

		case CampaignActionIcon::Play:
			Triangle{ Vec2{ rect.x + 14, rect.y + 10 }, Vec2{ rect.x + 14, rect.y + 30 }, Vec2{ rect.x + 30, rect.y + 20 } }.draw(iconColor);
			break;

		case CampaignActionIcon::Continue:
			Triangle{ Vec2{ rect.x + 10, rect.y + 10 }, Vec2{ rect.x + 10, rect.y + 30 }, Vec2{ rect.x + 21, rect.y + 20 } }.draw(iconColor);
			Triangle{ Vec2{ rect.x + 19, rect.y + 10 }, Vec2{ rect.x + 19, rect.y + 30 }, Vec2{ rect.x + 30, rect.y + 20 } }.draw(iconColor);
			break;

		case CampaignActionIcon::Delete:
			Line{ rect.x + 11, rect.y + 11, rect.x + 29, rect.y + 29 }.draw(3.0, iconColor);
			Line{ rect.x + 29, rect.y + 11, rect.x + 11, rect.y + 29 }.draw(3.0, iconColor);
			break;
		}
	}

	inline void DrawCampaignActionTooltip(const RectF& anchorRect, const StringView label)
	{
		const RectF tooltipRect{ (anchorRect.center().x - 64), (anchorRect.y - 34), 128, 26 };
		tooltipRect.rounded(8).draw(ColorF{ 0.08, 0.10, 0.12, 0.96 });
		tooltipRect.rounded(8).drawFrame(1, 0, ColorF{ 0.72, 0.82, 0.94, 0.84 });
		SimpleGUI::GetFont()(label).drawAt(tooltipRect.center(), Palette::White);
	}

	inline void DrawPlayAttentionEffect(const RectF& rect)
	{
		const double pulse = Periodic::Sine0_1(1.2s);
		rect.stretched(4 + pulse * 6).rounded(12).drawFrame((2.0 + pulse * 1.5), ColorF{ 1.0, 0.92, 0.42, (0.38 + pulse * 0.34) });

		for (int32 i = 0; i < 3; ++i)
		{
			const double offset = (pulse * 14.0 + i * 10.0);
			const Vec2 center{ (rect.x - 18 - offset), rect.centerY() };
			Triangle{ center.movedBy(-5, -6), center.movedBy(-5, 6), center.movedBy(4, 0) }.draw(ColorF{ 1.0, 0.92, 0.42, Max(0.18, (0.58 - i * 0.14)) });
		}
	}

   [[nodiscard]] inline String FormatCampaignTitle(const SkyCampaign::CampaignDefinition& campaign, const size_t clearCount)
	{
		if (clearCount <= 0)
		{
			return campaign.displayName;
		}

		return (clearCount == 1)
			? U"{} ★"_fmt(campaign.displayName)
			: U"{} ★*{}"_fmt(campaign.displayName, clearCount);
	}

	inline void DrawCampaignRow(const RectF& rect, const Font& titleFont, const Font& infoFont, const SkyCampaign::CampaignDefinition& campaign, const size_t clearCount, const bool selected)
	{
		const bool hovered = rect.mouseOver();
		rect.rounded(12).draw(selected
			? ColorF{ 0.20, 0.30, 0.44, 0.96 }
			: (hovered ? ColorF{ 0.14, 0.20, 0.30, 0.92 } : ColorF{ 0.10, 0.14, 0.22, 0.88 }));
		rect.rounded(12).drawFrame(2, 0, selected ? ColorF{ 0.92, 0.96, 1.0, 0.92 } : ColorF{ 0.42, 0.52, 0.64, 0.82 });
     titleFont(FormatCampaignTitle(campaign, clearCount)).draw(rect.pos.movedBy(14, 10), Palette::White);
		infoFont(U"Missions: {}"_fmt(campaign.missions.size())).draw(rect.pos.movedBy(14, 38), ColorF{ 0.82, 0.89, 0.98, 0.92 });
	}
}
