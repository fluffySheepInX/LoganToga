#include "BattleRendererWorldMeleeHelpers.h"

#include <cmath>

namespace
{
	void DrawThrustAttackEffect(const AttackVisualEffect& effect, const double t, const ColorF& ownerColor)
	{
		const Vec2 attackVector = (effect.end - effect.start);
		if (attackVector.lengthSq() <= 0.001)
		{
			return;
		}

		const Vec2 direction = attackVector.normalized();
		const Vec2 side{ -direction.y, direction.x };
		const double animationProgress = (1.0 - t);
		const double thrust = BattleRendererWorldInternal::GetThrustAmount(animationProgress);
		const double baseLength = (effect.sourceArchetype == UnitArchetype::Worker) ? 13.0 : 18.0;
		const double peakLength = (effect.sourceArchetype == UnitArchetype::Worker) ? 24.0 : 32.0;
		const double tipLength = baseLength + ((peakLength - baseLength) * thrust);
		const double lineWidth = (effect.sourceArchetype == UnitArchetype::Worker) ? 3.0 : 4.0;
		const double headLength = (effect.sourceArchetype == UnitArchetype::Worker) ? 6.0 : 8.0;
		const double headWidth = (effect.sourceArchetype == UnitArchetype::Worker) ? 4.0 : 5.0;
		const double frontOffset = (effect.sourceArchetype == UnitArchetype::Worker) ? 10.0 : 14.0;
		const Vec2 lungeOffset = BattleRendererWorldInternal::GetMeleeAttackOffset(effect.sourceArchetype, attackVector, animationProgress);
		const Vec2 origin = effect.start + lungeOffset + (direction * frontOffset);
		const Vec2 shaftEnd = origin + (direction * tipLength);
		const Vec2 headBase = shaftEnd - (direction * headLength);
		const ColorF shaftColor = (effect.sourceArchetype == UnitArchetype::Worker)
			? ColorF{ 0.96, 0.84, 0.36, 0.90 }
			: ColorF{ ownerColor.r, ownerColor.g, ownerColor.b, 0.92 };
		const ColorF tipColor{ 1.0, 0.96, 0.84, 0.95 };

		Line{ origin, shaftEnd }.draw(lineWidth + 2.0, ColorF{ 0.05, 0.07, 0.10, 0.35 + (0.25 * thrust) });
		Line{ origin, shaftEnd }.draw(lineWidth, shaftColor);
		Line{ headBase + (side * headWidth), shaftEnd }.draw(lineWidth - 0.5, tipColor);
		Line{ headBase - (side * headWidth), shaftEnd }.draw(lineWidth - 0.5, tipColor);
		Circle{ shaftEnd, 1.5 + (2.5 * thrust) }.draw(ColorF{ 1.0, 0.92, 0.72, 0.45 + (0.30 * thrust) });
	}

	void DrawArrowAttackEffect(const AttackVisualEffect& effect, const double t, const ColorF& ownerColor)
	{
		const Vec2 attackVector = (effect.end - effect.start);
		if (attackVector.lengthSq() <= 0.001)
		{
			return;
		}

		const Vec2 direction = attackVector.normalized();
		const Vec2 side{ -direction.y, direction.x };
		const double progress = (1.0 - t);
		const Vec2 arrowPos = effect.start + (attackVector * progress);
		const Vec2 tailPos = arrowPos - (direction * 18.0);
		const Vec2 headBase = arrowPos - (direction * 7.0);
		const ColorF trailColor{ ownerColor.r, ownerColor.g, ownerColor.b, 0.28 + (0.32 * progress) };
		const ColorF arrowColor{ 0.96, 0.92, 0.78, 0.94 };

		Line{ tailPos, arrowPos }.draw(2.5, trailColor);
		Line{ tailPos, arrowPos }.draw(1.2, arrowColor);
		Line{ headBase + (side * 4.0), arrowPos }.draw(1.4, arrowColor);
		Line{ headBase - (side * 4.0), arrowPos }.draw(1.4, arrowColor);
	}

	void DrawTracerAttackEffect(const AttackVisualEffect& effect, const double t, const ColorF& ownerColor)
	{
		const Vec2 attackVector = (effect.end - effect.start);
		if (attackVector.lengthSq() <= 0.001)
		{
			return;
		}

		const Vec2 direction = attackVector.normalized();
		const double progress = (1.0 - t);
		const Vec2 tracerHead = effect.start + (attackVector * progress);
		const Vec2 tracerTail = tracerHead - (direction * 26.0);
		const ColorF trailColor{ ownerColor.r, ownerColor.g, ownerColor.b, 0.30 + (0.42 * t) };
		const ColorF coreColor{ 1.0, 0.98, 0.84, 0.90 };

		Line{ tracerTail, tracerHead }.draw(4.0, trailColor);
		Line{ tracerTail, tracerHead }.draw(1.8, coreColor);
		Circle{ tracerHead, 2.6 }.draw(coreColor);
	}

