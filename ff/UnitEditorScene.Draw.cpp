# include "UnitEditorScene.h"
# include "FormationUi.h"
# include "MenuUi.h"

void UnitEditorScene::draw() const
{
  const RectF panel{ Arg::center = Scene::Center(), 1210, 688 };
	const RectF unitPanel{ 44, 84, 228, 540 };
	const RectF previewPanel{ 296, 84, 332, 252 };
	const RectF infoPanel{ 296, 356, 332, 268 };
	const RectF editPanel{ 652, 84, 548, 540 };
	const RectF statusBar{ 44, 638, 1156, 56 };
	const ff::UnitDefinition loadedDefinition = ff::GetUnitDefinition(m_unitId);
	const ff::UnitDefinition normalizedDefinition = GetNormalizedEditingDefinition();

	panel.rounded(24).draw(ColorF{ 0.06, 0.09, 0.16, 0.94 });
	panel.rounded(24).drawFrame(2, ColorF{ 0.82, 0.88, 1.0, 0.72 });
	unitPanel.rounded(18).draw(ColorF{ 0.10, 0.14, 0.24, 0.90 });
	unitPanel.rounded(18).drawFrame(2, ColorF{ 0.72, 0.80, 0.98, 0.56 });
	previewPanel.rounded(18).draw(ColorF{ 0.10, 0.14, 0.24, 0.90 });
	previewPanel.rounded(18).drawFrame(2, ColorF{ 0.72, 0.80, 0.98, 0.56 });
	editPanel.rounded(18).draw(ColorF{ 0.10, 0.14, 0.24, 0.90 });
	editPanel.rounded(18).drawFrame(2, ColorF{ 0.72, 0.80, 0.98, 0.56 });
	infoPanel.rounded(18).draw(ColorF{ 0.10, 0.14, 0.24, 0.90 });
	infoPanel.rounded(18).drawFrame(2, ColorF{ 0.72, 0.80, 0.98, 0.56 });
	statusBar.rounded(18).draw(ColorF{ 0.10, 0.14, 0.24, 0.90 });
	statusBar.rounded(18).drawFrame(2, ColorF{ 0.72, 0.80, 0.98, 0.56 });

    m_buttonFont(U"unit edit").draw(Vec2{ 44, 42 }, ColorF{ 0.90, 0.94, 1.0, 0.82 });
	DrawIconChip(GetTopHelpIcon(), U"?", ColorF{ 0.18, 0.24, 0.38, 0.96 }, ColorF{ 0.84, 0.90, 1.0, 0.84 });
	DrawIconChip(GetUnitPanelHeaderIcon(), U"≡", ColorF{ 0.18, 0.24, 0.38, 0.96 }, ColorF{ 0.84, 0.90, 1.0, 0.84 });
	DrawIconChip(GetPreviewPanelHeaderIcon(), U"◎", ColorF{ 0.18, 0.24, 0.38, 0.96 }, ColorF{ 0.84, 0.90, 1.0, 0.84 });
	DrawIconChip(GetEditPanelHeaderIcon(), U"✎", ColorF{ 0.18, 0.24, 0.38, 0.96 }, ColorF{ 0.84, 0.90, 1.0, 0.84 });
	DrawIconChip(GetInfoPanelHeaderIcon(), U"∑", ColorF{ 0.18, 0.24, 0.38, 0.96 }, ColorF{ 0.84, 0.90, 1.0, 0.84 });

	DrawUnitList();
	DrawPreviewPanel();
	DrawEditorPanel();
	DrawInfoPanel(loadedDefinition, normalizedDefinition);
    DrawStatusBar();

	if (m_pendingAction == PendingAction::None)
	{
		DrawHoverTooltip();
	}

	if (m_pendingAction != PendingAction::None)
	{
		DrawPendingActionDialog();
	}
}

