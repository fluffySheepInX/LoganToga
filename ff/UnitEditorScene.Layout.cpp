# include "UnitEditorScene.h"

RectF UnitEditorScene::GetUnitButton(const size_t index) const
{
 return RectF{ 58, (140 + (index * 68)), 200, 54 };
}

RectF UnitEditorScene::GetTopHelpIcon() const
{
   return RectF{ 1162, 46, 30, 30 };
}

RectF UnitEditorScene::GetUnitPanelHeaderIcon() const
{
   return RectF{ 58, 98, 28, 28 };
}

RectF UnitEditorScene::GetPreviewPanelHeaderIcon() const
{
   return RectF{ 310, 98, 28, 28 };
}

RectF UnitEditorScene::GetEditPanelHeaderIcon() const
{
   return RectF{ 666, 98, 28, 28 };
}

RectF UnitEditorScene::GetInfoPanelHeaderIcon() const
{
   return RectF{ 310, 370, 28, 28 };
}

RectF UnitEditorScene::GetUnitListHelpIcon() const
{
   return RectF{ 58, 586, 28, 28 };
}

RectF UnitEditorScene::GetPreviewLineIcon(const size_t index) const
{
  return RectF{ 318, (220 + (index * 30)), 24, 24 };
}

RectF UnitEditorScene::GetLabelFieldIcon() const
{
   return RectF{ 670, 132, 28, 28 };
}

RectF UnitEditorScene::GetRoleFieldIcon() const
{
   return RectF{ 670, 198, 28, 28 };
}

RectF UnitEditorScene::GetFieldRow(const size_t index) const
{
 return RectF{ 668, (250 + (index * 48)), 392, 40 };
}

RectF UnitEditorScene::GetFieldIconRect(const size_t index) const
{
	return RectF{ (GetFieldRow(index).x + 10), (GetFieldRow(index).y + 6), 26, 26 };
}

RectF UnitEditorScene::GetDecreaseButton(const size_t index) const
{
 return RectF{ 1072, (255 + (index * 48)), 28, 30 };
}

RectF UnitEditorScene::GetIncreaseButton(const size_t index) const
{
 return RectF{ 1108, (255 + (index * 48)), 28, 30 };
}

RectF UnitEditorScene::GetColorRow(const size_t index) const
{
 return RectF{ 668, (484 + (index * 30)), 206, 28 };
}

RectF UnitEditorScene::GetColorIconRect(const size_t index) const
{
	return RectF{ (GetColorRow(index).x + 8), (GetColorRow(index).y + 3), 22, 22 };
}

RectF UnitEditorScene::GetColorDecreaseButton(const size_t index) const
{
 return RectF{ 884, (483 + (index * 30)), 28, 30 };
}

RectF UnitEditorScene::GetColorIncreaseButton(const size_t index) const
{
 return RectF{ 920, (483 + (index * 30)), 28, 30 };
}

RectF UnitEditorScene::GetColorPreview() const
{
  return RectF{ 1036, 508, 116, 94 };
}

RectF UnitEditorScene::GetColorPreviewIcon() const
{
  return RectF{ 1036, 470, 28, 28 };
}

RectF UnitEditorScene::GetResetButton() const
{
  return RectF{ Arg::center = Vec2{ 748, 666 }, 108, 40 };
}

RectF UnitEditorScene::GetReloadButton() const
{
   return RectF{ Arg::center = Vec2{ 870, 666 }, 108, 40 };
}

RectF UnitEditorScene::GetSaveButton() const
{
    return RectF{ Arg::center = Vec2{ 992, 666 }, 108, 40 };
}

RectF UnitEditorScene::GetBackButton() const
{
   return RectF{ Arg::center = Vec2{ 1114, 666 }, 108, 40 };
}

RectF UnitEditorScene::GetDialogConfirmButton() const
{
	return RectF{ Arg::center = Scene::Center().movedBy(-96, 54), 160, 40 };
}

RectF UnitEditorScene::GetDialogCancelButton() const
{
	return RectF{ Arg::center = Scene::Center().movedBy(96, 54), 160, 40 };
}

RectF UnitEditorScene::GetInfoLineIcon(const size_t index) const
{
  return RectF{ 314, (388 + (index * 28)), 24, 24 };
}

RectF UnitEditorScene::GetKeyboardHelpIcon() const
{
    return RectF{ 534, 650, 24, 24 };
}

RectF UnitEditorScene::GetMetricIcon(const size_t index) const
{
    return RectF{ 314, (390 + (index * 30)), 24, 24 };
}

RectF UnitEditorScene::GetChangeListIcon() const
{
   return RectF{ 314, 532, 24, 24 };
}

StringView UnitEditorScene::GetFieldLabel(const size_t index) const
{
	switch (index)
	{
	case 0:
		return U"召喚コスト";
	case 1:
		return U"最大HP";
	case 2:
		return U"射程";
	case 3:
		return U"攻撃間隔";
	case 4:
	default:
		return U"攻撃力";
	}
}

StringView UnitEditorScene::GetColorLabel(const size_t index) const
{
	switch (index)
	{
	case 0:
		return U"R";
	case 1:
		return U"G";
	case 2:
		return U"B";
	case 3:
	default:
		return U"A";
	}
}

String UnitEditorScene::GetFieldValue(const size_t index) const
{
	switch (index)
	{
	case 0:
		return Format(m_editingDefinition.summonCost);
	case 1:
		return U"{:.1f}"_fmt(m_editingDefinition.maxHp);
	case 2:
		return U"{:.2f}"_fmt(m_editingDefinition.attackRange);
	case 3:
		return U"{:.2f}"_fmt(m_editingDefinition.attackInterval);
	case 4:
	default:
		return U"{:.2f}"_fmt(m_editingDefinition.attackDamage);
	}
}

String UnitEditorScene::GetColorValue(const size_t index) const
{
	const ColorF color = GetNormalizedEditingDefinition().color;
	switch (index)
	{
	case 0:
		return Format(Round(color.r * 255.0));
	case 1:
		return Format(Round(color.g * 255.0));
	case 2:
		return Format(Round(color.b * 255.0));
	case 3:
	default:
		return Format(Round(color.a * 255.0));
	}
}
