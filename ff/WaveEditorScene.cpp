# include "WaveEditorScene.h"

namespace
{
	constexpr std::array<ff::WaveType, 3> WaveTypes = {
		ff::WaveType::Standard,
		ff::WaveType::MidBoss,
		ff::WaveType::TrueBoss,
	};

	constexpr std::array<ff::WaveTrait, 4> WaveTraits = {
		ff::WaveTrait::None,
		ff::WaveTrait::Reinforced,
		ff::WaveTrait::Assault,
		ff::WaveTrait::Bounty,
	};

	template <class T, size_t N>
	int32 FindIndex(const std::array<T, N>& values, const T value)
	{
		for (size_t index = 0; index < values.size(); ++index)
		{
			if (values[index] == value)
			{
				return static_cast<int32>(index);
			}
		}

		return 0;
	}

	ff::WaveDefinition MakeWavePreset(const int32 waveNumber, const ff::WaveType type)
	{
		ff::WaveDefinition definition = ff::MakeDefaultWaveDefinition(waveNumber);
		if (type == definition.type)
		{
			return definition;
		}

		definition.type = type;
		definition.trait = ff::WaveTrait::None;
		definition.enemyHpMultiplier = 1.0;
		definition.enemySpeedMultiplier = 1.0;
		definition.enemyAttackIntervalMultiplier = 1.0;
		definition.rewardBonusPerKill = 0;
		definition.spawnQueue.clear();

		switch (type)
		{
		case ff::WaveType::MidBoss:
			definition.label = U"Mid Boss";
			definition.description = U"中ボス + 少数護衛";
			definition.accentColor = ColorF{ 0.96, 0.40, 0.22 };
			definition.spawnInterval = 0.82;
			definition.spawnQueue << ff::EnemyKind::MidBoss;
			for (int32 index = 0; index < (ff::MidBossBaseSupportEnemyCount + (waveNumber / ff::MidBossWaveInterval) - 1); ++index)
			{
				definition.spawnQueue << ff::EnemyKind::Normal;
			}
			break;

		case ff::WaveType::TrueBoss:
			definition.label = U"True Boss";
			definition.description = U"撃破でクリア";
			definition.accentColor = ColorF{ 0.76, 0.34, 0.96 };
			definition.spawnInterval = 1.15;
			definition.spawnQueue << ff::EnemyKind::TrueBoss;
			break;

		case ff::WaveType::Standard:
		default:
			definition = ff::MakeDefaultWaveDefinition(waveNumber);
			break;
		}

		return definition;
	}
}

WaveEditorScene::WaveEditorScene(const InitData& init)
	: App::Scene{ init }
	, m_titleFont{ 38, Typeface::Heavy }
	, m_buttonFont{ 22 }
	, m_infoFont{ 18 }
	, m_editingConfig{ ff::GetWaveConfig() }
	, m_debugStatus{ U"F5 / Ctrl+S で waveDefinitions.toml を操作できます" }
{
	Scene::SetBackground(ColorF{ 0.08, 0.11, 0.18 });
	ReloadEditingConfig();
}

void WaveEditorScene::update()
{
	ApplyEditorTextsToCurrentWave();

	if (HandlePendingActionConfirmation())
	{
		return;
	}

	if (HandleWaveSelection())
	{
		return;
	}

	if (HandlePrimaryActions())
	{
		return;
	}

	if (HandleAdjustments())
	{
		return;
	}

	HandleKeyboardShortcuts();
}

bool WaveEditorScene::RequestAction(const PendingAction action, const Optional<ff::EnemyKind> targetEnemyKind)
{
	if (HasUnsavedChanges())
	{
		m_pendingAction = action;
		m_pendingEnemyKind = targetEnemyKind;
		return true;
	}

	m_pendingAction = action;
	m_pendingEnemyKind = targetEnemyKind;
	CommitPendingAction();
	return true;
}

void WaveEditorScene::CommitPendingAction()
{
	const PendingAction action = m_pendingAction;
	const Optional<ff::EnemyKind> pendingEnemyKind = m_pendingEnemyKind;
	ClearPendingAction();

	switch (action)
	{
	case PendingAction::BackToFormation:
		changeScene(U"Formation");
		break;

	case PendingAction::OpenEnemyEditor:
		getData().editEnemyDefinitions = true;
		getData().unitEditorReturnToWaveEditor = true;
		getData().selectedEnemyKind = pendingEnemyKind.value_or(ff::EnemyKind::Normal);
		changeScene(U"UnitEditor");
		break;

	case PendingAction::ReloadFromDisk:
		ff::ReloadWaveDefinitionsFromDisk();
		ReloadEditingConfig();
		m_lastSaveSucceeded = true;
		m_debugStatus = U"再読込: {}"_fmt(ff::GetUserWaveDefinitionsPath());
		break;

	case PendingAction::None:
	default:
		break;
	}
}

