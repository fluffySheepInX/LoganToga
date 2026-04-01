# include "WaveEditorScene.h"

namespace
{
	ff::EnemyKind GetSpawnEnemyKind(const size_t index)
	{
		switch (index)
		{
		case 1:
			return ff::EnemyKind::MidBoss;
		case 2:
			return ff::EnemyKind::TrueBoss;
		case 0:
		default:
			return ff::EnemyKind::Normal;
		}
	}
}

bool WaveEditorScene::HandlePendingActionConfirmation()
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

bool WaveEditorScene::HandleWaveSelection()
{
	for (size_t index = 0; index < m_editingConfig.waves.size(); ++index)
	{
		if (GetWaveButton(index).leftClicked())
		{
			m_selectedWave = static_cast<int32>(index + 1);
			SyncEditorsFromCurrentWave();
			return true;
		}
	}

	return false;
}

bool WaveEditorScene::HandlePrimaryActions()
{
	if (KeyEscape.down() || GetBackButton().leftClicked())
	{
		return RequestAction(PendingAction::BackToFormation);
	}

	if ((KeyControl.pressed() && KeyS.down()) || GetSaveButton().leftClicked())
	{
		SaveCurrentDefinition();
		return true;
	}

	if (KeyF5.down() || GetReloadButton().leftClicked())
	{
		return RequestAction(PendingAction::ReloadFromDisk);
	}

	if (GetResetButton().leftClicked())
	{
		ResetCurrentWave();
		return true;
	}

	return false;
}

bool WaveEditorScene::HandleAdjustments()
{
	for (size_t index = 0; index < GlobalTimingFieldCount; ++index)
	{
		if (GetGlobalTimingDecreaseButton(index).leftClicked())
		{
			AdjustGlobalTiming(index, -1.0);
			return true;
		}

		if (GetGlobalTimingIncreaseButton(index).leftClicked())
		{
			AdjustGlobalTiming(index, 1.0);
			return true;
		}
	}

	if (GetTypeDecreaseButton().leftClicked())
	{
		CycleWaveType(-1);
		return true;
	}

	if (GetTypeIncreaseButton().leftClicked())
	{
		CycleWaveType(1);
		return true;
	}

	if (GetTraitDecreaseButton().leftClicked())
	{
		CycleWaveTrait(-1);
		return true;
	}

	if (GetTraitIncreaseButton().leftClicked())
	{
		CycleWaveTrait(1);
		return true;
	}

	for (size_t index = 0; index < WaveNumericFieldCount; ++index)
	{
		if (GetWaveNumericDecreaseButton(index).leftClicked())
		{
			AdjustWaveNumeric(index, -1.0);
			return true;
		}

		if (GetWaveNumericIncreaseButton(index).leftClicked())
		{
			AdjustWaveNumeric(index, 1.0);
			return true;
		}
	}

	for (size_t index = 0; index < SpawnKindCount; ++index)
	{
		if (GetSpawnDecreaseButton(index).leftClicked())
		{
			AdjustSpawnCount(index, -1);
			return true;
		}

		if (GetSpawnIncreaseButton(index).leftClicked())
		{
			AdjustSpawnCount(index, 1);
			return true;
		}

		if (GetSpawnEditButton(index).leftClicked())
		{
			return RequestAction(PendingAction::OpenEnemyEditor, GetSpawnEnemyKind(index));
		}
	}

	return false;
}

void WaveEditorScene::HandleKeyboardShortcuts()
{
	if (!Cursor::OnClientRect())
	{
		return;
	}

	if (KeyLeft.down())
	{
		m_selectedWave = Max(1, (m_selectedWave - 1));
		SyncEditorsFromCurrentWave();
	}
	else if (KeyRight.down())
	{
		m_selectedWave = Min(static_cast<int32>(m_editingConfig.waves.size()), (m_selectedWave + 1));
		SyncEditorsFromCurrentWave();
	}
}
