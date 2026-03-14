#include "MapEditScene.h"
#include "MenuButtonUi.h"
#include "SceneTransition.h"

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
	DrawSceneTransitionOverlay(getData());
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
	data.smallFont(U"Map").draw(panelRect.x + 16, panelRect.y + 48, Palette::White);
	data.smallFont(getCurrentMapLabel()).draw(panelRect.x + 16, panelRect.y + 72, ColorF{ 0.92, 0.95, 1.0 });
	drawButton(RectF{ panelRect.x + 16, panelRect.y + 96, (panelRect.w - 40) * 0.5, 28 }, U"Prev Map", data.smallFont);
	drawButton(RectF{ panelRect.x + 24 + ((panelRect.w - 40) * 0.5), panelRect.y + 96, (panelRect.w - 40) * 0.5, 28 }, U"Next Map", data.smallFont);

	drawButton(getPanelButtonRect(panelRect, 0), U"Save", data.uiFont);
	drawButton(getPanelButtonRect(panelRect, 1), U"Reload", data.uiFont);
	drawButton(getPanelButtonRect(panelRect, 2), U"Test Play", data.uiFont, true);
	drawButton(getPanelButtonRect(panelRect, 3), U"Back to Title", data.smallFont);

	data.smallFont(U"Tool").draw(panelRect.x + 16, panelRect.y + 268, Palette::White);
	drawButton(getPanelButtonRect(panelRect, 4), U"Select", data.uiFont, m_tool == Tool::Select);
	drawButton(getPanelButtonRect(panelRect, 5), U"Add Unit", data.uiFont, m_tool == Tool::AddUnit);
	drawButton(getPanelButtonRect(panelRect, 6), U"Add Obstacle", data.smallFont, m_tool == Tool::AddObstacle);
	drawButton(getPanelButtonRect(panelRect, 7), U"Add Resource", data.smallFont, m_tool == Tool::AddResource);

	data.smallFont(U"Placement").draw(panelRect.x + 16, panelRect.y + 480, Palette::White);
	data.smallFont(U"Unit Owner: {}"_fmt(toOwnerDisplayString(m_unitPlacementOwner))).draw(panelRect.x + 16, panelRect.y + 508, Palette::White);
	drawButton(RectF{ panelRect.x + 16, panelRect.y + 532, panelRect.w - 32, 28 }, U"Cycle Unit Owner", data.smallFont);
	data.smallFont(U"Archetype: {}"_fmt(toUnitArchetypeDisplayString(m_unitPlacementArchetype))).draw(panelRect.x + 16, panelRect.y + 572, Palette::White);
	drawButton(RectF{ panelRect.x + 16, panelRect.y + 596, panelRect.w - 32, 28 }, U"Cycle Unit Archetype", data.smallFont);
	data.smallFont(U"Resource Owner: {}"_fmt(toOwnerDisplayString(m_resourcePlacementOwner))).draw(panelRect.x + 16, panelRect.y + 636, Palette::White);
	drawButton(RectF{ panelRect.x + 16, panelRect.y + 660, panelRect.w - 32, 28 }, U"Cycle Resource Owner", data.smallFont);
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
	DrawMenuButton(rect, label, font, selected);
}
