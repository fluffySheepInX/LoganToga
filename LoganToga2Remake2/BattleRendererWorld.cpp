#include "BattleRenderer.h"

namespace
{
	[[nodiscard]] bool IsMeleeAttackArchetype(const UnitArchetype archetype)
	{
		return (archetype == UnitArchetype::Worker)
			|| (archetype == UnitArchetype::Soldier);
	}

	[[nodiscard]] double GetAttackAnimationProgress(const AttackVisualEffect& effect)
	{
		if (effect.totalFrames <= 0)
		{
			return 0.0;
		}

		return Clamp(1.0 - (static_cast<double>(effect.framesRemaining) / effect.totalFrames), 0.0, 1.0);
	}

	[[nodiscard]] double GetThrustAmount(const double progress)
	{
		return (progress <= 0.5) ? (progress * 2.0) : ((1.0 - progress) * 2.0);
	}

	[[nodiscard]] Vec2 GetMeleeAttackOffset(const UnitArchetype archetype, const Vec2& attackVector, const double progress)
	{
		if (attackVector.lengthSq() <= 0.001)
		{
			return Vec2::Zero();
		}

		const Vec2 direction = attackVector.normalized();
		const double thrust = GetThrustAmount(progress);
		const double offsetDistance = (archetype == UnitArchetype::Worker) ? 5.0 : 7.0;
		return (direction * offsetDistance * thrust);
	}

	[[nodiscard]] Vec2 GetUnitRenderOffset(const UnitState& unit, const BattleState& state)
	{
		for (const auto& effect : state.attackVisualEffects)
		{
			if ((effect.sourceUnitId != unit.id)
				|| !IsMeleeAttackArchetype(effect.sourceArchetype)
				|| (effect.framesRemaining <= 0)
				|| (effect.totalFrames <= 0))
			{
				continue;
			}

			return GetMeleeAttackOffset(unit.archetype, effect.end - effect.start, GetAttackAnimationProgress(effect));
		}

		return Vec2::Zero();
	}

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
		const double thrust = GetThrustAmount(animationProgress);
		const double baseLength = (effect.sourceArchetype == UnitArchetype::Worker) ? 13.0 : 18.0;
		const double peakLength = (effect.sourceArchetype == UnitArchetype::Worker) ? 24.0 : 32.0;
		const double tipLength = baseLength + ((peakLength - baseLength) * thrust);
		const double lineWidth = (effect.sourceArchetype == UnitArchetype::Worker) ? 3.0 : 4.0;
		const double headLength = (effect.sourceArchetype == UnitArchetype::Worker) ? 6.0 : 8.0;
		const double headWidth = (effect.sourceArchetype == UnitArchetype::Worker) ? 4.0 : 5.0;
		const double frontOffset = (effect.sourceArchetype == UnitArchetype::Worker) ? 10.0 : 14.0;
		const Vec2 lungeOffset = GetMeleeAttackOffset(effect.sourceArchetype, attackVector, animationProgress);
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

