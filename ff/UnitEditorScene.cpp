# include "UnitEditorScene.h"

UnitEditorScene::UnitEditorScene(const InitData& init)
	: App::Scene{ init }
	, m_titleFont{ 38, Typeface::Heavy }
	, m_buttonFont{ 22 }
	, m_infoFont{ 18 }
 , m_editingEnemyDefinitions{ getData().editEnemyDefinitions }
	, m_unitId{ getData().selectedFormationUnit.value_or(ff::UnitId::GuardPlayer) }
	, m_editingDefinition{ ff::GetUnitDefinition(m_unitId) }
   , m_enemyKind{ getData().selectedEnemyKind.value_or(ff::EnemyKind::Normal) }
	, m_editingEnemyDefinition{ ff::GetEnemyDefinition(m_enemyKind.value_or(ff::EnemyKind::Normal)) }
	, m_debugStatus{ U"F5 / 再読込 で unitDefinitions.toml を読み直せます" }
{
	Scene::SetBackground(ColorF{ 0.08, 0.11, 0.18 });
    if (m_editingEnemyDefinitions)
	{
		m_debugStatus = U"F5 / 再読込 で enemyDefinitions.toml を読み直せます";
	}
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

bool UnitEditorScene::RequestAction(const PendingAction action, const Optional<ff::UnitId> targetUnit, const Optional<ff::EnemyKind> targetEnemy)
{
	if ((action == PendingAction::SwitchUnit) && targetUnit && (*targetUnit == m_unitId))
	{
		return false;
	}

	if ((action == PendingAction::SwitchEnemy) && targetEnemy && m_enemyKind && (*targetEnemy == *m_enemyKind))
	{
		return false;
	}

	if (HasUnsavedChanges())
	{
		m_pendingAction = action;
		m_pendingUnitId = targetUnit;
        m_pendingEnemyKind = targetEnemy;
		return true;
	}

	m_pendingAction = action;
	m_pendingUnitId = targetUnit;
  m_pendingEnemyKind = targetEnemy;
	CommitPendingAction();
	return true;
}

void UnitEditorScene::CommitPendingAction()
{
	const PendingAction action = m_pendingAction;
	const Optional<ff::UnitId> pendingUnitId = m_pendingUnitId;
   const Optional<ff::EnemyKind> pendingEnemyKind = m_pendingEnemyKind;
	ClearPendingAction();

	switch (action)
	{
	case PendingAction::BackToFormation:
      changeScene(getData().unitEditorReturnToWaveEditor ? U"WaveEditor" : U"Formation");
		break;

	case PendingAction::SwitchUnit:
		if (pendingUnitId)
		{
			LoadUnit(*pendingUnitId);
		}
		break;

	case PendingAction::SwitchEnemy:
		if (pendingEnemyKind)
		{
			LoadEnemy(*pendingEnemyKind);
		}
		break;

	case PendingAction::ReloadFromDisk:
        if (IsEnemyEditor())
		{
			ff::ReloadEnemyDefinitionsFromDisk();
			LoadEnemy(m_enemyKind.value_or(ff::EnemyKind::Normal));
			m_debugStatus = U"再読込: {}"_fmt(ff::GetUserEnemyDefinitionsPath());
		}
		else
		{
			ff::ReloadUnitDefinitionsFromDisk();
			LoadUnit(m_unitId);
			m_debugStatus = U"再読込: {}"_fmt(ff::GetUserUnitDefinitionsPath());
		}
		m_lastSaveSucceeded = true;
		break;

	case PendingAction::ResetToDefault:
       if (IsEnemyEditor())
		{
			m_editingEnemyDefinition = ff::GetDefaultEnemyDefinition(m_enemyKind.value_or(ff::EnemyKind::Normal));
		}
		else
		{
			m_editingDefinition = ff::GetDefaultUnitDefinition(m_unitId);
		}
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
   m_pendingEnemyKind.reset();
}

void UnitEditorScene::LoadUnit(const ff::UnitId unitId)
{
  m_editingEnemyDefinitions = false;
	m_unitId = unitId;
	getData().selectedFormationUnit = unitId;
    getData().editEnemyDefinitions = false;
	m_editingDefinition = ff::GetUnitDefinition(unitId);
	SyncEditorsFromDefinition();
	m_lastSaveSucceeded = true;
	m_hasValidationError = false;
	m_validationStatus.clear();
	m_debugStatus = U"編集中ユニットを切り替えました";
}

void UnitEditorScene::LoadEnemy(const ff::EnemyKind enemyKind)
{
	m_editingEnemyDefinitions = true;
	m_enemyKind = enemyKind;
	getData().selectedEnemyKind = enemyKind;
	getData().editEnemyDefinitions = true;
	m_editingEnemyDefinition = ff::GetEnemyDefinition(enemyKind);
	SyncEditorsFromDefinition();
	m_lastSaveSucceeded = true;
	m_hasValidationError = false;
	m_validationStatus.clear();
	m_debugStatus = U"編集中エネミーを切り替えました";
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

    if (IsEnemyEditor())
	{
		ff::SetEnemyDefinition(GetNormalizedEditingEnemyDefinition());
		m_editingEnemyDefinition = ff::GetEnemyDefinition(m_enemyKind.value_or(ff::EnemyKind::Normal));
		m_lastSaveSucceeded = ff::SaveCurrentEnemyDefinitionsToDisk();
		m_debugStatus = m_lastSaveSucceeded
			? U"保存成功: {}"_fmt(ff::GetUserEnemyDefinitionsPath())
			: U"保存失敗: {}"_fmt(ff::GetUserEnemyDefinitionsPath());
	}
	else
	{
		ff::SetUnitDefinition(GetNormalizedEditingDefinition());
		m_editingDefinition = ff::GetUnitDefinition(m_unitId);
		m_lastSaveSucceeded = ff::SaveCurrentUnitDefinitionsToDisk();
		m_debugStatus = m_lastSaveSucceeded
			? U"保存成功: {}"_fmt(ff::GetUserUnitDefinitionsPath())
			: U"保存失敗: {}"_fmt(ff::GetUserUnitDefinitionsPath());
	}

	SyncEditorsFromDefinition();
	SaveAppDataToDisk(getData());
}

bool UnitEditorScene::IsEnemyEditor() const
{
	return m_editingEnemyDefinitions;
}

String UnitEditorScene::GetCurrentDefinitionPath() const
{
	return IsEnemyEditor() ? ff::GetUserEnemyDefinitionsPath() : ff::GetUserUnitDefinitionsPath();
}

String UnitEditorScene::GetCurrentStableId() const
{
	return IsEnemyEditor()
		? String{ ff::GetEnemyStableId(m_enemyKind.value_or(ff::EnemyKind::Normal)) }
		: String{ ff::GetUnitStableId(m_unitId) };
}

String UnitEditorScene::GetCurrentDisplayName() const
{
	return IsEnemyEditor()
		? GetNormalizedEditingEnemyDefinition().label
		: GetNormalizedEditingDefinition().label;
}

String UnitEditorScene::GetBackButtonTooltipText() const
{
	return getData().unitEditorReturnToWaveEditor ? U"Wave編集へ戻る" : U"編成画面へ戻る";
}
