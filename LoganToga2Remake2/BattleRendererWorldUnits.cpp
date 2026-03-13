#include "BattleRendererWorldMeleeHelpers.h"

#include <cmath>

namespace
{
	[[nodiscard]] Vec2 GetUnitRenderOffset(const UnitState& unit, const BattleState& state)
	{
		for (const auto& effect : state.attackVisualEffects)
		{
			if ((effect.sourceUnitId != unit.id)
				|| !BattleRendererWorldInternal::IsMeleeAttackArchetype(effect.sourceArchetype)
				|| (effect.framesRemaining <= 0)
				|| (effect.totalFrames <= 0))
			{
				continue;
			}

			return BattleRendererWorldInternal::GetMeleeAttackOffset(unit.archetype, effect.end - effect.start, BattleRendererWorldInternal::GetAttackAnimationProgress(effect));
		}

		return Vec2::Zero();
	}

	void DrawWorkerDecoration(const UnitState& unit, const BattleState& state, const Vec2& renderPosition)
	{
		const double helmetRadius = unit.radius * 0.72;
		const Vec2 helmetCenter = renderPosition.movedBy(0, -unit.radius * 0.40);
		Circle{ helmetCenter, helmetRadius }.draw(ColorF{ 0.96, 0.82, 0.20, 0.96 });
		RectF{ helmetCenter.x - (helmetRadius * 0.9), helmetCenter.y + (helmetRadius * 0.2), helmetRadius * 1.8, helmetRadius * 0.42 }.draw(ColorF{ 0.72, 0.56, 0.12, 0.96 });
		RectF{ renderPosition.x - (unit.radius * 0.75), renderPosition.y + (unit.radius * 0.18), unit.radius * 1.5, unit.radius * 0.34 }.draw(ColorF{ 0.28, 0.24, 0.18, 0.88 });

		const Vec2 toolHandleStart = renderPosition.movedBy(unit.radius * 0.48, -unit.radius * 0.08);
		const Vec2 toolHandleEnd = renderPosition.movedBy(unit.radius * 1.18, -unit.radius * 0.88);
		Line{ toolHandleStart, toolHandleEnd }.draw(3, ColorF{ 0.84, 0.88, 0.94, 0.96 });
		RectF{ toolHandleEnd.x - (unit.radius * 0.34), toolHandleEnd.y - (unit.radius * 0.18), unit.radius * 0.72, unit.radius * 0.28 }.draw(ColorF{ 0.58, 0.62, 0.70, 0.96 });

		if (!(unit.isSelected && state.pendingConstructionArchetype))
		{
			return;
		}

		Circle{ renderPosition, unit.radius + 8 }.drawFrame(2.5, ColorF{ 1.0, 0.84, 0.24, 0.92 });
		RectF{ Arg::center(renderPosition), unit.radius * 3.4, unit.radius * 3.4 }.drawFrame(1.5, ColorF{ 1.0, 0.90, 0.42, 0.40 });
		Line{ renderPosition.movedBy(unit.radius * 0.9, -unit.radius * 0.7), state.buildingPreviewPosition }.draw(1.5, ColorF{ 1.0, 0.86, 0.32, 0.28 });
	}

	void DrawSpinnerDecoration(const UnitState& unit, const Vec2& renderPosition)
	{
		const double spinRate = (unit.movementDistanceLastFrame > 0.1) ? 14.0 : 4.5;
		const double baseAngle = (Scene::Time() * spinRate);
		Circle{ renderPosition, unit.radius * 0.62 }.draw(ColorF{ 0.12, 0.14, 0.20, 0.92 });
		Circle{ renderPosition, unit.radius * 0.26 }.draw(ColorF{ 0.98, 0.88, 0.34, 0.95 });

		for (int32 bladeIndex = 0; bladeIndex < 3; ++bladeIndex)
		{
			const double angle = baseAngle + ((Math::TwoPi / 3.0) * bladeIndex);
			const Vec2 direction{ std::cos(angle), std::sin(angle) };
			const Vec2 side{ -direction.y, direction.x };
			const Vec2 bladeStart = renderPosition + (direction * (unit.radius * 0.15));
			const Vec2 bladeEnd = renderPosition + (direction * (unit.radius * 1.18));
			const ColorF bladeColor = (unit.movementDistanceLastFrame > 0.1)
				? ColorF{ 1.0, 0.92, 0.52, 0.94 }
				: ColorF{ 0.82, 0.84, 0.90, 0.88 };

			Quad{
				bladeStart + (side * (unit.radius * 0.18)),
				bladeEnd + (side * (unit.radius * 0.10)),
				bladeEnd - (side * (unit.radius * 0.10)),
				bladeStart - (side * (unit.radius * 0.18))
			}.draw(bladeColor);
		}

		Circle{ renderPosition, unit.radius + 2.5 }.drawFrame(1.4, ColorF{ 1.0, 0.92, 0.58, 0.32 });
	}

