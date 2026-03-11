#include "BattleRenderer.h"

void BattleRenderer::draw(const BattleState& state, const BattleConfigData& config, const GameData& gameData) const
{
	drawWorld(state);
	drawBuildings(state, gameData);
	drawUnits(state, gameData);
	drawHud(state, config, gameData);
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

void BattleRenderer::drawBuildings(const BattleState& state, const GameData& gameData) const
{
	for (const auto& building : state.buildings)
	{
		const auto* unit = state.findUnit(building.unitId);
		if (!(unit && unit->isAlive && (unit->archetype == UnitArchetype::Base)))
		{
			continue;
		}

		Circle{ unit->position, unit->radius + 16 }.draw(ColorF{ GetOwnerColor(unit->owner), 0.12 });

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
		gameData.smallFont(U"{} x{}"_fmt(GetArchetypeLabel(currentItem.archetype), building.productionQueue.size())).drawAt(unit->position.movedBy(0, unit->radius + 30), Palette::White);
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

		if (unit.archetype == UnitArchetype::Base)
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
	String productionText;
	for (const auto& slot : config.playerProductionSlots)
	{
		if (!productionText.isEmpty())
		{
			productionText += U" / ";
		}

		const int32 cost = FindUnitDefinition(config, slot.archetype) ? FindUnitDefinition(config, slot.archetype)->cost : 0;
		productionText += U"{}: {} ({}G)"_fmt(slot.slot, GetArchetypeLabel(slot.archetype), cost);
	}

	String queueText = U"Queue: idle";
	for (const auto& building : state.buildings)
	{
		const auto* unit = state.findUnit(building.unitId);
		if (unit && unit->isAlive && (unit->owner == Owner::Player) && (unit->archetype == UnitArchetype::Base))
		{
			if (!building.productionQueue.isEmpty())
			{
				const auto& currentItem = building.productionQueue.front();
				queueText = U"Queue: {} x{} ({:.1f}s)"_fmt(GetArchetypeLabel(currentItem.archetype), building.productionQueue.size(), currentItem.remainingTime);
			}
			break;
		}
	}

	RoundRect{ 16, 16, 440, 170, 8 }.draw(ColorF{ 0.0, 0.0, 0.0, 0.55 });
	gameData.uiFont(config.hud.title).draw(28, 26, Palette::White);
	gameData.smallFont(config.hud.controls).draw(28, 66, Palette::White);
	gameData.smallFont(productionText).draw(28, 88, Palette::White);
	gameData.smallFont(queueText).draw(28, 110, Palette::White);
	gameData.smallFont(config.hud.escapeHint).draw(28, 132, Palette::White);
	gameData.smallFont(U"Gold: {}"_fmt(state.playerGold)).draw(28, 154, Palette::Gold);

	if (state.winner)
	{
		const String label = (*state.winner == Owner::Player) ? U"PLAYER WIN" : U"ENEMY WIN";
		gameData.titleFont(label).drawAt(Scene::CenterF().movedBy(0, -30), Palette::White);
		gameData.smallFont(config.hud.winHint).drawAt(Scene::CenterF().movedBy(0, 18), Palette::White);
	}
}
