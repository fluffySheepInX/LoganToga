#include "BattleScene.h"

namespace
{
	constexpr int32 PauseMenuItemCount = 3;

	[[nodiscard]] Array<String> GetPauseMenuLabels()
	{
		return{ U"Resume", U"Retry", U"Back to Title" };
	}

	[[nodiscard]] RectF GetPausePanelRect()
	{
		return RectF{ Arg::center(Scene::CenterF()), 420, 320 };
	}

	[[nodiscard]] RectF GetPauseMenuItemRect(const int32 index)
	{
		const RectF panelRect = GetPausePanelRect();
		return RectF{ panelRect.x + 30, panelRect.y + 78 + (index * 58), panelRect.w - 60, 44 };
	}
}

BattleScene::BattleScene(const SceneBase::InitData& init)
	: SceneBase{ init }
	, m_clock{ 1.0 / 60.0 }
{
	m_camera.jumpTo(m_session.state().worldBounds.center(), 1.0);
}

void BattleScene::update()
{
	if (m_session.state().winner)
	{
		m_isPaused = false;
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

	if (KeyEscape.down())
	{
		togglePause();
		return;
	}

	if (m_isPaused)
	{
		updatePauseMenu();
		return;
	}

	m_camera.update();

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
	if (m_isPaused)
	{
		drawPauseMenu();
	}
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

void BattleScene::clearTransientInputState()
{
	resetCameraPan();
	auto& state = m_session.state();
	state.isSelecting = false;
	state.selectionStart = Vec2::Zero();
	state.selectionRect = RectF{ 0, 0, 0, 0 };
	state.isCommandDragging = false;
	state.commandDragStart = Vec2::Zero();
	state.commandDragCurrent = Vec2::Zero();
}

void BattleScene::togglePause()
{
	if (m_session.state().winner)
	{
		return;
	}

	m_isPaused = !m_isPaused;
	clearTransientInputState();
	if (m_isPaused)
	{
		m_pauseMenuIndex = 0;
	}
}

void BattleScene::updatePauseMenu()
{
	if (KeyEscape.down())
	{
		m_isPaused = false;
		clearTransientInputState();
		return;
	}

	if (KeyUp.down())
	{
		m_pauseMenuIndex = (m_pauseMenuIndex + PauseMenuItemCount - 1) % PauseMenuItemCount;
	}
	else if (KeyDown.down())
	{
		m_pauseMenuIndex = (m_pauseMenuIndex + 1) % PauseMenuItemCount;
	}

	for (int32 i = 0; i < PauseMenuItemCount; ++i)
	{
		if (GetPauseMenuItemRect(i).mouseOver())
		{
			m_pauseMenuIndex = i;
			break;
		}
	}

	const bool decidedByMouse = MouseL.down() && GetPauseMenuItemRect(m_pauseMenuIndex).mouseOver();
	const bool decidedByKey = KeyEnter.down();
	if (!(decidedByMouse || decidedByKey))
	{
		return;
	}

	switch (m_pauseMenuIndex)
	{
	case 0:
		m_isPaused = false;
		clearTransientInputState();
		break;
	case 1:
		changeScene(U"Battle");
		break;
	case 2:
		changeScene(U"Title");
		break;
	default:
		break;
	}
}

void BattleScene::drawPauseMenu() const
{
	const auto& data = getData();
	const RectF panelRect = GetPausePanelRect();
	const Array<String> labels = GetPauseMenuLabels();

	Scene::Rect().draw(ColorF{ 0.0, 0.0, 0.0, 0.45 });
	RoundRect{ panelRect, 14 }.draw(ColorF{ 0.04, 0.06, 0.10, 0.96 });
	RoundRect{ panelRect, 14 }.drawFrame(2, 0, ColorF{ 0.32, 0.48, 0.82, 0.96 });
	data.titleFont(U"PAUSED").drawAt(panelRect.center().movedBy(0, -110), Palette::White);

	for (int32 i = 0; i < labels.size(); ++i)
	{
		const RectF buttonRect = GetPauseMenuItemRect(i);
		const bool isHovered = buttonRect.mouseOver();
		const bool isSelected = (m_pauseMenuIndex == i);
		const ColorF fillColor = isSelected
			? ColorF{ 0.20, 0.28, 0.48, 0.98 }
			: (isHovered ? ColorF{ 0.13, 0.18, 0.30, 0.98 } : ColorF{ 0.08, 0.11, 0.18, 0.96 });
		RoundRect{ buttonRect, 10 }.draw(fillColor);
		RoundRect{ buttonRect, 10 }.drawFrame(isSelected ? 3 : 2, 0, ColorF{ 0.55, 0.72, 1.0, 0.96 });
		data.uiFont(labels[i]).drawAt(buttonRect.center(), Palette::White);
	}

	data.smallFont(U"Esc: resume / Up-Down: select / Enter: confirm").drawAt(panelRect.center().movedBy(0, 108), ColorF{ 0.82, 0.87, 0.94 });
	data.smallFont(m_session.config().hud.controls).drawAt(panelRect.center().movedBy(0, 132), ColorF{ 0.72, 0.78, 0.86 });
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
