#pragma once

#include "BattleConfig.h"
#include "BattleState.h"
#include "GameData.h"

class BattleRenderer
{
public:
	void draw(const BattleState& state, const BattleConfigData& config, const GameData& gameData, const Camera2D& camera) const;

private:
	void drawWorld(const BattleState& state) const;
	void drawSquads(const BattleState& state) const;
	void drawObstacles(const BattleConfigData& config, const GameData& gameData) const;
	void drawConstructionPreview(const BattleState& state, const BattleConfigData& config, const GameData& gameData) const;
	void drawPendingConstructionOrders(const BattleState& state, const BattleConfigData& config, const GameData& gameData) const;
	void drawResourcePoints(const BattleState& state, const GameData& gameData) const;
	void drawBuildings(const BattleState& state, const GameData& gameData) const;
	void drawAttackEffects(const BattleState& state) const;
	void drawMeleeAttackEffects(const BattleState& state) const;
	void drawUnits(const BattleState& state, const GameData& gameData) const;
	void drawHud(const BattleState& state, const BattleConfigData& config, const GameData& gameData) const;
};