void UnitEditorScene::DrawUnitList() const
{
	const auto& unitTypes = GetFormationUnitTypes();

	for (size_t index = 0; index < unitTypes.size(); ++index)
	{
		const ff::UnitId unitId = unitTypes[index];
		const ff::UnitDefinition definition = (unitId == m_unitId) ? GetNormalizedEditingDefinition() : ff::GetUnitDefinition(unitId);
		DrawFormationUnitButton(GetUnitButton(index), m_buttonFont, m_infoFont, definition, (unitId == m_unitId));
	}

	DrawIconChip(GetUnitListHelpIcon(), U"?", ColorF{ 0.16, 0.22, 0.34, 0.96 }, ColorF{ 0.84, 0.90, 1.0, 0.80 });
}

void UnitEditorScene::DrawPreviewPanel() const
{
  DrawFormationUnitButton(RectF{ 352, 150, 220, 56 }, m_buttonFont, m_infoFont, GetNormalizedEditingDefinition(), true);
	DrawLabeledValue(GetPreviewLineIcon(0), U"#", Vec2{ 352, 222 }, U"ID {}"_fmt(ff::GetUnitStableId(m_unitId)), ColorF{ 0.88, 0.92, 1.0, 0.76 });
	DrawLabeledValue(GetPreviewLineIcon(1), U"◎", Vec2{ 352, 252 }, GetNormalizedEditingDefinition().label, Palette::White);
	DrawLabeledValue(GetPreviewLineIcon(2), U"↓", Vec2{ 352, 282 }, ff::GetUserUnitDefinitionsPath(), ColorF{ 0.84, 0.90, 1.0, 0.72 });
}

void UnitEditorScene::DrawEditorPanel() const
{
	DrawIconChip(GetLabelFieldIcon(), U"✎", ColorF{ 0.16, 0.22, 0.34, 0.96 }, ColorF{ 0.84, 0.90, 1.0, 0.80 });
    SimpleGUI::TextBox(m_labelEditState, Vec2{ 706, 134 }, 450, 24);
	DrawIconChip(GetRoleFieldIcon(), U"☰", ColorF{ 0.16, 0.22, 0.34, 0.96 }, ColorF{ 0.84, 0.90, 1.0, 0.80 });
 SimpleGUI::TextBox(m_roleEditState, Vec2{ 706, 200 }, 450, 32);

	for (size_t index = 0; index < FieldCount; ++index)
	{
		const RectF fieldRect = GetFieldRow(index);
		const bool changed = IsFieldChanged(index);
		const bool selected = (m_selectedFieldIndex == index);
		const ColorF fillColor = selected
			? ColorF{ 0.20, 0.24, 0.38, 0.96 }
			: (changed ? ColorF{ 0.18, 0.19, 0.30, 0.94 } : ColorF{ 0.12, 0.16, 0.27, 0.92 });
		const ColorF frameColor = changed ? ColorF{ 1.0, 0.78, 0.44, 0.92 } : ColorF{ 0.70, 0.78, 0.92, 0.58 };

		fieldRect.rounded(14).draw(fillColor);
		fieldRect.rounded(14).drawFrame(2, frameColor);
		DrawFieldIcon(index, GetFieldIconRect(index));
      SimpleGUI::TextBox(m_numericEditStates[index], fieldRect.pos.movedBy(50, 9), 258, 16);
		DrawMenuButton(GetDecreaseButton(index), m_infoFont, U"-");
		DrawMenuButton(GetIncreaseButton(index), m_infoFont, U"+");
	}

	for (size_t index = 0; index < ColorChannelCount; ++index)
	{
		const RectF colorRect = GetColorRow(index);
		colorRect.rounded(12).draw(ColorF{ 0.12, 0.16, 0.27, 0.92 });
		colorRect.rounded(12).drawFrame(2, ColorF{ 0.70, 0.78, 0.92, 0.58 });
		DrawColorChannelIcon(index, GetColorIconRect(index));
		m_infoFont(GetColorValue(index)).draw(colorRect.pos.movedBy(42, 9), ColorF{ 0.90, 0.94, 1.0, 0.86 });
		DrawMenuButton(GetColorDecreaseButton(index), m_infoFont, U"-");
		DrawMenuButton(GetColorIncreaseButton(index), m_infoFont, U"+");
	}

	const RectF swatch = GetColorPreview();
	swatch.rounded(18).draw(GetNormalizedEditingDefinition().color);
	swatch.rounded(18).drawFrame(2, ColorF{ 0.92, 0.96, 1.0, 0.86 });
	DrawIconChip(GetColorPreviewIcon(), U"●", ColorF{ 0.16, 0.22, 0.34, 0.96 }, ColorF{ 0.92, 0.96, 1.0, 0.86 }, GetNormalizedEditingDefinition().color);
}

