# include "BattleScene.h"

BattleScene::BattleScene() = default;

void BattleScene::handleInput()
{
    handleSelectionInput();
    handleCommandInput();
    handleProductionInput();
}

void BattleScene::fixedUpdate(const double deltaTime)
{
    m_session.update(deltaTime);
}

void BattleScene::draw() const
{
    m_renderer.draw(m_session.state());
}

void BattleScene::handleSelectionInput()
{
    auto& state = m_session.state();

    if (MouseL.down())
    {
        state.isSelecting = true;
        state.selectionStart = Cursor::PosF();
        state.selectionRect = s3d::RectF{ state.selectionStart, 0, 0 };
    }

    if (state.isSelecting && MouseL.pressed())
    {
        state.selectionRect = s3d::RectF::FromPoints(state.selectionStart, Cursor::PosF());
    }

    if (state.isSelecting && MouseL.up())
    {
        s3d::RectF selectionRect = state.selectionRect;
        if ((selectionRect.w < 4.0) && (selectionRect.h < 4.0))
        {
            const s3d::Vec2 cursorPos = Cursor::PosF();
            selectionRect = s3d::RectF{ cursorPos.x - 6.0, cursorPos.y - 6.0, 12.0, 12.0 };
        }

        m_session.enqueue(SelectUnitsInRectCommand{ selectionRect, KeyShift.pressed() });
        state.isSelecting = false;
        state.selectionRect = s3d::RectF{ 0, 0, 0, 0 };
    }
}

void BattleScene::handleCommandInput()
{
    if (!MouseR.down())
    {
        return;
    }

    const auto selectedIds = m_session.getSelectedPlayerUnitIds();
    if (selectedIds.isEmpty())
    {
        return;
    }

    if (const auto targetUnitId = m_session.findEnemyAt(Cursor::PosF()))
    {
        m_session.enqueue(AttackUnitCommand{ selectedIds, *targetUnitId });
        return;
    }

    m_session.enqueue(MoveUnitsCommand{ selectedIds, Cursor::PosF() });
}

void BattleScene::handleProductionInput()
{
    if (Key1.down())
    {
        m_session.trySpawnPlayerUnit(UnitArchetype::Soldier);
    }

    if (Key2.down())
    {
        m_session.trySpawnPlayerUnit(UnitArchetype::Archer);
    }
}
