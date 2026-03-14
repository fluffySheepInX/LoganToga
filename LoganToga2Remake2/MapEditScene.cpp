#include "MapEditScene.h"

MapEditScene::MapEditScene(const SceneBase::InitData& init)
	: SceneBase{ init }
	, m_battleConfigPath{ U"config/battle.toml" }
	, m_mapConfigPath{ resolveMapConfigPath(m_battleConfigPath) }
{
	reloadConfig();
}

void MapEditScene::update()
{
	if (KeyEscape.down())
	{
		changeScene(U"Title");
		return;
	}

	if (MouseL.up())
	{
		m_dragOffset.reset();
	}

	if (s3d::KeyDelete.down())
	{
		deleteSelection();
		return;
	}

	if ((s3d::KeyControl.pressed() && s3d::KeyS.down()) || s3d::KeyF5.down())
	{
		saveMap();
		return;
	}

	if (KeyR.down())
	{
		reloadConfig();
		return;
	}

	if (handleLeftPanelInput() || handleRightPanelInput())
	{
		return;
	}

	handleCanvasInput();
}

void MapEditScene::draw() const
{
	Scene::Rect().draw(ColorF{ 0.07, 0.09, 0.12 });

	const RectF leftPanel = getLeftPanelRect();
	const RectF rightPanel = getRightPanelRect();
	const RectF canvasRect = getCanvasRect();

	leftPanel.draw(ColorF{ 0.12, 0.15, 0.19 });
	leftPanel.drawFrame(2, ColorF{ 0.28, 0.40, 0.62 });
	rightPanel.draw(ColorF{ 0.12, 0.15, 0.19 });
	rightPanel.drawFrame(2, ColorF{ 0.28, 0.40, 0.62 });
	canvasRect.draw(ColorF{ 0.08, 0.10, 0.13 });
	canvasRect.drawFrame(2, ColorF{ 0.28, 0.40, 0.62 });

	drawCanvas(canvasRect);
	drawLeftPanel(leftPanel);
	drawRightPanel(rightPanel);
}

String MapEditScene::resolveMapConfigPath(const String& battleConfigPath)
{
	const TOMLReader rootToml{ battleConfigPath };
	if (!rootToml)
	{
		throw Error{ U"Failed to load battle config: " + battleConfigPath };
	}

	const String mapSource = rootToml[U"sources"][U"map"].get<String>();
	return ResolveBattleConfigSourcePath(battleConfigPath, mapSource);
}

void MapEditScene::reloadConfig()
{
	m_config = LoadBattleConfig(m_battleConfigPath);
	getData().baseBattleConfig = m_config;
	m_selection = {};
	m_dragOffset.reset();
	m_statusMessage = U"Reloaded map config";
}

void MapEditScene::saveMap()
{
	String output;

	for (const auto& placement : m_config.initialUnits)
	{
		output += U"[[initial_units]]\n";
		output += U"owner = \"{}\"\n"_fmt(toOwnerTomlString(placement.owner));
		output += U"archetype = \"{}\"\n"_fmt(toUnitArchetypeTomlString(placement.archetype));
		output += U"x = {:.1f}\n"_fmt(placement.position.x);
		output += U"y = {:.1f}\n\n"_fmt(placement.position.y);
	}

	for (const auto& obstacle : m_config.obstacles)
	{
		output += U"[[obstacles]]\n";
		output += U"label = \"{}\"\n"_fmt(obstacle.label);
		output += U"x = {:.1f}\n"_fmt(obstacle.rect.x);
		output += U"y = {:.1f}\n"_fmt(obstacle.rect.y);
		output += U"width = {:.1f}\n"_fmt(obstacle.rect.w);
		output += U"height = {:.1f}\n"_fmt(obstacle.rect.h);
		output += U"blocks_movement = {}\n\n"_fmt(obstacle.blocksMovement ? U"true" : U"false");
	}

	for (const auto& resourcePoint : m_config.resourcePoints)
	{
		output += U"[[resource_points]]\n";
		output += U"label = \"{}\"\n"_fmt(resourcePoint.label);
		output += U"owner = \"{}\"\n"_fmt(toOwnerTomlString(resourcePoint.owner));
		output += U"x = {:.1f}\n"_fmt(resourcePoint.position.x);
		output += U"y = {:.1f}\n"_fmt(resourcePoint.position.y);
		output += U"radius = {:.1f}\n"_fmt(resourcePoint.radius);
		output += U"income_amount = {}\n"_fmt(resourcePoint.incomeAmount);
		output += U"capture_time = {:.1f}\n\n"_fmt(resourcePoint.captureTime);
	}

	{
		s3d::TextWriter writer{ m_mapConfigPath };
		if (!writer)
		{
			throw Error{ U"Failed to open map config for write: " + m_mapConfigPath };
		}

		writer.write(output);
	}
	reloadConfig();
	m_statusMessage = U"Saved battle_map.toml";
}

