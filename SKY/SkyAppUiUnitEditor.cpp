# include "SkyAppUiUnitEditorInternal.hpp"
# include "MainSettings.hpp"
# include "MainUi.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
 namespace
	{
		[[nodiscard]] Array<String> WrapTooltipText(const StringView text, const double maxWidth)
		{
			const Font& font = SimpleGUI::GetFont();
			Array<String> lines;
			String current;

			for (const char32 ch : text)
			{
				if (ch == U'\n')
				{
					lines << current;
					current.clear();
					continue;
				}

				const String candidate = (current + ch);
				if ((not current.isEmpty()) && (font(candidate).region().w > maxWidth))
				{
					lines << current;
					current = String{ ch };
				}
				else
				{
					current = candidate;
				}
			}

			if (not current.isEmpty())
			{
				lines << current;
			}

			if (lines.isEmpty())
			{
				lines << U"";
			}

			return lines;
		}
	}

	void DrawUnitEditor(const SkyAppPanels& panels,
     const bool uiEditMode,
		UnitEditorSettings& unitEditorSettings,
       UnitEditorSelection& activeSelection,
       UnitEditorPage& activePage,
		Array<SpawnedSapper>& spawnedSappers,
		Array<SpawnedSapper>& enemySappers,
		TimedMessage& unitEditorMessage)
	{
		using namespace UiParameterEditorDetail;

       const Rect listPanel = panels.unitEditorList;
		const Rect detailPanel = panels.unitEditor;
      UiInternal::DrawNinePatchPanelFrame(listPanel, uiEditMode ? U"Target Units [Drag]" : U"Target Units", ColorF{ 0.98, 0.95 }, UiInternal::DefaultPanelFrameColor, UiInternal::DefaultPanelTitleColor, MainSupport::PanelSkinTarget::UnitEditor);
		UiInternal::DrawNinePatchPanelFrame(detailPanel, uiEditMode ? U"Unit Parameters [Drag]" : U"Unit Parameters", ColorF{ 0.98, 0.95 }, UiInternal::DefaultPanelFrameColor, UiInternal::DefaultPanelTitleColor, MainSupport::PanelSkinTarget::UnitEditor);

		if (uiEditMode)
		{
         SimpleGUI::GetFont()(U"Drag either panel to move the editor").draw((listPanel.x + 16), (listPanel.y + 34), UiInternal::EditorTextOnLightSecondaryColor());
			return;
		}

        SimpleGUI::GetFont()(U"Select target").draw((listPanel.x + 16), (listPanel.y + 34), UiInternal::EditorTextOnLightSecondaryColor());

        Array<UnitEditorSelection> selections;
		selections.reserve(GetUnitDefinitions().size() * 2);
		for (const UnitTeam team : { UnitTeam::Player, UnitTeam::Enemy })
		{
			for (const auto& definition : GetUnitDefinitions())
			{
				selections << UnitEditorSelection{ .team = team, .unitType = definition.unitType };
			}
		}

		const auto countUnits = [&](const UnitEditorSelection& selection)
			{
                const Array<SpawnedSapper>& sappers = ((selection.team == UnitTeam::Player) ? spawnedSappers : enemySappers);
				int32 count = 0;
				for (const auto& sapper : sappers)
				{
                  if ((0.0 < sapper.hitPoints) && (sapper.unitType == selection.unitType))
					{
						++count;
					}
				}
				return count;
			};
      String hoveredDescription;
		Optional<Rect> hoveredRect;
        const auto drawTargetButton = [&](const Rect& rect, const UnitEditorSelection& selection)
			{
               const bool selected = (activeSelection == selection);
				const bool hovered = rect.mouseOver();
				rect.draw(selected ? ColorF{ 0.33, 0.53, 0.82 } : (hovered ? ColorF{ 0.94, 0.95, 0.98 } : ColorF{ 0.98, 0.97, 0.95 }))
					.drawFrame(1, 0, selected ? ColorF{ 0.20, 0.32, 0.52 } : ColorF{ 0.58, 0.56, 0.52 });
             SimpleGUI::GetFont()(ToUnitEditorSectionLabel(selection.team, selection.unitType)).draw((rect.x + 12), (rect.y + 8), selected ? UiInternal::EditorTextOnSelectedPrimaryColor() : UiInternal::EditorTextOnCardPrimaryColor());
				SimpleGUI::GetFont()(U"live {}"_fmt(countUnits(selection))).draw((rect.x + 12), (rect.y + 30), selected ? UiInternal::EditorTextOnSelectedSecondaryColor() : UiInternal::EditorTextOnCardSecondaryColor());
               if (hovered)
				{
					hoveredDescription = U"編集対象のユニット種別です。Reset / Apply / Save はこの選択中ユニット設定に対して作用します。";
                   hoveredRect = rect;
				}
				if (hovered && MouseL.down())
				{
                    activeSelection = selection;
				}
			};

        for (size_t i = 0; i < selections.size(); ++i)
		{
         drawTargetButton(Rect{ (listPanel.x + 12), (listPanel.y + 58 + static_cast<int32>(i) * 60), (listPanel.w - 24), 52 }, selections[i]);
		}

      const UnitTeam team = activeSelection.team;
		const SapperUnitType unitType = activeSelection.unitType;
		UnitParameters& parameters = GetUnitParameters(unitEditorSettings, team, unitType);
        ExplosionSkillParameters& explosionSkillParameters = GetExplosionSkillParameters(unitEditorSettings, team, unitType);
        BuildMillSkillParameters& buildMillSkillParameters = GetBuildMillSkillParameters(unitEditorSettings, team, unitType);
		HealSkillParameters& healSkillParameters = GetHealSkillParameters(unitEditorSettings, team, unitType);
		ScoutSkillParameters& scoutSkillParameters = GetScoutSkillParameters(unitEditorSettings, team, unitType);
		const UniqueSkillType uniqueSkillType = GetUnitUniqueSkillType(unitType);
        const bool explosionSkillSupported = CanUnitUseExplosionSkill(unitType);
		ClampUnitParameters(parameters);
        ClampExplosionSkillParameters(explosionSkillParameters);
     Rect{ detailPanel.x, (detailPanel.bottomY() - 46), detailPanel.w, 1 }.draw(ColorF{ 0.80, 0.78, 0.72 });
       const Rect selectedUnitRect{ (detailPanel.x + 12), (detailPanel.y + 34), (detailPanel.w - 24), 34 };
		selectedUnitRect.rounded(10).draw(ColorF{ 0.92, 0.96, 1.0, 0.82 });
		selectedUnitRect.rounded(10).drawFrame(1, 0, ColorF{ 0.74, 0.80, 0.90, 0.90 });
        SimpleGUI::GetFont()(ToUnitEditorSectionLabel(team, unitType)).draw((selectedUnitRect.x + 12), (selectedUnitRect.y + 6), UiInternal::EditorTextOnCardPrimaryColor());

		const Array<UnitEditorPage> pages{
			UnitEditorPage::Basic,
			UnitEditorPage::Combat,
			UnitEditorPage::Footprint,
          UnitEditorPage::Skill,
		};
       const int32 pageButtonGap = 8;
		const int32 pageButtonWidth = Max(56, ((detailPanel.w - 32 - pageButtonGap * (static_cast<int32>(pages.size()) - 1)) / static_cast<int32>(pages.size())));
		for (size_t i = 0; i < pages.size(); ++i)
		{
			const UnitEditorPage page = pages[i];
          const bool enabled = true;
			const Rect pageButton{ (detailPanel.x + 16 + static_cast<int32>(i) * (pageButtonWidth + pageButtonGap)), (detailPanel.y + 78), pageButtonWidth, 28 };
			const bool selected = (activePage == page);
			const bool hovered = pageButton.mouseOver();
            pageButton.draw(enabled
				? (selected ? ColorF{ 0.33, 0.53, 0.82 } : (hovered ? ColorF{ 0.94, 0.95, 0.98 } : ColorF{ 0.98, 0.97, 0.95 }))
				: ColorF{ 0.90, 0.90, 0.92 })
				.drawFrame(1, 0, enabled
					? (selected ? ColorF{ 0.20, 0.32, 0.52 } : ColorF{ 0.58, 0.56, 0.52 })
					: ColorF{ 0.68, 0.68, 0.72 });
		   SimpleGUI::GetFont()(ToUnitEditorPageLabel(page)).drawAt(pageButton.center(), enabled
				? (selected ? UiInternal::EditorTextOnSelectedPrimaryColor() : UiInternal::EditorTextOnCardPrimaryColor())
				: ColorF{ 0.45, 0.45, 0.48 });
           if (hovered)
			{
               hoveredRect = pageButton;
				switch (page)
				{
				case UnitEditorPage::Combat:
					hoveredDescription = U"戦闘性能の調整ページです。射程、停止距離、ダメージ、攻撃間隔を編集します。";
					break;

				case UnitEditorPage::Footprint:
					hoveredDescription = U"衝突・接触判定の調整ページです。大型ユニットや車の当たり判定調整に使います。";
					break;

				case UnitEditorPage::Skill:
                    hoveredDescription = explosionSkillSupported
						? U"スキルページです。共通の爆破スキルと、このユニット固有スキルの数値を編集します。Player / Enemy は別々に調整されます。"
						: U"スキルページです。このユニットの固有スキル数値を編集します。爆破非対応ユニットでも固有スキルは調整できます。";
					break;

				case UnitEditorPage::Basic:
				default:
					hoveredDescription = U"基本性能の調整ページです。耐久、移動速度、出撃コストを編集します。";
					break;
				}
			}
           if (enabled && hovered && MouseL.down())
			{
				activePage = page;
			}
		}

        UnitEditorDetail::DrawUnitEditorSetupSection(detailPanel,
			unitEditorSettings,
			team,
			unitType,
			activePage,
			parameters,
			hoveredDescription,
			hoveredRect,
			unitEditorMessage);

      const int32 parameterRowsTop = (activePage == UnitEditorPage::Basic)
			? ((team == UnitTeam::Enemy) ? (detailPanel.y + 280) : (detailPanel.y + 244))
         : ((activePage == UnitEditorPage::Footprint) ? (detailPanel.y + 176) : (detailPanel.y + 146));
		if (activePage == UnitEditorPage::Skill)
		{
			DrawExplosionSkillParameterRows(detailPanel, ToUnitEditorSliderBase(team, unitType), explosionSkillParameters, parameterRowsTop, hoveredDescription, hoveredRect);
           DrawUniqueSkillParameterRows(detailPanel,
				ToUnitEditorSliderBase(team, unitType),
				uniqueSkillType,
				buildMillSkillParameters,
				healSkillParameters,
				scoutSkillParameters,
				(parameterRowsTop + 388),
				hoveredDescription,
				hoveredRect);
		}
		else
		{
			DrawUnitParameterRows(detailPanel, ToUnitEditorSliderBase(team, unitType), parameters, activePage, parameterRowsTop, hoveredDescription, hoveredRect);
		}
		ClampUnitParameters(parameters);
		ClampExplosionSkillParameters(explosionSkillParameters);
		ClampBuildMillSkillParameters(buildMillSkillParameters);
		ClampHealSkillParameters(healSkillParameters);
		ClampScoutSkillParameters(scoutSkillParameters);

       UnitEditorDetail::DrawUnitEditorFooter(detailPanel,
			unitEditorSettings,
			team,
			unitType,
			activePage,
			parameters,
			explosionSkillParameters,
			buildMillSkillParameters,
			healSkillParameters,
			scoutSkillParameters,
			spawnedSappers,
			enemySappers,
			hoveredDescription,
			hoveredRect,
			unitEditorMessage);

		if (hoveredRect && (not hoveredDescription.isEmpty()))
		{
			const int32 tooltipWidth = 300;
			const int32 tooltipGap = 12;
          const Array<String> tooltipLines = WrapTooltipText(hoveredDescription, (tooltipWidth - 24));
			const int32 tooltipHeight = Max(76, (40 + static_cast<int32>(tooltipLines.size()) * 22));
			int32 tooltipX = (hoveredRect->x - tooltipWidth - tooltipGap);
			if (tooltipX < 8)
			{
				tooltipX = (hoveredRect->rightX() + tooltipGap);
			}
			const int32 tooltipY = Clamp((hoveredRect->y + hoveredRect->h / 2 - tooltipHeight / 2), 8, (Scene::Height() - tooltipHeight - 8));
			const Rect tooltipRect{ tooltipX, tooltipY, tooltipWidth, tooltipHeight };
			tooltipRect.draw(ColorF{ 0.97, 0.98, 1.0, 0.96 })
				.drawFrame(2, 0, ColorF{ 0.70, 0.78, 0.88, 0.95 });
         SimpleGUI::GetFont()(U"Help").draw((tooltipRect.x + 12), (tooltipRect.y + 8), UiInternal::EditorTextOnCardSecondaryColor());
         for (size_t i = 0; i < tooltipLines.size(); ++i)
			{
               SimpleGUI::GetFont()(tooltipLines[i]).draw((tooltipRect.x + 12), (tooltipRect.y + 32 + static_cast<int32>(i) * 22), UiInternal::EditorTextOnCardPrimaryColor());
			}
		}
	}
}
