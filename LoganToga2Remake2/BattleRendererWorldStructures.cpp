#include "BattleRenderer.h"

void BattleRenderer::drawConstructionPreview(const BattleState& state, const BattleConfigData& config, const GameData& gameData) const
{
	if (state.pendingRepairTargeting)
	{
		const Vec2 position = state.repairPreviewPosition;
		const UnitState* hoveredTurret = nullptr;
		for (const auto& building : state.buildings)
		{
			const auto* unit = state.findUnit(building.unitId);
			if (!(unit && unit->isAlive && (unit->owner == Owner::Player) && (unit->archetype == UnitArchetype::Turret) && building.isConstructed))
			{
				continue;
			}

			if (Circle{ unit->position, unit->radius + 8.0 }.intersects(position))
			{
				hoveredTurret = unit;
				break;
			}
		}

		const bool canRepairHoveredTurret = (hoveredTurret && (hoveredTurret->hp < hoveredTurret->maxHp));
		const ColorF accentColor = canRepairHoveredTurret
			? ColorF{ 0.36, 0.94, 0.54, 0.95 }
			: ColorF{ 0.96, 0.74, 0.24, 0.95 };
		Circle{ position, 14 }.draw(ColorF{ accentColor, 0.16 });
		Circle{ position, 14 }.drawFrame(2, accentColor);
		Line{ position + Vec2{ -10, 0 }, position + Vec2{ 10, 0 } }.draw(2, accentColor);
		Line{ position + Vec2{ 0, -10 }, position + Vec2{ 0, 10 } }.draw(2, accentColor);
		gameData.smallFont(canRepairHoveredTurret ? U"Repair turret" : U"Select damaged turret").drawAt(position.movedBy(0, -28), accentColor);

		if (hoveredTurret)
		{
			Circle{ hoveredTurret->position, hoveredTurret->radius + 10 }.draw(ColorF{ accentColor, 0.10 });
			Circle{ hoveredTurret->position, hoveredTurret->radius + 10 }.drawFrame(2, accentColor);
		}

		return;
	}

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