void MapEditScene::startTestPlay()
{
	auto& data = getData();
	data.baseBattleConfig = m_config;
	BeginNewRun(data.runState, true);
	ResetBonusRoomSceneState(data.bonusRoomProgress);
	SaveContinueRun(data, ContinueResumeScene::Battle);
	changeScene(U"Battle");
}

void MapEditScene::drawCanvas(const RectF& canvasRect) const
{
	const double scale = getCanvasScale();
	const Vec2 origin = getCanvasOrigin();
	const RectF worldRect{ origin.x, origin.y, m_config.world.width * scale, m_config.world.height * scale };

	worldRect.draw(ColorF{ 0.15, 0.19, 0.16 });
	worldRect.drawFrame(2, ColorF{ 0.42, 0.54, 0.46 });

	for (int32 x = 0; x <= static_cast<int32>(m_config.world.width); x += 64)
	{
		const double screenX = origin.x + (x * scale);
		Line{ screenX, worldRect.y, screenX, worldRect.y + worldRect.h }.draw(1, ColorF{ 1.0, 1.0, 1.0, 0.08 });
	}

	for (int32 y = 0; y <= static_cast<int32>(m_config.world.height); y += 64)
	{
		const double screenY = origin.y + (y * scale);
		Line{ worldRect.x, screenY, worldRect.x + worldRect.w, screenY }.draw(1, ColorF{ 1.0, 1.0, 1.0, 0.08 });
	}

	for (size_t i = 0; i < m_config.obstacles.size(); ++i)
	{
		const bool selected = (m_selection.kind == SelectionKind::Obstacle) && (m_selection.index == static_cast<int32>(i));
		const auto& obstacle = m_config.obstacles[i];
		const RectF rect = worldToScreen(obstacle.rect);
		rect.draw(obstacle.blocksMovement ? ColorF{ 0.45, 0.22, 0.18, 0.80 } : ColorF{ 0.35, 0.25, 0.14, 0.60 });
		rect.drawFrame(selected ? 3 : 1, selected ? ColorF{ Palette::Yellow } : ColorF{ 0.90, 0.72, 0.62 });
		getData().smallFont(obstacle.label).draw(rect.x + 6, rect.y + 4, Palette::White);
	}

	for (size_t i = 0; i < m_config.resourcePoints.size(); ++i)
	{
		const bool selected = (m_selection.kind == SelectionKind::ResourcePoint) && (m_selection.index == static_cast<int32>(i));
		const auto& resourcePoint = m_config.resourcePoints[i];
		const Vec2 center = worldToScreen(resourcePoint.position);
		const double radius = resourcePoint.radius * scale;
		Circle{ center, radius }.drawFrame(selected ? 4 : 2, getOwnerColor(resourcePoint.owner));
		Circle{ center, radius * 0.35 }.draw(getOwnerColor(resourcePoint.owner));
		getData().smallFont(resourcePoint.label).drawAt(center.movedBy(0, -radius - 12), Palette::White);
	}

	for (size_t i = 0; i < m_config.initialUnits.size(); ++i)
	{
		const bool selected = (m_selection.kind == SelectionKind::InitialUnit) && (m_selection.index == static_cast<int32>(i));
		const auto& placement = m_config.initialUnits[i];
		const Vec2 center = worldToScreen(placement.position);
		const double radius = (placement.archetype == UnitArchetype::Base) ? 18.0 : 12.0;
		Circle{ center, radius }.draw(getOwnerColor(placement.owner));
		Circle{ center, radius }.drawFrame(selected ? 4 : 2, selected ? ColorF{ Palette::Yellow } : ColorF{ 0.12, 0.12, 0.12 });
		getData().smallFont(toUnitArchetypeShortString(placement.archetype)).drawAt(center, Palette::White);
	}

	getData().smallFont(U"World: {} x {}"_fmt(static_cast<int32>(m_config.world.width), static_cast<int32>(m_config.world.height)))
		.draw(canvasRect.x + 12, canvasRect.y + 10, Palette::White);
	getData().smallFont(U"{} | Unit:{} Obstacle:{} Resource:{}"_fmt(m_statusMessage, m_config.initialUnits.size(), m_config.obstacles.size(), m_config.resourcePoints.size()))
		.draw(canvasRect.x + 12, canvasRect.y + canvasRect.h - 26, ColorF{ 0.92, 0.95, 1.0 });
}

