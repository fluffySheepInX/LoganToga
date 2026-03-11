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
