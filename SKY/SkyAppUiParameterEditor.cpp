# include "SkyAppUiParameterEditorInternal.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
	namespace UiParameterEditorDetail
	{
		String FormatMillParameterValue(const double value, const int32 decimals, const StringView suffix)
		{
			switch (decimals)
			{
			case 0:
				return U"{:.0f}{}"_fmt(value, suffix);

			case 1:
				return U"{:.1f}{}"_fmt(value, suffix);

			default:
				return U"{:.2f}{}"_fmt(value, suffix);
			}
		}

		void DrawUnitParameterSectionHeader(const Rect& panel,
			const int32 top,
			const StringView title,
			const StringView caption)
		{
			const Rect sectionRect{ (panel.x + 8), top, (panel.w - 16), 38 };
			sectionRect.rounded(10).draw(ColorF{ 0.90, 0.94, 1.0, 0.70 });
			sectionRect.rounded(10).drawFrame(1, 0, ColorF{ 0.74, 0.80, 0.90, 0.86 });
           SimpleGUI::GetFont()(title).draw((sectionRect.x + 12), (sectionRect.y + 6), UiInternal::EditorTextOnCardPrimaryColor());
			SimpleGUI::GetFont()(caption).draw((sectionRect.x + 12), (sectionRect.y + 22), UiInternal::EditorTextOnCardSecondaryColor());
		}

		StringView ToUnitAiRoleLabel(const UnitAiRole aiRole)
		{
			switch (aiRole)
			{
			case UnitAiRole::AssaultBase:
				return U"AssaultBase";

			case UnitAiRole::Support:
				return U"Support";

			case UnitAiRole::SecureResources:
			default:
				return U"SecureResources";
			}
		}

		StringView ToUnitEditorPageLabel(const UnitEditorPage page)
		{
			switch (page)
			{
			case UnitEditorPage::Combat:
				return U"Combat";

			case UnitEditorPage::Footprint:
				return U"Footprint";

			case UnitEditorPage::Skill:
				return U"Skill";

			case UnitEditorPage::Basic:
			default:
				return U"Basic";
			}
		}

		void DrawUnitAiRoleSelector(const Rect& panel, const double top, UnitAiRole& aiRole)
		{
         SimpleGUI::GetFont()(U"AI Role").draw((panel.x + 16), top, UiInternal::EditorTextOnLightPrimaryColor());
			SimpleGUI::GetFont()(ToUnitAiRoleLabel(aiRole)).draw((panel.x + 108), top, UiInternal::EditorTextOnLightSecondaryColor());

			const Rect secureButton{ (panel.x + panel.w - 236), static_cast<int32>(top - 2), 72, 24 };
			const Rect assaultButton{ (panel.x + panel.w - 156), static_cast<int32>(top - 2), 72, 24 };
			const Rect supportButton{ (panel.x + panel.w - 76), static_cast<int32>(top - 2), 64, 24 };

			if (DrawMillStepButton(secureButton, U"Secure"))
			{
				aiRole = UnitAiRole::SecureResources;
			}

			if (DrawMillStepButton(assaultButton, U"Assault"))
			{
				aiRole = UnitAiRole::AssaultBase;
			}

			if (DrawMillStepButton(supportButton, U"Support"))
			{
				aiRole = UnitAiRole::Support;
			}
		}

		StringView ToUnitFootprintTypeLabel(const UnitFootprintType footprintType)
		{
			switch (footprintType)
			{
			case UnitFootprintType::Capsule:
				return U"Capsule";

			case UnitFootprintType::Circle:
			default:
				return U"Circle";
			}
		}

		StringView ToMovementTypeLabel(const MovementType movementType)
		{
			switch (movementType)
			{
			case MovementType::Tank:
				return U"Tank";

			case MovementType::Infantry:
			default:
				return U"Infantry";
			}
		}

		double RoundMillParameterValue(const double value, const double roundStep)
		{
			if (roundStep <= 0.0)
			{
				return value;
			}

			return (Math::Round(value / roundStep) * roundStep);
		}

		void ApplyMillParameterDelta(double& value, const MillParameterEditorSpec& spec, const double delta)
		{
			value = Clamp(RoundMillParameterValue((value + delta), spec.sliderRoundStep), spec.minValue, spec.maxValue);
		}

		bool DrawMillStepButton(const Rect& rect, const StringView label)
		{
			static const Font buttonFont{ 14, Typeface::Bold };
			const bool hovered = rect.mouseOver();
            rect.draw(hovered ? ColorF{ 0.90, 0.93, 0.98 } : ColorF{ 0.80, 0.84, 0.90 })
				.drawFrame(1, 0, ColorF{ 0.32, 0.38, 0.46 });
           buttonFont(label).drawAt(rect.center(), UiInternal::EditorTextOnCardPrimaryColor());
			return hovered && MouseL.down();
		}

		void DrawMillDragSlider(const RectF& trackRect, const int32 sliderId, double& value, const MillParameterEditorSpec& spec)
		{
			static Optional<int32> activeSliderId;

			const double ratio = Math::Saturate((value - spec.minValue) / Max(0.0001, (spec.maxValue - spec.minValue)));
			const RectF knobRect{ Arg::center = Vec2{ (trackRect.x + trackRect.w * ratio), trackRect.centerY() }, 14, 22 };
			const bool hovered = trackRect.stretched(0, 8).mouseOver() || knobRect.mouseOver();

			if (MouseL.down() && hovered)
			{
				activeSliderId = sliderId;
			}

			if (activeSliderId && (*activeSliderId == sliderId))
			{
				if (MouseL.pressed())
				{
					const double cursorRatio = Math::Saturate((Cursor::PosF().x - trackRect.x) / Max(1.0, trackRect.w));
					value = Clamp(RoundMillParameterValue((spec.minValue + (spec.maxValue - spec.minValue) * cursorRatio), spec.sliderRoundStep), spec.minValue, spec.maxValue);
				}
				else
				{
					activeSliderId.reset();
				}
			}

			trackRect.rounded(4).draw(ColorF{ 0.12, 0.14, 0.17, 0.92 });
			RectF{ trackRect.pos, (trackRect.w * ratio), trackRect.h }.rounded(4).draw(ColorF{ 0.38, 0.70, 0.96, 0.95 });
			trackRect.rounded(4).drawFrame(1.0, ColorF{ 0.82, 0.88, 0.95, 0.65 });
			knobRect.rounded(4).draw((activeSliderId && (*activeSliderId == sliderId)) ? ColorF{ 0.96, 0.98, 1.0 } : ColorF{ 0.90, 0.94, 0.98 })
				.drawFrame(1.0, 0.0, ColorF{ 0.25, 0.34, 0.50, 0.95 });
		}

		void DrawMillParameterEditorRow(const Rect& panel,
			const double top,
			const int32 sliderId,
			double& value,
			const MillParameterEditorSpec& spec)
		{
			value = Clamp(RoundMillParameterValue(value, spec.sliderRoundStep), spec.minValue, spec.maxValue);

           SimpleGUI::GetFont()(spec.label).draw((panel.x + 16), top, UiInternal::EditorTextOnCardPrimaryColor());
			SimpleGUI::GetFont()(FormatMillParameterValue(value, spec.decimals, spec.suffix)).draw((panel.x + panel.w - 108), top, UiInternal::EditorTextOnCardPrimaryColor());

			const int32 buttonWidth = 40;
			const int32 buttonHeight = 24;
			const int32 buttonGap = 6;
			const int32 buttonStartX = (panel.x + 16);
			const int32 buttonY = static_cast<int32>(top + 20);
			const Array<std::pair<String, double>> buttonSpecs{
				{ U"-S", -spec.smallStep },
				{ U"-M", -spec.mediumStep },
				{ U"-L", -spec.largeStep },
				{ U"+S", spec.smallStep },
				{ U"+M", spec.mediumStep },
				{ U"+L", spec.largeStep },
			};

			for (size_t i = 0; i < buttonSpecs.size(); ++i)
			{
				const Rect buttonRect{ (buttonStartX + static_cast<int32>(i) * (buttonWidth + buttonGap)), buttonY, buttonWidth, buttonHeight };
				if (DrawMillStepButton(buttonRect, buttonSpecs[i].first))
				{
					ApplyMillParameterDelta(value, spec, buttonSpecs[i].second);
				}
			}

			DrawMillDragSlider(RectF{ (panel.x + 16), (top + 50), (panel.w - 32), 8 }, sliderId, value, spec);
		}

      bool DrawMillParameterEditorCard(const Rect& rect,
			const int32 sliderId,
			double& value,
			const MillParameterEditorSpec& spec)
		{
			value = Clamp(RoundMillParameterValue(value, spec.sliderRoundStep), spec.minValue, spec.maxValue);
			const bool hovered = rect.mouseOver();

			rect.draw(ColorF{ 1.0, 1.0, 1.0, 0.76 })
				.drawFrame(1, 0, ColorF{ 0.80, 0.82, 0.86, 0.85 });

          const int32 paddingX = 12;
			const int32 labelY = (rect.y + 8);
			const int32 buttonHeight = ((rect.h <= 72) ? 20 : 24);
			const int32 sliderY = (rect.bottomY() - 12);
			const int32 buttonY = (sliderY - buttonHeight - 4);
			const int32 buttonWidth = 34;
			const int32 buttonGap = 4;
			const int32 buttonStartX = (rect.x + paddingX);
			const Array<std::pair<String, double>> buttonSpecs{
				{ U"-S", -spec.smallStep },
				{ U"-M", -spec.mediumStep },
				{ U"-L", -spec.largeStep },
				{ U"+S", spec.smallStep },
				{ U"+M", spec.mediumStep },
				{ U"+L", spec.largeStep },
			};

           SimpleGUI::GetFont()(spec.label).draw((rect.x + paddingX), labelY, UiInternal::EditorTextOnCardPrimaryColor());
			SimpleGUI::GetFont()(FormatMillParameterValue(value, spec.decimals, spec.suffix)).draw((rect.rightX() - 74), labelY, UiInternal::EditorTextOnCardPrimaryColor());

			for (size_t i = 0; i < buttonSpecs.size(); ++i)
			{
				const Rect buttonRect{ (buttonStartX + static_cast<int32>(i) * (buttonWidth + buttonGap)), buttonY, buttonWidth, buttonHeight };
				if (DrawMillStepButton(buttonRect, buttonSpecs[i].first))
				{
					ApplyMillParameterDelta(value, spec, buttonSpecs[i].second);
				}
			}

			DrawMillDragSlider(RectF{ (rect.x + paddingX), sliderY, (rect.w - paddingX * 2), 8 }, sliderId, value, spec);
           return hovered;
		}

      StringView ToUnitEditorSectionLabel(const UnitTeam team, const SapperUnitType unitType)
		{
         return GetUnitEditorSectionLabel(team, unitType);
		}

       int32 ToUnitEditorSliderBase(const UnitTeam team, const SapperUnitType unitType)
		{
            return static_cast<int32>(GetUnitParameterSlotIndex(team, unitType) * 100);
		}
	}
}
