# pragma once
# include <Siv3D.hpp>
# include <numeric>
# include <algorithm>

class BeatVisualizerAddon : public IAddon
{
public:
	struct Settings
	{
		size_t maxHistorySize = 50;
		size_t minHistorySizeForBeat = 10;
		double beatThresholdFactor = 1.5;
		int32 beatCooldownFrames = 10;
		double beatPulseDecay = 3.0;
		double lowBandRatio = 1.0 / 8.0;
		double maxEnergySmoothing = 0.1;
		double barHeightRatio = 0.4;
		double barSpacing = 2.0;
		double barAlpha = 0.7;
		bool drawBpm = true;
	};

	static constexpr StringView AddonName{ U"BeatVisualizerAddon" };

	static void Register(const Settings& settings = Settings{})
	{
		Addon::Register<BeatVisualizerAddon>(AddonName);
		SetSettings(settings);
	}

	static void SetSettings(const Settings& settings)
	{
		if (auto p = Addon::GetAddon<BeatVisualizerAddon>(AddonName))
		{
			p->m_settings = settings;
		}
	}

	static void SetFont(const Font& font)
	{
		if (auto p = Addon::GetAddon<BeatVisualizerAddon>(AddonName))
		{
			p->m_font = font;
		}
	}

	static void Reset()
	{
		if (auto p = Addon::GetAddon<BeatVisualizerAddon>(AddonName))
		{
			p->m_energyHistory.clear();
			p->m_beatCooldown = 0;
			p->m_smoothedMaxEnergy = 1e-6;
			p->m_lastBeatTime = -1.0;
			p->m_estimatedBpm = 0.0;
			p->m_beatPulse = 0.0;
		}
	}

	static void Draw(const Audio& audio)
	{
		if (auto p = Addon::GetAddon<BeatVisualizerAddon>(AddonName))
		{
			p->updateInternal(audio);
			p->drawInternal();
		}
	}

private:
	Settings m_settings;
	Font m_font{ 16 };
	FFTResult m_fft;
	Array<double> m_energyHistory;

	int32 m_beatCooldown = 0;
	double m_smoothedMaxEnergy = 1e-6;
	double m_lastBeatTime = -1.0;
	double m_estimatedBpm = 0.0;
	double m_beatPulse = 0.0;

	bool init() override
	{
		return true;
	}

	void updateInternal(const Audio& audio)
	{
		const double nowTime = Scene::Time();
		const double minBeatInterval = (m_estimatedBpm > 0.0) ? (60.0 / m_estimatedBpm) * 0.5 : 0.2;

		if (audio.isPlaying())
		{
			FFT::Analyze(m_fft, audio);

			const size_t bufferSize = m_fft.buffer.size();
			const size_t lowBandSize = static_cast<size_t>(bufferSize * m_settings.lowBandRatio);

			double energy = 0.0;
			for (size_t i = 0; i < lowBandSize; ++i)
			{
				const auto value = m_fft.buffer[i];
				energy += value * value;
			}

			if (lowBandSize > 0)
			{
				energy /= static_cast<double>(lowBandSize);
			}

			m_energyHistory << energy;
			if (m_energyHistory.size() > m_settings.maxHistorySize)
			{
				m_energyHistory.pop_front();
			}

			if (m_energyHistory.size() >= m_settings.minHistorySizeForBeat)
			{
				const double averageEnergy = std::accumulate(m_energyHistory.begin(), m_energyHistory.end(), 0.0) / m_energyHistory.size();
				double variance = 0.0;
				for (const auto& historyValue : m_energyHistory)
				{
					const double diff = historyValue - averageEnergy;
					variance += diff * diff;
				}
				variance /= m_energyHistory.size();
				const double standardDeviation = Math::Sqrt(variance);
				const double threshold = averageEnergy + (m_settings.beatThresholdFactor * standardDeviation);

				if ((m_beatCooldown <= 0) && (energy > threshold) && ((m_lastBeatTime < 0.0) || ((nowTime - m_lastBeatTime) >= minBeatInterval)))
				{
					const double beatStrength = Math::Clamp((energy - threshold) / (standardDeviation + 1e-6), 0.0, 1.0);
					m_beatPulse = Max(m_beatPulse, 0.5 + (0.5 * beatStrength));

					if (m_lastBeatTime >= 0.0)
					{
						const double interval = nowTime - m_lastBeatTime;
						if ((interval >= 0.2) && (interval <= 2.0))
						{
							const double instantBpm = 60.0 / interval;
							m_estimatedBpm = (m_estimatedBpm > 0.0) ? Math::Lerp(m_estimatedBpm, instantBpm, 0.2) : instantBpm;
						}
					}

					m_lastBeatTime = nowTime;
					m_beatCooldown = m_settings.beatCooldownFrames;
				}
			}
		}

		if (m_beatCooldown > 0)
		{
			--m_beatCooldown;
		}

		m_beatPulse = Max(0.0, m_beatPulse - (Scene::DeltaTime() * m_settings.beatPulseDecay));
	}

	void drawInternal() const
	{
		if (m_energyHistory.isEmpty())
		{
			return;
		}

		const ScopedViewport2D viewport{ Scene::Rect() };

		double maxEnergy = *std::max_element(m_energyHistory.begin(), m_energyHistory.end());
		if (maxEnergy < 1e-6)
		{
			maxEnergy = 1e-6;
		}
		const_cast<double&>(m_smoothedMaxEnergy) = Math::Lerp(m_smoothedMaxEnergy, maxEnergy, m_settings.maxEnergySmoothing);

		const double maxBarHeight = Scene::Height() * m_settings.barHeightRatio;
		const double totalSpacing = m_settings.barSpacing * (m_energyHistory.size() - 1);
		const double barWidth = (Scene::Width() - totalSpacing) / static_cast<double>(m_energyHistory.size());

		for (size_t i = 0; i < m_energyHistory.size(); ++i)
		{
			const double normalizedHeight = (m_energyHistory[i] / m_smoothedMaxEnergy) * maxBarHeight * (1.0 + m_beatPulse * 0.2);
			const double x = i * (barWidth + m_settings.barSpacing);
			const double y = Scene::Height() - normalizedHeight;
			const ColorF barColor = ColorF(HSV(200 - (m_beatPulse * 60.0), 0.5, 0.9), m_settings.barAlpha);
			RectF(x, y, barWidth, normalizedHeight).draw(barColor);
		}

		if (m_settings.drawBpm)
		{
			m_font(U"BPM: {:.1f}"_fmt(m_estimatedBpm)).draw(10, 10, Palette::White);
		}
	}
};