	void DrawMachineGunDecoration(const UnitState& unit, const Vec2& renderPosition)
	{
		RectF{ renderPosition.x - (unit.radius * 0.68), renderPosition.y - (unit.radius * 0.44), unit.radius * 1.36, unit.radius * 0.88 }.draw(ColorF{ 0.14, 0.18, 0.16, 0.94 });
		RectF{ renderPosition.x - (unit.radius * 0.84), renderPosition.y + (unit.radius * 0.12), unit.radius * 1.06, unit.radius * 0.26 }.draw(ColorF{ 0.30, 0.36, 0.32, 0.96 });
		RectF{ renderPosition.x + (unit.radius * 0.12), renderPosition.y - (unit.radius * 0.18), unit.radius * 1.08, unit.radius * 0.18 }.draw(ColorF{ 0.82, 0.86, 0.90, 0.96 });
		Circle{ renderPosition.movedBy(-(unit.radius * 0.54), 0), unit.radius * 0.22 }.draw(ColorF{ 0.92, 0.78, 0.26, 0.96 });
		Circle{ renderPosition, unit.radius + 2.0 }.drawFrame(1.2, ColorF{ 0.78, 0.90, 0.72, 0.36 });
	}

	void DrawGoliathDecoration(const UnitState& unit, const Vec2& renderPosition)
	{
		const RectF chassis{ renderPosition.x - (unit.radius * 0.78), renderPosition.y - (unit.radius * 0.42), unit.radius * 1.56, unit.radius * 0.92 };
		chassis.draw(ColorF{ 0.26, 0.18, 0.14, 0.96 });
		RectF{ renderPosition.x - (unit.radius * 0.24), renderPosition.y - (unit.radius * 0.86), unit.radius * 0.56, unit.radius * 0.48 }.draw(ColorF{ 0.96, 0.66, 0.24, 0.94 });
		Circle{ renderPosition.movedBy(-(unit.radius * 0.56), unit.radius * 0.48), unit.radius * 0.22 }.draw(ColorF{ 0.12, 0.12, 0.16, 0.96 });
		Circle{ renderPosition.movedBy(unit.radius * 0.58, unit.radius * 0.48), unit.radius * 0.22 }.draw(ColorF{ 0.12, 0.12, 0.16, 0.96 });
		Line{ renderPosition.movedBy(unit.radius * 0.04, -(unit.radius * 0.58)), renderPosition.movedBy(unit.radius * 0.34, -(unit.radius * 1.12)) }.draw(2.0, ColorF{ 0.96, 0.84, 0.46, 0.96 });
		Circle{ renderPosition.movedBy(unit.radius * 0.38, -(unit.radius * 1.18)), unit.radius * 0.12 }.draw(ColorF{ 1.0, 0.92, 0.64, 0.96 });

		if (!unit.isDetonating)
		{
			return;
		}

		const double pulse = 0.5 + (0.5 * std::sin(Scene::Time() * 22.0));
		Circle{ renderPosition, unit.radius + 5.0 + (pulse * 3.0) }.drawFrame(2.6, ColorF{ 1.0, 0.40, 0.22, 0.45 + (0.35 * pulse) });
	}

	void DrawSniperDecoration(const UnitState& unit, const Vec2& renderPosition)
	{
		RectF{ renderPosition.x - (unit.radius * 0.82), renderPosition.y - (unit.radius * 0.18), unit.radius * 1.64, unit.radius * 0.36 }.draw(ColorF{ 0.18, 0.16, 0.22, 0.96 });
		RectF{ renderPosition.x - (unit.radius * 0.22), renderPosition.y - (unit.radius * 0.42), unit.radius * 0.42, unit.radius * 0.16 }.draw(ColorF{ 0.76, 0.18, 0.26, 0.94 });
		Circle{ renderPosition.movedBy(-(unit.radius * 0.10), -(unit.radius * 0.34)), unit.radius * 0.14 }.draw(ColorF{ 0.92, 0.20, 0.28, 0.96 });
		RectF{ renderPosition.x - (unit.radius * 0.34), renderPosition.y + (unit.radius * 0.18), unit.radius * 0.78, unit.radius * 0.18 }.draw(ColorF{ 0.42, 0.36, 0.48, 0.94 });
		Circle{ renderPosition, unit.radius + 2.2 }.drawFrame(1.2, ColorF{ 0.82, 0.60, 0.96, 0.34 });
	}