void MapEditScene::drawLeftPanel(const RectF& panelRect) const
{
	const auto& data = getData();
	data.uiFont(U"MAP EDIT").draw(panelRect.x + 16, panelRect.y + 14, Palette::White);
	data.smallFont(U"_DEBUG only / Esc: Title / Del: Delete / Ctrl+S,F5: Save / R: Reload").draw(panelRect.x + 16, panelRect.y + 48, ColorF{ 0.84, 0.90, 1.0 });

	drawButton(getPanelButtonRect(panelRect, 0), U"Save", data.uiFont);
	drawButton(getPanelButtonRect(panelRect, 1), U"Reload", data.uiFont);
	drawButton(getPanelButtonRect(panelRect, 2), U"Test Play", data.uiFont, true);
	drawButton(getPanelButtonRect(panelRect, 3), U"Back to Title", data.smallFont);

	data.smallFont(U"Tool").draw(panelRect.x + 16, panelRect.y + 212, Palette::White);
	drawButton(getPanelButtonRect(panelRect, 4), U"Select", data.uiFont, m_tool == Tool::Select);
	drawButton(getPanelButtonRect(panelRect, 5), U"Add Unit", data.uiFont, m_tool == Tool::AddUnit);
	drawButton(getPanelButtonRect(panelRect, 6), U"Add Obstacle", data.smallFont, m_tool == Tool::AddObstacle);
	drawButton(getPanelButtonRect(panelRect, 7), U"Add Resource", data.smallFont, m_tool == Tool::AddResource);

	data.smallFont(U"Placement").draw(panelRect.x + 16, panelRect.y + 424, Palette::White);
	data.smallFont(U"Unit Owner: {}"_fmt(toOwnerDisplayString(m_unitPlacementOwner))).draw(panelRect.x + 16, panelRect.y + 452, Palette::White);
	drawButton(RectF{ panelRect.x + 16, panelRect.y + 476, panelRect.w - 32, 28 }, U"Cycle Unit Owner", data.smallFont);
	data.smallFont(U"Archetype: {}"_fmt(toUnitArchetypeDisplayString(m_unitPlacementArchetype))).draw(panelRect.x + 16, panelRect.y + 516, Palette::White);
	drawButton(RectF{ panelRect.x + 16, panelRect.y + 540, panelRect.w - 32, 28 }, U"Cycle Unit Archetype", data.smallFont);
	data.smallFont(U"Resource Owner: {}"_fmt(toOwnerDisplayString(m_resourcePlacementOwner))).draw(panelRect.x + 16, panelRect.y + 580, Palette::White);
	drawButton(RectF{ panelRect.x + 16, panelRect.y + 604, panelRect.w - 32, 28 }, U"Cycle Resource Owner", data.smallFont);
}

