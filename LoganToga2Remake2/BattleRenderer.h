#pragma once

#include "BattleConfig.h"
#include "GameData.h"

class BattleRenderer
{
public:
	void draw(const BattleState& state, const BattleConfigData& config, const GameData& gameData) const;

private:
	void drawWorld(const BattleState& state) const;
	void drawBuildings(const BattleState& state, const GameData& gameData) const;
	void drawUnits(const BattleState& state, const GameData& gameData) const;
	void drawHud(const BattleState& state, const BattleConfigData& config, const GameData& gameData) const;
};
