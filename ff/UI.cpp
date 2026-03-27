# include "UI.h"
# include "FormationUi.h"
# include "GameConstants.h"

namespace ff
{
 namespace
	{
     bool IsSummonSlotTriggered(const size_t index)
		{
			switch (index)
			{
			case 0:
				return Key1.down();
			case 1:
				return Key2.down();
			case 2:
				return Key3.down();
			case 3:
				return Key4.down();
			case 4:
				return Key5.down();
			case 5:
				return Key6.down();
			case 6:
				return Key7.down();
			case 7:
				return Key8.down();
			default:
				return false;
			}
		}

		bool IsTooltipModifierPressed()
		{
			return KeyControl.pressed();
		}

		double GetSummonAvailabilityRate(const int32 resourceCount, const int32 summonCost)
		{
			if (summonCost <= 0)
			{
				return 1.0;
			}

			return Clamp((static_cast<double>(resourceCount) / static_cast<double>(summonCost)), 0.0, 1.0);
		}

		struct SummonAllyButton
		{
			RectF rect;
           size_t slotIndex = 0;
			String label;
          Optional<AllyBehavior> behavior;
			ColorF fillColor;
		};

     Array<SummonAllyButton> BuildSummonAllyButtons(const Array<Optional<AllyBehavior>>& formationSlots)
		{
           const Vec2 center = Scene::Rect().bottomCenter().movedBy(0, -108);
			const SizeF buttonSize{ 175, 44 };
			const double spacingX = 12.0;
			const double spacingY = 14.0;
           const double totalWidth = ((buttonSize.x * 4.0) + (spacingX * 3.0));
			const double left = (center.x - (totalWidth * 0.5));
			const double top = (center.y - (buttonSize.y + (spacingY * 0.5)));
			Array<SummonAllyButton> buttons;
			buttons.reserve(formationSlots.size());

         for (size_t index = 0; index < formationSlots.size(); ++index)
			{
				const size_t column = (index % 4);
				const size_t row = (index / 4);
				const double x = (left + (column * (buttonSize.x + spacingX)));
				const double y = (top + (row * (buttonSize.y + spacingY)));

				if (formationSlots[index])
				{
					buttons << SummonAllyButton{
						RectF{ x, y, buttonSize },
                       index,
						U"{}: {}召喚"_fmt(index + 1, GetAllyBehaviorLabel(*formationSlots[index])),
						formationSlots[index],
						GetAllyBehaviorColor(*formationSlots[index]).setA(0.92)
					};
				}
				else
				{
					buttons << SummonAllyButton{
						RectF{ x, y, buttonSize },
                      index,
						U"{}: 未格納"_fmt(index + 1),
						none,
						ColorF{ 0.36, 0.36, 0.38, 0.78 }
					};
				}
			}

			return buttons;
		}
	}

   Optional<SummonInputResult> CheckSummonAllyButtonPressed(const Array<Optional<AllyBehavior>>& formationSlots)
	{
      for (const auto& button : BuildSummonAllyButtons(formationSlots))
		{
         if (button.behavior
				&& ((button.rect.mouseOver() && MouseL.down()) || IsSummonSlotTriggered(button.slotIndex)))
			{
             return SummonInputResult{ button.slotIndex, *button.behavior };
			}
		}

		return none;
	}

