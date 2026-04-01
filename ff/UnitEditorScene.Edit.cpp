# include "UnitEditorScene.h"

void UnitEditorScene::AdjustValue(const size_t index, const double direction)
{
  if (IsEnemyEditor())
	{
		switch (index)
		{
		case 0:
			m_editingEnemyDefinition.maxHp = Max(1.0, (m_editingEnemyDefinition.maxHp + direction));
			break;

		case 1:
			m_editingEnemyDefinition.speed = Max(0.1, (m_editingEnemyDefinition.speed + (0.1 * direction)));
			break;

		case 2:
			m_editingEnemyDefinition.attackRange = Max(0.1, (m_editingEnemyDefinition.attackRange + (0.1 * direction)));
			break;

		case 3:
			m_editingEnemyDefinition.attackInterval = Max(0.05, (m_editingEnemyDefinition.attackInterval + (0.05 * direction)));
			break;

		case 4:
		default:
			m_editingEnemyDefinition.attackDamage = Max(0.1, (m_editingEnemyDefinition.attackDamage + (0.25 * direction)));
			break;
		}

		m_selectedFieldIndex = index;
		SyncNumericEditorsFromDefinition();
		m_lastSaveSucceeded = true;
		m_hasValidationError = false;
		m_validationStatus.clear();
		m_debugStatus = U"数値を調整しました（未保存）";
		return;
	}

	switch (index)
	{
	case 0:
		m_editingDefinition.summonCost = Max(1, (m_editingDefinition.summonCost + static_cast<int32>(direction)));
		break;

	case 1:
		m_editingDefinition.maxHp = Max(1.0, (m_editingDefinition.maxHp + direction));
		break;

	case 2:
		m_editingDefinition.attackRange = Max(0.1, (m_editingDefinition.attackRange + (0.1 * direction)));
		break;

	case 3:
		m_editingDefinition.attackInterval = Max(0.05, (m_editingDefinition.attackInterval + (0.05 * direction)));
		break;

	case 4:
	default:
		m_editingDefinition.attackDamage = Max(0.1, (m_editingDefinition.attackDamage + (0.25 * direction)));
		break;
	}

	m_selectedFieldIndex = index;
	SyncNumericEditorsFromDefinition();
	m_lastSaveSucceeded = true;
	m_hasValidationError = false;
	m_validationStatus.clear();
	m_debugStatus = U"数値を調整しました（未保存）";
}

void UnitEditorScene::AdjustColor(const size_t index, const double direction)
{
	const double delta = 0.05 * direction;
  ColorF& color = IsEnemyEditor() ? m_editingEnemyDefinition.color : m_editingDefinition.color;
	switch (index)
	{
	case 0:
     color.r = Clamp(color.r + delta, 0.0, 1.0);
		break;
	case 1:
     color.g = Clamp(color.g + delta, 0.0, 1.0);
		break;
	case 2:
     color.b = Clamp(color.b + delta, 0.0, 1.0);
		break;
	case 3:
	default:
     color.a = Clamp(color.a + delta, 0.0, 1.0);
		break;
	}

	m_lastSaveSucceeded = true;
	m_hasValidationError = false;
	m_validationStatus.clear();
	m_debugStatus = U"色を調整しました（未保存）";
}

void UnitEditorScene::SyncEditorsFromDefinition()
{
  m_labelEditState.text = IsEnemyEditor() ? m_editingEnemyDefinition.label : m_editingDefinition.label;
	m_roleEditState.text = IsEnemyEditor() ? m_editingEnemyDefinition.roleDescription : m_editingDefinition.roleDescription;
	SyncNumericEditorsFromDefinition();
}

void UnitEditorScene::SyncNumericEditorsFromDefinition()
{
	for (size_t index = 0; index < FieldCount; ++index)
	{
		m_numericEditStates[index].text = GetFieldValue(index);
	}
}

