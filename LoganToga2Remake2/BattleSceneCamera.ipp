Vec2 BattleScene::screenToWorld(const Vec2& screenPosition) const
{
	return ((screenPosition - Scene::CenterF()) / m_camera.getScale()) + m_camera.getCenter();
}

RectF BattleScene::getCameraWorldRect(const double margin) const
{
	const Vec2 halfViewport = ((Scene::Size() * 0.5) / m_camera.getScale());
	return RectF{
		m_camera.getCenter().x - halfViewport.x - margin,
		m_camera.getCenter().y - halfViewport.y - margin,
		(halfViewport.x * 2.0) + (margin * 2.0),
		(halfViewport.y * 2.0) + (margin * 2.0)
	};
}

Vec2 BattleScene::clampCameraCenter(const Vec2& desiredCenter) const
{
	const RectF& worldBounds = m_session.state().worldBounds;
    return Vec2{
		Clamp(desiredCenter.x, worldBounds.leftX(), worldBounds.rightX()),
		Clamp(desiredCenter.y, worldBounds.topY(), worldBounds.bottomY())
	};
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
