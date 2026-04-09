# include "SkyAppUiParameterEditorInternal.hpp"
# include "MainSettings.hpp"
# include "MainUi.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
	void DrawUnitEditor(const SkyAppPanels& panels,
     const bool uiEditMode,
		UnitEditorSettings& unitEditorSettings,
		UnitEditorSection& activeSection,
		Array<SpawnedSapper>& spawnedSappers,
		Array<SpawnedSapper>& enemySappers,
		TimedMessage& unitEditorMessage)
	{
		using namespace UiParameterEditorDetail;

       const Rect listPanel = panels.unitEditorList;
		const Rect detailPanel = panels.unitEditor;
       UiInternal::DrawPanelFrame(listPanel, uiEditMode ? U"Target Units [Drag]" : U"Target Units", ColorF{ 0.98, 0.95 });
		UiInternal::DrawPanelFrame(detailPanel, uiEditMode ? U"Unit Parameters [Drag]" : U"Unit Parameters", ColorF{ 0.98, 0.95 });

		if (uiEditMode)
		{
			SimpleGUI::GetFont()(U"Drag either panel to move the editor").draw((listPanel.x + 16), (listPanel.y + 34), ColorF{ 0.18 });
			return;
		}

		SimpleGUI::GetFont()(U"Select target").draw((listPanel.x + 16), (listPanel.y + 34), ColorF{ 0.18 });

		const Array<UnitEditorSection> sections{
			UnitEditorSection::PlayerInfantry,
			UnitEditorSection::PlayerArcaneInfantry,
			UnitEditorSection::EnemyInfantry,
			UnitEditorSection::EnemyArcaneInfantry,
		};
		const auto countUnits = [&](const UnitEditorSection section)
			{
				const UnitTeam team = ToUnitEditorTeam(section);
				const SapperUnitType unitType = ToUnitEditorUnitType(section);
				const Array<SpawnedSapper>& sappers = ((team == UnitTeam::Player) ? spawnedSappers : enemySappers);
				int32 count = 0;
				for (const auto& sapper : sappers)
				{
					if ((0.0 < sapper.hitPoints) && (sapper.unitType == unitType))
					{
						++count;
					}
				}
				return count;
			};
		const auto drawTargetButton = [&](const Rect& rect, const UnitEditorSection section)
			{
				const bool selected = (activeSection == section);
				const bool hovered = rect.mouseOver();
				rect.draw(selected ? ColorF{ 0.33, 0.53, 0.82 } : (hovered ? ColorF{ 0.94, 0.95, 0.98 } : ColorF{ 0.98, 0.97, 0.95 }))
					.drawFrame(1, 0, selected ? ColorF{ 0.20, 0.32, 0.52 } : ColorF{ 0.58, 0.56, 0.52 });
				SimpleGUI::GetFont()(ToUnitEditorSectionLabel(section)).draw((rect.x + 12), (rect.y + 8), selected ? ColorF{ 0.98 } : ColorF{ 0.14 });
				SimpleGUI::GetFont()(U"live {}"_fmt(countUnits(section))).draw((rect.x + 12), (rect.y + 30), selected ? ColorF{ 0.96 } : ColorF{ 0.28 });
				if (hovered && MouseL.down())
				{
					activeSection = section;
				}
			};

		for (size_t i = 0; i < sections.size(); ++i)
		{
			drawTargetButton(Rect{ (listPanel.x + 12), (listPanel.y + 58 + static_cast<int32>(i) * 60), (listPanel.w - 24), 52 }, sections[i]);
		}

        const UnitTeam team = ToUnitEditorTeam(activeSection);
		const SapperUnitType unitType = ToUnitEditorUnitType(activeSection);
		UnitParameters& parameters = GetUnitParameters(unitEditorSettings, team, unitType);
		ClampUnitParameters(parameters);
     Rect{ detailPanel.x, (detailPanel.bottomY() - 46), detailPanel.w, 1 }.draw(ColorF{ 0.80, 0.78, 0.72 });
		SimpleGUI::GetFont()(ToUnitEditorSectionLabel(activeSection)).draw((detailPanel.x + 16), (detailPanel.y + 38), ColorF{ 0.16 });
		DrawMovementTypeSelector(detailPanel, (detailPanel.y + 70), parameters.movementType);
		DrawUnitParameterRows(detailPanel, ToUnitEditorSliderBase(activeSection), parameters);
		ClampUnitParameters(parameters);

		const Rect resetButton{ (detailPanel.x + 16), (detailPanel.y + detailPanel.h - 36), 92, 28 };
		const Rect applyButton{ (detailPanel.x + 116), (detailPanel.y + detailPanel.h - 36), 92, 28 };
		const Rect saveButton{ (detailPanel.x + detailPanel.w - 108), (detailPanel.y + detailPanel.h - 36), 92, 28 };

		if (DrawTextButton(resetButton, U"Reset"))
		{
			parameters = MakeDefaultUnitParameters(team, unitType);
			unitEditorMessage.show(U"ユニット設定を既定値に戻しました", 3.0);
		}

		if (DrawTextButton(applyButton, U"Apply"))
		{
			ApplyUnitParametersToSpawned((team == UnitTeam::Player) ? spawnedSappers : enemySappers, team, unitType, parameters);
			unitEditorMessage.show(U"出撃中ユニットへ反映しました", 3.0);
		}

		if (DrawTextButton(saveButton, U"Save TOML"))
		{
			unitEditorMessage.show(SaveUnitEditorSettings(unitEditorSettings) ? U"Unit 設定を保存" : U"Unit 設定保存失敗", 3.0);
		}

		if (unitEditorMessage.isVisible())
		{
			SimpleGUI::GetFont()(unitEditorMessage.text).draw((detailPanel.x + 16), (detailPanel.y + detailPanel.h - 66), ColorF{ 0.12 });
		}
	}
}