void WaveEditorScene::ClearPendingAction()
{
	m_pendingAction = PendingAction::None;
	m_pendingEnemyKind.reset();
}

void WaveEditorScene::ReloadEditingConfig()
{
	m_editingConfig = ff::GetWaveConfig();
	m_selectedWave = Clamp(m_selectedWave, 1, static_cast<int32>(m_editingConfig.waves.size()));
	SyncEditorsFromCurrentWave();
}

void WaveEditorScene::SaveCurrentDefinition()
{
	ApplyEditorTextsToCurrentWave();
	ff::SetWaveConfig(m_editingConfig);
	m_editingConfig = ff::GetWaveConfig();
	SyncEditorsFromCurrentWave();
	m_lastSaveSucceeded = ff::SaveCurrentWaveDefinitionsToDisk();
	m_debugStatus = m_lastSaveSucceeded
		? U"保存成功: {}"_fmt(ff::GetUserWaveDefinitionsPath())
		: U"保存失敗: {}"_fmt(ff::GetUserWaveDefinitionsPath());
}

void WaveEditorScene::ResetCurrentWave()
{
	CurrentWave() = ff::MakeDefaultWaveDefinition(m_selectedWave);
	SyncEditorsFromCurrentWave();
	m_lastSaveSucceeded = true;
	m_debugStatus = U"Wave {} を初期値に戻しました（未保存）"_fmt(m_selectedWave);
}

void WaveEditorScene::SyncEditorsFromCurrentWave()
{
	const auto& wave = CurrentWave();
	m_labelEditState.text = wave.label;
	m_descriptionEditState.text = wave.description;
}

void WaveEditorScene::ApplyEditorTextsToCurrentWave()
{
	CurrentWave().label = m_labelEditState.text;
	CurrentWave().description = m_descriptionEditState.text;
}

void WaveEditorScene::ApplyTypePreset(const ff::WaveType type)
{
	const ff::WaveDefinition preset = MakeWavePreset(m_selectedWave, type);
	CurrentWave().type = preset.type;
	CurrentWave().label = preset.label;
	CurrentWave().description = preset.description;
	CurrentWave().accentColor = preset.accentColor;
	CurrentWave().spawnInterval = preset.spawnInterval;
	CurrentWave().spawnQueue = preset.spawnQueue;
	ApplyTraitPreset(ff::WaveTrait::None);
	SyncEditorsFromCurrentWave();
}

void WaveEditorScene::ApplyTraitPreset(const ff::WaveTrait trait)
{
	auto& wave = CurrentWave();
	wave.trait = trait;
	wave.enemyHpMultiplier = 1.0;
	wave.enemySpeedMultiplier = 1.0;
	wave.enemyAttackIntervalMultiplier = 1.0;
	wave.rewardBonusPerKill = 0;

	switch (trait)
	{
	case ff::WaveTrait::Reinforced:
		wave.enemyHpMultiplier = 1.35;
		break;

	case ff::WaveTrait::Assault:
		wave.enemySpeedMultiplier = 1.18;
		wave.enemyAttackIntervalMultiplier = 0.86;
		break;

	case ff::WaveTrait::Bounty:
		wave.rewardBonusPerKill = 1;
		break;

	case ff::WaveTrait::None:
	default:
		break;
	}
}

void WaveEditorScene::AdjustGlobalTiming(const size_t index, const double direction)
{
	switch (index)
	{
	case 0:
		m_editingConfig.waveStartDelay = Max(0.05, (m_editingConfig.waveStartDelay + (0.1 * direction)));
		break;
	case 1:
		m_editingConfig.waveClearDelay = Max(0.05, (m_editingConfig.waveClearDelay + (0.1 * direction)));
		break;
	case 2:
	default:
		m_editingConfig.waveBannerDuration = Max(0.05, (m_editingConfig.waveBannerDuration + (0.1 * direction)));
		break;
	}
}

