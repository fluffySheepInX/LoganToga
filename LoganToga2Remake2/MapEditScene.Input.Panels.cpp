#include "MapEditScene.h"

bool MapEditScene::handleLeftPanelInput()
{
	const RectF panelRect = getLeftPanelRect();
	if (!panelRect.mouseOver() || !MouseL.down())
	{
		return false;
	}

	if (isButtonClicked(getPanelButtonRect(panelRect, 0)))
	{
		saveMap();
		return true;
	}

	if (isButtonClicked(getPanelButtonRect(panelRect, 1)))
	{
		reloadConfig();
		return true;
	}

	if (isButtonClicked(getPanelButtonRect(panelRect, 2)))
	{
		startTestPlay();
		return true;
	}

	if (isButtonClicked(getPanelButtonRect(panelRect, 3)))
	{
		changeScene(U"Title");
		return true;
	}

	if (isButtonClicked(getPanelButtonRect(panelRect, 4)))
	{
		m_tool = Tool::Select;
		m_statusMessage = U"Tool: Select";
		return true;
	}

	if (isButtonClicked(getPanelButtonRect(panelRect, 5)))
	{
		m_tool = Tool::AddUnit;
		m_statusMessage = U"Tool: Add Unit";
		return true;
	}

	if (isButtonClicked(getPanelButtonRect(panelRect, 6)))
	{
		m_tool = Tool::AddObstacle;
		m_statusMessage = U"Tool: Add Obstacle";
		return true;
	}

	if (isButtonClicked(getPanelButtonRect(panelRect, 7)))
	{
		m_tool = Tool::AddResource;
		m_statusMessage = U"Tool: Add Resource";
		return true;
	}

	const RectF cycleUnitOwnerRect{ panelRect.x + 16, panelRect.y + 476, panelRect.w - 32, 28 };
	if (isButtonClicked(cycleUnitOwnerRect))
	{
		m_unitPlacementOwner = cycleUnitPlacementOwner(m_unitPlacementOwner);
		return true;
	}

	const RectF cycleArchetypeRect{ panelRect.x + 16, panelRect.y + 540, panelRect.w - 32, 28 };
	if (isButtonClicked(cycleArchetypeRect))
	{
		m_unitPlacementArchetype = cycleUnitArchetype(m_unitPlacementArchetype);
		return true;
	}

	const RectF cycleResourceOwnerRect{ panelRect.x + 16, panelRect.y + 604, panelRect.w - 32, 28 };
	if (isButtonClicked(cycleResourceOwnerRect))
	{
		m_resourcePlacementOwner = cycleResourceOwner(m_resourcePlacementOwner);
		return true;
	}

	return true;
}

bool MapEditScene::handleRightPanelInput()
{
	const RectF panelRect = getRightPanelRect();
	if (!panelRect.mouseOver() || !MouseL.down())
	{
		return false;
	}

	if (auto* placement = getSelectedUnit())
	{
		if (isButtonClicked(RectF{ panelRect.x + 16, panelRect.y + 170, panelRect.w - 32, 30 }))
		{
			placement->owner = cycleUnitPlacementOwner(placement->owner);
			return true;
		}
		if (isButtonClicked(RectF{ panelRect.x + 16, panelRect.y + 208, panelRect.w - 32, 30 }))
		{
			placement->archetype = cycleUnitArchetype(placement->archetype);
			return true;
		}
		if (isButtonClicked(RectF{ panelRect.x + 16, panelRect.y + 246, panelRect.w - 32, 30 }))
		{
			deleteSelection();
			return true;
		}
		return true;
	}

	if (auto* obstacle = getSelectedObstacle())
	{
		if (isButtonClicked(RectF{ panelRect.x + 16, panelRect.y + 194, panelRect.w - 32, 30 }))
		{
			obstacle->rect.w += 16.0;
			clampObstacle(*obstacle);
			return true;
		}
		if (isButtonClicked(RectF{ panelRect.x + 16, panelRect.y + 232, panelRect.w - 32, 30 }))
		{
			obstacle->rect.w = Max(16.0, obstacle->rect.w - 16.0);
			clampObstacle(*obstacle);
			return true;
		}
		if (isButtonClicked(RectF{ panelRect.x + 16, panelRect.y + 270, panelRect.w - 32, 30 }))
		{
			obstacle->rect.h += 16.0;
			clampObstacle(*obstacle);
			return true;
		}
		if (isButtonClicked(RectF{ panelRect.x + 16, panelRect.y + 308, panelRect.w - 32, 30 }))
		{
			obstacle->rect.h = Max(16.0, obstacle->rect.h - 16.0);
			clampObstacle(*obstacle);
			return true;
		}
		if (isButtonClicked(RectF{ panelRect.x + 16, panelRect.y + 346, panelRect.w - 32, 30 }))
		{
			obstacle->blocksMovement = !obstacle->blocksMovement;
			return true;
		}
		if (isButtonClicked(RectF{ panelRect.x + 16, panelRect.y + 384, panelRect.w - 32, 30 }))
		{
			deleteSelection();
			return true;
		}
		return true;
	}

	if (auto* resourcePoint = getSelectedResourcePoint())
	{
		if (isButtonClicked(RectF{ panelRect.x + 16, panelRect.y + 246, panelRect.w - 32, 30 }))
		{
			resourcePoint->owner = cycleResourceOwner(resourcePoint->owner);
			return true;
		}
		if (isButtonClicked(RectF{ panelRect.x + 16, panelRect.y + 284, panelRect.w - 32, 30 }))
		{
			resourcePoint->radius += 4.0;
			return true;
		}
		if (isButtonClicked(RectF{ panelRect.x + 16, panelRect.y + 322, panelRect.w - 32, 30 }))
		{
			resourcePoint->radius = Max(8.0, resourcePoint->radius - 4.0);
			return true;
		}
		if (isButtonClicked(RectF{ panelRect.x + 16, panelRect.y + 360, panelRect.w - 32, 30 }))
		{
			++resourcePoint->incomeAmount;
			return true;
		}
		if (isButtonClicked(RectF{ panelRect.x + 16, panelRect.y + 398, panelRect.w - 32, 30 }))
		{
			resourcePoint->incomeAmount = Max(0, resourcePoint->incomeAmount - 1);
			return true;
		}
		if (isButtonClicked(RectF{ panelRect.x + 16, panelRect.y + 436, panelRect.w - 32, 30 }))
		{
			resourcePoint->captureTime += 0.1;
			return true;
		}
		if (isButtonClicked(RectF{ panelRect.x + 16, panelRect.y + 474, panelRect.w - 32, 30 }))
		{
			resourcePoint->captureTime = Max(0.1, resourcePoint->captureTime - 0.1);
			return true;
		}
		if (isButtonClicked(RectF{ panelRect.x + 16, panelRect.y + 512, panelRect.w - 32, 30 }))
		{
			deleteSelection();
			return true;
		}
		return true;
	}

	return true;
}
