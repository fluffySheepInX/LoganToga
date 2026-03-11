#include "BattleRenderer.h"

void BattleRenderer::draw(const BattleState& state, const BattleConfigData& config, const GameData& gameData, const Vec2& cameraCenter) const
{
	{
		const s3d::Transformer2D transformer{ s3d::Mat3x2::Translate(Scene::CenterF() - cameraCenter) };
		drawWorld(state);
		drawSquads(state);
		drawObstacles(config, gameData);
		drawConstructionPreview(state, config, gameData);
		drawResourcePoints(state, gameData);
		drawBuildings(state, gameData);
		drawUnits(state, gameData);
	}

	drawHud(state, config, gameData);
}
