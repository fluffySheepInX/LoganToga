#pragma once

#include "Remake2Common.h"

struct FixedStepClock
{
	explicit FixedStepClock(const double stepSeconds)
		: m_stepSeconds{ stepSeconds } {}

	[[nodiscard]] double stepSeconds() const noexcept
	{
		return m_stepSeconds;
	}

	[[nodiscard]] size_t beginFrame()
	{
		m_accumulator += Min(Scene::DeltaTime(), 0.25);

		size_t stepCount = 0;
		while ((m_accumulator >= m_stepSeconds) && (stepCount < m_maxStepsPerFrame))
		{
			m_accumulator -= m_stepSeconds;
			++stepCount;
		}

		return stepCount;
	}

	[[nodiscard]] double interpolationAlpha() const noexcept
	{
		return Clamp(m_accumulator / m_stepSeconds, 0.0, 1.0);
	}

private:
	double m_stepSeconds = 1.0 / 60.0;
	double m_accumulator = 0.0;
	size_t m_maxStepsPerFrame = 8;
};