	void DrawRocketBarrageEffect(const AttackVisualEffect& effect, const double t, const ColorF& ownerColor)
	{
		const Vec2 attackVector = (effect.end - effect.start);
		if (attackVector.lengthSq() <= 0.001)
		{
			return;
		}

		const Vec2 direction = attackVector.normalized();
		const double progress = (1.0 - t);
		const Vec2 rocketHead = effect.start + (attackVector * progress);
		const Vec2 rocketTail = rocketHead - (direction * 30.0);
		const ColorF smokeColor{ ownerColor.r, ownerColor.g, ownerColor.b, 0.18 + (0.26 * t) };
		const ColorF flameColor{ 1.0, 0.74, 0.34, 0.88 };
		const ColorF rocketColor{ 0.94, 0.92, 0.86, 0.92 };

		Line{ rocketTail, rocketHead }.draw(5.0, smokeColor);
		Line{ rocketTail, rocketHead }.draw(2.4, rocketColor);
		Circle{ rocketTail, 3.5 + (1.0 * t) }.draw(flameColor);
		Circle{ effect.end, 18.0 + (26.0 * progress) }.drawFrame(3.0, ColorF{ 1.0, 0.68, 0.32, 0.24 + (0.42 * t) });
		Circle{ effect.end, 8.0 + (12.0 * progress) }.draw(ColorF{ 1.0, 0.86, 0.54, 0.12 + (0.18 * t) });
	}

	void DrawBeamAttackEffect(const AttackVisualEffect& effect, const double t, const ColorF& ownerColor)
	{
		const ColorF beamColor{ ownerColor.r, ownerColor.g, ownerColor.b, 0.25 + (0.60 * t) };
		const ColorF coreColor{ 1.0, 0.95, 0.82, 0.35 + (0.55 * t) };

		Line{ effect.start, effect.end }.draw(6, beamColor);
		Line{ effect.start, effect.end }.draw(2.5, coreColor);
		Circle{ effect.start, 7.0 + (4.0 * t) }.draw(ColorF{ 1.0, 0.92, 0.72, 0.25 + (0.45 * t) });
		Circle{ effect.end, 10.0 + (8.0 * (1.0 - t)) }.drawFrame(2.5, ColorF{ 1.0, 0.88, 0.58, 0.25 + (0.50 * t) });
	}

	void DrawHealBeamEffect(const AttackVisualEffect& effect, const double t)
	{
		const ColorF beamColor{ 0.38, 0.94, 0.78, 0.24 + (0.58 * t) };
		const ColorF coreColor{ 0.92, 1.0, 0.96, 0.38 + (0.50 * t) };

		Line{ effect.start, effect.end }.draw(5.0, beamColor);
		Line{ effect.start, effect.end }.draw(2.0, coreColor);
		Circle{ effect.start, 6.0 + (3.0 * t) }.draw(ColorF{ 0.82, 1.0, 0.90, 0.24 + (0.36 * t) });
		Circle{ effect.end, 8.0 + (6.0 * (1.0 - t)) }.drawFrame(2.0, ColorF{ 0.76, 1.0, 0.86, 0.28 + (0.42 * t) });
	}

	void DrawSpinAttackEffect(const AttackVisualEffect& effect, const double t, const ColorF& ownerColor)
	{
		const double progress = (1.0 - t);
		const double radius = 12.0 + (10.0 * progress);
		const double baseAngle = (progress * Math::TwoPi * 2.5);
		const ColorF trailColor{ ownerColor.r, ownerColor.g, ownerColor.b, 0.24 + (0.38 * t) };
		const ColorF coreColor{ 1.0, 0.92, 0.56, 0.35 + (0.45 * t) };

		for (int32 bladeIndex = 0; bladeIndex < 3; ++bladeIndex)
		{
			const double angle = baseAngle + ((Math::TwoPi / 3.0) * bladeIndex);
			const Vec2 direction{ std::cos(angle), std::sin(angle) };
			Line{ effect.start + (direction * (radius * 0.28)), effect.start + (direction * radius) }.draw(3.2, trailColor);
		}

		Circle{ effect.start, radius * 0.82 }.drawFrame(2.0, trailColor);
		Circle{ effect.start, 4.0 + (3.0 * t) }.draw(coreColor);
	}
}

void BattleRenderer::drawAttackEffects(const BattleState& state) const
{
	for (const auto& effect : state.attackVisualEffects)
	{
		if ((effect.framesRemaining <= 0) || (effect.totalFrames <= 0))
		{
			continue;
		}

		const double t = Clamp(static_cast<double>(effect.framesRemaining) / effect.totalFrames, 0.0, 1.0);
		const ColorF ownerColor = GetOwnerColor(effect.owner);

		switch (effect.sourceArchetype)
		{
		case UnitArchetype::Healer:
			DrawHealBeamEffect(effect, t);
			break;
		case UnitArchetype::MachineGun:
			DrawTracerAttackEffect(effect, t, ownerColor);
			break;
		case UnitArchetype::Katyusha:
			DrawRocketBarrageEffect(effect, t, ownerColor);
			break;
		case UnitArchetype::Sniper:
			DrawTracerAttackEffect(effect, t, ownerColor);
			break;
		case UnitArchetype::Archer:
			DrawArrowAttackEffect(effect, t, ownerColor);
			break;
		case UnitArchetype::Spinner:
			DrawSpinAttackEffect(effect, t, ownerColor);
			break;
		case UnitArchetype::Turret:
		default:
			DrawBeamAttackEffect(effect, t, ownerColor);
			break;
		}
	}
}

void BattleRenderer::drawMeleeAttackEffects(const BattleState& state) const
{
	for (const auto& effect : state.attackVisualEffects)
	{
		if ((effect.framesRemaining <= 0)
			|| (effect.totalFrames <= 0)
			|| !BattleRendererWorldInternal::IsMeleeAttackArchetype(effect.sourceArchetype))
		{
			continue;
		}

		const double t = Clamp(static_cast<double>(effect.framesRemaining) / effect.totalFrames, 0.0, 1.0);
		DrawThrustAttackEffect(effect, t, GetOwnerColor(effect.owner));
	}
}
