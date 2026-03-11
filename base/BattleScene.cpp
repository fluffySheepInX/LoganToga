#include "BattleScene.h"

BattleScene::BattleScene(const SceneBase::InitData& init)
	: SceneBase{ init }
	, m_clock{ 1.0 / 60.0 }
{
	m_camera.jumpTo(m_session.state().worldBounds.center(), 1.0);
}

void BattleScene::update()
{
	m_camera.update();

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

	m_inputController.handleFormationInput(m_session);

	const Vec2 cursorScreenPos = Cursor::PosF();
	const Vec2 constructionCursorWorldPos = screenToWorld(cursorScreenPos);
	m_constructionController.handleInput(m_session, constructionCursorWorldPos);

	if (!m_session.state().pendingConstructionArchetype)
	{
		const bool isCursorOnCommandPanel = m_inputController.isCursorOnCommandPanel(m_session, cursorScreenPos);
		const bool handledCommandPanelClick = m_inputController.handleCommandPanelClick(m_session, cursorScreenPos);
		if (isCursorOnCommandPanel)
		{
			resetCameraPan();
		}

		const Vec2 cursorWorldPos = screenToWorld(cursorScreenPos);
		if (!isCursorOnCommandPanel)
		{
			const bool allowClickSelection = updateCameraPan();
			m_inputController.handleSelectionInput(m_session, cursorWorldPos, allowClickSelection);
			m_inputController.handleCommandInput(m_session, cursorWorldPos);
		}
		else if (handledCommandPanelClick)
		{
			m_session.state().isSelecting = false;
			m_session.state().selectionRect = RectF{ 0, 0, 0, 0 };
		}
	}
	else
	{
		resetCameraPan();
	}

	handleProductionInput();

	const size_t fixedSteps = m_clock.beginFrame();
	for (size_t i = 0; i < fixedSteps; ++i)
	{
		m_session.update(m_clock.stepSeconds());
	}
}

void BattleScene::draw() const
{
	m_renderer.draw(m_session.state(), m_session.config(), getData(), m_camera);
}

Vec2 BattleScene::screenToWorld(const Vec2& screenPosition) const
{
	return ((screenPosition - Scene::CenterF()) / m_camera.getScale()) + m_camera.getCenter();
}

Vec2 BattleScene::clampCameraCenter(const Vec2& desiredCenter) const
{
	const RectF& worldBounds = m_session.state().worldBounds;
	const Vec2 halfViewport = ((Scene::Size() * 0.5) / m_camera.getScale());
	Vec2 clamped = desiredCenter;

	if (worldBounds.w <= (halfViewport.x * 2.0))
	{
		clamped.x = worldBounds.center().x;
	}
	else
	{
		clamped.x = Clamp(clamped.x, worldBounds.leftX() + halfViewport.x, worldBounds.rightX() - halfViewport.x);
	}

	if (worldBounds.h <= (halfViewport.y * 2.0))
	{
		clamped.y = worldBounds.center().y;
	}
	else
	{
		clamped.y = Clamp(clamped.y, worldBounds.topY() + halfViewport.y, worldBounds.bottomY() - halfViewport.y);
	}

	return clamped;
}

bool BattleScene::updateCameraPan()
{
	if (MouseL.down())
	{
		m_cameraPanStartCursor = Cursor::PosF();
		m_cameraPanStartCenter = m_camera.getCenter();
		m_isCameraPanning = false;
	}

	if (m_cameraPanStartCursor && MouseL.pressed())
	{
		const Vec2 dragDelta = (*m_cameraPanStartCursor - Cursor::PosF()) / m_camera.getScale();
		if (dragDelta.lengthSq() >= 16.0)
		{
			m_isCameraPanning = true;
		}

		if (m_isCameraPanning)
		{
			const Vec2 nextCenter = clampCameraCenter(m_cameraPanStartCenter + dragDelta);
			m_camera.setTargetCenter(nextCenter);
			m_camera.jumpTo(nextCenter, m_camera.getScale());
		}
	}

	if (!(m_cameraPanStartCursor && MouseL.up()))
	{
		return false;
	}

	const Vec2 releaseDelta = (Cursor::PosF() - *m_cameraPanStartCursor);
	const bool isClick = (!m_isCameraPanning) && (releaseDelta.lengthSq() < 16.0);
	resetCameraPan();
	return isClick;
}

void BattleScene::resetCameraPan()
{
	m_cameraPanStartCursor.reset();
	m_isCameraPanning = false;
}

bool BattleScene::isProductionSlotTriggered(const int32 slot)
{
	switch (slot)
	{
	case 1:
		return Key1.down();
	case 2:
		return Key2.down();
	case 3:
		return Key3.down();
	default:
		return false;
	}
}

void BattleScene::handleProductionInput()
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
