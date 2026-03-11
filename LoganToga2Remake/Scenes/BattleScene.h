#pragma once
# include <Siv3D.hpp>
# include "../Core/Game.h"
# include "../Battle/BattleSession.h"
# include "../Render/BattleRenderer.h"

class BattleScene : public AppScene
{
public:
    BattleScene();

    void handleInput() override;
    void fixedUpdate(double deltaTime) override;
    void draw() const override;

private:
    BattleSession m_session;
    BattleRenderer m_renderer;

    void handleSelectionInput();
    void handleCommandInput();
    void handleProductionInput();
};
