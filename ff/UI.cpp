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

		struct SummonAllyButtonVisualState
		{
			int32 baseSummonCost = 0;
			int32 summonCost = 0;
			bool discounted = false;
			double summonAvailabilityRate = 0.0;
			bool affordable = false;
			bool hovered = false;
			bool hotkeyPressed = false;
			bool deniedFlash = false;
			ColorF fillColor;
			ColorF frameColor;
			ColorF labelColor;
			String infoLabel;
			ColorF infoColor;
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

		SummonAllyButtonVisualState BuildSummonAllyButtonVisualState(const SummonAllyButton& button, const int32 resourceCount, const WaveTrait activeWaveTrait, const SummonDiscountTraitConfig& summonDiscountTraits, const Optional<size_t>& deniedSlotIndex, const double deniedFlashTimer)
		{
			SummonAllyButtonVisualState state;
			state.baseSummonCost = button.behavior ? GetSummonCost(*button.behavior) : 0;
			state.summonCost = button.behavior ? GetSummonCost(*button.behavior, activeWaveTrait, summonDiscountTraits) : 0;
			state.discounted = (button.behavior && (state.summonCost < state.baseSummonCost));
			state.summonAvailabilityRate = button.behavior ? GetSummonAvailabilityRate(resourceCount, state.summonCost) : 0.0;
			state.affordable = (button.behavior && (resourceCount >= state.summonCost));
			state.hovered = button.rect.mouseOver();
			state.hotkeyPressed = IsSummonSlotTriggered(button.slotIndex);
			state.deniedFlash = (deniedSlotIndex && (*deniedSlotIndex == button.slotIndex) && (deniedFlashTimer > 0.0));

			const ColorF baseColor = state.affordable ? button.fillColor : ColorF{ 0.36, 0.36, 0.38, 0.78 };
			const bool emphasized = ((state.hovered || state.hotkeyPressed) && state.affordable);
			const double deniedPulse = (0.55 + (0.45 * Periodic::Sine0_1(0.10s)));
			state.fillColor = state.deniedFlash
				? baseColor.lerp(ColorF{ 0.96, 0.22, 0.22, 0.94 }, (0.55 * deniedPulse))
				: (emphasized ? baseColor.lerp(ColorF{ 0.92, 0.96, 1.0, 0.96 }, state.hotkeyPressed ? 0.28 : 0.18) : baseColor);
			state.frameColor = state.deniedFlash
				? ColorF{ 1.0, 0.62, 0.62, (0.82 + (0.18 * deniedPulse)) }
				: (state.discounted
					? ColorF{ 0.98, 0.86, 0.48, 0.94 }
					: (button.behavior ? ColorF{ 0.86, 0.95, 0.88, 0.9 } : ColorF{ 0.62, 0.62, 0.66, 0.72 }));
			state.labelColor = state.affordable ? ColorF{ 1.0, 1.0, 1.0 } : ColorF{ 0.86, 0.86, 0.88 };

			if (button.behavior)
			{
				state.infoLabel = state.deniedFlash
					? U"資源不足 / [{}] Key"_fmt(button.slotIndex + 1)
					: (state.discounted
						? U"Cost {} (-{} {}) / [{}] Key"_fmt(state.summonCost, (state.baseSummonCost - state.summonCost), GetWaveTraitLabel(activeWaveTrait), button.slotIndex + 1)
						: U"Cost {} / [{}] Key"_fmt(state.summonCost, button.slotIndex + 1));
				state.infoColor = state.deniedFlash
					? ColorF{ 1.0, 0.90, 0.90, 0.96 }
					: (state.discounted ? ColorF{ 1.0, 0.92, 0.64, 0.96 } : (state.affordable ? ColorF{ 0.96, 0.98, 0.96, 0.92 } : ColorF{ 0.80, 0.80, 0.84, 0.9 }));
			}
			else
			{
				state.infoLabel = U"Empty / [{}] Key"_fmt(button.slotIndex + 1);
				state.infoColor = ColorF{ 0.80, 0.80, 0.84, 0.9 };
			}

			return state;
		}

		void DrawSummonAvailabilityGauge(const RectF& buttonRect, const double summonAvailabilityRate)
		{
			const RectF gaugeRect{ buttonRect.x + 10, buttonRect.y + buttonRect.h - 12, buttonRect.w - 20, 6 };
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

		bool ShouldShowSummonAllyTooltip(const SummonAllyButton& button, const SummonAllyButtonVisualState& state)
		{
			return (button.behavior && state.hovered && IsTooltipModifierPressed());
		}

		void DrawSummonAllyButton(const Font& font, const SummonAllyButton& button, const SummonAllyButtonVisualState& state)
		{
			if ((state.hovered || state.hotkeyPressed) && state.affordable)
			{
				Cursor::RequestStyle(CursorStyle::Hand);
			}

			button.rect.rounded(12).draw(state.fillColor);

			if (button.behavior && !state.affordable)
			{
				DrawSummonAvailabilityGauge(button.rect, state.summonAvailabilityRate);
			}

			button.rect.rounded(12).drawFrame(2, state.frameColor);
			font(button.label).drawAt(17, button.rect.center().movedBy(0, -8), state.labelColor);
			font(state.infoLabel).drawAt(14, button.rect.center().movedBy(0, 10), state.infoColor);
		}

		void DrawSummonAllyTooltip(const Font& font, const AllyBehavior behavior)
		{
			const String title = U"{}"_fmt(GetAllyBehaviorLabel(behavior));
			const String description = U"{}"_fmt(GetAllyBehaviorRoleDescription(behavior));
			const Vec2 cursorPos = Cursor::PosF().movedBy(18, 20);
			const RectF tooltipRect{ cursorPos, 240, 60 };
			const ColorF accent = GetAllyBehaviorColor(behavior);
			tooltipRect.rounded(12).draw(ColorF{ 0.06, 0.08, 0.14, 0.94 });
			tooltipRect.rounded(12).drawFrame(2, accent.lerp(Palette::White, 0.20));
			font(title).draw(15, tooltipRect.pos.movedBy(12, 8), Palette::White);
			font(description).draw(13, tooltipRect.pos.movedBy(12, 32), ColorF{ 0.88, 0.92, 1.0, 0.92 });
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

  void DrawSummonAllyButtons(const Font& font, const int32 resourceCount, const Array<Optional<AllyBehavior>>& formationSlots, const WaveTrait activeWaveTrait, const SummonDiscountTraitConfig& summonDiscountTraits, const Optional<size_t>& deniedSlotIndex, const double deniedFlashTimer)
	{
        Optional<AllyBehavior> hoveredBehaviorForTooltip;
		const auto buttons = BuildSummonAllyButtons(formationSlots);

        for (const auto& button : buttons)
		{
            const auto state = BuildSummonAllyButtonVisualState(button, resourceCount, activeWaveTrait, summonDiscountTraits, deniedSlotIndex, deniedFlashTimer);
			DrawSummonAllyButton(font, button, state);

			if (ShouldShowSummonAllyTooltip(button, state))
			{
				hoveredBehaviorForTooltip = *button.behavior;
			}
		}

		if (hoveredBehaviorForTooltip)
		{
           DrawSummonAllyTooltip(font, *hoveredBehaviorForTooltip);
		}
	}
}
