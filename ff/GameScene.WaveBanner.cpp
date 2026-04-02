# include "GameScene.h"

namespace
{
	String GetWaveTraitBannerText(const int32 wave)
	{
		if (!ff::HasWaveDefinition(wave))
		{
			return U"";
		}

		const auto& definition = ff::GetWaveDefinition(wave);
		if (definition.trait == ff::WaveTrait::None)
		{
			return U"";
		}

		return U"特性: {} / {}"_fmt(String{ ff::GetWaveTraitLabel(definition.trait) }, String{ ff::GetWaveTraitDescription(definition.trait) });
	}
}

void GameScene::DrawWaveBanner() const
{
	if ((m_currentWave <= 0) && ((m_currentWave + 1) <= 0))
	{
		return;
	}

	if ((m_waveBannerTimer > 0.0) && (m_currentWave > 0))
	{
		const auto& waveDefinition = ff::GetWaveDefinition(m_currentWave);
		const ColorF accent = waveDefinition.accentColor;
		const String traitLine = GetWaveTraitBannerText(m_currentWave);
		const double alpha = Min(1.0, (m_waveBannerTimer / ff::GetWaveBannerDuration()));
		const RectF waveRect{ Arg::center = Scene::Center().movedBy(0, -220), 320, traitLine.isEmpty() ? 68 : 90 };
		const ColorF accentText{ accent, alpha };
		const ColorF accentFrame{ accent.lerp(Palette::White, 0.35), (0.92 * alpha) };
		waveRect.rounded(14).draw(ColorF{ 0.10, 0.08, 0.18, (0.76 * alpha) });
		waveRect.rounded(14).drawFrame(2, accentFrame);
		m_font(U"Wave {} Start"_fmt(m_currentWave)).drawAt(20, waveRect.center().movedBy(0, traitLine.isEmpty() ? -12 : -22), ColorF{ 1.0, 1.0, 1.0, alpha });
		m_font(U"{} / {}"_fmt(waveDefinition.label, waveDefinition.description)).drawAt(14, waveRect.center().movedBy(0, traitLine.isEmpty() ? 14 : 4), accentText);
		if (not traitLine.isEmpty())
		{
			m_font(traitLine).drawAt(12, waveRect.center().movedBy(0, 28), ColorF{ 0.92, 0.96, 1.0, alpha });
		}
		return;
	}

	if ((not m_waveActive) && (m_nextWaveTimer <= ff::GetWaveBannerDuration()))
	{
		const int32 nextWave = (m_currentWave + 1);
		if (!ff::HasWaveDefinition(nextWave))
		{
			return;
		}

		const auto& waveDefinition = ff::GetWaveDefinition(nextWave);
		const double previewDuration = ff::GetWaveBannerDuration();
		const ColorF accent = waveDefinition.accentColor;
		const String traitLine = GetWaveTraitBannerText(nextWave);
		const double alpha = Min(1.0, (previewDuration - Max(0.0, m_nextWaveTimer)) / previewDuration);
		const ColorF accentText{ accent, alpha };
		const ColorF accentFrame{ accent, (0.78 * alpha) };
		const RectF previewRect{ Arg::center = Scene::Center().movedBy(0, -220), 320, traitLine.isEmpty() ? 56 : 78 };
		previewRect.rounded(14).draw(ColorF{ 0.08, 0.10, 0.18, (0.52 * alpha) });
		previewRect.rounded(14).drawFrame(2, accentFrame);
		m_font(U"Next Wave {}: {}"_fmt(nextWave, waveDefinition.label)).drawAt(18, previewRect.center().movedBy(0, traitLine.isEmpty() ? -8 : -16), ColorF{ 1.0, 1.0, 1.0, alpha });
		m_font(waveDefinition.description).drawAt(13, previewRect.center().movedBy(0, traitLine.isEmpty() ? 12 : 6), accentText);
		if (not traitLine.isEmpty())
		{
			m_font(traitLine).drawAt(12, previewRect.center().movedBy(0, 26), ColorF{ 0.92, 0.96, 1.0, alpha });
		}
	}
}