void UnitEditorScene::DrawInfoPanel(const ff::UnitDefinition& loadedDefinition, const ff::UnitDefinition& normalizedDefinition) const
{
	const Array<String> changeLines = BuildChangeLines(loadedDefinition, normalizedDefinition);
   const size_t maxVisibleChangeLines = 3;
	const size_t visibleChangeLines = Min(changeLines.size(), maxVisibleChangeLines);
   const auto clipText = [](const String& text, const size_t maxLength)
	{
		if (text.size() <= maxLength)
		{
			return text;
		}

		return text.substr(0, (maxLength - 3)) + U"...";
	};
	const double dps = (normalizedDefinition.attackInterval > 0.0) ? (normalizedDefinition.attackDamage / normalizedDefinition.attackInterval) : 0.0;
	const double hpPerCost = (normalizedDefinition.summonCost > 0) ? (normalizedDefinition.maxHp / normalizedDefinition.summonCost) : 0.0;
	const double dummyKillSeconds = (dps > 0.0) ? (100.0 / dps) : 0.0;

    DrawLabeledValue(GetMetricIcon(0), U"✦", Vec2{ 348, 390 }, U"DPS {:.2f}"_fmt(dps), Palette::White);
	DrawLabeledValue(GetMetricIcon(1), U"+", Vec2{ 348, 420 }, U"HP/Cost {:.2f}"_fmt(hpPerCost), Palette::White);
	DrawLabeledValue(GetMetricIcon(2), U"◎", Vec2{ 348, 450 }, U"100HP {:.2f}s"_fmt(dummyKillSeconds), Palette::White);
	DrawLabeledValue(GetMetricIcon(3), U"×", Vec2{ 348, 480 }, U"RangeDPS {:.2f}"_fmt(normalizedDefinition.attackRange * dps), Palette::White);

	DrawIconChip(GetChangeListIcon(), U"≡", ColorF{ 0.16, 0.22, 0.34, 0.96 }, ColorF{ 0.84, 0.90, 1.0, 0.80 });
	if (changeLines.isEmpty())
	{
        m_infoFont(U"変更はありません").draw(Vec2{ 348, 532 }, ColorF{ 0.82, 0.92, 1.0, 0.78 });
	}
	else
	{
     for (size_t index = 0; index < visibleChangeLines; ++index)
		{
            m_infoFont(clipText(changeLines[index], 20)).draw(Vec2{ 348, (532 + (index * 24)) }, ColorF{ 0.92, 0.95, 1.0, 0.86 });
		}

		if (changeLines.size() > visibleChangeLines)
		{
			m_infoFont(U"…ほか {} 件"_fmt(changeLines.size() - visibleChangeLines)).draw(Vec2{ 348, (532 + (visibleChangeLines * 24)) }, ColorF{ 0.82, 0.92, 1.0, 0.70 });
		}
	}
}

