# include "UnitEditorScene.h"
# include "FormationUi.h"

bool UnitEditorScene::HandleUnitSelection()
{
    if (IsEnemyEditor())
	{
		const auto& enemyKinds = ff::GetAvailableEnemyKinds();

		for (size_t index = 0; index < enemyKinds.size(); ++index)
		{
			if (GetUnitButton(index).leftClicked())
			{
				return RequestAction(PendingAction::SwitchEnemy, none, enemyKinds[index]);
			}
		}

		return false;
	}

	const auto& unitTypes = GetFormationUnitTypes();

	for (size_t index = 0; index < unitTypes.size(); ++index)
	{
		if (GetUnitButton(index).leftClicked())
		{
          return RequestAction(PendingAction::SwitchUnit, unitTypes[index], none);
		}
	}

	return false;
}

bool UnitEditorScene::HandlePrimaryActions()
{
	if (KeyEscape.down() || GetBackButton().leftClicked())
	{
		return RequestAction(PendingAction::BackToFormation);
	}

	if (KeyF5.down() || GetReloadButton().leftClicked())
	{
		return RequestAction(PendingAction::ReloadFromDisk);
	}

	if (GetResetButton().leftClicked())
	{
		return RequestAction(PendingAction::ResetToDefault);
	}

	if (GetSaveButton().leftClicked())
	{
		SaveCurrentDefinition();
		return true;
	}

	return false;
}

bool UnitEditorScene::HandleFieldAdjustment()
{
	for (size_t index = 0; index < FieldCount; ++index)
	{
		if (GetFieldRow(index).leftClicked())
		{
			m_selectedFieldIndex = index;
		}

		if (GetDecreaseButton(index).leftClicked())
		{
			AdjustValue(index, -1.0);
			return true;
		}

		if (GetIncreaseButton(index).leftClicked())
		{
			AdjustValue(index, 1.0);
			return true;
		}
	}

	for (size_t index = 0; index < ColorChannelCount; ++index)
	{
		if (GetColorDecreaseButton(index).leftClicked())
		{
			AdjustColor(index, -1.0);
			return true;
		}

		if (GetColorIncreaseButton(index).leftClicked())
		{
			AdjustColor(index, 1.0);
			return true;
		}
	}

	return false;
}

void UnitEditorScene::HandleKeyboardShortcuts()
{
	if (!Cursor::OnClientRect())
	{
		return;
	}

	const double step = (KeyShift.pressed() ? 5.0 : 1.0);

	if (KeyUp.down())
	{
		AdjustValue(m_selectedFieldIndex, step);
	}
	else if (KeyDown.down())
	{
		AdjustValue(m_selectedFieldIndex, -step);
	}
}

bool UnitEditorScene::HandlePendingActionConfirmation()
{
	if (m_pendingAction == PendingAction::None)
	{
		return false;
	}

	if (KeyEscape.down() || GetDialogCancelButton().leftClicked())
	{
		ClearPendingAction();
		return true;
	}

	if (GetDialogConfirmButton().leftClicked())
	{
		CommitPendingAction();
		return true;
	}

	return true;
}