  void DrawSummonAllyButtons(const Font& font, const int32 resourceCount, const Array<Optional<AllyBehavior>>& formationSlots, const Optional<size_t>& deniedSlotIndex, const double deniedFlashTimer)
	{
        Optional<AllyBehavior> hoveredBehaviorForTooltip;

       for (const auto& button : BuildSummonAllyButtons(formationSlots))
		{
            const int32 summonCost = button.behavior ? GetSummonCost(*button.behavior) : 0;
         const double summonAvailabilityRate = button.behavior ? GetSummonAvailabilityRate(resourceCount, summonCost) : 0.0;
			const bool affordable = (button.behavior && (resourceCount >= summonCost));
           const bool hovered = button.rect.mouseOver();
			const bool hotkeyPressed = IsSummonSlotTriggered(button.slotIndex);
			const bool deniedFlash = (deniedSlotIndex && (*deniedSlotIndex == button.slotIndex) && (deniedFlashTimer > 0.0));

          if ((hovered || hotkeyPressed) && affordable)
			{
				Cursor::RequestStyle(CursorStyle::Hand);
			}

            const ColorF baseColor = affordable ? button.fillColor : ColorF{ 0.36, 0.36, 0.38, 0.78 };
			const bool emphasized = ((hovered || hotkeyPressed) && affordable);
         const double deniedPulse = (0.55 + (0.45 * Periodic::Sine0_1(0.10s)));
			const ColorF fillColor = deniedFlash
				? baseColor.lerp(ColorF{ 0.96, 0.22, 0.22, 0.94 }, (0.55 * deniedPulse))
				: (emphasized ? baseColor.lerp(ColorF{ 0.92, 0.96, 1.0, 0.96 }, hotkeyPressed ? 0.28 : 0.18) : baseColor);
			const ColorF frameColor = deniedFlash
				? ColorF{ 1.0, 0.62, 0.62, (0.82 + (0.18 * deniedPulse)) }
				: (button.behavior ? ColorF{ 0.86, 0.95, 0.88, 0.9 } : ColorF{ 0.62, 0.62, 0.66, 0.72 });
			button.rect.rounded(12).draw(fillColor);

			if (button.behavior && !affordable)
			{
				const RectF gaugeRect{ button.rect.x + 10, button.rect.y + button.rect.h - 12, button.rect.w - 20, 6 };
				gaugeRect.rounded(3).draw(ColorF{ 0.02, 0.03, 0.05, 0.36 });

				if (summonAvailabilityRate > 0.0)
				{
					const double fillWidth = (gaugeRect.w * summonAvailabilityRate);
					const RectF fillGaugeRect{ gaugeRect.x, gaugeRect.y, fillWidth, gaugeRect.h };

					if (fillWidth >= gaugeRect.h)
					{
						fillGaugeRect.rounded(3).draw(ColorF{ 0.96, 0.98, 1.0, 0.20 });
					}
					else
					{
						fillGaugeRect.draw(ColorF{ 0.96, 0.98, 1.0, 0.20 });
					}
				}

				if (summonAvailabilityRate < 1.0)
				{
					const RectF shortageRect{
						gaugeRect.x + (gaugeRect.w * summonAvailabilityRate),
						gaugeRect.y,
						gaugeRect.w * (1.0 - summonAvailabilityRate),
						gaugeRect.h
					};
					shortageRect.draw(ColorF{ 0.0, 0.0, 0.0, 0.24 });
				}
			}

			button.rect.rounded(12).drawFrame(2, frameColor);
			font(button.label).drawAt(17, button.rect.center().movedBy(0, -8), affordable ? ColorF{ 1.0, 1.0, 1.0 } : ColorF{ 0.86, 0.86, 0.88 });

			if (button.behavior && hovered && IsTooltipModifierPressed())
			{
				hoveredBehaviorForTooltip = *button.behavior;
			}

			if (button.behavior)
			{
               const String infoLabel = deniedFlash
					? U"資源不足 / [{}] Key"_fmt(button.slotIndex + 1)
					: U"Cost {} / [{}] Key"_fmt(summonCost, button.slotIndex + 1);
				const ColorF infoColor = deniedFlash
					? ColorF{ 1.0, 0.90, 0.90, 0.96 }
					: (affordable ? ColorF{ 0.96, 0.98, 0.96, 0.92 } : ColorF{ 0.80, 0.80, 0.84, 0.9 });
				font(infoLabel).drawAt(14, button.rect.center().movedBy(0, 10), infoColor);
			}
			else
			{
                font(U"Empty / [{}] Key"_fmt(button.slotIndex + 1)).drawAt(14, button.rect.center().movedBy(0, 10), ColorF{ 0.80, 0.80, 0.84, 0.9 });
			}
		}

		if (hoveredBehaviorForTooltip)
		{
			const String title = U"{}"_fmt(GetAllyBehaviorLabel(*hoveredBehaviorForTooltip));
			const String description = U"{}"_fmt(GetAllyBehaviorRoleDescription(*hoveredBehaviorForTooltip));
			const Vec2 cursorPos = Cursor::PosF().movedBy(18, 20);
			const RectF tooltipRect{ cursorPos, 240, 60 };
			const ColorF accent = GetAllyBehaviorColor(*hoveredBehaviorForTooltip);
			tooltipRect.rounded(12).draw(ColorF{ 0.06, 0.08, 0.14, 0.94 });
			tooltipRect.rounded(12).drawFrame(2, accent.lerp(Palette::White, 0.20));
			font(title).draw(15, tooltipRect.pos.movedBy(12, 8), Palette::White);
			font(description).draw(13, tooltipRect.pos.movedBy(12, 32), ColorF{ 0.88, 0.92, 1.0, 0.92 });
		}
	}
}
