# include "FormationScene.h"
# include "FormationUi.h"
# include "MenuUi.h"

namespace
{
	int32 CountAssignedUnits(const Array<Optional<ff::AllyBehavior>>& slots)
	{
		return static_cast<int32>(std::count_if(slots.begin(), slots.end(), [](const auto& slot)
			{
				return static_cast<bool>(slot);
			}));
	}

	Array<Optional<ff::AllyBehavior>> MakeRandomFormationSlots()
	{
		const auto& unitTypes = GetFormationUnitTypes();
		Array<Optional<ff::AllyBehavior>> slots = ff::MakeEmptyFormationSlots();

		for (auto& slot : slots)
		{
			slot = unitTypes[Random(unitTypes.size() - 1)];
		}

		return slots;
	}

	void ClearFormationSlots(Array<Optional<ff::AllyBehavior>>& slots)
	{
		for (auto& slot : slots)
		{
			slot.reset();
		}
	}
}

FormationScene::FormationScene(const InitData& init)
	: App::Scene{ init }
	, m_titleFont{ 40, Typeface::Heavy }
	, m_buttonFont{ 22 }
	, m_infoFont{ 18 }
   , m_editingFormationSlots{ getData().formationSlots }
	, m_selectedFormationUnit{ getData().selectedFormationUnit }
{
	Scene::SetBackground(ColorF{ 0.08, 0.11, 0.18 });
}

void FormationScene::update()
{
  if (KeyEscape.down() || GetBackButton().leftClicked())
	{
		changeScene(U"Title");
		return;
	}

	if (GetConfirmButton().leftClicked())
	{
		getData().formationSlots = m_editingFormationSlots;
		getData().selectedFormationUnit = m_selectedFormationUnit;
      SaveAppDataToDisk(getData());
		changeScene(U"Title");
		return;
	}

	if (GetRandomButton().leftClicked())
	{
		m_editingFormationSlots = MakeRandomFormationSlots();
		return;
	}

	if (GetClearButton().leftClicked())
	{
		ClearFormationSlots(m_editingFormationSlots);
		return;
	}

	for (size_t index = 0; index < getData().formationPresets.size(); ++index)
	{
		if (GetPresetLoadButton(index).leftClicked())
		{
			m_editingFormationSlots = getData().formationPresets[index];
			return;
		}

		if (GetPresetSaveButton(index).leftClicked())
		{
			getData().formationPresets[index] = m_editingFormationSlots;
         SaveAppDataToDisk(getData());
			return;
		}
	}

	const auto& unitTypes = GetFormationUnitTypes();

	for (size_t index = 0; index < unitTypes.size(); ++index)
	{
		if (GetUnitButton(index).leftClicked())
		{
         m_selectedFormationUnit = unitTypes[index];
			return;
		}
	}

    for (size_t index = 0; index < m_editingFormationSlots.size(); ++index)
	{
      if (GetSlotButton(index).leftClicked() && m_selectedFormationUnit)
		{
          m_editingFormationSlots[index] = m_selectedFormationUnit;
			return;
		}

		if (GetSlotButton(index).rightClicked())
		{
            m_editingFormationSlots[index].reset();
			return;
		}
	}
}

void FormationScene::draw() const
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

	m_titleFont(U"編成").drawAt(Scene::Center().movedBy(0, -258), Palette::White);
	m_infoFont(U"左でユニットを選択し、右の8スロットに左クリックで格納").drawAt(Scene::Center().movedBy(0, -212), ColorF{ 0.82, 0.88, 1.0, 0.90 });
    m_infoFont(U"右クリックで空に戻す / ランダム・クリアあり / 確定とプリセット保存は次回起動でも維持").drawAt(Scene::Center().movedBy(0, -184), ColorF{ 0.82, 0.88, 1.0, 0.72 });
	m_buttonFont(U"ユニット一覧").drawAt(rosterPanel.center().movedBy(0, -145), Palette::White);
	m_buttonFont(U"スロット").drawAt(slotPanel.center().movedBy(0, -145), Palette::White);
	m_buttonFont(U"プリセット").drawAt(presetPanel.center().movedBy(0, -28), Palette::White);
	DrawMenuButton(GetRandomButton(), m_infoFont, U"ランダム");
	DrawMenuButton(GetClearButton(), m_infoFont, U"クリア");

	const auto& unitTypes = GetFormationUnitTypes();

	for (size_t index = 0; index < unitTypes.size(); ++index)
	{
        DrawFormationUnitButton(GetUnitButton(index), m_buttonFont, m_infoFont, unitTypes[index], (m_selectedFormationUnit == unitTypes[index]));
	}

    for (size_t index = 0; index < m_editingFormationSlots.size(); ++index)
	{
		const RectF slotRect = GetSlotButton(index);
		const bool hovered = slotRect.mouseOver();
       const Optional<ff::AllyBehavior>& unit = m_editingFormationSlots[index];
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

	for (size_t index = 0; index < getData().formationPresets.size(); ++index)
	{
		const RectF presetCard = GetPresetCard(index);
		const int32 unitCount = CountAssignedUnits(getData().formationPresets[index]);

		presetCard.rounded(14).draw(ColorF{ 0.12, 0.16, 0.27, 0.92 });
		presetCard.rounded(14).drawFrame(2, ColorF{ 0.70, 0.78, 0.92, 0.58 });
		m_infoFont(U"Preset {}"_fmt(index + 1)).drawAt(presetCard.center().movedBy(0, -24), Palette::White);
		m_infoFont(U"{} / 8 slots"_fmt(unitCount)).drawAt(presetCard.center().movedBy(0, -2), ColorF{ 0.84, 0.90, 1.0, 0.74 });

		DrawMenuButton(GetPresetLoadButton(index), m_infoFont, U"読込");
		DrawMenuButton(GetPresetSaveButton(index), m_infoFont, U"保存");
	}

   DrawMenuButton(GetConfirmButton(), m_buttonFont, U"確定");
	DrawMenuButton(GetBackButton(), m_buttonFont, U"戻る");
}

RectF FormationScene::GetUnitButton(const size_t index) const
{
 return RectF{ 185, (175 + (index * 62)), 220, 56 };
}

RectF FormationScene::GetSlotButton(const size_t index) const
{
	const double width = 190;
	const double spacingX = 20;
	const double spacingY = 20;
	const size_t column = (index % 2);
	const size_t row = (index / 2);
	return RectF{ (540 + (column * (width + spacingX))), (195 + (row * (52 + spacingY))), width, 52 };
}

RectF FormationScene::GetBackButton() const
{
 return RectF{ Arg::center = Scene::Center().movedBy(120, 274), 220, 52 };
}

RectF FormationScene::GetConfirmButton() const
{
	return RectF{ Arg::center = Scene::Center().movedBy(-120, 274), 220, 52 };
}

RectF FormationScene::GetPresetCard(const size_t index) const
{
	return RectF{ Arg::center = Scene::Center().movedBy((-260 + (260 * static_cast<double>(index))), 182), 220, 72 };
}

RectF FormationScene::GetPresetLoadButton(const size_t index) const
{
	return RectF{ Arg::center = GetPresetCard(index).center().movedBy(-52, 20), 92, 30 };
}

RectF FormationScene::GetPresetSaveButton(const size_t index) const
{
	return RectF{ Arg::center = GetPresetCard(index).center().movedBy(52, 20), 92, 30 };
}

RectF FormationScene::GetRandomButton() const
{
	return RectF{ 548, 154, 148, 34 };
}

RectF FormationScene::GetClearButton() const
{
	return RectF{ 764, 154, 148, 34 };
}
