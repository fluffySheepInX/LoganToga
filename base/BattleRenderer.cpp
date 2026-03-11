#include "BattleRenderer.h"

void BattleRenderer::draw(const BattleState& state, const BattleConfigData& config, const GameData& gameData) const
{
	drawWorld(state);
	drawSquads(state);
	drawObstacles(config, gameData);
	drawConstructionPreview(state, config, gameData);
	drawResourcePoints(state, gameData);
	drawBuildings(state, gameData);
	drawUnits(state, gameData);
	drawHud(state, config, gameData);
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

void BattleRenderer::drawHud(const BattleState& state, const BattleConfigData& config, const GameData& gameData) const
{
	int32 playerResourceIncome = 0;
	int32 playerResourceCount = 0;
	for (const auto& resourcePoint : state.resourcePoints)
	{
		if (resourcePoint.owner == Owner::Player)
		{
			playerResourceIncome += resourcePoint.incomeAmount;
			++playerResourceCount;
		}
	}

	String productionText;
	for (const auto& slot : config.playerProductionSlots)
	{
		if (!productionText.isEmpty())
		{
			productionText += U" / ";
		}

		const int32 cost = FindUnitDefinition(config, slot.archetype) ? FindUnitDefinition(config, slot.archetype)->cost : 0;
		productionText += s3d::Format(slot.slot, U": ", GetArchetypeLabel(slot.archetype), U" @", GetArchetypeLabel(slot.producer), U" (", cost, U"G)");
	}

	String constructionText = U"Build: none";
	for (const auto& slot : config.playerConstructionSlots)
	{
		const int32 cost = FindUnitDefinition(config, slot.archetype) ? FindUnitDefinition(config, slot.archetype)->cost : 0;
		constructionText = s3d::Format(U"Build: ", slot.slot, U": ", GetArchetypeLabel(slot.archetype), U" (", cost, U"G)");
		break;
	}

	String queueText = U"Queue: idle";
	for (const auto& building : state.buildings)
	{
		const auto* unit = state.findUnit(building.unitId);
		if (unit && unit->isAlive && (unit->owner == Owner::Player))
		{
			if (!building.isConstructed)
			{
				queueText = s3d::Format(U"Build: ", GetArchetypeLabel(unit->archetype), U" (", building.constructionRemaining, U"s)");
				break;
			}

			if (!building.productionQueue.isEmpty())
			{
				const auto& currentItem = building.productionQueue.front();
				queueText = s3d::Format(U"Queue: ", GetArchetypeLabel(currentItem.archetype), U" @", GetArchetypeLabel(unit->archetype), U" x", building.productionQueue.size(), U" (", currentItem.remainingTime, U"s)");
				break;
			}
		}
	}

	RoundRect{ 16, 16, 480, 216, 8 }.draw(ColorF{ 0.0, 0.0, 0.0, 0.55 });
	gameData.uiFont(config.hud.title).draw(28, 26, Palette::White);
	gameData.smallFont(config.hud.controls).draw(28, 66, Palette::White);
	gameData.smallFont(productionText).draw(28, 88, Palette::White);
	gameData.smallFont(constructionText).draw(28, 110, Palette::White);
	gameData.smallFont(queueText).draw(28, 132, Palette::White);
	const String formationText = U"Formation: Q="
		+ GetFormationLabel(FormationType::Line)
		+ U" / W="
		+ GetFormationLabel(FormationType::Column)
		+ U" / Current="
		+ GetFormationLabel(state.playerFormation);
	gameData.smallFont(formationText).draw(28, 154, Palette::White);
	gameData.smallFont(s3d::Format(U"Resource: ", playerResourceCount, U" pts / +", playerResourceIncome, U" income")).draw(28, 176, Palette::White);
	gameData.smallFont(config.hud.escapeHint).draw(28, 198, Palette::White);
	gameData.smallFont(s3d::Format(U"Gold: ", state.playerGold)).draw(28, 220, Palette::Gold);

	if (state.winner)
	{
		const String label = (*state.winner == Owner::Player) ? U"PLAYER WIN" : U"ENEMY WIN";
		gameData.titleFont(label).drawAt(Scene::CenterF().movedBy(0, -30), Palette::White);
		gameData.smallFont(config.hud.winHint).drawAt(Scene::CenterF().movedBy(0, 18), Palette::White);
	}
}
