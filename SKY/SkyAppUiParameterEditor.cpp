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
			buttonFont(label).drawAt(rect.center(), ColorF{ 0.16 });
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

			SimpleGUI::GetFont()(spec.label).draw((panel.x + 16), top, ColorF{ 0.14 });
			SimpleGUI::GetFont()(FormatMillParameterValue(value, spec.decimals, spec.suffix)).draw((panel.x + panel.w - 108), top, ColorF{ 0.14 });

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

		void DrawMillParameterEditorCard(const Rect& rect,
			const int32 sliderId,
			double& value,
			const MillParameterEditorSpec& spec)
		{
			value = Clamp(RoundMillParameterValue(value, spec.sliderRoundStep), spec.minValue, spec.maxValue);

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

			SimpleGUI::GetFont()(spec.label).draw((rect.x + paddingX), labelY, ColorF{ 0.14 });
			SimpleGUI::GetFont()(FormatMillParameterValue(value, spec.decimals, spec.suffix)).draw((rect.rightX() - 74), labelY, ColorF{ 0.14 });

			for (size_t i = 0; i < buttonSpecs.size(); ++i)
			{
				const Rect buttonRect{ (buttonStartX + static_cast<int32>(i) * (buttonWidth + buttonGap)), buttonY, buttonWidth, buttonHeight };
				if (DrawMillStepButton(buttonRect, buttonSpecs[i].first))
				{
					ApplyMillParameterDelta(value, spec, buttonSpecs[i].second);
				}
			}

			DrawMillDragSlider(RectF{ (rect.x + paddingX), sliderY, (rect.w - paddingX * 2), 8 }, sliderId, value, spec);
		}

		StringView ToUnitEditorSectionLabel(const UnitEditorSection section)
		{
			switch (section)
			{
			case UnitEditorSection::PlayerArcaneInfantry:
				return U"Player Arcane";

			case UnitEditorSection::EnemyInfantry:
				return U"Enemy Infantry";

			case UnitEditorSection::EnemyArcaneInfantry:
				return U"Enemy Arcane";

			case UnitEditorSection::PlayerInfantry:
			default:
				return U"Player Infantry";
			}
		}

		UnitTeam ToUnitEditorTeam(const UnitEditorSection section)
		{
			switch (section)
			{
			case UnitEditorSection::EnemyInfantry:
			case UnitEditorSection::EnemyArcaneInfantry:
				return UnitTeam::Enemy;

			case UnitEditorSection::PlayerInfantry:
			case UnitEditorSection::PlayerArcaneInfantry:
			default:
				return UnitTeam::Player;
			}
		}

		SapperUnitType ToUnitEditorUnitType(const UnitEditorSection section)
		{
			switch (section)
			{
			case UnitEditorSection::PlayerArcaneInfantry:
			case UnitEditorSection::EnemyArcaneInfantry:
				return SapperUnitType::ArcaneInfantry;

			case UnitEditorSection::PlayerInfantry:
			case UnitEditorSection::EnemyInfantry:
			default:
				return SapperUnitType::Infantry;
			}
		}

		int32 ToUnitEditorSliderBase(const UnitEditorSection section)
		{
			switch (section)
			{
			case UnitEditorSection::PlayerArcaneInfantry:
				return 100;

			case UnitEditorSection::EnemyInfantry:
				return 200;

			case UnitEditorSection::EnemyArcaneInfantry:
				return 300;

			case UnitEditorSection::PlayerInfantry:
			default:
				return 0;
			}
		}

		void ClampUnitParameters(UnitParameters& parameters)
		{
			parameters.maxHitPoints = Clamp(parameters.maxHitPoints, 1.0, 500.0);
			parameters.moveSpeed = Clamp(parameters.moveSpeed, 0.5, 12.0);
			parameters.attackRange = Clamp(parameters.attackRange, 0.5, 12.0);
			parameters.attackDamage = Clamp(parameters.attackDamage, 0.0, 80.0);
			parameters.attackInterval = Clamp(parameters.attackInterval, 0.05, 5.0);
			parameters.manaCost = Clamp(parameters.manaCost, 0.0, 300.0);
		}

		void DrawMovementTypeSelector(const Rect& panel, const double top, MovementType& movementType)
		{
         SimpleGUI::GetFont()(U"Type").draw((panel.x + 16), top, ColorF{ 0.14 });

            const Rect infantryButton{ (panel.x + panel.w - 146), static_cast<int32>(top - 2), 64, 24 };
			const Rect tankButton{ (panel.x + panel.w - 74), static_cast<int32>(top - 2), 58, 24 };

			if (DrawMillStepButton(infantryButton, U"Infantry"))
			{
				movementType = MovementType::Infantry;
			}

			if (DrawMillStepButton(tankButton, U"Tank"))
			{
				movementType = MovementType::Tank;
			}
		}

		void ApplyUnitParametersToSpawned(Array<SpawnedSapper>& sappers, const UnitTeam team, const SapperUnitType unitType, const UnitParameters& parameters)
		{
			for (auto& sapper : sappers)
			{
				if ((sapper.team != team) || (sapper.unitType != unitType))
				{
					continue;
				}

				const double hitPointRatio = ((0.0 < sapper.maxHitPoints) ? (sapper.hitPoints / sapper.maxHitPoints) : 1.0);
				ApplyUnitParameters(sapper, parameters);
				sapper.hitPoints = Clamp((sapper.maxHitPoints * hitPointRatio), 0.0, sapper.maxHitPoints);
			}
		}

		void DrawUnitParameterRows(const Rect& panel, const int32 sliderBase, UnitParameters& parameters)
		{
           const int32 cardX = (panel.x + 10);
			const int32 cardWidth = (panel.w - 20);
			const int32 cardHeight = 64;
            const int32 cardTop = (panel.y + 112);
			const int32 cardStep = 68;

			DrawMillParameterEditorCard(Rect{ cardX, (cardTop + cardStep * 0), cardWidth, cardHeight },
				(sliderBase + 0),
				parameters.maxHitPoints,
				MillParameterEditorSpec{ U"Max HP", U"", 1.0, 500.0, 5.0, 20.0, 50.0, 1.0, 0 });
			DrawMillParameterEditorCard(Rect{ cardX, (cardTop + cardStep * 1), cardWidth, cardHeight },
				(sliderBase + 1),
				parameters.moveSpeed,
				MillParameterEditorSpec{ U"Move Speed", U"", 0.5, 12.0, 0.1, 0.5, 1.0, 0.05, 2 });
			DrawMillParameterEditorCard(Rect{ cardX, (cardTop + cardStep * 2), cardWidth, cardHeight },
				(sliderBase + 2),
				parameters.attackRange,
				MillParameterEditorSpec{ U"Attack Range", U"", 0.5, 12.0, 0.1, 0.5, 1.0, 0.05, 2 });
			DrawMillParameterEditorCard(Rect{ cardX, (cardTop + cardStep * 3), cardWidth, cardHeight },
				(sliderBase + 3),
				parameters.attackDamage,
				MillParameterEditorSpec{ U"Attack Damage", U"", 0.0, 80.0, 1.0, 5.0, 10.0, 1.0, 0 });
			DrawMillParameterEditorCard(Rect{ cardX, (cardTop + cardStep * 4), cardWidth, cardHeight },
				(sliderBase + 4),
				parameters.attackInterval,
				MillParameterEditorSpec{ U"Attack Interval", U"s", 0.05, 5.0, 0.05, 0.25, 0.5, 0.05, 2 });
			DrawMillParameterEditorCard(Rect{ cardX, (cardTop + cardStep * 5), cardWidth, cardHeight },
				(sliderBase + 5),
				parameters.manaCost,
				MillParameterEditorSpec{ U"Mana Cost", U"", 0.0, 300.0, 1.0, 5.0, 10.0, 1.0, 0 });
		}
	}
}
