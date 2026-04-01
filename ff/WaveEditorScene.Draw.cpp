# include "WaveEditorScene.h"
# include "MenuUi.h"

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
       m_infoFont(GetWaveNumericLabel(index)).draw(row.pos.movedBy(12, 8), Palette::White);
		m_infoFont(GetWaveNumericValue(index)).draw(row.pos.movedBy(180, 8), ColorF{ 0.92, 0.96, 1.0, 0.84 });
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
		DrawMenuButton(GetSpawnEditButton(index), m_infoFont, U"編集");
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
 const Vec2 statusTextPos{ 242, 648 };
	const Vec2 shortcutTextPos{ 242, 670 };
	const ColorF dirtyFill = HasUnsavedChanges() ? ColorF{ 0.34, 0.24, 0.12, 0.96 } : ColorF{ 0.14, 0.26, 0.18, 0.96 };
	const ColorF dirtyFrame = HasUnsavedChanges() ? ColorF{ 1.0, 0.82, 0.44, 0.92 } : ColorF{ 0.72, 0.98, 0.82, 0.92 };
	const ColorF statusColor = m_lastSaveSucceeded ? ColorF{ 0.88, 0.92, 1.0, 0.82 } : ColorF{ 1.0, 0.78, 0.70, 0.92 };

	dirtyChip.rounded(14).draw(dirtyFill);
	dirtyChip.rounded(14).drawFrame(2, dirtyFrame);
	m_infoFont(HasUnsavedChanges() ? U"未保存あり" : U"保存済み").drawAt(dirtyChip.center(), Palette::White);
 m_infoFont(statusText).draw(statusTextPos, statusColor);
	m_infoFont(U"F5 再読込 / Ctrl+S 保存 / ←→ Wave").draw(shortcutTextPos, ColorF{ 0.88, 0.92, 1.0, 0.76 });

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
