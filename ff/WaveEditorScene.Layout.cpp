# include "WaveEditorScene.h"

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
 return RectF{ 326, (484 + (index * 28)), 360, 28 };
}

RectF WaveEditorScene::GetWaveNumericDecreaseButton(const size_t index) const
{
  return RectF{ 642, (486 + (index * 28)), 20, 24 };
}

RectF WaveEditorScene::GetWaveNumericIncreaseButton(const size_t index) const
{
  return RectF{ 666, (486 + (index * 28)), 20, 24 };
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

RectF WaveEditorScene::GetSpawnEditButton(const size_t index) const
{
	return RectF{ 1008, (180 + (index * 42)), 108, 30 };
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
