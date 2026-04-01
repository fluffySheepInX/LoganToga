# include "FormationScene.h"
# include "FormationUi.h"
# include "MenuUi.h"

void FormationScene::draw() const
{
	DrawPanels();
	DrawHeader();
	DrawUnitList();
	DrawSlots();
	DrawPresets();
	DrawBottomButtons();
}

void FormationScene::DrawPanels() const
{
	const RectF panel{ Arg::center = Scene::Center(), 980, 620 };
	const RectF rosterPanel{ 150, 130, 290, 360 };
	const RectF slotPanel{ 500, 130, 460, 420 };
	const RectF presetPanel{ Arg::center = Scene::Center().movedBy(0, 182), 780, 96 };

	panel.rounded(24).draw(ColorF{ 0.06, 0.09, 0.16, 0.92 });
	panel.rounded(24).drawFrame(2, ColorF{ 0.82, 0.88, 1.0, 0.72 });
	rosterPanel.rounded(18).draw(ColorF{ 0.10, 0.14, 0.24, 0.90 });
	rosterPanel.rounded(18).drawFrame(2, ColorF{ 0.72, 0.80, 0.98, 0.56 });
	slotPanel.rounded(18).draw(ColorF{ 0.10, 0.14, 0.24, 0.90 });
	slotPanel.rounded(18).drawFrame(2, ColorF{ 0.72, 0.80, 0.98, 0.56 });
	presetPanel.rounded(18).draw(ColorF{ 0.10, 0.14, 0.24, 0.90 });
	presetPanel.rounded(18).drawFrame(2, ColorF{ 0.72, 0.80, 0.98, 0.56 });
}

void FormationScene::DrawHeader() const
{
	m_titleFont(U"編成").drawAt(Scene::Center().movedBy(0, -258), Palette::White);
	m_infoFont(U"左でユニットを選択し、右の8スロットに左クリックで格納").drawAt(Scene::Center().movedBy(0, -212), ColorF{ 0.82, 0.88, 1.0, 0.90 });
	m_infoFont(U"右クリックで空に戻す / ランダム・クリアあり / 確定とプリセット保存は次回起動でも維持").drawAt(Scene::Center().movedBy(0, -184), ColorF{ 0.82, 0.88, 1.0, 0.72 });
	m_buttonFont(U"ユニット一覧").drawAt(RectF{ 150, 130, 290, 360 }.center().movedBy(0, -145), Palette::White);
	m_buttonFont(U"スロット").drawAt(RectF{ 500, 130, 460, 420 }.center().movedBy(0, -145), Palette::White);
	m_buttonFont(U"プリセット").drawAt(RectF{ Arg::center = Scene::Center().movedBy(0, 182), 780, 96 }.center().movedBy(0, -28), Palette::White);
	DrawMenuButton(GetUnitEditButton(), m_infoFont, U"選択中を編集");
	DrawMenuButton(GetWaveEditButton(), m_infoFont, U"Wave編集");
	DrawMenuButton(GetRandomButton(), m_infoFont, U"ランダム");
	DrawMenuButton(GetClearButton(), m_infoFont, U"クリア");
}

void FormationScene::DrawUnitList() const
{
	const auto& unitTypes = GetFormationUnitTypes();

	for (size_t index = 0; index < unitTypes.size(); ++index)
	{
		DrawFormationUnitButton(GetUnitButton(index), m_buttonFont, m_infoFont, ff::GetUnitDefinition(unitTypes[index]), (m_editingFormation.selectedUnit == unitTypes[index]));
	}
}

void FormationScene::DrawSlots() const
{
	for (size_t index = 0; index < m_editingFormation.slots.size(); ++index)
	{
		const RectF slotRect = GetSlotButton(index);
		const bool hovered = slotRect.mouseOver();
		const Optional<ff::UnitId>& unit = m_editingFormation.slots[index];
		const ColorF frameColor = unit ? GetAllyBehaviorColor(*unit) : ColorF{ 0.70, 0.78, 0.92, 0.58 };

		slotRect.rounded(14).draw(hovered ? ColorF{ 0.18, 0.22, 0.34, 0.96 } : ColorF{ 0.12, 0.16, 0.27, 0.92 });
		slotRect.rounded(14).drawFrame(2, frameColor);
		m_infoFont(GetFormationSlotLabel(unit, index)).drawAt(slotRect.center().movedBy(0, -8), Palette::White);

		if (unit)
		{
			m_infoFont(U"Cost {}"_fmt(ff::GetSummonCost(*unit))).drawAt(slotRect.center().movedBy(0, 14), ColorF{ 0.90, 0.94, 1.0, 0.80 });
		}
		else
		{
			m_infoFont(U"左クリックで格納").drawAt(slotRect.center().movedBy(0, 14), ColorF{ 0.90, 0.94, 1.0, 0.60 });
		}
	}
}

void FormationScene::DrawPresets() const
{
	for (size_t index = 0; index < getData().formationPresets.size(); ++index)
	{
		const RectF presetCard = GetPresetCard(index);
		const int32 unitCount = ff::CountAssignedFormationUnits(getData().formationPresets[index]);

		presetCard.rounded(14).draw(ColorF{ 0.12, 0.16, 0.27, 0.92 });
		presetCard.rounded(14).drawFrame(2, ColorF{ 0.70, 0.78, 0.92, 0.58 });
		m_infoFont(U"Preset {}"_fmt(index + 1)).drawAt(presetCard.center().movedBy(0, -24), Palette::White);
		m_infoFont(U"{} / 8 slots"_fmt(unitCount)).drawAt(presetCard.center().movedBy(0, -2), ColorF{ 0.84, 0.90, 1.0, 0.74 });

		DrawMenuButton(GetPresetLoadButton(index), m_infoFont, U"読込");
		DrawMenuButton(GetPresetSaveButton(index), m_infoFont, U"保存");
	}
}

void FormationScene::DrawBottomButtons() const
{
	DrawMenuButton(GetConfirmButton(), m_buttonFont, U"確定");
	DrawMenuButton(GetBackButton(), m_buttonFont, U"戻る");
}
