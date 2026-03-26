# include "UI.h"
# include "GameConstants.h"

namespace ff
{
 namespace
	{
        struct SummonAllyButton
		{
			RectF rect;
			String label;
			AllyBehavior behavior;
			ColorF fillColor;
		};

		Array<SummonAllyButton> BuildSummonAllyButtons()
		{
			const Vec2 center = Scene::Rect().bottomCenter().movedBy(0, -34);
			const SizeF buttonSize{ 220, 44 };
			const double spacing = 18.0;
			const double totalWidth = ((buttonSize.x * 3.0) + (spacing * 2.0));
			const double left = (center.x - (totalWidth * 0.5));

			return{
				{ RectF{ left, center.y - (buttonSize.y * 0.5), buttonSize }, U"追跡召喚", AllyBehavior::ChaseEnemies, ColorF{ 0.58, 0.26, 0.24, 0.92 } },
				{ RectF{ (left + buttonSize.x + spacing), center.y - (buttonSize.y * 0.5), buttonSize }, U"護衛召喚", AllyBehavior::GuardPlayer, ColorF{ 0.18, 0.40, 0.24, 0.92 } },
				{ RectF{ (left + ((buttonSize.x + spacing) * 2.0)), center.y - (buttonSize.y * 0.5), buttonSize }, U"衛星召喚", AllyBehavior::OrbitPlayer, ColorF{ 0.22, 0.34, 0.62, 0.92 } },
			};
		}
	}

    Optional<AllyBehavior> CheckSummonAllyButtonPressed()
	{
        for (const auto& button : BuildSummonAllyButtons())
		{
			if (button.rect.mouseOver() && MouseL.down())
			{
				return button.behavior;
			}
		}

		return none;
	}

   void DrawSummonAllyButtons(const Font& font, const int32 resourceCount)
	{
     for (const auto& button : BuildSummonAllyButtons())
		{
           const int32 summonCost = GetSummonCost(button.behavior);
			const bool affordable = (resourceCount >= summonCost);
            const bool hovered = button.rect.mouseOver();

            if (hovered && affordable)
			{
				Cursor::RequestStyle(CursorStyle::Hand);
			}

            const ColorF baseColor = affordable ? button.fillColor : ColorF{ 0.36, 0.36, 0.38, 0.78 };
			button.rect.rounded(12).draw((hovered && affordable) ? baseColor.lerp(ColorF{ 0.92, 0.96, 1.0, 0.96 }, 0.18) : baseColor);
			button.rect.rounded(12).drawFrame(2, ColorF{ 0.86, 0.95, 0.88, 0.9 });
            font(button.label).drawAt(20, button.rect.center().movedBy(0, -8), affordable ? ColorF{ 1.0, 1.0, 1.0 } : ColorF{ 0.86, 0.86, 0.88 });
			font(U"Cost {}"_fmt(summonCost)).drawAt(16, button.rect.center().movedBy(0, 10), affordable ? ColorF{ 0.96, 0.98, 0.96, 0.92 } : ColorF{ 0.80, 0.80, 0.84, 0.9 });
		}
	}
}