void UnitEditorScene::DrawStatusBar() const
{
	const RectF dirtyChip{ 60, 648, 162, 34 };
	const String statusText = m_hasValidationError ? m_validationStatus : m_debugStatus;
   const String displayStatus = (statusText.size() > 18) ? (statusText.substr(0, 15) + U"...") : statusText;
	const ColorF dirtyFill = HasUnsavedChanges() ? ColorF{ 0.34, 0.24, 0.12, 0.96 } : ColorF{ 0.14, 0.26, 0.18, 0.96 };
	const ColorF dirtyFrame = HasUnsavedChanges() ? ColorF{ 1.0, 0.82, 0.44, 0.92 } : ColorF{ 0.72, 0.98, 0.82, 0.92 };
	const ColorF statusColor = m_hasValidationError ? ColorF{ 1.0, 0.72, 0.72, 0.94 } : (m_lastSaveSucceeded ? ColorF{ 0.88, 0.92, 1.0, 0.82 } : ColorF{ 1.0, 0.78, 0.70, 0.92 });

	dirtyChip.rounded(14).draw(dirtyFill);
	dirtyChip.rounded(14).drawFrame(2, dirtyFrame);
	m_infoFont(HasUnsavedChanges() ? U"未保存あり" : U"保存済み").drawAt(dirtyChip.center(), Palette::White);
	m_infoFont(displayStatus).draw(Vec2{ 242, 656 }, statusColor);
	DrawIconChip(GetKeyboardHelpIcon(), U"⌨", ColorF{ 0.16, 0.22, 0.34, 0.96 }, ColorF{ 0.84, 0.90, 1.0, 0.80 });
  m_infoFont(U"↑↓ / Shift").draw(Vec2{ 566, 656 }, ColorF{ 0.88, 0.92, 1.0, 0.76 });
	DrawBottomButtons();
}

void UnitEditorScene::DrawBottomButtons() const
{
	DrawMenuButton(GetResetButton(), m_buttonFont, U"↺");
	DrawMenuButton(GetReloadButton(), m_buttonFont, U"⟳");
	DrawMenuButton(GetSaveButton(), m_buttonFont, U"↓");
	DrawMenuButton(GetBackButton(), m_buttonFont, U"←");
}

void UnitEditorScene::DrawIconChip(const RectF& rect, const String& icon, const ColorF& fillColor, const ColorF& frameColor, const ColorF& iconColor) const
{
	const bool hovered = rect.mouseOver();
	const ColorF drawFill = hovered ? fillColor.lerp(ColorF{ 0.26, 0.34, 0.52, 0.98 }, 0.28) : fillColor;
	const ColorF drawFrame = hovered ? frameColor.lerp(Palette::White, 0.30) : frameColor;

	rect.rounded(10).draw(drawFill);
	rect.rounded(10).drawFrame(2, drawFrame);
	m_infoFont(icon).drawAt(18, rect.center(), iconColor);
}

void UnitEditorScene::DrawLabeledValue(const RectF& iconRect, const String& icon, const Vec2& pos, const String& value, const ColorF& textColor) const
{
	DrawIconChip(iconRect, icon, ColorF{ 0.16, 0.22, 0.34, 0.96 }, ColorF{ 0.84, 0.90, 1.0, 0.80 });
	m_infoFont(value).draw(pos, textColor);
}

void UnitEditorScene::DrawFieldIcon(const size_t index, const RectF& rect) const
{
	switch (index)
	{
	case 0:
		DrawIconChip(rect, U"◈", ColorF{ 0.34, 0.24, 0.12, 0.96 }, ColorF{ 1.0, 0.82, 0.44, 0.92 });
		break;

	case 1:
		DrawIconChip(rect, U"+", ColorF{ 0.26, 0.16, 0.18, 0.96 }, ColorF{ 1.0, 0.58, 0.64, 0.92 });
		break;

	case 2:
		DrawIconChip(rect, U"◎", ColorF{ 0.14, 0.22, 0.32, 0.96 }, ColorF{ 0.66, 0.88, 1.0, 0.92 });
		break;

	case 3:
		DrawIconChip(rect, U"◷", ColorF{ 0.18, 0.18, 0.30, 0.96 }, ColorF{ 0.82, 0.78, 1.0, 0.92 });
		break;

	case 4:
	default:
		DrawIconChip(rect, U"✦", ColorF{ 0.26, 0.18, 0.14, 0.96 }, ColorF{ 1.0, 0.76, 0.46, 0.92 });
		break;
	}
}

