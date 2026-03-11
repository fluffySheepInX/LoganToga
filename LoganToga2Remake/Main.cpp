# include <Siv3D.hpp>
# include "Core/Game.h"
# include "Scenes/BattleScene.h"

void Main()
{
    s3d::Window::Resize(1280, 720);
    s3d::Scene::SetBackground(s3d::ColorF{ 0.11, 0.13, 0.16 });

    Game game{ std::make_unique<BattleScene>() };
    game.run();
}
