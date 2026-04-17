# include "SkyAppUiParameterEditorInternal.hpp"
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

		[[nodiscard]] String ToModelReferenceLabel(const UnitEditorSettings& unitEditorSettings, const UnitTeam team, const SapperUnitType unitType)
		{
			const FilePath& modelPath = GetUnitModelPath(unitEditorSettings, team, unitType);

			if (modelPath.isEmpty())
			{
				return U"Default: {}"_fmt(GetUnitRenderModelLabel(GetUnitRenderModel(team, unitType)));
			}

			return FileSystem::FileName(modelPath);
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

		if (activePage == UnitEditorPage::Basic)
		{
         SimpleGUI::GetFont()(U"Setup").draw((detailPanel.x + 16), (detailPanel.y + 118), UiInternal::EditorTextOnLightSecondaryColor());
			Rect{ (detailPanel.x + 86), (detailPanel.y + 130), (detailPanel.w - 102), 1 }.draw(ColorF{ 0.78, 0.82, 0.88 });
			DrawMovementTypeSelector(detailPanel, (detailPanel.y + 138), parameters.movementType);
			   const Rect typeLabelRect{ (detailPanel.x + 16), (detailPanel.y + 134), 88, 28 };
			const Rect infantryButton{ (detailPanel.x + detailPanel.w - 146), (detailPanel.y + 136), 64, 24 };
			const Rect tankButton{ (detailPanel.x + detailPanel.w - 74), (detailPanel.y + 136), 58, 24 };
			if (typeLabelRect.mouseOver())
			{
				hoveredDescription = U"移動タイプです。Infantry は素直な歩兵挙動、Tank は車両向けの重い移動カーブになります。";
               hoveredRect = typeLabelRect;
			}
			else if (infantryButton.mouseOver())
			{
				hoveredDescription = U"歩兵向けの移動タイプです。小回りが利きやすく、細かい進路変更に向いています。";
               hoveredRect = infantryButton;
			}
			else if (tankButton.mouseOver())
			{
				hoveredDescription = U"車両向けの移動タイプです。動き出しと停止がやや重く、車らしい進行になります。";
               hoveredRect = tankButton;
			}

			if (team == UnitTeam::Enemy)
			{
              DrawUnitAiRoleSelector(detailPanel, (detailPanel.y + 174), parameters.aiRole);
				const Rect aiRoleLabelRect{ (detailPanel.x + 16), (detailPanel.y + 170), 96, 28 };
				const Rect secureButton{ (detailPanel.x + detailPanel.w - 236), (detailPanel.y + 172), 72, 24 };
				const Rect assaultButton{ (detailPanel.x + detailPanel.w - 156), (detailPanel.y + 172), 72, 24 };
				const Rect supportButton{ (detailPanel.x + detailPanel.w - 76), (detailPanel.y + 172), 64, 24 };

				if (aiRoleLabelRect.mouseOver())
				{
					hoveredDescription = U"敵 AI の役割です。資源確保を優先するか、拠点突撃を優先するか、味方支援に回るかを決めます。";
					hoveredRect = aiRoleLabelRect;
				}
				else if (secureButton.mouseOver())
				{
					hoveredDescription = U"資源エリア確保を優先する役割です。未確保やプレイヤー支配中の資源地点へ向かいやすくなります。";
					hoveredRect = secureButton;
				}
				else if (assaultButton.mouseOver())
				{
					hoveredDescription = U"敵拠点への攻勢を優先する役割です。資源よりもプレイヤー本拠地へ進軍しやすくなります。";
					hoveredRect = assaultButton;
				}
				else if (supportButton.mouseOver())
				{
					hoveredDescription = U"味方主力の近くで行動する支援役です。近い非 Support 味方を追従し、いなければ前線寄りの集結地点へ向かいます。";
					hoveredRect = supportButton;
				}
			}

			const int32 modelRowY = ((team == UnitTeam::Enemy) ? (detailPanel.y + 206) : (detailPanel.y + 172));
			const Rect modelLabelRect{ (detailPanel.x + 16), modelRowY, 90, 28 };
			const Rect modelPathRect{ (detailPanel.x + 108), modelRowY, Max(96, detailPanel.w - 260), 28 };
			const Rect browseButton{ (detailPanel.rightX() - 138), modelRowY, 58, 28 };
			const Rect clearButton{ (detailPanel.rightX() - 72), modelRowY, 56, 28 };
            FilePath& currentModelPath = GetUnitModelPath(unitEditorSettings, team, unitType);

			modelPathRect.rounded(6).draw(ColorF{ 0.99, 0.99, 1.0, 0.92 });
			modelPathRect.rounded(6).drawFrame(1, 0, ColorF{ 0.78, 0.80, 0.84 });
			SimpleGUI::GetFont()(U"Model").draw((modelLabelRect.x), (modelLabelRect.y + 4), UiInternal::EditorTextOnLightSecondaryColor());
			SimpleGUI::GetFont()(ToModelReferenceLabel(unitEditorSettings, team, unitType)).draw((modelPathRect.x + 10), (modelPathRect.y + 4), UiInternal::EditorTextOnCardPrimaryColor());

			if (modelLabelRect.mouseOver())
			{
				hoveredDescription = U"このユニット種別の描画に使うモデル参照です。Browse でモデルファイルを選ぶと、そのモデルを優先して描画します。";
				hoveredRect = modelPathRect;
			}
			else if (modelPathRect.mouseOver())
			{
				hoveredDescription = currentModelPath.isEmpty()
					? U"現在は既定モデルを使用しています。"
					: currentModelPath;
				hoveredRect = modelPathRect;
			}
			else if (browseButton.mouseOver())
			{
				hoveredDescription = U"ファイルダイアログを開いて、このユニット種別の描画モデルを選択します。";
				hoveredRect = browseButton;
			}
			else if (clearButton.mouseOver())
			{
				hoveredDescription = U"カスタムモデル指定を解除して、既定モデルへ戻します。";
				hoveredRect = clearButton;
			}

			if (DrawTextButton(browseButton, U"Browse"))
			{
				if (const auto selectedPath = Dialog::OpenFile({ FileFilter::AllFiles() }))
				{
					const UnitRenderModel renderModel = GetUnitRenderModel(team, unitType);
					const UnitModel previewModel{ *selectedPath, BirdDisplayHeight, GetUnitRenderModelProceduralAnimationType(renderModel) };

					if (previewModel.isLoaded())
					{
						currentModelPath = *selectedPath;
						unitEditorMessage.show(U"モデル参照を更新しました", 3.0);
					}
					else
					{
						unitEditorMessage.show(U"モデル読み込み失敗", 3.0);
					}
				}
			}

			if (DrawTextButton(clearButton, U"Clear"))
			{
				currentModelPath.clear();
				unitEditorMessage.show(U"既定モデルに戻しました", 3.0);
			}
		}
		else if (activePage == UnitEditorPage::Footprint)
		{
         SimpleGUI::GetFont()(U"Setup").draw((detailPanel.x + 16), (detailPanel.y + 118), UiInternal::EditorTextOnLightSecondaryColor());
			Rect{ (detailPanel.x + 86), (detailPanel.y + 130), (detailPanel.w - 102), 1 }.draw(ColorF{ 0.78, 0.82, 0.88 });
			DrawFootprintTypeSelector(detailPanel, (detailPanel.y + 138), parameters);
			   const Rect footprintLabelRect{ (detailPanel.x + 16), (detailPanel.y + 134), 120, 28 };
			const Rect circleButton{ (detailPanel.x + detailPanel.w - 152), (detailPanel.y + 136), 64, 24 };
			const Rect capsuleButton{ (detailPanel.x + detailPanel.w - 80), (detailPanel.y + 136), 64, 24 };
			if (footprintLabelRect.mouseOver())
			{
				hoveredDescription = U"接触判定の形です。Circle は丸、Capsule は前後に長い形で、車や横幅のある機体に向きます。";
               hoveredRect = footprintLabelRect;
			}
			else if (circleButton.mouseOver())
			{
				hoveredDescription = U"円形フットプリントです。歩兵などの小型ユニット向けで、扱いが単純です。";
               hoveredRect = circleButton;
			}
			else if (capsuleButton.mouseOver())
			{
				hoveredDescription = U"カプセル型フットプリントです。前後長を持つため、車両や縦長ユニットの接触判定を自然にできます。";
               hoveredRect = capsuleButton;
			}
		}

      const int32 parameterRowsTop = (activePage == UnitEditorPage::Basic)
			? ((team == UnitTeam::Enemy) ? (detailPanel.y + 246) : (detailPanel.y + 210))
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

		const Rect resetButton{ (detailPanel.x + 16), (detailPanel.y + detailPanel.h - 36), 92, 28 };
		const Rect applyButton{ (detailPanel.x + 116), (detailPanel.y + detailPanel.h - 36), 92, 28 };
       const Rect editorTextColorsButton{ (detailPanel.rightX() - 40), (detailPanel.y + detailPanel.h - 34), 24, 24 };
		const Rect saveButton{ (editorTextColorsButton.x - 100), (detailPanel.y + detailPanel.h - 36), 92, 28 };
		if (resetButton.mouseOver())
		{
           hoveredDescription = (activePage == UnitEditorPage::Skill)
                ? U"この選択中の陣営 / ユニット種別のスキル設定を既定値へ戻します。爆破と固有スキルの両方が対象です。"
				: U"この選択ユニット種別の値を既定値へ戻します。保存前でも editor 上の値は即時変更されます。";
           hoveredRect = resetButton;
		}
		else if (applyButton.mouseOver())
		{
           hoveredDescription = (activePage == UnitEditorPage::Skill)
                ? U"スキル設定は即時反映です。次回の使用時から、爆破と固有スキルのコスト・威力・効果時間・範囲へ反映されます。"
				: U"今いる同種ユニットへ設定を反映します。交戦距離や停止距離も次フレーム以降に更新されます。";
           hoveredRect = applyButton;
		}
		else if (saveButton.mouseOver())
		{
			hoveredDescription = U"現在の unit editor 設定を TOML に保存します。次回起動時もこの値を読み込みます。";
           hoveredRect = saveButton;
		}
		else if (editorTextColorsButton.mouseOver())
		{
			hoveredDescription = U"editor 系 UI の共通文字色設定を開きます。保存すると各 editor の文字色に反映されます。";
			hoveredRect = editorTextColorsButton;
		}

		if (DrawTextButton(resetButton, U"Reset"))
		{
         if (activePage == UnitEditorPage::Skill)
			{
				explosionSkillParameters = MakeDefaultExplosionSkillParameters(team, unitType);
              buildMillSkillParameters = MakeDefaultBuildMillSkillParameters(team, unitType);
				healSkillParameters = MakeDefaultHealSkillParameters(team, unitType);
				scoutSkillParameters = MakeDefaultScoutSkillParameters(team, unitType);
				unitEditorMessage.show(U"スキル設定を既定値に戻しました", 3.0);
			}
			else
			{
				parameters = MakeDefaultUnitParameters(team, unitType);
				GetUnitModelPath(unitEditorSettings, team, unitType).clear();
				unitEditorMessage.show(U"ユニット設定を既定値に戻しました", 3.0);
			}
		}

		if (DrawTextButton(applyButton, U"Apply"))
		{
           if (activePage == UnitEditorPage::Skill)
			{
             unitEditorMessage.show(U"スキル設定は即時反映です", 3.0);
			}
			else
			{
				ApplyUnitParametersToSpawned((team == UnitTeam::Player) ? spawnedSappers : enemySappers, team, unitType, parameters);
				unitEditorMessage.show(U"出撃中ユニットへ反映しました", 3.0);
			}
		}

		if (DrawTextButton(saveButton, U"Save TOML"))
		{
			unitEditorMessage.show(SaveUnitEditorSettings(unitEditorSettings) ? U"Unit 設定を保存" : U"Unit 設定保存失敗", 3.0);
		}

		if (UiInternal::DrawEditorIconButton(editorTextColorsButton, U"色"))
		{
			UiInternal::OpenSharedEditorTextColorEditor();
		}

		if (unitEditorMessage.isVisible())
		{
          SimpleGUI::GetFont()(unitEditorMessage.text).draw((detailPanel.x + 16), (detailPanel.y + detailPanel.h - 66), UiInternal::EditorTextOnLightPrimaryColor());
		}

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