void UnitEditorScene::DrawColorChannelIcon(const size_t index, const RectF& rect) const
{
	const ColorF frameColor = ColorF{ 0.92, 0.96, 1.0, 0.76 };
	rect.rounded(10).draw(ColorF{ 0.14, 0.18, 0.28, 0.96 });
	rect.rounded(10).drawFrame(2, frameColor);

	const Vec2 center = rect.center();
	const double radius = 6.5;
	switch (index)
	{
	case 0:
		Circle{ center, radius }.draw(Palette::Red);
		break;

	case 1:
		Circle{ center, radius }.draw(Palette::Lime);
		break;

	case 2:
		Circle{ center, radius }.draw(Palette::Skyblue);
		break;

	case 3:
	default:
		Circle{ center, radius }.draw(ColorF{ 1.0, 1.0, 1.0, 0.55 });
		Circle{ center, radius }.drawFrame(1.5, ColorF{ 1.0, 1.0, 1.0, 0.95 });
		break;
	}
}

void UnitEditorScene::DrawHoverTooltip() const
{
	const Optional<String> tooltip = GetHoveredTooltipText();
	if (!tooltip)
	{
		return;
	}

	const int32 lineCount = static_cast<int32>(tooltip->count(U'\n')) + 1;
	const double tooltipWidth = Clamp((90.0 + (tooltip->size() * 10.0)), 180.0, 460.0);
	const double tooltipHeight = 22.0 + (lineCount * 20.0);
	const RectF tooltipRect{
		Clamp(Cursor::PosF().x + 18.0, 16.0, Scene::Width() - tooltipWidth - 16.0),
		Clamp(Cursor::PosF().y + 18.0, 16.0, Scene::Height() - tooltipHeight - 16.0),
		tooltipWidth,
		tooltipHeight
	};

	RoundRect{ tooltipRect, 10 }.draw(ColorF{ 0.03, 0.05, 0.08, 0.96 });
	RoundRect{ tooltipRect, 10 }.drawFrame(2, 0, ColorF{ 0.84, 0.90, 1.0, 0.88 });
	m_infoFont(*tooltip).draw(tooltipRect.pos.movedBy(12, 8), Palette::White);
}

void UnitEditorScene::DrawPendingActionDialog() const
{
	const RectF overlay = Scene::Rect();
	const RectF dialog{ Arg::center = Scene::Center(), 430, 188 };

	overlay.draw(ColorF{ 0.0, 0.0, 0.0, 0.46 });
	dialog.rounded(20).draw(ColorF{ 0.08, 0.11, 0.19, 0.98 });
	dialog.rounded(20).drawFrame(2, ColorF{ 0.92, 0.96, 1.0, 0.72 });
	m_buttonFont(U"未保存の変更があります").drawAt(dialog.center().movedBy(0, -46), Palette::White);
	m_infoFont(GetPendingActionMessage()).drawAt(dialog.center().movedBy(0, -8), ColorF{ 0.90, 0.94, 1.0, 0.86 });
	m_infoFont(U"変更を破棄して続行しますか？").drawAt(dialog.center().movedBy(0, 18), ColorF{ 0.90, 0.94, 1.0, 0.72 });
	DrawMenuButton(GetDialogConfirmButton(), m_infoFont, U"破棄して続行");
	DrawMenuButton(GetDialogCancelButton(), m_infoFont, U"キャンセル");
}
