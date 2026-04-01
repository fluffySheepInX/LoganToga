# include "FormationScene.h"
# include "FormationUi.h"

bool FormationScene::HandleSceneNavigation()
{
	if (KeyEscape.down() || GetBackButton().leftClicked())
	{
		changeScene(U"Title");
		return true;
	}

	if (GetConfirmButton().leftClicked())
	{
		ApplyFormationEditState(getData(), m_editingFormation);
		SaveAppDataToDisk(getData());
		changeScene(U"Title");
		return true;
	}

	return false;
}

bool FormationScene::HandleUtilityActions()
{
	if (GetRandomButton().leftClicked())
	{
		m_editingFormation.slots = ff::MakeRandomFormationSlots();
		return true;
	}

	if (GetClearButton().leftClicked())
	{
		ff::ClearFormationSlots(m_editingFormation.slots);
		return true;
	}

	if (GetUnitEditButton().leftClicked() && m_editingFormation.selectedUnit)
	{
		ApplyFormationEditState(getData(), m_editingFormation);
		getData().editEnemyDefinitions = false;
		getData().unitEditorReturnToWaveEditor = false;
		changeScene(U"UnitEditor");
		return true;
	}

	if (GetWaveEditButton().leftClicked())
	{
		ApplyFormationEditState(getData(), m_editingFormation);
		changeScene(U"WaveEditor");
		return true;
	}

	return false;
}

bool FormationScene::HandlePresetInput()
{
	for (size_t index = 0; index < getData().formationPresets.size(); ++index)
	{
		if (GetPresetLoadButton(index).leftClicked())
		{
			m_editingFormation.slots = getData().formationPresets[index];
			return true;
		}

		if (GetPresetSaveButton(index).leftClicked())
		{
			getData().formationPresets[index] = m_editingFormation.slots;
			SaveAppDataToDisk(getData());
			return true;
		}
	}

	return false;
}

bool FormationScene::HandleUnitSelection()
{
	const auto& unitTypes = GetFormationUnitTypes();

	for (size_t index = 0; index < unitTypes.size(); ++index)
	{
		if (GetUnitButton(index).leftClicked())
		{
			m_editingFormation.selectedUnit = unitTypes[index];
			return true;
		}
	}

	return false;
}

bool FormationScene::HandleSlotInput()
{
	for (size_t index = 0; index < m_editingFormation.slots.size(); ++index)
	{
		if (GetSlotButton(index).leftClicked())
		{
			if (ff::AssignSelectedFormationUnit(m_editingFormation, index))
			{
				return true;
			}
		}

		if (GetSlotButton(index).rightClicked())
		{
			if (ff::ClearFormationSlot(m_editingFormation.slots, index))
			{
				return true;
			}
		}
	}

	return false;
}
