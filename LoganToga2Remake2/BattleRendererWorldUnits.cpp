#include "BattleRendererWorldMeleeHelpers.h"

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
			Circle{ renderPosition, unit.attackRange }.draw(ColorF{ 1.0, 0.64, 0.18, 0.08 });
			Circle{ renderPosition, unit.attackRange }.drawFrame(1.5, ColorF{ 1.0, 0.68, 0.22, 0.30 });
		}

		Circle{ renderPosition, unit.radius }.draw(color);
		if (unit.archetype == UnitArchetype::Worker)
		{
			DrawWorkerDecoration(unit, state, renderPosition);
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