void MapEditScene::drawRightPanel(const RectF& panelRect) const
{
	const auto& data = getData();
	data.uiFont(U"INSPECT").draw(panelRect.x + 16, panelRect.y + 14, Palette::White);

	if (m_selection.kind == SelectionKind::None)
	{
		data.smallFont(U"Select an object on the map.").draw(panelRect.x + 16, panelRect.y + 54, Palette::White);
		return;
	}

	const double textX = panelRect.x + 16;
	const double buttonX = panelRect.x + 16;
	const double buttonW = panelRect.w - 32;

	if (const auto* placement = getSelectedUnit())
	{
		data.smallFont(U"Type: Initial Unit").draw(textX, panelRect.y + 54, Palette::White);
		data.smallFont(U"Owner: {}"_fmt(toOwnerDisplayString(placement->owner))).draw(textX, panelRect.y + 82, Palette::White);
		data.smallFont(U"Archetype: {}"_fmt(toUnitArchetypeDisplayString(placement->archetype))).draw(textX, panelRect.y + 108, Palette::White);
		data.smallFont(U"Pos: ({:.1f}, {:.1f})"_fmt(placement->position.x, placement->position.y)).draw(textX, panelRect.y + 134, Palette::White);
		drawButton(RectF{ buttonX, panelRect.y + 170, buttonW, 30 }, U"Cycle Owner", data.smallFont);
		drawButton(RectF{ buttonX, panelRect.y + 208, buttonW, 30 }, U"Cycle Archetype", data.smallFont);
		drawButton(RectF{ buttonX, panelRect.y + 246, buttonW, 30 }, U"Delete", data.smallFont);
		return;
	}

	if (const auto* obstacle = getSelectedObstacle())
	{
		data.smallFont(U"Type: Obstacle").draw(textX, panelRect.y + 54, Palette::White);
		data.smallFont(U"Label: {}"_fmt(obstacle->label)).draw(textX, panelRect.y + 82, Palette::White);
		data.smallFont(U"Pos: ({:.1f}, {:.1f})"_fmt(obstacle->rect.x, obstacle->rect.y)).draw(textX, panelRect.y + 108, Palette::White);
		data.smallFont(U"Size: {:.1f} x {:.1f}"_fmt(obstacle->rect.w, obstacle->rect.h)).draw(textX, panelRect.y + 134, Palette::White);
		data.smallFont(U"Blocks: {}"_fmt(obstacle->blocksMovement ? U"Yes" : U"No")).draw(textX, panelRect.y + 160, Palette::White);
		drawButton(RectF{ buttonX, panelRect.y + 194, buttonW, 30 }, U"Width +16", data.smallFont);
		drawButton(RectF{ buttonX, panelRect.y + 232, buttonW, 30 }, U"Width -16", data.smallFont);
		drawButton(RectF{ buttonX, panelRect.y + 270, buttonW, 30 }, U"Height +16", data.smallFont);
		drawButton(RectF{ buttonX, panelRect.y + 308, buttonW, 30 }, U"Height -16", data.smallFont);
		drawButton(RectF{ buttonX, panelRect.y + 346, buttonW, 30 }, U"Toggle Blocks Movement", data.smallFont);
		drawButton(RectF{ buttonX, panelRect.y + 384, buttonW, 30 }, U"Delete", data.smallFont);
		return;
	}

	if (const auto* resourcePoint = getSelectedResourcePoint())
	{
		data.smallFont(U"Type: Resource Point").draw(textX, panelRect.y + 54, Palette::White);
		data.smallFont(U"Label: {}"_fmt(resourcePoint->label)).draw(textX, panelRect.y + 82, Palette::White);
		data.smallFont(U"Owner: {}"_fmt(toOwnerDisplayString(resourcePoint->owner))).draw(textX, panelRect.y + 108, Palette::White);
		data.smallFont(U"Pos: ({:.1f}, {:.1f})"_fmt(resourcePoint->position.x, resourcePoint->position.y)).draw(textX, panelRect.y + 134, Palette::White);
		data.smallFont(U"Radius: {:.1f}"_fmt(resourcePoint->radius)).draw(textX, panelRect.y + 160, Palette::White);
		data.smallFont(U"Income: {}"_fmt(resourcePoint->incomeAmount)).draw(textX, panelRect.y + 186, Palette::White);
		data.smallFont(U"Capture: {:.1f}"_fmt(resourcePoint->captureTime)).draw(textX, panelRect.y + 212, Palette::White);
		drawButton(RectF{ buttonX, panelRect.y + 246, buttonW, 30 }, U"Cycle Owner", data.smallFont);
		drawButton(RectF{ buttonX, panelRect.y + 284, buttonW, 30 }, U"Radius +4", data.smallFont);
		drawButton(RectF{ buttonX, panelRect.y + 322, buttonW, 30 }, U"Radius -4", data.smallFont);
		drawButton(RectF{ buttonX, panelRect.y + 360, buttonW, 30 }, U"Income +1", data.smallFont);
		drawButton(RectF{ buttonX, panelRect.y + 398, buttonW, 30 }, U"Income -1", data.smallFont);
		drawButton(RectF{ buttonX, panelRect.y + 436, buttonW, 30 }, U"Capture +0.1", data.smallFont);
		drawButton(RectF{ buttonX, panelRect.y + 474, buttonW, 30 }, U"Capture -0.1", data.smallFont);
		drawButton(RectF{ buttonX, panelRect.y + 512, buttonW, 30 }, U"Delete", data.smallFont);
	}
}