void WaveEditorScene::AdjustWaveNumeric(const size_t index, const double direction)
{
	auto& wave = CurrentWave();

	switch (index)
	{
	case 0:
		wave.spawnInterval = Max(0.05, (wave.spawnInterval + (0.05 * direction)));
		break;
	case 1:
		wave.enemyHpMultiplier = Max(0.10, (wave.enemyHpMultiplier + (0.05 * direction)));
		break;
	case 2:
		wave.enemySpeedMultiplier = Max(0.10, (wave.enemySpeedMultiplier + (0.05 * direction)));
		break;
	case 3:
		wave.enemyAttackIntervalMultiplier = Max(0.10, (wave.enemyAttackIntervalMultiplier + (0.05 * direction)));
		break;
	case 4:
	default:
		wave.rewardBonusPerKill = Max(0, (wave.rewardBonusPerKill + static_cast<int32>(direction)));
		break;
	}
}

void WaveEditorScene::AdjustSpawnCount(const size_t index, const int32 delta)
{
	Array<int32> counts = GetSpawnCounts();
	counts[index] = Max(0, (counts[index] + delta));

	if ((counts[0] + counts[1] + counts[2]) <= 0)
	{
		counts[index] = 1;
	}

	SetSpawnCounts(counts);
}

void WaveEditorScene::CycleWaveType(const int32 direction)
{
	const int32 currentIndex = FindIndex(WaveTypes, CurrentWave().type);
	const int32 nextIndex = (currentIndex + direction + static_cast<int32>(WaveTypes.size())) % static_cast<int32>(WaveTypes.size());
	ApplyTypePreset(WaveTypes[nextIndex]);
}

void WaveEditorScene::CycleWaveTrait(const int32 direction)
{
	const int32 currentIndex = FindIndex(WaveTraits, CurrentWave().trait);
	const int32 nextIndex = (currentIndex + direction + static_cast<int32>(WaveTraits.size())) % static_cast<int32>(WaveTraits.size());
	ApplyTraitPreset(WaveTraits[nextIndex]);
}

ff::WaveDefinition& WaveEditorScene::CurrentWave()
{
	return m_editingConfig.waves[m_selectedWave - 1];
}

const ff::WaveDefinition& WaveEditorScene::CurrentWave() const
{
	return m_editingConfig.waves[m_selectedWave - 1];
}

bool WaveEditorScene::HasUnsavedChanges() const
{
	ff::WaveConfig current = m_editingConfig;
	current.waves[m_selectedWave - 1].label = m_labelEditState.text;
	current.waves[m_selectedWave - 1].description = m_descriptionEditState.text;
	return (ff::BuildWaveDefinitionsToml(current) != ff::BuildWaveDefinitionsToml(ff::GetWaveConfig()));
}

String WaveEditorScene::GetPendingActionMessage() const
{
	switch (m_pendingAction)
	{
	case PendingAction::BackToFormation:
		return U"編成画面へ戻ります";
	case PendingAction::ReloadFromDisk:
		return U"ファイルから再読込します";
	case PendingAction::OpenEnemyEditor:
		return U"敵エディタを開きます";
	case PendingAction::None:
	default:
		return U"未保存の変更があります";
	}
}

Array<int32> WaveEditorScene::GetSpawnCounts() const
{
	Array<int32> counts(SpawnKindCount, 0);

	for (const auto kind : CurrentWave().spawnQueue)
	{
		for (size_t index = 0; index < SpawnKindCount; ++index)
		{
			const ff::EnemyKind spawnKind = (index == 0)
				? ff::EnemyKind::Normal
				: (index == 1)
					? ff::EnemyKind::MidBoss
					: ff::EnemyKind::TrueBoss;
			if (spawnKind == kind)
			{
				++counts[index];
				break;
			}
		}
	}

	return counts;
}

void WaveEditorScene::SetSpawnCounts(const Array<int32>& counts)
{
	auto& queue = CurrentWave().spawnQueue;
	queue.clear();

	for (size_t index = 0; index < SpawnKindCount; ++index)
	{
		const ff::EnemyKind spawnKind = (index == 0)
			? ff::EnemyKind::Normal
			: (index == 1)
				? ff::EnemyKind::MidBoss
				: ff::EnemyKind::TrueBoss;

		for (int32 count = 0; count < counts[index]; ++count)
		{
			queue << spawnKind;
		}
	}
}
