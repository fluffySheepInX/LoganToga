#include "MapEditScene.h"

void MapEditScene::handleCanvasInput()
{
	const RectF canvasRect = getCanvasRect();
	if (!canvasRect.mouseOver())
	{
		return;
	}

	const Vec2 cursorWorld = clampWorldPosition(screenToWorld(Cursor::PosF()));

	if (MouseL.down())
	{
		switch (m_tool)
		{
		case Tool::AddUnit:
			m_config.initialUnits << InitialUnitPlacement{
				.owner = m_unitPlacementOwner,
				.archetype = m_unitPlacementArchetype,
				.position = cursorWorld,
			};
			m_selection = Selection{ SelectionKind::InitialUnit, static_cast<int32>(m_config.initialUnits.size() - 1) };
			m_statusMessage = U"Added initial unit";
			return;
		case Tool::AddObstacle:
		{
			ObstacleConfig obstacle;
			obstacle.label = U"Obstacle {}"_fmt(m_config.obstacles.size() + 1);
			obstacle.rect = RectF{ cursorWorld.x - 48.0, cursorWorld.y - 48.0, 96.0, 96.0 };
			clampObstacle(obstacle);
			m_config.obstacles << obstacle;
			m_selection = Selection{ SelectionKind::Obstacle, static_cast<int32>(m_config.obstacles.size() - 1) };
			m_statusMessage = U"Added obstacle";
			return;
		}
		case Tool::AddResource:
		{
			ResourcePointConfig resourcePoint;
			resourcePoint.label = U"Resource {}"_fmt(m_config.resourcePoints.size() + 1);
			resourcePoint.owner = m_resourcePlacementOwner;
			resourcePoint.position = cursorWorld;
			m_config.resourcePoints << resourcePoint;
			m_selection = Selection{ SelectionKind::ResourcePoint, static_cast<int32>(m_config.resourcePoints.size() - 1) };
			m_statusMessage = U"Added resource point";
			return;
		}
		case Tool::Select:
		default:
			m_selection = hitTest(cursorWorld);
			beginDragging(cursorWorld);
			return;
		}
	}

	if (MouseL.pressed() && m_dragOffset && (m_selection.kind != SelectionKind::None))
	{
		updateDragging(cursorWorld);
	}
}

void MapEditScene::beginDragging(const Vec2& cursorWorld)
{
	if (auto* placement = getSelectedUnit())
	{
		m_dragOffset = placement->position - cursorWorld;
		return;
	}

	if (auto* obstacle = getSelectedObstacle())
	{
		m_dragOffset = obstacle->rect.pos - cursorWorld;
		return;
	}

	if (auto* resourcePoint = getSelectedResourcePoint())
	{
		m_dragOffset = resourcePoint->position - cursorWorld;
		return;
	}

	m_dragOffset.reset();
}

void MapEditScene::updateDragging(const Vec2& cursorWorld)
{
	const Vec2 targetWorld = cursorWorld + *m_dragOffset;

	if (auto* placement = getSelectedUnit())
	{
		placement->position = clampWorldPosition(targetWorld);
		return;
	}

	if (auto* obstacle = getSelectedObstacle())
	{
		obstacle->rect.x = targetWorld.x;
		obstacle->rect.y = targetWorld.y;
		clampObstacle(*obstacle);
		return;
	}

	if (auto* resourcePoint = getSelectedResourcePoint())
	{
		resourcePoint->position = clampWorldPosition(targetWorld);
	}
}

MapEditScene::Selection MapEditScene::hitTest(const Vec2& cursorWorld) const
{
	for (int32 i = static_cast<int32>(m_config.initialUnits.size()) - 1; i >= 0; --i)
	{
		const auto& placement = m_config.initialUnits[static_cast<size_t>(i)];
		const double radius = (placement.archetype == UnitArchetype::Base) ? 22.0 : 16.0;
		if (placement.position.distanceFrom(cursorWorld) <= radius)
		{
			return Selection{ SelectionKind::InitialUnit, i };
		}
	}

	for (int32 i = static_cast<int32>(m_config.resourcePoints.size()) - 1; i >= 0; --i)
	{
		const auto& resourcePoint = m_config.resourcePoints[static_cast<size_t>(i)];
		if (resourcePoint.position.distanceFrom(cursorWorld) <= resourcePoint.radius)
		{
			return Selection{ SelectionKind::ResourcePoint, i };
		}
	}

	for (int32 i = static_cast<int32>(m_config.obstacles.size()) - 1; i >= 0; --i)
	{
		const auto& obstacle = m_config.obstacles[static_cast<size_t>(i)];
		if (obstacle.rect.contains(cursorWorld))
		{
			return Selection{ SelectionKind::Obstacle, i };
		}
	}

	return {};
}

void MapEditScene::deleteSelection()
{
	switch (m_selection.kind)
	{
	case SelectionKind::InitialUnit:
		if (isValidIndex(m_config.initialUnits, m_selection.index))
		{
			m_config.initialUnits.remove_at(static_cast<size_t>(m_selection.index));
			m_statusMessage = U"Deleted initial unit";
		}
		break;
	case SelectionKind::Obstacle:
		if (isValidIndex(m_config.obstacles, m_selection.index))
		{
			m_config.obstacles.remove_at(static_cast<size_t>(m_selection.index));
			m_statusMessage = U"Deleted obstacle";
		}
		break;
	case SelectionKind::ResourcePoint:
		if (isValidIndex(m_config.resourcePoints, m_selection.index))
		{
			m_config.resourcePoints.remove_at(static_cast<size_t>(m_selection.index));
			m_statusMessage = U"Deleted resource point";
		}
		break;
	case SelectionKind::None:
	default:
		return;
	}

	m_selection = {};
	m_dragOffset.reset();
}