void MapEditScene::drawButton(const RectF& rect, const String& label, const Font& font, const bool selected)
{
	const bool hovered = rect.mouseOver();
	const ColorF fillColor = selected
		? (hovered ? ColorF{ 0.34, 0.48, 0.76 } : ColorF{ 0.24, 0.38, 0.64 })
		: (hovered ? ColorF{ 0.24, 0.29, 0.38 } : ColorF{ 0.18, 0.22, 0.29 });
	const ColorF frameColor = hovered ? ColorF{ 0.82, 0.90, 1.0 } : ColorF{ 0.42, 0.56, 0.78 };
	rect.draw(fillColor);
	rect.drawFrame(2, frameColor);
	font(label).drawAt(rect.center(), Palette::White);
}

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

Owner MapEditScene::cycleUnitPlacementOwner(const Owner owner)
{
	return (owner == Owner::Player) ? Owner::Enemy : Owner::Player;
}

Owner MapEditScene::cycleResourceOwner(const Owner owner)
{
	switch (owner)
	{
	case Owner::Neutral:
		return Owner::Player;
	case Owner::Player:
		return Owner::Enemy;
	case Owner::Enemy:
	default:
		return Owner::Neutral;
	}
}

UnitArchetype MapEditScene::cycleUnitArchetype(const UnitArchetype archetype)
{
	switch (archetype)
	{
	case UnitArchetype::Base:
		return UnitArchetype::Barracks;
	case UnitArchetype::Barracks:
		return UnitArchetype::Stable;
	case UnitArchetype::Stable:
		return UnitArchetype::Turret;
	case UnitArchetype::Turret:
		return UnitArchetype::Worker;
	case UnitArchetype::Worker:
		return UnitArchetype::Soldier;
	case UnitArchetype::Soldier:
		return UnitArchetype::Archer;
	case UnitArchetype::Archer:
		return UnitArchetype::Sniper;
	case UnitArchetype::Sniper:
		return UnitArchetype::Katyusha;
	case UnitArchetype::Katyusha:
		return UnitArchetype::MachineGun;
	case UnitArchetype::MachineGun:
		return UnitArchetype::Goliath;
	case UnitArchetype::Goliath:
		return UnitArchetype::Healer;
	case UnitArchetype::Healer:
		return UnitArchetype::Spinner;
	case UnitArchetype::Spinner:
	default:
		return UnitArchetype::Base;
	}
}

