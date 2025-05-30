#pragma once
#include "Game.hpp"

Game::Game(GameData& saveData)
	: FsScene(U"Game"),
	m_saveData{ saveData }
{

}

Co::Task<void> Game::start()
{
	co_await mainLoop(); // メインループ実行
}

Co::Task<void> Game::mainLoop()
{
	const Font font{ FontMethod::MSDF, 48, Typeface::Bold };

	while (true)
	{
		if (shouldExit == false)
		{
			co_return;
		}

		co_await Co::NextFrame();
	}
}

void Game::draw() const
{
	FsScene::draw();
}
