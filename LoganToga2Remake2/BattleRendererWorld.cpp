#include "BattleRenderer.h"

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
		const ColorF beamColor{ ownerColor.r, ownerColor.g, ownerColor.b, 0.25 + (0.60 * t) };
		const ColorF coreColor{ 1.0, 0.95, 0.82, 0.35 + (0.55 * t) };

		Line{ effect.start, effect.end }.draw(6, beamColor);
		Line{ effect.start, effect.end }.draw(2.5, coreColor);
		Circle{ effect.start, 7.0 + (4.0 * t) }.draw(ColorF{ 1.0, 0.92, 0.72, 0.25 + (0.45 * t) });
		Circle{ effect.end, 10.0 + (8.0 * (1.0 - t)) }.drawFrame(2.5, ColorF{ 1.0, 0.88, 0.58, 0.25 + (0.50 * t) });
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

		const ColorF color = GetOwnerColor(unit.owner);
		Circle{ unit.position, unit.radius }.draw(color);

		if (IsBuildingArchetype(unit.archetype))
		{
			Circle{ unit.position, unit.radius + 10 }.drawFrame(4, color);
		}

		if (unit.isSelected)
		{
			Circle{ unit.position, unit.radius + 5 }.drawFrame(2, Palette::Yellow);
		}

		const double hpRate = (unit.maxHp > 0) ? (static_cast<double>(unit.hp) / unit.maxHp) : 0.0;
		const RectF barBack{ unit.position.x - 18, unit.position.y - unit.radius - 14, 36, 5 };
		barBack.draw(ColorF{ 0.1 });
		RectF{ barBack.pos, 36 * hpRate, barBack.h }.draw(ColorF{ 0.3, 0.95, 0.45 });

		gameData.smallFont(GetArchetypeLabel(unit.archetype)).drawAt(unit.position.movedBy(0, unit.radius + 10), Palette::White);
	}
}
