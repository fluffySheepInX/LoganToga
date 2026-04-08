# include "SkyAppUi.hpp"
# include "MainUi.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
	namespace
	{
		struct MillParameterEditorSpec
		{
			StringView label;
			StringView suffix;
			double minValue;
			double maxValue;
			double smallStep;
			double mediumStep;
			double largeStep;
			double sliderRoundStep;
			int32 decimals;
		};

		[[nodiscard]] String FormatMillParameterValue(const double value, const int32 decimals, const StringView suffix)
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

		[[nodiscard]] double RoundMillParameterValue(const double value, const double roundStep)
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

		[[nodiscard]] bool DrawMillStepButton(const Rect& rect, const StringView label)
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
			SimpleGUI::GetFont()(U"Slider").draw((trackRect.x + trackRect.w - 44), (trackRect.y - 22), ColorF{ 0.24 });
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
	}

	void DrawMillStatusEditor(const SkyAppPanels& panels,
		MapData& mapData,
		const size_t selectedMillIndex,
		const FilePathView path,
		TimedMessage& mapDataMessage)
	{
		if (mapData.placedModels.size() <= selectedMillIndex)
		{
			return;
		}

		PlacedModel& mill = mapData.placedModels[selectedMillIndex];
		const Rect panel = panels.millStatusEditor;
		panel.draw(ColorF{ 0.98, 0.95 });
		panel.drawFrame(2, 0, ColorF{ 0.25 });
		SimpleGUI::GetFont()(U"Mill Status Editor").draw((panel.x + 16), (panel.y + 12), ColorF{ 0.12 });
		SimpleGUI::GetFont()(U"Pos: {:.1f}, {:.1f}, {:.1f}"_fmt(mill.position.x, mill.position.y, mill.position.z)).draw((panel.x + 16), (panel.y + 36), ColorF{ 0.18 });

		mill.attackRange = Clamp(mill.attackRange, 1.0, 20.0);
		mill.attackDamage = Clamp(mill.attackDamage, 1.0, 80.0);
		mill.attackInterval = Clamp(mill.attackInterval, 0.2, 5.0);
		mill.suppressionDuration = Clamp(mill.suppressionDuration, 0.2, 10.0);
		mill.suppressionMoveSpeedMultiplier = Clamp(mill.suppressionMoveSpeedMultiplier, 0.1, 1.0);
		mill.suppressionAttackDamageMultiplier = Clamp(mill.suppressionAttackDamageMultiplier, 0.1, 1.0);
		mill.suppressionAttackIntervalMultiplier = Clamp(mill.suppressionAttackIntervalMultiplier, 1.0, 10.0);

		DrawMillParameterEditorRow(panel,
			(panel.y + 62),
			0,
			mill.attackRange,
			MillParameterEditorSpec{ U"Range", U"", 1.0, 20.0, 0.5, 2.0, 5.0, 0.1, 1 });
		DrawMillParameterEditorRow(panel,
			(panel.y + 128),
			1,
			mill.attackDamage,
			MillParameterEditorSpec{ U"Damage", U"", 1.0, 80.0, 1.0, 5.0, 10.0, 1.0, 0 });
		DrawMillParameterEditorRow(panel,
			(panel.y + 194),
			2,
			mill.attackInterval,
			MillParameterEditorSpec{ U"Interval", U"s", 0.2, 5.0, 0.1, 0.25, 0.5, 0.05, 2 });
		DrawMillParameterEditorRow(panel,
			(panel.y + 260),
			3,
			mill.suppressionDuration,
			MillParameterEditorSpec{ U"Suppress Time", U"s", 0.2, 10.0, 0.1, 0.5, 1.0, 0.05, 2 });
		DrawMillParameterEditorRow(panel,
			(panel.y + 326),
			4,
			mill.suppressionMoveSpeedMultiplier,
			MillParameterEditorSpec{ U"Move Rate", U"x", 0.1, 1.0, 0.05, 0.1, 0.2, 0.05, 2 });
		DrawMillParameterEditorRow(panel,
			(panel.y + 392),
			5,
			mill.suppressionAttackDamageMultiplier,
			MillParameterEditorSpec{ U"Atk Damage Rate", U"x", 0.1, 1.0, 0.05, 0.1, 0.2, 0.05, 2 });
		DrawMillParameterEditorRow(panel,
			(panel.y + 458),
			6,
			mill.suppressionAttackIntervalMultiplier,
			MillParameterEditorSpec{ U"Atk Interval Rate", U"x", 1.0, 10.0, 0.1, 0.5, 1.0, 0.05, 2 });

     const Rect resetButton{ (panel.x + 16), (panel.y + 540), 136, 30 };
		const Rect saveButton{ (panel.x + panel.w - 152), (panel.y + 540), 136, 30 };

		if (DrawTextButton(resetButton, U"推奨値に戻す"))
		{
			mill.attackRange = MillDefenseRange;
			mill.attackDamage = MillDefenseDamage;
			mill.attackInterval = MillDefenseInterval;
          mill.suppressionDuration = MillSuppressionDuration;
			mill.suppressionMoveSpeedMultiplier = MillSuppressionMoveSpeedMultiplier;
			mill.suppressionAttackDamageMultiplier = MillSuppressionAttackDamageMultiplier;
			mill.suppressionAttackIntervalMultiplier = MillSuppressionAttackIntervalMultiplier;
			mapDataMessage.show(U"Mill ステータスを既定値に戻しました", 3.0);
		}

		if (DrawTextButton(saveButton, U"Save TOML"))
		{
			mapDataMessage.show(SaveMapData(mapData, path) ? U"Mill ステータスを保存" : U"Mill ステータス保存失敗", 3.0);
		}
	}
}
