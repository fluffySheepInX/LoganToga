# include "UnitEditorScene.h"

UnitEditorScene::UnitEditorScene(const InitData& init)
	: App::Scene{ init }
	, m_titleFont{ 38, Typeface::Heavy }
	, m_buttonFont{ 22 }
	, m_infoFont{ 18 }
	, m_unitId{ getData().selectedFormationUnit.value_or(ff::UnitId::GuardPlayer) }
	, m_editingDefinition{ ff::GetUnitDefinition(m_unitId) }
	, m_debugStatus{ U"F5 / 再読込 で unitDefinitions.toml を読み直せます" }
{
	Scene::SetBackground(ColorF{ 0.08, 0.11, 0.18 });
	SyncEditorsFromDefinition();
}

void UnitEditorScene::update()
{
	ApplyEditorTextsToDefinition();

	if (HandlePendingActionConfirmation())
	{
		return;
	}

	if (HandleUnitSelection())
	{
		return;
	}

	if (HandlePrimaryActions())
	{
		return;
	}

	if (HandleFieldAdjustment())
	{
		return;
	}

	HandleKeyboardShortcuts();
}

bool UnitEditorScene::RequestAction(const PendingAction action, const Optional<ff::UnitId> targetUnit)
{
	if ((action == PendingAction::SwitchUnit) && targetUnit && (*targetUnit == m_unitId))
	{
		return false;
	}

	if (HasUnsavedChanges())
	{
		m_pendingAction = action;
		m_pendingUnitId = targetUnit;
		return true;
	}

	m_pendingAction = action;
	m_pendingUnitId = targetUnit;
	CommitPendingAction();
	return true;
}

void UnitEditorScene::CommitPendingAction()
{
	const PendingAction action = m_pendingAction;
	const Optional<ff::UnitId> pendingUnitId = m_pendingUnitId;
	ClearPendingAction();

	switch (action)
	{
	case PendingAction::BackToFormation:
		changeScene(U"Formation");
		break;

	case PendingAction::SwitchUnit:
		if (pendingUnitId)
		{
			LoadUnit(*pendingUnitId);
		}
		break;

	case PendingAction::ReloadFromDisk:
		ff::ReloadUnitDefinitionsFromDisk();
		LoadUnit(m_unitId);
		m_lastSaveSucceeded = true;
		m_debugStatus = U"再読込: {}"_fmt(ff::GetUserUnitDefinitionsPath());
		break;

	case PendingAction::ResetToDefault:
		m_editingDefinition = ff::GetDefaultUnitDefinition(m_unitId);
		SyncEditorsFromDefinition();
		m_lastSaveSucceeded = true;
		m_debugStatus = U"初期値に戻しました（未保存）";
		break;

	case PendingAction::None:
	default:
		break;
	}
}

void UnitEditorScene::ClearPendingAction()
{
	m_pendingAction = PendingAction::None;
	m_pendingUnitId.reset();
}

void UnitEditorScene::LoadUnit(const ff::UnitId unitId)
{
	m_unitId = unitId;
	getData().selectedFormationUnit = unitId;
	m_editingDefinition = ff::GetUnitDefinition(unitId);
	SyncEditorsFromDefinition();
	m_lastSaveSucceeded = true;
	m_hasValidationError = false;
	m_validationStatus.clear();
	m_debugStatus = U"編集中ユニットを切り替えました";
}

void UnitEditorScene::SaveCurrentDefinition()
{
	ApplyEditorTextsToDefinition();
	if (m_hasValidationError)
	{
		m_lastSaveSucceeded = false;
		m_debugStatus = U"保存前に入力エラーを解消してください";
		return;
	}

	ff::SetUnitDefinition(GetNormalizedEditingDefinition());
	m_editingDefinition = ff::GetUnitDefinition(m_unitId);
	SyncEditorsFromDefinition();
	SaveAppDataToDisk(getData());
	m_lastSaveSucceeded = ff::SaveCurrentUnitDefinitionsToDisk();
	m_debugStatus = m_lastSaveSucceeded
		? U"保存成功: {}"_fmt(ff::GetUserUnitDefinitionsPath())
		: U"保存失敗: {}"_fmt(ff::GetUserUnitDefinitionsPath());
}
