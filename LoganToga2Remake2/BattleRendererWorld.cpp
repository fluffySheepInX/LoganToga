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