void UnitEditorScene::ApplyEditorTextsToDefinition()
{
  if (IsEnemyEditor())
	{
		m_editingEnemyDefinition.label = m_labelEditState.text;
		m_editingEnemyDefinition.roleDescription = m_roleEditState.text;
	}
	else
	{
		m_editingDefinition.label = m_labelEditState.text;
		m_editingDefinition.roleDescription = m_roleEditState.text;
	}
	m_hasValidationError = false;
	m_validationStatus.clear();

	for (size_t index = 0; index < FieldCount; ++index)
	{
		if (!ApplyNumericEditor(index))
		{
			m_hasValidationError = true;
			m_validationStatus = U"{} の入力値が不正です"_fmt(GetFieldLabel(index));
			break;
		}
	}
}

bool UnitEditorScene::ApplyNumericEditor(const size_t index)
{
	const String& text = m_numericEditStates[index].text;
	if (text.isEmpty())
	{
		return false;
	}

	switch (index)
	{
	case 0:
       if (IsEnemyEditor())
		{
			if (const auto value = ParseOpt<double>(text))
			{
				m_editingEnemyDefinition.maxHp = Max(1.0, *value);
				return true;
			}
			return false;
		}

		if (const auto value = ParseOpt<int32>(text))
		{
			m_editingDefinition.summonCost = Max(1, *value);
			return true;
		}
		return false;

	case 1:
      if (IsEnemyEditor())
		{
			if (const auto value = ParseOpt<double>(text))
			{
				m_editingEnemyDefinition.speed = Max(0.1, *value);
				return true;
			}
			return false;
		}

		if (const auto value = ParseOpt<double>(text))
		{
			m_editingDefinition.maxHp = Max(1.0, *value);
			return true;
		}
		return false;

	case 2:
      if (IsEnemyEditor())
		{
			if (const auto value = ParseOpt<double>(text))
			{
				m_editingEnemyDefinition.attackRange = Max(0.1, *value);
				return true;
			}
			return false;
		}

		if (const auto value = ParseOpt<double>(text))
		{
			m_editingDefinition.attackRange = Max(0.1, *value);
			return true;
		}
		return false;

	case 3:
      if (IsEnemyEditor())
		{
			if (const auto value = ParseOpt<double>(text))
			{
				m_editingEnemyDefinition.attackInterval = Max(0.05, *value);
				return true;
			}
			return false;
		}

		if (const auto value = ParseOpt<double>(text))
		{
			m_editingDefinition.attackInterval = Max(0.05, *value);
			return true;
		}
		return false;

	case 4:
	default:
      if (IsEnemyEditor())
		{
			if (const auto value = ParseOpt<double>(text))
			{
				m_editingEnemyDefinition.attackDamage = Max(0.1, *value);
				return true;
			}
			return false;
		}

		if (const auto value = ParseOpt<double>(text))
		{
			m_editingDefinition.attackDamage = Max(0.1, *value);
			return true;
		}
		return false;
	}
}

bool UnitEditorScene::HasUnsavedChanges() const
{
	return IsDirty() || HasPendingTextStateChanges();
}

bool UnitEditorScene::HasPendingTextStateChanges() const
{
    const String currentLabel = IsEnemyEditor() ? m_editingEnemyDefinition.label : m_editingDefinition.label;
	const String currentRole = IsEnemyEditor() ? m_editingEnemyDefinition.roleDescription : m_editingDefinition.roleDescription;
	if ((m_labelEditState.text != currentLabel)
		|| (m_roleEditState.text != currentRole))
	{
		return true;
	}

	for (size_t index = 0; index < FieldCount; ++index)
	{
		if (!DoesNumericEditorMatch(index))
		{
			return true;
		}
	}

	return false;
}

