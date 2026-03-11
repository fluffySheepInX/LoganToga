#include "BattleScene.h"

BattleScene::BattleScene(const SceneBase::InitData& init)
	: SceneBase{ init }
	, m_clock{ 1.0 / 60.0 }
{
	m_cameraCenter = m_session.state().worldBounds.center();
}

void BattleScene::update()
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

	m_inputController.handleFormationInput(m_session);

	const Vec2 constructionCursorWorldPos = screenToWorld(Cursor::PosF());
	m_constructionController.handleInput(m_session, constructionCursorWorldPos);

	if (!m_session.state().pendingConstructionArchetype)
	{
		const bool allowClickSelection = updateCameraPan();
		const Vec2 cursorWorldPos = screenToWorld(Cursor::PosF());
		m_inputController.handleSelectionInput(m_session, cursorWorldPos, allowClickSelection);
		m_inputController.handleCommandInput(m_session, cursorWorldPos);
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
	m_renderer.draw(m_session.state(), m_session.config(), getData(), m_cameraCenter);
}

Vec2 BattleScene::screenToWorld(const Vec2& screenPosition) const
{
	return screenPosition + (m_cameraCenter - Scene::CenterF());
}

Vec2 BattleScene::clampCameraCenter(const Vec2& desiredCenter) const
{
	const RectF& worldBounds = m_session.state().worldBounds;
	const Vec2 halfViewport = (Scene::Size() * 0.5);
	Vec2 clamped = desiredCenter;

	if (worldBounds.w <= Scene::Width())
	{
		clamped.x = worldBounds.center().x;
	}
	else
	{
		clamped.x = Clamp(clamped.x, worldBounds.leftX() + halfViewport.x, worldBounds.rightX() - halfViewport.x);
	}

	if (worldBounds.h <= Scene::Height())
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
		m_cameraPanStartCenter = m_cameraCenter;
		m_isCameraPanning = false;
	}

	if (m_cameraPanStartCursor && MouseL.pressed())
	{
		const Vec2 dragDelta = (*m_cameraPanStartCursor - Cursor::PosF());
		if (dragDelta.lengthSq() >= 16.0)
		{
			m_isCameraPanning = true;
		}

		if (m_isCameraPanning)
		{
			m_cameraCenter = clampCameraCenter(m_cameraPanStartCenter + dragDelta);
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
