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

		void ClampUnitParameters(UnitParameters& parameters)
		{
           parameters.maxHitPoints = Clamp(parameters.maxHitPoints, 1.0, 1000.0);
			parameters.moveSpeed = Clamp(parameters.moveSpeed, 0.5, 24.0);
			parameters.attackRange = Clamp(parameters.attackRange, 0.5, 24.0);
           parameters.stopDistance = Clamp(parameters.stopDistance, 0.0, 24.0);
			parameters.attackDamage = Clamp(parameters.attackDamage, 0.0, 160.0);
			parameters.attackInterval = Clamp(parameters.attackInterval, 0.05, 10.0);
           parameters.visionRange = Clamp(parameters.visionRange, 0.5, 40.0);
		   parameters.manaCost = Clamp(parameters.manaCost, 0.0, 1600.0);
           parameters.footprintRadius = Clamp(parameters.footprintRadius, 0.1, 4.0);
			parameters.footprintHalfLength = Clamp(parameters.footprintHalfLength, 0.0, 6.0);
		}

		void ClampExplosionSkillParameters(ExplosionSkillParameters& parameters)
		{
			parameters.radius = Clamp(parameters.radius, 0.5, 12.0);
			parameters.unitDamage = Clamp(parameters.unitDamage, 0.0, 300.0);
			parameters.baseDamage = Clamp(parameters.baseDamage, 0.0, 300.0);
			parameters.cooldownSeconds = Clamp(parameters.cooldownSeconds, 0.1, 30.0);
			parameters.gunpowderCost = Clamp(parameters.gunpowderCost, 0.0, 200.0);
			parameters.effectLifetime = Clamp(parameters.effectLifetime, 0.05, 2.0);
			parameters.effectThickness = Clamp(parameters.effectThickness, 1.0, 20.0);
			parameters.effectOffsetY = Clamp(parameters.effectOffsetY, 0.0, 4.0);
			parameters.effectColor = ColorF{
				Clamp(parameters.effectColor.r, 0.0, 1.0),
				Clamp(parameters.effectColor.g, 0.0, 1.0),
				Clamp(parameters.effectColor.b, 0.0, 1.0),
				Clamp(parameters.effectColor.a, 0.0, 1.0),
			};
		}

		void ClampBuildMillSkillParameters(BuildMillSkillParameters& parameters)
		{
			parameters.manaCost = Clamp(parameters.manaCost, 0.0, 200.0);
			parameters.gunpowderCost = Clamp(parameters.gunpowderCost, 0.0, 200.0);
			parameters.forwardOffset = Clamp(parameters.forwardOffset, 1.0, 10.0);
		}

		void ClampHealSkillParameters(HealSkillParameters& parameters)
		{
			parameters.manaCost = Clamp(parameters.manaCost, 0.0, 200.0);
			parameters.radius = Clamp(parameters.radius, 0.5, 12.0);
			parameters.amount = Clamp(parameters.amount, 1.0, 200.0);
		}

		void ClampScoutSkillParameters(ScoutSkillParameters& parameters)
		{
			parameters.gunpowderCost = Clamp(parameters.gunpowderCost, 0.0, 200.0);
			parameters.durationSeconds = Clamp(parameters.durationSeconds, 0.1, 30.0);
			parameters.visionMultiplier = Clamp(parameters.visionMultiplier, 1.0, 4.0);
		}

		void DrawMovementTypeSelector(const Rect& panel, const double top, MovementType& movementType)
		{
           SimpleGUI::GetFont()(U"Type").draw((panel.x + 16), top, UiInternal::EditorTextOnLightPrimaryColor());

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

		void DrawFootprintTypeSelector(const Rect& panel, const double top, UnitParameters& parameters)
		{
           SimpleGUI::GetFont()(U"Footprint").draw((panel.x + 16), top, UiInternal::EditorTextOnLightPrimaryColor());
			SimpleGUI::GetFont()(ToUnitFootprintTypeLabel(parameters.footprintType)).draw((panel.x + 108), top, UiInternal::EditorTextOnLightSecondaryColor());

			const Rect circleButton{ (panel.x + panel.w - 152), static_cast<int32>(top - 2), 64, 24 };
			const Rect capsuleButton{ (panel.x + panel.w - 80), static_cast<int32>(top - 2), 64, 24 };

			if (DrawMillStepButton(circleButton, U"Circle"))
			{
				parameters.footprintType = UnitFootprintType::Circle;
			}

			if (DrawMillStepButton(capsuleButton, U"Capsule"))
			{
				parameters.footprintType = UnitFootprintType::Capsule;
				parameters.footprintHalfLength = Max(parameters.footprintHalfLength,
					parameters.footprintRadius * ((parameters.movementType == MovementType::Tank) ? 2.0 : 1.1));
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

       void DrawUnitParameterRows(const Rect& panel, const int32 sliderBase, UnitParameters& parameters, const UnitEditorPage page, const int32 top, String& hoveredDescription, Optional<Rect>& hoveredRect)
		{
           const StringView sectionTitle = (page == UnitEditorPage::Combat)
				? U"Combat Stats"
				: ((page == UnitEditorPage::Footprint) ? U"Collision Stats" : U"Core Stats");
			const StringView sectionCaption = (page == UnitEditorPage::Combat)
				? U"S / M / L buttons = small / medium / large step"
				: U"Slider for rough tuning, S / M / L for precise step edits";
			DrawUnitParameterSectionHeader(panel, top, sectionTitle, sectionCaption);

			  const int32 cardX = (panel.x + 10);
			const int32 cardWidth = (panel.w - 20);
				const int32 cardHeight = 82;
			const int32 cardTop = (top + 48);
			const int32 cardStep = 88;
			const auto drawUnitCard = [&](const int32 rowIndex, const int32 sliderId, double& value, const MillParameterEditorSpec& spec, const StringView description)
				{
                  const Rect cardRect{ cardX, (cardTop + cardStep * rowIndex), cardWidth, cardHeight };
					if (DrawMillParameterEditorCard(cardRect, sliderId, value, spec))
					{
						hoveredDescription = String{ description };
                       hoveredRect = cardRect;
					}
				};

			switch (page)
			{
			case UnitEditorPage::Combat:
             drawUnitCard(0, (sliderBase + 2), parameters.attackRange,
					MillParameterEditorSpec{ U"Attack Range", U"", 0.5, 24.0, 0.1, 0.5, 1.0, 0.05, 2 },
					U"敵へ攻撃できる距離です。青い円で表示され、射程内に入った相手へ通常攻撃を行います。");
				drawUnitCard(1, (sliderBase + 3), parameters.stopDistance,
					MillParameterEditorSpec{ U"Stop Distance", U"", 0.0, 24.0, 0.05, 0.25, 0.5, 0.05, 2 },
					U"交戦時に保ちたい距離です。オレンジの円で表示され、既存出撃ユニットも Apply 後にこの距離へ寄ります。");
				drawUnitCard(2, (sliderBase + 4), parameters.attackDamage,
					MillParameterEditorSpec{ U"Attack Damage", U"", 0.0, 160.0, 1.0, 5.0, 10.0, 1.0, 0 },
					U"1回の攻撃で与える基本ダメージです。耐久との兼ね合いで戦闘時間が大きく変わります。");
				drawUnitCard(3, (sliderBase + 5), parameters.attackInterval,
					MillParameterEditorSpec{ U"Attack Interval", U"s", 0.05, 10.0, 0.05, 0.25, 0.5, 0.05, 2 },
					U"攻撃の待ち時間です。小さいほど手数が増え、継続火力が上がります。");
				return;

			case UnitEditorPage::Footprint:
             drawUnitCard(0, (sliderBase + 7), parameters.footprintRadius,
					MillParameterEditorSpec{ U"Footprint Radius", U"", 0.1, 4.0, 0.05, 0.1, 0.25, 0.05, 2 },
					U"ユニットの横幅に相当する当たり判定半径です。接触距離、選択しやすさ、障害物回避にも影響します。");
				drawUnitCard(1, (sliderBase + 8), parameters.footprintHalfLength,
					MillParameterEditorSpec{ U"Footprint HalfLength", U"", 0.0, 6.0, 0.05, 0.25, 0.5, 0.05, 2 },
					U"Capsule 型フットプリントの前後半長です。車のような縦長ユニットで長さ方向の接触判定を調整します。");
				return;

			case UnitEditorPage::Basic:
			default:
             drawUnitCard(0, (sliderBase + 0), parameters.maxHitPoints,
					MillParameterEditorSpec{ U"Max HP", U"", 1.0, 1000.0, 5.0, 20.0, 50.0, 1.0, 0 },
					U"最大耐久です。高いほど前線に残りやすくなりますが、倒しにくさも増えます。");
				drawUnitCard(1, (sliderBase + 1), parameters.moveSpeed,
					MillParameterEditorSpec{ U"Move Speed", U"", 0.5, 24.0, 0.1, 0.5, 1.0, 0.05, 2 },
					U"移動速度です。集結、追撃、引き撃ち、資源エリアへの到達速度に影響します。");
              drawUnitCard(2, (sliderBase + 9), parameters.visionRange,
					MillParameterEditorSpec{ U"Vision Range", U"", 0.5, 40.0, 0.1, 0.5, 1.0, 0.05, 2 },
					U"索敵できる距離です。Fog of War 使用時に敵や拠点、資源地点を発見できる範囲へ影響します。");
				drawUnitCard(3, (sliderBase + 6), parameters.manaCost,
					MillParameterEditorSpec{ U"Mana Cost", U"", 0.0, 1600.0, 1.0, 5.0, 10.0, 1.0, 0 },
					U"出撃時に必要な魔力です。強さだけでなく量産しやすさの調整にも使います。");
				return;
			}
		}

		void DrawExplosionSkillParameterRows(const Rect& panel,
			const int32 sliderBase,
			ExplosionSkillParameters& parameters,
			const int32 top,
			String& hoveredDescription,
			Optional<Rect>& hoveredRect)
		{
			ClampExplosionSkillParameters(parameters);
			DrawUnitParameterSectionHeader(panel, top, U"Explosion Skill", U"Gameplay and effect tuning for the selected team/unit");

			const int32 horizontalGap = 8;
			const int32 cardX = (panel.x + 10);
			const int32 cardWidth = ((panel.w - 20 - horizontalGap) / 2);
			const int32 cardHeight = 78;
			const int32 cardTop = (top + 48);
			const int32 cardStep = 84;
			const auto drawSkillCard = [&](const int32 index, const int32 sliderId, double& value, const MillParameterEditorSpec& spec, const StringView description)
				{
					const int32 column = (index % 2);
					const int32 row = (index / 2);
					const Rect cardRect{ (cardX + (cardWidth + horizontalGap) * column), (cardTop + cardStep * row), cardWidth, cardHeight };
					if (DrawMillParameterEditorCard(cardRect, sliderId, value, spec))
					{
						hoveredDescription = String{ description };
						hoveredRect = cardRect;
					}
				};

			drawSkillCard(0, (sliderBase + 20), parameters.radius,
				MillParameterEditorSpec{ U"Radius", U"", 0.5, 12.0, 0.1, 0.5, 1.0, 0.05, 2 },
				U"爆破ダメージとリング演出の半径です。大きいほど巻き込みやすくなります。");
			drawSkillCard(1, (sliderBase + 21), parameters.unitDamage,
				MillParameterEditorSpec{ U"Unit Damage", U"", 0.0, 300.0, 1.0, 5.0, 10.0, 1.0, 0 },
				U"爆風がユニットへ与えるダメージです。密集戦での突破力に直結します。");
			drawSkillCard(2, (sliderBase + 22), parameters.baseDamage,
				MillParameterEditorSpec{ U"Base Damage", U"", 0.0, 300.0, 1.0, 5.0, 10.0, 1.0, 0 },
				U"敵拠点が爆風圏内にある時に与えるダメージです。拠点破壊の速さに影響します。");
			drawSkillCard(3, (sliderBase + 23), parameters.cooldownSeconds,
				MillParameterEditorSpec{ U"Cooldown", U"s", 0.1, 30.0, 0.1, 0.5, 1.0, 0.05, 2 },
				U"同ユニットが再び爆破スキルを使えるまでの待ち時間です。");
			drawSkillCard(4, (sliderBase + 24), parameters.gunpowderCost,
				MillParameterEditorSpec{ U"Gunpowder", U"", 0.0, 200.0, 1.0, 5.0, 10.0, 1.0, 0 },
				U"爆破スキル発動時に消費する火薬です。兵メニューの表示コストにも反映されます。");
			drawSkillCard(5, (sliderBase + 25), parameters.effectLifetime,
				MillParameterEditorSpec{ U"Effect Time", U"s", 0.05, 2.0, 0.05, 0.1, 0.2, 0.05, 2 },
				U"爆破リング演出の表示時間です。長いほど余韻が残ります。");
			drawSkillCard(6, (sliderBase + 26), parameters.effectThickness,
				MillParameterEditorSpec{ U"Effect Width", U"", 1.0, 20.0, 0.5, 1.0, 2.0, 0.1, 1 },
				U"爆破リングの線の太さです。大きいほど強い印象のエフェクトになります。");
			drawSkillCard(7, (sliderBase + 27), parameters.effectOffsetY,
				MillParameterEditorSpec{ U"Effect Height", U"", 0.0, 4.0, 0.05, 0.1, 0.25, 0.05, 2 },
				U"爆破エフェクトの表示高さです。モデル中心より上に出したい時に使います。");

		}

		void DrawUniqueSkillParameterRows(const Rect& panel,
			const int32 sliderBase,
			const UniqueSkillType uniqueSkillType,
			BuildMillSkillParameters& buildParameters,
			HealSkillParameters& healParameters,
			ScoutSkillParameters& scoutParameters,
			const int32 top,
			String& hoveredDescription,
			Optional<Rect>& hoveredRect)
		{
			const int32 horizontalGap = 8;
			const int32 cardX = (panel.x + 10);
			const int32 cardWidth = ((panel.w - 20 - horizontalGap) / 2);
			const int32 cardHeight = 78;
			const int32 cardTop = (top + 48);
			const int32 cardStep = 84;
			const auto drawSkillCard = [&](const int32 index, const int32 sliderId, double& value, const MillParameterEditorSpec& spec, const StringView description)
				{
					const int32 column = (index % 2);
					const int32 row = (index / 2);
					const Rect cardRect{ (cardX + (cardWidth + horizontalGap) * column), (cardTop + cardStep * row), cardWidth, cardHeight };
					if (DrawMillParameterEditorCard(cardRect, sliderId, value, spec))
					{
						hoveredDescription = String{ description };
						hoveredRect = cardRect;
					}
				};

			switch (uniqueSkillType)
			{
			case UniqueSkillType::BuildMill:
				ClampBuildMillSkillParameters(buildParameters);
				DrawUnitParameterSectionHeader(panel, top, U"Unique Skill: Build Mill", U"Cost and placement tuning for infantry");
				drawSkillCard(0, (sliderBase + 40), buildParameters.manaCost,
					MillParameterEditorSpec{ U"Mana Cost", U"", 0.0, 200.0, 1.0, 5.0, 10.0, 1.0, 0 },
					U"Mill 建築時に消費する魔力です。建築の回転率と他行動との競合を調整できます。");
				drawSkillCard(1, (sliderBase + 41), buildParameters.gunpowderCost,
					MillParameterEditorSpec{ U"Gunpowder", U"", 0.0, 200.0, 1.0, 5.0, 10.0, 1.0, 0 },
					U"Mill 建築時に消費する火薬です。偵察や爆破とのリソース競合を調整できます。");
				drawSkillCard(2, (sliderBase + 42), buildParameters.forwardOffset,
					MillParameterEditorSpec{ U"Build Offset", U"", 1.0, 10.0, 0.1, 0.5, 1.0, 0.05, 2 },
					U"歩兵の前方どれくらいの位置に Mill を建てるかです。近すぎると建てにくく、遠すぎると前に出やすくなります。");
				return;

			case UniqueSkillType::Heal:
				ClampHealSkillParameters(healParameters);
				DrawUnitParameterSectionHeader(panel, top, U"Unique Skill: Heal", U"Area heal tuning for arcane infantry");
				drawSkillCard(0, (sliderBase + 43), healParameters.manaCost,
					MillParameterEditorSpec{ U"Mana Cost", U"", 0.0, 200.0, 1.0, 5.0, 10.0, 1.0, 0 },
					U"回復発動時に消費する魔力です。継戦能力と量産性のバランスに効きます。");
				drawSkillCard(1, (sliderBase + 44), healParameters.radius,
					MillParameterEditorSpec{ U"Radius", U"", 0.5, 12.0, 0.1, 0.5, 1.0, 0.05, 2 },
					U"回復が届く範囲です。前線をまとめて維持できるかどうかに影響します。");
				drawSkillCard(2, (sliderBase + 45), healParameters.amount,
					MillParameterEditorSpec{ U"Heal Amount", U"", 1.0, 200.0, 1.0, 5.0, 10.0, 1.0, 0 },
					U"1回の回復で戻す耐久量です。高すぎると耐久戦が長引きやすくなります。");
				return;

			case UniqueSkillType::Scout:
				ClampScoutSkillParameters(scoutParameters);
				DrawUnitParameterSectionHeader(panel, top, U"Unique Skill: Scout", U"Recon tuning for vehicle units");
				drawSkillCard(0, (sliderBase + 46), scoutParameters.gunpowderCost,
					MillParameterEditorSpec{ U"Gunpowder", U"", 0.0, 200.0, 1.0, 5.0, 10.0, 1.0, 0 },
					U"偵察発動時に消費する火薬です。爆破との使い分けに影響します。");
				drawSkillCard(1, (sliderBase + 47), scoutParameters.durationSeconds,
					MillParameterEditorSpec{ U"Duration", U"s", 0.1, 30.0, 0.1, 0.5, 1.0, 0.05, 2 },
					U"視界拡張が続く秒数です。長いほど偵察の操作頻度が下がります。");
				drawSkillCard(2, (sliderBase + 48), scoutParameters.visionMultiplier,
					MillParameterEditorSpec{ U"Vision x", U"", 1.0, 4.0, 0.05, 0.1, 0.25, 0.05, 2 },
					U"偵察中に元の視界を何倍にするかです。高いほど広範囲を一度に暴けます。");
				return;
			}
		}
	}
}
