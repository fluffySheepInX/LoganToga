# include "WaveEditorScene.h"
# include "MenuUi.h"

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

	constexpr std::array<ff::EnemyKind, 3> SpawnKinds = {
		ff::EnemyKind::Normal,
		ff::EnemyKind::MidBoss,
		ff::EnemyKind::TrueBoss,
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

void WaveEditorScene::draw() const
{
	const RectF panel{ Arg::center = Scene::Center(), 1210, 688 };
	const RectF wavePanel{ 44, 84, 228, 540 };
	const RectF editorPanel{ 296, 84, 430, 540 };
	const RectF spawnPanel{ 748, 84, 452, 540 };
	const RectF statusBar{ 44, 638, 1156, 56 };

	panel.rounded(24).draw(ColorF{ 0.06, 0.09, 0.16, 0.94 });
	panel.rounded(24).drawFrame(2, ColorF{ 0.82, 0.88, 1.0, 0.72 });
	wavePanel.rounded(18).draw(ColorF{ 0.10, 0.14, 0.24, 0.90 });
	wavePanel.rounded(18).drawFrame(2, ColorF{ 0.72, 0.80, 0.98, 0.56 });
	editorPanel.rounded(18).draw(ColorF{ 0.10, 0.14, 0.24, 0.90 });
	editorPanel.rounded(18).drawFrame(2, ColorF{ 0.72, 0.80, 0.98, 0.56 });
	spawnPanel.rounded(18).draw(ColorF{ 0.10, 0.14, 0.24, 0.90 });
	spawnPanel.rounded(18).drawFrame(2, ColorF{ 0.72, 0.80, 0.98, 0.56 });
	statusBar.rounded(18).draw(ColorF{ 0.10, 0.14, 0.24, 0.90 });
	statusBar.rounded(18).drawFrame(2, ColorF{ 0.72, 0.80, 0.98, 0.56 });

	m_buttonFont(U"wave edit").draw(Vec2{ 44, 42 }, ColorF{ 0.90, 0.94, 1.0, 0.82 });
	m_infoFont(U"現在 Wave {} / {}"_fmt(m_selectedWave, m_editingConfig.waves.size())).draw(Vec2{ 240, 46 }, ColorF{ 0.84, 0.90, 1.0, 0.70 });

	DrawWaveList();
	DrawEditorPanel();
	DrawSpawnPanel();
	DrawStatusBar();

	if (m_pendingAction != PendingAction::None)
	{
		DrawPendingActionDialog();
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

bool WaveEditorScene::RequestAction(const PendingAction action)
{
	if (HasUnsavedChanges())
	{
		m_pendingAction = action;
		return true;
	}

	m_pendingAction = action;
	CommitPendingAction();
	return true;
}

void WaveEditorScene::CommitPendingAction()
{
	const PendingAction action = m_pendingAction;
	ClearPendingAction();

	switch (action)
	{
	case PendingAction::BackToFormation:
		changeScene(U"Formation");
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
		for (size_t index = 0; index < SpawnKinds.size(); ++index)
		{
			if (SpawnKinds[index] == kind)
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

	for (size_t index = 0; index < SpawnKinds.size(); ++index)
	{
		for (int32 count = 0; count < counts[index]; ++count)
		{
			queue << SpawnKinds[index];
		}
	}
}

void WaveEditorScene::DrawWaveList() const
{
	m_buttonFont(U"ウェーブ一覧").draw(Vec2{ 76, 108 }, Palette::White);
	m_infoFont(U"クリックで切替 / ←→ でも移動").draw(Vec2{ 76, 136 }, ColorF{ 0.84, 0.90, 1.0, 0.66 });

	for (size_t index = 0; index < m_editingConfig.waves.size(); ++index)
	{
		const RectF rect = GetWaveButton(index);
		const bool selected = (static_cast<int32>(index + 1) == m_selectedWave);
		const auto& wave = m_editingConfig.waves[index];
		const ColorF fill = selected ? ColorF{ 0.24, 0.30, 0.46, 0.98 } : ColorF{ 0.12, 0.16, 0.27, 0.92 };
		const ColorF frame = selected ? ColorF{ 0.96, 0.86, 0.50, 0.96 } : wave.accentColor;

		rect.rounded(12).draw(fill);
		rect.rounded(12).drawFrame(2, ColorF{ frame, selected ? 1.0 : 0.72 });
		m_infoFont(U"W{}"_fmt(index + 1)).drawAt(rect.center().movedBy(0, -8), Palette::White);
		m_infoFont(wave.label).drawAt(14, rect.center().movedBy(0, 10), ColorF{ 0.88, 0.92, 1.0, 0.76 });
	}
}

void WaveEditorScene::DrawEditorPanel() const
{
	m_buttonFont(U"基本設定").draw(Vec2{ 326, 108 }, Palette::White);

	for (size_t index = 0; index < GlobalTimingFieldCount; ++index)
	{
		const RectF row = GetGlobalTimingRow(index);
		row.rounded(12).draw(ColorF{ 0.12, 0.16, 0.27, 0.92 });
		row.rounded(12).drawFrame(2, ColorF{ 0.70, 0.78, 0.92, 0.58 });
		m_infoFont(GetGlobalTimingLabel(index)).draw(row.pos.movedBy(12, 10), Palette::White);
		m_infoFont(GetGlobalTimingValue(index)).draw(row.pos.movedBy(160, 10), ColorF{ 0.92, 0.96, 1.0, 0.84 });
		DrawMenuButton(GetGlobalTimingDecreaseButton(index), m_infoFont, U"-");
		DrawMenuButton(GetGlobalTimingIncreaseButton(index), m_infoFont, U"+");
	}

	m_infoFont(U"ラベル").draw(Vec2{ 326, 260 }, Palette::White);
	SimpleGUI::TextBox(m_labelEditState, Vec2{ 326, 286 }, 360, 24);
	m_infoFont(U"説明").draw(Vec2{ 326, 326 }, Palette::White);
	SimpleGUI::TextBox(m_descriptionEditState, Vec2{ 326, 352 }, 360, 24);

	const RectF typeRect = GetTypeButton();
	typeRect.rounded(12).draw(ColorF{ 0.12, 0.16, 0.27, 0.92 });
	typeRect.rounded(12).drawFrame(2, ColorF{ 0.70, 0.78, 0.92, 0.58 });
	m_infoFont(U"タイプ").draw(typeRect.pos.movedBy(12, 10), Palette::White);
	m_infoFont(GetTypeLabel()).draw(typeRect.pos.movedBy(140, 10), ColorF{ 0.92, 0.96, 1.0, 0.84 });
	DrawMenuButton(GetTypeDecreaseButton(), m_infoFont, U"-");
	DrawMenuButton(GetTypeIncreaseButton(), m_infoFont, U"+");

	const RectF traitRect = GetTraitButton();
	traitRect.rounded(12).draw(ColorF{ 0.12, 0.16, 0.27, 0.92 });
	traitRect.rounded(12).drawFrame(2, ColorF{ 0.70, 0.78, 0.92, 0.58 });
	m_infoFont(U"特性プリセット").draw(traitRect.pos.movedBy(12, 10), Palette::White);
	m_infoFont(GetTraitLabel()).draw(traitRect.pos.movedBy(140, 10), ColorF{ 0.92, 0.96, 1.0, 0.84 });
	DrawMenuButton(GetTraitDecreaseButton(), m_infoFont, U"-");
	DrawMenuButton(GetTraitIncreaseButton(), m_infoFont, U"+");

	for (size_t index = 0; index < WaveNumericFieldCount; ++index)
	{
		const RectF row = GetWaveNumericRow(index);
		row.rounded(12).draw(ColorF{ 0.12, 0.16, 0.27, 0.92 });
		row.rounded(12).drawFrame(2, ColorF{ 0.70, 0.78, 0.92, 0.58 });
		m_infoFont(GetWaveNumericLabel(index)).draw(row.pos.movedBy(12, 10), Palette::White);
		m_infoFont(GetWaveNumericValue(index)).draw(row.pos.movedBy(180, 10), ColorF{ 0.92, 0.96, 1.0, 0.84 });
		DrawMenuButton(GetWaveNumericDecreaseButton(index), m_infoFont, U"-");
		DrawMenuButton(GetWaveNumericIncreaseButton(index), m_infoFont, U"+");
	}
}

void WaveEditorScene::DrawSpawnPanel() const
{
	const auto& wave = CurrentWave();
	const Array<int32> spawnCounts = GetSpawnCounts();
	const int32 totalEnemyCount = spawnCounts[0] + spawnCounts[1] + spawnCounts[2];

	m_buttonFont(U"出現キュー").draw(Vec2{ 778, 108 }, Palette::White);
	m_infoFont(U"総数 {} / 保存先 {}"_fmt(totalEnemyCount, ff::GetUserWaveDefinitionsPath())).draw(Vec2{ 778, 136 }, ColorF{ 0.84, 0.90, 1.0, 0.66 });

	for (size_t index = 0; index < SpawnKindCount; ++index)
	{
		const RectF row = GetSpawnRow(index);
		row.rounded(12).draw(ColorF{ 0.12, 0.16, 0.27, 0.92 });
		row.rounded(12).drawFrame(2, ColorF{ 0.70, 0.78, 0.92, 0.58 });
		m_infoFont(GetSpawnLabel(index)).draw(row.pos.movedBy(12, 10), Palette::White);
		m_infoFont(Format(spawnCounts[index])).draw(row.pos.movedBy(170, 10), ColorF{ 0.92, 0.96, 1.0, 0.84 });
		DrawMenuButton(GetSpawnDecreaseButton(index), m_infoFont, U"-");
		DrawMenuButton(GetSpawnIncreaseButton(index), m_infoFont, U"+");
	}

	const RectF preview{ 778, 308, 392, 268 };
	preview.rounded(16).draw(ColorF{ 0.11, 0.15, 0.25, 0.92 });
	preview.rounded(16).drawFrame(2, ColorF{ wave.accentColor, 0.80 });
	m_buttonFont(U"Wave {} Preview"_fmt(m_selectedWave)).draw(Vec2{ 804, 332 }, Palette::White);
	m_infoFont(U"{} / {}"_fmt(wave.label, wave.description)).draw(Vec2{ 804, 372 }, ColorF{ 0.90, 0.94, 1.0, 0.84 });
	m_infoFont(U"タイプ: {}"_fmt(GetTypeLabel())).draw(Vec2{ 804, 412 }, ColorF{ 0.86, 0.90, 1.0, 0.76 });
	m_infoFont(U"特性: {}"_fmt(GetTraitLabel())).draw(Vec2{ 804, 442 }, ColorF{ 0.86, 0.90, 1.0, 0.76 });
	m_infoFont(U"倍率 HP {:.2f} / SPD {:.2f} / ATK {:.2f}"_fmt(wave.enemyHpMultiplier, wave.enemySpeedMultiplier, wave.enemyAttackIntervalMultiplier)).draw(Vec2{ 804, 472 }, ColorF{ 0.86, 0.90, 1.0, 0.76 });
	m_infoFont(U"間隔 {:.2f}s / 撃破資源 +{}"_fmt(wave.spawnInterval, wave.rewardBonusPerKill)).draw(Vec2{ 804, 502 }, ColorF{ 0.86, 0.90, 1.0, 0.76 });
	m_infoFont(U"Normal {} / Mid {} / Boss {}"_fmt(spawnCounts[0], spawnCounts[1], spawnCounts[2])).draw(Vec2{ 804, 532 }, ColorF{ 0.96, 0.86, 0.56, 0.92 });
}

void WaveEditorScene::DrawStatusBar() const
{
	const RectF dirtyChip{ 60, 648, 162, 34 };
	const String statusText = (m_debugStatus.size() > 34) ? (m_debugStatus.substr(0, 31) + U"...") : m_debugStatus;
	const ColorF dirtyFill = HasUnsavedChanges() ? ColorF{ 0.34, 0.24, 0.12, 0.96 } : ColorF{ 0.14, 0.26, 0.18, 0.96 };
	const ColorF dirtyFrame = HasUnsavedChanges() ? ColorF{ 1.0, 0.82, 0.44, 0.92 } : ColorF{ 0.72, 0.98, 0.82, 0.92 };
	const ColorF statusColor = m_lastSaveSucceeded ? ColorF{ 0.88, 0.92, 1.0, 0.82 } : ColorF{ 1.0, 0.78, 0.70, 0.92 };

	dirtyChip.rounded(14).draw(dirtyFill);
	dirtyChip.rounded(14).drawFrame(2, dirtyFrame);
	m_infoFont(HasUnsavedChanges() ? U"未保存あり" : U"保存済み").drawAt(dirtyChip.center(), Palette::White);
	m_infoFont(statusText).draw(Vec2{ 242, 656 }, statusColor);
	m_infoFont(U"F5 再読込 / Ctrl+S 保存 / ←→ Wave" ).draw(Vec2{ 600, 656 }, ColorF{ 0.88, 0.92, 1.0, 0.76 });

	DrawMenuButton(GetResetButton(), m_buttonFont, U"↺");
	DrawMenuButton(GetReloadButton(), m_buttonFont, U"⟳");
	DrawMenuButton(GetSaveButton(), m_buttonFont, U"↓");
	DrawMenuButton(GetBackButton(), m_buttonFont, U"←");
}

void WaveEditorScene::DrawPendingActionDialog() const
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

RectF WaveEditorScene::GetWaveButton(const size_t index) const
{
	const size_t column = (index % 2);
	const size_t row = (index / 2);
	return RectF{ (60 + (column * 98)), (170 + (row * 42)), 88, 36 };
}

RectF WaveEditorScene::GetGlobalTimingRow(const size_t index) const
{
	return RectF{ 326, (154 + (index * 42)), 360, 34 };
}

RectF WaveEditorScene::GetGlobalTimingDecreaseButton(const size_t index) const
{
	return RectF{ 642, (156 + (index * 42)), 20, 30 };
}

RectF WaveEditorScene::GetGlobalTimingIncreaseButton(const size_t index) const
{
	return RectF{ 666, (156 + (index * 42)), 20, 30 };
}

RectF WaveEditorScene::GetTypeButton() const
{
	return RectF{ 326, 398, 360, 34 };
}

RectF WaveEditorScene::GetTypeDecreaseButton() const
{
	return RectF{ 642, 400, 20, 30 };
}

RectF WaveEditorScene::GetTypeIncreaseButton() const
{
	return RectF{ 666, 400, 20, 30 };
}

RectF WaveEditorScene::GetTraitButton() const
{
	return RectF{ 326, 442, 360, 34 };
}

RectF WaveEditorScene::GetTraitDecreaseButton() const
{
	return RectF{ 642, 444, 20, 30 };
}

RectF WaveEditorScene::GetTraitIncreaseButton() const
{
	return RectF{ 666, 444, 20, 30 };
}

RectF WaveEditorScene::GetWaveNumericRow(const size_t index) const
{
	return RectF{ 326, (490 + (index * 26)), 360, 24 };
}

RectF WaveEditorScene::GetWaveNumericDecreaseButton(const size_t index) const
{
	return RectF{ 642, (487 + (index * 26)), 20, 24 };
}

RectF WaveEditorScene::GetWaveNumericIncreaseButton(const size_t index) const
{
	return RectF{ 666, (487 + (index * 26)), 20, 24 };
}

RectF WaveEditorScene::GetSpawnRow(const size_t index) const
{
	return RectF{ 778, (178 + (index * 42)), 392, 34 };
}

RectF WaveEditorScene::GetSpawnDecreaseButton(const size_t index) const
{
	return RectF{ 1126, (180 + (index * 42)), 20, 30 };
}

RectF WaveEditorScene::GetSpawnIncreaseButton(const size_t index) const
{
	return RectF{ 1150, (180 + (index * 42)), 20, 30 };
}

RectF WaveEditorScene::GetResetButton() const
{
	return RectF{ Arg::center = Vec2{ 834, 666 }, 108, 40 };
}

RectF WaveEditorScene::GetReloadButton() const
{
	return RectF{ Arg::center = Vec2{ 956, 666 }, 108, 40 };
}

RectF WaveEditorScene::GetSaveButton() const
{
	return RectF{ Arg::center = Vec2{ 1078, 666 }, 108, 40 };
}

RectF WaveEditorScene::GetBackButton() const
{
	return RectF{ Arg::center = Vec2{ 1190, 666 }, 88, 40 };
}

RectF WaveEditorScene::GetDialogConfirmButton() const
{
	return RectF{ Arg::center = Scene::Center().movedBy(-96, 54), 160, 40 };
}

RectF WaveEditorScene::GetDialogCancelButton() const
{
	return RectF{ Arg::center = Scene::Center().movedBy(96, 54), 160, 40 };
}

StringView WaveEditorScene::GetGlobalTimingLabel(const size_t index) const
{
	switch (index)
	{
	case 0:
		return U"開始待機";
	case 1:
		return U"クリア待機";
	case 2:
	default:
		return U"バナー秒数";
	}
}

String WaveEditorScene::GetGlobalTimingValue(const size_t index) const
{
	switch (index)
	{
	case 0:
		return U"{:.2f}s"_fmt(m_editingConfig.waveStartDelay);
	case 1:
		return U"{:.2f}s"_fmt(m_editingConfig.waveClearDelay);
	case 2:
	default:
		return U"{:.2f}s"_fmt(m_editingConfig.waveBannerDuration);
	}
}

StringView WaveEditorScene::GetWaveNumericLabel(const size_t index) const
{
	switch (index)
	{
	case 0:
		return U"出現間隔";
	case 1:
		return U"HP倍率";
	case 2:
		return U"Speed倍率";
	case 3:
		return U"攻撃間隔倍率";
	case 4:
	default:
		return U"撃破資源ボーナス";
	}
}

String WaveEditorScene::GetWaveNumericValue(const size_t index) const
{
	const auto& wave = CurrentWave();
	switch (index)
	{
	case 0:
		return U"{:.2f}s"_fmt(wave.spawnInterval);
	case 1:
		return U"{:.2f}"_fmt(wave.enemyHpMultiplier);
	case 2:
		return U"{:.2f}"_fmt(wave.enemySpeedMultiplier);
	case 3:
		return U"{:.2f}"_fmt(wave.enemyAttackIntervalMultiplier);
	case 4:
	default:
		return Format(wave.rewardBonusPerKill);
	}
}

StringView WaveEditorScene::GetSpawnLabel(const size_t index) const
{
	switch (index)
	{
	case 0:
		return U"Normal";
	case 1:
		return U"Mid Boss";
	case 2:
	default:
		return U"True Boss";
	}
}

String WaveEditorScene::GetTypeLabel() const
{
	switch (CurrentWave().type)
	{
	case ff::WaveType::MidBoss:
		return U"Mid Boss";
	case ff::WaveType::TrueBoss:
		return U"True Boss";
	case ff::WaveType::Standard:
	default:
		return U"Standard";
	}
}

String WaveEditorScene::GetTraitLabel() const
{
	return String{ ff::GetWaveTraitLabel(CurrentWave().trait) }.isEmpty()
		? U"None"
		: String{ ff::GetWaveTraitLabel(CurrentWave().trait) };
}
