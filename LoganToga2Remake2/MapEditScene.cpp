#include "MapEditScene.h"

MapEditScene::MapEditScene(const SceneBase::InitData& init)
	: SceneBase{ init }
	, m_battleConfigPath{ U"config/battle.toml" }
	, m_mapConfigPath{ resolveMapConfigPath(m_battleConfigPath) }
{
	reloadConfig();
}

InitialUnitPlacement* MapEditScene::getSelectedUnit()
{
	if ((m_selection.kind != SelectionKind::InitialUnit) || !isValidIndex(m_config.initialUnits, m_selection.index))
	{
		return nullptr;
	}
	return &m_config.initialUnits[static_cast<size_t>(m_selection.index)];
}

const InitialUnitPlacement* MapEditScene::getSelectedUnit() const
{
	if ((m_selection.kind != SelectionKind::InitialUnit) || !isValidIndex(m_config.initialUnits, m_selection.index))
	{
		return nullptr;
	}
	return &m_config.initialUnits[static_cast<size_t>(m_selection.index)];
}

ObstacleConfig* MapEditScene::getSelectedObstacle()
{
	if ((m_selection.kind != SelectionKind::Obstacle) || !isValidIndex(m_config.obstacles, m_selection.index))
	{
		return nullptr;
	}
	return &m_config.obstacles[static_cast<size_t>(m_selection.index)];
}

const ObstacleConfig* MapEditScene::getSelectedObstacle() const
{
	if ((m_selection.kind != SelectionKind::Obstacle) || !isValidIndex(m_config.obstacles, m_selection.index))
	{
		return nullptr;
	}
	return &m_config.obstacles[static_cast<size_t>(m_selection.index)];
}

ResourcePointConfig* MapEditScene::getSelectedResourcePoint()
{
	if ((m_selection.kind != SelectionKind::ResourcePoint) || !isValidIndex(m_config.resourcePoints, m_selection.index))
	{
		return nullptr;
	}
	return &m_config.resourcePoints[static_cast<size_t>(m_selection.index)];
}

const ResourcePointConfig* MapEditScene::getSelectedResourcePoint() const
{
	if ((m_selection.kind != SelectionKind::ResourcePoint) || !isValidIndex(m_config.resourcePoints, m_selection.index))
	{
		return nullptr;
	}
	return &m_config.resourcePoints[static_cast<size_t>(m_selection.index)];
}

void MapEditScene::clampObstacle(ObstacleConfig& obstacle) const
{
	obstacle.rect.w = Clamp(obstacle.rect.w, 16.0, m_config.world.width);
	obstacle.rect.h = Clamp(obstacle.rect.h, 16.0, m_config.world.height);
	obstacle.rect.x = Clamp(obstacle.rect.x, 0.0, m_config.world.width - obstacle.rect.w);
	obstacle.rect.y = Clamp(obstacle.rect.y, 0.0, m_config.world.height - obstacle.rect.h);
}