bool UnitEditorScene::DoesNumericEditorMatch(const size_t index) const
{
	const String& text = m_numericEditStates[index].text;
	if (text.isEmpty())
	{
		return false;
	}

	switch (index)
	{
	case 0:
       if (IsEnemyEditor())
		{
			if (const auto value = ParseOpt<double>(text))
			{
				return (AbsDiff(Max(1.0, *value), m_editingEnemyDefinition.maxHp) < 0.0001);
			}
			return false;
		}

		if (const auto value = ParseOpt<int32>(text))
		{
			return (Max(1, *value) == m_editingDefinition.summonCost);
		}
		return false;

	case 1:
      if (IsEnemyEditor())
		{
			if (const auto value = ParseOpt<double>(text))
			{
				return (AbsDiff(Max(0.1, *value), m_editingEnemyDefinition.speed) < 0.0001);
			}
			return false;
		}

		if (const auto value = ParseOpt<double>(text))
		{
			return (AbsDiff(Max(1.0, *value), m_editingDefinition.maxHp) < 0.0001);
		}
		return false;

	case 2:
      if (IsEnemyEditor())
		{
			if (const auto value = ParseOpt<double>(text))
			{
				return (AbsDiff(Max(0.1, *value), m_editingEnemyDefinition.attackRange) < 0.0001);
			}
			return false;
		}

		if (const auto value = ParseOpt<double>(text))
		{
			return (AbsDiff(Max(0.1, *value), m_editingDefinition.attackRange) < 0.0001);
		}
		return false;

	case 3:
      if (IsEnemyEditor())
		{
			if (const auto value = ParseOpt<double>(text))
			{
				return (AbsDiff(Max(0.05, *value), m_editingEnemyDefinition.attackInterval) < 0.0001);
			}
			return false;
		}

		if (const auto value = ParseOpt<double>(text))
		{
			return (AbsDiff(Max(0.05, *value), m_editingDefinition.attackInterval) < 0.0001);
		}
		return false;

	case 4:
	default:
      if (IsEnemyEditor())
		{
			if (const auto value = ParseOpt<double>(text))
			{
				return (AbsDiff(Max(0.1, *value), m_editingEnemyDefinition.attackDamage) < 0.0001);
			}
			return false;
		}

		if (const auto value = ParseOpt<double>(text))
		{
			return (AbsDiff(Max(0.1, *value), m_editingDefinition.attackDamage) < 0.0001);
		}
		return false;
	}
}

bool UnitEditorScene::IsDirty() const
{
   return IsEnemyEditor()
		? !AreSameDefinition(GetNormalizedEditingEnemyDefinition(), ff::GetEnemyDefinition(m_enemyKind.value_or(ff::EnemyKind::Normal)))
		: !AreSameDefinition(GetNormalizedEditingDefinition(), ff::GetUnitDefinition(m_unitId));
}

bool UnitEditorScene::IsFieldChanged(const size_t index) const
{
    if (IsEnemyEditor())
	{
		const ff::EnemyDefinition loadedDefinition = ff::GetEnemyDefinition(m_enemyKind.value_or(ff::EnemyKind::Normal));
		switch (index)
		{
		case 0:
			return (loadedDefinition.maxHp != GetNormalizedEditingEnemyDefinition().maxHp);
		case 1:
			return (loadedDefinition.speed != GetNormalizedEditingEnemyDefinition().speed);
		case 2:
			return (loadedDefinition.attackRange != GetNormalizedEditingEnemyDefinition().attackRange);
		case 3:
			return (loadedDefinition.attackInterval != GetNormalizedEditingEnemyDefinition().attackInterval);
		case 4:
		default:
			return (loadedDefinition.attackDamage != GetNormalizedEditingEnemyDefinition().attackDamage);
		}
	}

	const ff::UnitDefinition loadedDefinition = ff::GetUnitDefinition(m_unitId);
	switch (index)
	{
	case 0:
		return (loadedDefinition.summonCost != GetNormalizedEditingDefinition().summonCost);
	case 1:
		return (loadedDefinition.maxHp != GetNormalizedEditingDefinition().maxHp);
	case 2:
		return (loadedDefinition.attackRange != GetNormalizedEditingDefinition().attackRange);
	case 3:
		return (loadedDefinition.attackInterval != GetNormalizedEditingDefinition().attackInterval);
	case 4:
	default:
		return (loadedDefinition.attackDamage != GetNormalizedEditingDefinition().attackDamage);
	}
}

ff::UnitDefinition UnitEditorScene::GetNormalizedEditingDefinition() const
{
	ff::UnitDefinition definition = m_editingDefinition;
	ff::NormalizeUnitDefinition(definition, ff::GetDefaultUnitDefinition(m_unitId));
	return definition;
}

ff::EnemyDefinition UnitEditorScene::GetNormalizedEditingEnemyDefinition() const
{
	ff::EnemyDefinition definition = m_editingEnemyDefinition;
	ff::NormalizeEnemyDefinition(definition, ff::GetDefaultEnemyDefinition(m_enemyKind.value_or(ff::EnemyKind::Normal)));
	return definition;
}