	void DrawKatyushaDecoration(const UnitState& unit, const Vec2& renderPosition)
	{
		RectF{ renderPosition.x - (unit.radius * 0.88), renderPosition.y - (unit.radius * 0.34), unit.radius * 1.76, unit.radius * 0.78 }.draw(ColorF{ 0.24, 0.26, 0.30, 0.94 });
		RectF{ renderPosition.x - (unit.radius * 0.42), renderPosition.y - (unit.radius * 0.82), unit.radius * 0.92, unit.radius * 0.30 }.draw(ColorF{ 0.58, 0.24, 0.32, 0.96 });
		for (int32 railIndex = 0; railIndex < 3; ++railIndex)
		{
			const double offsetY = (-0.34 + (railIndex * 0.22)) * unit.radius;
			RectF{ renderPosition.x - (unit.radius * 0.22), renderPosition.y + offsetY, unit.radius * 1.12, unit.radius * 0.10 }.draw(ColorF{ 0.92, 0.82, 0.46, 0.94 });
		}
		Circle{ renderPosition.movedBy(-(unit.radius * 0.52), unit.radius * 0.44), unit.radius * 0.20 }.draw(ColorF{ 0.12, 0.12, 0.16, 0.96 });
		Circle{ renderPosition.movedBy(unit.radius * 0.58, unit.radius * 0.44), unit.radius * 0.20 }.draw(ColorF{ 0.12, 0.12, 0.16, 0.96 });
		Circle{ renderPosition, unit.radius + 2.4 }.drawFrame(1.2, ColorF{ 0.96, 0.58, 0.40, 0.34 });
	}

	void DrawHealerDecoration(const UnitState& unit, const Vec2& renderPosition)
	{
		Circle{ renderPosition, unit.radius * 0.76 }.draw(ColorF{ 0.92, 0.98, 1.0, 0.94 });
		RectF{ renderPosition.x - (unit.radius * 0.20), renderPosition.y - (unit.radius * 0.70), unit.radius * 0.40, unit.radius * 1.40 }.draw(ColorF{ 0.36, 0.86, 0.70, 0.96 });
		RectF{ renderPosition.x - (unit.radius * 0.70), renderPosition.y - (unit.radius * 0.20), unit.radius * 1.40, unit.radius * 0.40 }.draw(ColorF{ 0.36, 0.86, 0.70, 0.96 });
		Circle{ renderPosition, unit.radius + 2.4 }.drawFrame(1.4, ColorF{ 0.54, 0.94, 0.78, 0.34 });
	}
}

void BattleRenderer::drawUnits(const BattleState& state, const GameData& gameData) const
{
	for (const auto& unit : state.units)
	{
		if (!unit.isAlive)
		{
			continue;
		}

		const Vec2 renderPosition = unit.position + GetUnitRenderOffset(unit, state);
		const ColorF color = GetOwnerColor(unit.owner);
		if (unit.isSelected && (unit.attackRange > 0.0))
		{
			const ColorF rangeFill = (unit.archetype == UnitArchetype::Healer)
				? ColorF{ 0.40, 0.94, 0.74, 0.08 }
				: ColorF{ 1.0, 0.64, 0.18, 0.08 };
			const ColorF rangeFrame = (unit.archetype == UnitArchetype::Healer)
				? ColorF{ 0.44, 0.98, 0.80, 0.30 }
				: ColorF{ 1.0, 0.68, 0.22, 0.30 };
			Circle{ renderPosition, unit.attackRange }.draw(rangeFill);
			Circle{ renderPosition, unit.attackRange }.drawFrame(1.5, rangeFrame);
		}

		Circle{ renderPosition, unit.radius }.draw(color);
		if (unit.archetype == UnitArchetype::Worker)
		{
			DrawWorkerDecoration(unit, state, renderPosition);
		}
		else if (unit.archetype == UnitArchetype::Healer)
		{
			DrawHealerDecoration(unit, renderPosition);
		}
		else if (unit.archetype == UnitArchetype::Sniper)
		{
			DrawSniperDecoration(unit, renderPosition);
		}
		else if (unit.archetype == UnitArchetype::Katyusha)
		{
			DrawKatyushaDecoration(unit, renderPosition);
		}
		else if (unit.archetype == UnitArchetype::MachineGun)
		{
			DrawMachineGunDecoration(unit, renderPosition);
		}
		else if (unit.archetype == UnitArchetype::Goliath)
		{
			DrawGoliathDecoration(unit, renderPosition);
		}
		else if (unit.archetype == UnitArchetype::Spinner)
		{
			DrawSpinnerDecoration(unit, renderPosition);
		}

		if (IsBuildingArchetype(unit.archetype))
		{
			Circle{ renderPosition, unit.radius + 10 }.drawFrame(4, color);
		}

		if (unit.isSelected)
		{
			Circle{ renderPosition, unit.radius + 5 }.drawFrame(2, Palette::Yellow);
		}

		const double hpRate = (unit.maxHp > 0) ? (static_cast<double>(unit.hp) / unit.maxHp) : 0.0;
		const RectF barBack{ renderPosition.x - 18, renderPosition.y - unit.radius - 14, 36, 5 };
		barBack.draw(ColorF{ 0.1 });
		RectF{ barBack.pos, 36 * hpRate, barBack.h }.draw(ColorF{ 0.3, 0.95, 0.45 });

		gameData.smallFont(GetArchetypeLabel(unit.archetype)).drawAt(renderPosition.movedBy(0, unit.radius + 10), Palette::White);
	}
}
