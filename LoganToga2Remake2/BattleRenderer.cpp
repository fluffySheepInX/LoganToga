#include "BattleRenderer.h"

void BattleRenderer::draw(const BattleState& state, const BattleConfigData& config, const GameData& gameData, const Camera2D& camera, const double interpolationAlpha) const
{
	{
		const auto transformer = camera.createTransformer();
		drawWorld(state);
		drawSquads(state);
		drawObstacles(config, gameData);
		drawConstructionPreview(state, config, gameData);
		drawPendingConstructionOrders(state, config, gameData);
		drawResourcePoints(state, gameData);
		drawBuildings(state, gameData);
		drawAttackEffects(state);
		drawUnits(state, gameData, interpolationAlpha);
		drawMeleeAttackEffects(state);
	}

	drawHud(state, config, gameData);
}
