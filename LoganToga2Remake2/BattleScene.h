#pragma once

#include "BattleRenderer.h"
#include "BattleSession.h"
#include "FixedStepClock.h"
#include "GameData.h"

class BattleScene : public SceneBase
{
public:
	explicit BattleScene(const SceneBase::InitData& init)
		: SceneBase{ init }
		, m_clock{ 1.0 / 60.0 } {}

	void update() override
	{
		if (KeyEscape.down())
		{
			changeScene(U"Title");
			return;
		}

		if (m_session.state().winner)
		{
			if (KeyEnter.down())
			{
				changeScene(U"Title");
				return;
			}

			if (KeyR.down())
			{
				changeScene(U"Battle");
				return;
			}
		}

		handleSelectionInput();
		handleCommandInput();
		handleProductionInput();

		const size_t fixedSteps = m_clock.beginFrame();
		for (size_t i = 0; i < fixedSteps; ++i)
		{
			m_session.update(m_clock.stepSeconds());
		}
	}

	void draw() const override
	{
		m_renderer.draw(m_session.state(), m_session.config(), getData());
	}

private:
	BattleSession m_session;
	BattleRenderer m_renderer;
	FixedStepClock m_clock;

	[[nodiscard]] static bool isProductionSlotTriggered(const int32 slot)
	{
		switch (slot)
		{
		case 1:
			return Key1.down();
		case 2:
			return Key2.down();
		default:
			return false;
		}
	}

	void handleSelectionInput()
	{
		auto& state = m_session.state();

		if (MouseL.down())
		{
			state.isSelecting = true;
			state.selectionStart = Cursor::PosF();
			state.selectionRect = RectF{ state.selectionStart, 0, 0 };
		}

		if (state.isSelecting && MouseL.pressed())
		{
			state.selectionRect = RectF::FromPoints(state.selectionStart, Cursor::PosF());
		}

		if (state.isSelecting && MouseL.up())
		{
			RectF selectionRect = state.selectionRect;
			if ((selectionRect.w < 4.0) && (selectionRect.h < 4.0))
			{
				const Vec2 cursorPos = Cursor::PosF();
				selectionRect = RectF{ cursorPos.x - 6.0, cursorPos.y - 6.0, 12.0, 12.0 };
			}

			m_session.enqueue(SelectUnitsInRectCommand{ selectionRect, KeyShift.pressed() });
			state.isSelecting = false;
			state.selectionRect = RectF{ 0, 0, 0, 0 };
		}
	}

	void handleCommandInput()
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

	void handleProductionInput()
	{
		if (KeyX.down())
		{
			m_session.cancelLastPlayerProduction();
		}

		for (const auto& slot : m_session.config().playerProductionSlots)
		{
			if (isProductionSlotTriggered(slot.slot))
			{
				m_session.trySpawnPlayerUnit(slot.archetype);
			}
		}
	}
};