ColorF MapEditScene::getOwnerColor(const Owner owner)
{
	switch (owner)
	{
	case Owner::Player:
		return ColorF{ 0.24, 0.58, 0.98 };
	case Owner::Enemy:
		return ColorF{ 0.92, 0.26, 0.26 };
	case Owner::Neutral:
	default:
		return ColorF{ 0.78, 0.78, 0.78 };
	}
}

String MapEditScene::toOwnerDisplayString(const Owner owner)
{
	switch (owner)
	{
	case Owner::Player:
		return U"Player";
	case Owner::Enemy:
		return U"Enemy";
	case Owner::Neutral:
	default:
		return U"Neutral";
	}
}

String MapEditScene::toOwnerTomlString(const Owner owner)
{
	switch (owner)
	{
	case Owner::Player:
		return U"player";
	case Owner::Enemy:
		return U"enemy";
	case Owner::Neutral:
	default:
		return U"neutral";
	}
}

String MapEditScene::toUnitArchetypeDisplayString(const UnitArchetype archetype)
{
	switch (archetype)
	{
	case UnitArchetype::Base:
		return U"Base";
	case UnitArchetype::Barracks:
		return U"Barracks";
	case UnitArchetype::Stable:
		return U"Stable";
	case UnitArchetype::Turret:
		return U"Turret";
	case UnitArchetype::Worker:
		return U"Worker";
	case UnitArchetype::Soldier:
		return U"Soldier";
	case UnitArchetype::Archer:
		return U"Archer";
	case UnitArchetype::Sniper:
		return U"Sniper";
	case UnitArchetype::Katyusha:
		return U"Katyusha";
	case UnitArchetype::MachineGun:
		return U"MachineGun";
	case UnitArchetype::Goliath:
		return U"Goliath";
	case UnitArchetype::Healer:
		return U"Healer";
	case UnitArchetype::Spinner:
	default:
		return U"Spinner";
	}
}

String MapEditScene::toUnitArchetypeTomlString(const UnitArchetype archetype)
{
	switch (archetype)
	{
	case UnitArchetype::Base:
		return U"base";
	case UnitArchetype::Barracks:
		return U"barracks";
	case UnitArchetype::Stable:
		return U"stable";
	case UnitArchetype::Turret:
		return U"turret";
	case UnitArchetype::Worker:
		return U"worker";
	case UnitArchetype::Soldier:
		return U"soldier";
	case UnitArchetype::Archer:
		return U"archer";
	case UnitArchetype::Sniper:
		return U"sniper";
	case UnitArchetype::Katyusha:
		return U"katyusha";
	case UnitArchetype::MachineGun:
		return U"machine_gun";
	case UnitArchetype::Goliath:
		return U"goliath";
	case UnitArchetype::Healer:
		return U"healer";
	case UnitArchetype::Spinner:
	default:
		return U"spinner";
	}
}

String MapEditScene::toUnitArchetypeShortString(const UnitArchetype archetype)
{
	switch (archetype)
	{
	case UnitArchetype::Base:
		return U"B";
	case UnitArchetype::Barracks:
		return U"Br";
	case UnitArchetype::Stable:
		return U"St";
	case UnitArchetype::Turret:
		return U"T";
	case UnitArchetype::Worker:
		return U"W";
	case UnitArchetype::Soldier:
		return U"S";
	case UnitArchetype::Archer:
		return U"A";
	case UnitArchetype::Sniper:
		return U"Sn";
	case UnitArchetype::Katyusha:
		return U"K";
	case UnitArchetype::MachineGun:
		return U"MG";
	case UnitArchetype::Goliath:
		return U"G";
	case UnitArchetype::Healer:
		return U"H";
	case UnitArchetype::Spinner:
	default:
		return U"Sp";
	}
}