	void DrawBeamAttackEffect(const AttackVisualEffect& effect, const double t, const ColorF& ownerColor)
	{
		const ColorF beamColor{ ownerColor.r, ownerColor.g, ownerColor.b, 0.25 + (0.60 * t) };
		const ColorF coreColor{ 1.0, 0.95, 0.82, 0.35 + (0.55 * t) };

		Line{ effect.start, effect.end }.draw(6, beamColor);
		Line{ effect.start, effect.end }.draw(2.5, coreColor);
		Circle{ effect.start, 7.0 + (4.0 * t) }.draw(ColorF{ 1.0, 0.92, 0.72, 0.25 + (0.45 * t) });
		Circle{ effect.end, 10.0 + (8.0 * (1.0 - t)) }.drawFrame(2.5, ColorF{ 1.0, 0.88, 0.58, 0.25 + (0.50 * t) });
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

void BattleRenderer::drawSquads(const BattleState& state) const
{
	for (const auto& squad : state.squads)
	{
		for (const auto unitId : squad.unitIds)
		{
			if (const auto* unit = state.findUnit(unitId))
			{
				Line{ unit->position, squad.destination + unit->formationOffset }.draw(1, ColorF{ GetOwnerColor(squad.owner), 0.2 });
			}
		}

		Circle{ squad.destination, 8 }.drawFrame(2, GetOwnerColor(squad.owner));
	}
}

void BattleRenderer::drawWorld(const BattleState& state) const
{
	state.worldBounds.draw(ColorF{ 0.14, 0.17, 0.20 });

	for (int32 x = static_cast<int32>(state.worldBounds.x); x <= static_cast<int32>(state.worldBounds.rightX()); x += 64)
	{
		Line{ x, state.worldBounds.y, x, state.worldBounds.bottomY() }.draw(1, ColorF{ 0.18, 0.21, 0.24 });
	}

	for (int32 y = static_cast<int32>(state.worldBounds.y); y <= static_cast<int32>(state.worldBounds.bottomY()); y += 64)
	{
		Line{ state.worldBounds.x, y, state.worldBounds.rightX(), y }.draw(1, ColorF{ 0.18, 0.21, 0.24 });
	}

	if (state.isSelecting)
	{
		state.selectionRect.drawFrame(2, Palette::Skyblue);
	}

	if (state.isCommandDragging)
	{
		const Vec2 drag = (state.commandDragCurrent - state.commandDragStart);
		if (drag.lengthSq() >= 1.0)
		{
			const Vec2 direction = drag.normalized();
			const Vec2 side{ -direction.y, direction.x };
			const Vec2 arrowHeadBase = state.commandDragCurrent - (direction * 18.0);
			Line{ state.commandDragStart, state.commandDragCurrent }.draw(3, Palette::Orange);
			Line{ arrowHeadBase + (side * 8.0), state.commandDragCurrent }.draw(3, Palette::Orange);
			Line{ arrowHeadBase - (side * 8.0), state.commandDragCurrent }.draw(3, Palette::Orange);
			Circle{ state.commandDragStart, 4 }.draw(Palette::Orange);
		}
	}
}

void BattleRenderer::drawObstacles(const BattleConfigData& config, const GameData& gameData) const
{
	for (const auto& obstacle : config.obstacles)
	{
		obstacle.rect.draw(ColorF{ 0.24, 0.20, 0.18 });
		obstacle.rect.drawFrame(2, ColorF{ 0.45, 0.36, 0.30 });
		gameData.smallFont(obstacle.label).drawAt(obstacle.rect.center().movedBy(0, -6), ColorF{ 0.95, 0.88, 0.76 });
	}
}

void BattleRenderer::drawConstructionPreview(const BattleState& state, const BattleConfigData& config, const GameData& gameData) const
{
	if (!state.pendingConstructionArchetype)
	{
		return;
	}

	const auto* definition = FindUnitDefinition(config, *state.pendingConstructionArchetype);
	if (!definition)
	{
		return;
	}

	const Vec2 position = state.buildingPreviewPosition;
	Circle{ position, definition->radius }.draw(ColorF{ 0.85, 0.9, 1.0, 0.18 });
	Circle{ position, definition->radius }.drawFrame(2, Palette::Skyblue);
	gameData.smallFont(s3d::Format(U"Place ", GetArchetypeLabel(*state.pendingConstructionArchetype))).drawAt(position.movedBy(0, -definition->radius - 16), Palette::Skyblue);
}

void BattleRenderer::drawPendingConstructionOrders(const BattleState& state, const BattleConfigData& config, const GameData& gameData) const
{
	for (const auto& order : state.pendingConstructionOrders)
	{
		const auto* worker = state.findUnit(order.workerUnitId);
		const auto* definition = FindUnitDefinition(config, order.archetype);
		if (!(worker && worker->isAlive && definition))
		{
			continue;
		}

		const ColorF accentColor{ 1.0, 0.86, 0.28, 0.92 };
		Line{ worker->position, order.position }.draw(2.0, ColorF{ accentColor, 0.45 });
		Circle{ worker->position, worker->radius + 10 }.drawFrame(2, accentColor);
		Circle{ order.position, definition->radius }.draw(ColorF{ accentColor, 0.10 });
		Circle{ order.position, definition->radius }.drawFrame(2.5, accentColor);
		RectF{ Arg::center(order.position), definition->radius * 2.4, definition->radius * 2.4 }.drawFrame(1.5, ColorF{ accentColor, 0.42 });
		gameData.smallFont(s3d::Format(U"BUILD ", GetArchetypeLabel(order.archetype))).drawAt(order.position.movedBy(0, -definition->radius - 16), Palette::Yellow);
	}
}

void BattleRenderer::drawResourcePoints(const BattleState& state, const GameData& gameData) const
{
	for (const auto& resourcePoint : state.resourcePoints)
	{
		const ColorF ownerColor = GetOwnerColor(resourcePoint.owner);
		Circle{ resourcePoint.position, resourcePoint.radius }.draw(ColorF{ ownerColor, 0.16 });
		Circle{ resourcePoint.position, resourcePoint.radius }.drawFrame(3, ownerColor);

		if (resourcePoint.capturingOwner)
		{
			const double progress = (resourcePoint.captureTime > 0.0)
				? Clamp(resourcePoint.captureProgress / resourcePoint.captureTime, 0.0, 1.0)
				: 1.0;
			const RectF barBack{ resourcePoint.position.x - 28, resourcePoint.position.y + resourcePoint.radius + 10, 56, 5 };
			barBack.draw(ColorF{ 0.05, 0.05, 0.05, 0.85 });
			RectF{ barBack.pos, barBack.w * progress, barBack.h }.draw(GetOwnerColor(*resourcePoint.capturingOwner));
		}

		gameData.smallFont(s3d::Format(resourcePoint.label, U" (+", resourcePoint.incomeAmount, U")")).drawAt(resourcePoint.position.movedBy(0, -resourcePoint.radius - 14), Palette::White);
	}
}

void BattleRenderer::drawBuildings(const BattleState& state, const GameData& gameData) const
{
	for (const auto& building : state.buildings)
	{
		const auto* unit = state.findUnit(building.unitId);
		if (!(unit && unit->isAlive && IsBuildingArchetype(unit->archetype)))
		{
			continue;
		}

		Circle{ unit->position, unit->radius + 16 }.draw(ColorF{ GetOwnerColor(unit->owner), 0.12 });

		if (!building.isConstructed)
		{
			const double progress = (building.constructionTotal > 0.0)
				? (1.0 - (building.constructionRemaining / building.constructionTotal))
				: 1.0;
			const RectF barBack{ unit->position.x - 30, unit->position.y + unit->radius + 16, 60, 6 };
			barBack.draw(ColorF{ 0.05, 0.05, 0.05, 0.85 });
			RectF{ barBack.pos, barBack.w * Clamp(progress, 0.0, 1.0), barBack.h }.draw(ColorF{ 0.58, 0.77, 1.0 });
			gameData.smallFont(U"Constructing").drawAt(unit->position.movedBy(0, unit->radius + 30), Palette::White);
			continue;
		}

		if (building.productionQueue.isEmpty())
		{
			continue;
		}

		const auto& currentItem = building.productionQueue.front();
		const double progress = (currentItem.totalTime > 0.0)
			? (1.0 - (currentItem.remainingTime / currentItem.totalTime))
			: 1.0;
		const RectF barBack{ unit->position.x - 26, unit->position.y + unit->radius + 16, 52, 6 };
		barBack.draw(ColorF{ 0.05, 0.05, 0.05, 0.85 });
		RectF{ barBack.pos, barBack.w * Clamp(progress, 0.0, 1.0), barBack.h }.draw(ColorF{ 0.95, 0.82, 0.28 });
		gameData.smallFont(s3d::Format(GetArchetypeLabel(currentItem.archetype), U" x", building.productionQueue.size())).drawAt(unit->position.movedBy(0, unit->radius + 30), Palette::White);
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
		case UnitArchetype::Archer:
			DrawArrowAttackEffect(effect, t, ownerColor);
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
		if ((effect.framesRemaining <= 0) || (effect.totalFrames <= 0) || !IsMeleeAttackArchetype(effect.sourceArchetype))
		{
			continue;
		}

		const double t = Clamp(static_cast<double>(effect.framesRemaining) / effect.totalFrames, 0.0, 1.0);
		DrawThrustAttackEffect(effect, t, GetOwnerColor(effect.owner));
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
