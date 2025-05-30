#pragma once
#include "Common.h"

class Game : public FsScene
{
public:
	Game(GameData& saveData);
private:
	Co::Task<void> start() override;

	Co::Task<void> mainLoop();

	void draw() const override;

	GameData& m_saveData;
	const Font font{ FontMethod::MSDF, 48, Typeface::Bold };
};
