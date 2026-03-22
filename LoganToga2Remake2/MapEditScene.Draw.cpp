#include "MapEditScene.h"

#include "Localization.h"
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

 getData().smallFont(Localization::FormatText(U"map_edit.canvas.world", static_cast<int32>(m_config.world.width), static_cast<int32>(m_config.world.height)))
		.draw(canvasRect.x + 12, canvasRect.y + 10, Palette::White);
  getData().smallFont(Localization::FormatText(U"map_edit.canvas.summary", m_statusMessage, m_config.initialUnits.size(), m_config.obstacles.size(), m_config.resourcePoints.size()))
		.draw(canvasRect.x + 12, canvasRect.y + canvasRect.h - 26, ColorF{ 0.92, 0.95, 1.0 });
}

void MapEditScene::drawLeftPanel(const RectF& panelRect) const
{
	const auto& data = getData();
  data.uiFont(Localization::GetText(U"map_edit.title")).draw(panelRect.x + 16, panelRect.y + 14, Palette::White);
	data.smallFont(Localization::GetText(U"map_edit.map_label")).draw(panelRect.x + 16, panelRect.y + 48, Palette::White);
	data.smallFont(getCurrentMapLabel()).draw(panelRect.x + 16, panelRect.y + 72, ColorF{ 0.92, 0.95, 1.0 });
 drawButton(RectF{ panelRect.x + 16, panelRect.y + 96, (panelRect.w - 40) * 0.5, 28 }, Localization::GetText(U"map_edit.button.prev_map"), data.smallFont);
	drawButton(RectF{ panelRect.x + 24 + ((panelRect.w - 40) * 0.5), panelRect.y + 96, (panelRect.w - 40) * 0.5, 28 }, Localization::GetText(U"map_edit.button.next_map"), data.smallFont);

 drawButton(getPanelButtonRect(panelRect, 0), Localization::GetText(U"map_edit.button.save"), data.uiFont);
	drawButton(getPanelButtonRect(panelRect, 1), Localization::GetText(U"map_edit.button.reload"), data.uiFont);
	drawButton(getPanelButtonRect(panelRect, 2), Localization::GetText(U"map_edit.button.test_play"), data.uiFont, true);
	drawButton(getPanelButtonRect(panelRect, 3), Localization::GetText(U"map_edit.button.back_to_title"), data.smallFont);

  data.smallFont(Localization::GetText(U"map_edit.tool_label")).draw(panelRect.x + 16, panelRect.y + 268, Palette::White);
	drawButton(getPanelButtonRect(panelRect, 4), Localization::GetText(U"map_edit.button.select"), data.uiFont, m_tool == Tool::Select);
	drawButton(getPanelButtonRect(panelRect, 5), Localization::GetText(U"map_edit.button.add_unit"), data.uiFont, m_tool == Tool::AddUnit);
	drawButton(getPanelButtonRect(panelRect, 6), Localization::GetText(U"map_edit.button.add_obstacle"), data.smallFont, m_tool == Tool::AddObstacle);
	drawButton(getPanelButtonRect(panelRect, 7), Localization::GetText(U"map_edit.button.add_resource"), data.smallFont, m_tool == Tool::AddResource);

 data.smallFont(Localization::GetText(U"map_edit.placement_label")).draw(panelRect.x + 16, panelRect.y + 480, Palette::White);
	data.smallFont(Localization::FormatText(U"map_edit.placement.unit_owner", toOwnerDisplayString(m_unitPlacementOwner))).draw(panelRect.x + 16, panelRect.y + 508, Palette::White);
	drawButton(RectF{ panelRect.x + 16, panelRect.y + 532, panelRect.w - 32, 28 }, Localization::GetText(U"map_edit.button.cycle_unit_owner"), data.smallFont);
	data.smallFont(Localization::FormatText(U"map_edit.placement.archetype", toUnitArchetypeDisplayString(m_unitPlacementArchetype))).draw(panelRect.x + 16, panelRect.y + 572, Palette::White);
	drawButton(RectF{ panelRect.x + 16, panelRect.y + 596, panelRect.w - 32, 28 }, Localization::GetText(U"map_edit.button.cycle_unit_archetype"), data.smallFont);
	data.smallFont(Localization::FormatText(U"map_edit.placement.resource_owner", toOwnerDisplayString(m_resourcePlacementOwner))).draw(panelRect.x + 16, panelRect.y + 636, Palette::White);
	drawButton(RectF{ panelRect.x + 16, panelRect.y + 660, panelRect.w - 32, 28 }, Localization::GetText(U"map_edit.button.cycle_resource_owner"), data.smallFont);
}

void MapEditScene::drawRightPanel(const RectF& panelRect) const
{
	const auto& data = getData();
   data.uiFont(Localization::GetText(U"map_edit.inspect.title")).draw(panelRect.x + 16, panelRect.y + 14, Palette::White);

	if (m_selection.kind == SelectionKind::None)
	{
       data.smallFont(Localization::GetText(U"map_edit.inspect.none")) .draw(panelRect.x + 16, panelRect.y + 54, Palette::White);
		return;
	}

	const double textX = panelRect.x + 16;
	const double buttonX = panelRect.x + 16;
	const double buttonW = panelRect.w - 32;

	if (const auto* placement = getSelectedUnit())
	{
        data.smallFont(Localization::FormatText(U"map_edit.inspect.type", Localization::GetText(U"map_edit.kind.initial_unit"))).draw(textX, panelRect.y + 54, Palette::White);
		data.smallFont(Localization::FormatText(U"map_edit.inspect.owner", toOwnerDisplayString(placement->owner))).draw(textX, panelRect.y + 82, Palette::White);
		data.smallFont(Localization::FormatText(U"map_edit.inspect.archetype", toUnitArchetypeDisplayString(placement->archetype))).draw(textX, panelRect.y + 108, Palette::White);
		data.smallFont(Localization::FormatText(U"map_edit.inspect.position", placement->position.x, placement->position.y)).draw(textX, panelRect.y + 134, Palette::White);
		drawButton(RectF{ buttonX, panelRect.y + 170, buttonW, 30 }, Localization::GetText(U"map_edit.button.cycle_owner"), data.smallFont);
		drawButton(RectF{ buttonX, panelRect.y + 208, buttonW, 30 }, Localization::GetText(U"map_edit.button.cycle_archetype"), data.smallFont);
		drawButton(RectF{ buttonX, panelRect.y + 246, buttonW, 30 }, Localization::GetText(U"map_edit.button.delete"), data.smallFont);
		return;
	}

	if (const auto* obstacle = getSelectedObstacle())
	{
        data.smallFont(Localization::FormatText(U"map_edit.inspect.type", Localization::GetText(U"map_edit.kind.obstacle"))).draw(textX, panelRect.y + 54, Palette::White);
		data.smallFont(Localization::FormatText(U"map_edit.inspect.label", obstacle->label)).draw(textX, panelRect.y + 82, Palette::White);
		data.smallFont(Localization::FormatText(U"map_edit.inspect.position", obstacle->rect.x, obstacle->rect.y)).draw(textX, panelRect.y + 108, Palette::White);
		data.smallFont(Localization::FormatText(U"map_edit.inspect.size", obstacle->rect.w, obstacle->rect.h)).draw(textX, panelRect.y + 134, Palette::White);
		data.smallFont(Localization::FormatText(U"map_edit.inspect.blocks", obstacle->blocksMovement ? Localization::GetText(U"common.yes") : Localization::GetText(U"common.no"))).draw(textX, panelRect.y + 160, Palette::White);
		drawButton(RectF{ buttonX, panelRect.y + 194, buttonW, 30 }, Localization::GetText(U"map_edit.button.width_plus"), data.smallFont);
		drawButton(RectF{ buttonX, panelRect.y + 232, buttonW, 30 }, Localization::GetText(U"map_edit.button.width_minus"), data.smallFont);
		drawButton(RectF{ buttonX, panelRect.y + 270, buttonW, 30 }, Localization::GetText(U"map_edit.button.height_plus"), data.smallFont);
		drawButton(RectF{ buttonX, panelRect.y + 308, buttonW, 30 }, Localization::GetText(U"map_edit.button.height_minus"), data.smallFont);
		drawButton(RectF{ buttonX, panelRect.y + 346, buttonW, 30 }, Localization::GetText(U"map_edit.button.toggle_blocks_movement"), data.smallFont);
		drawButton(RectF{ buttonX, panelRect.y + 384, buttonW, 30 }, Localization::GetText(U"map_edit.button.delete"), data.smallFont);
		return;
	}

	if (const auto* resourcePoint = getSelectedResourcePoint())
	{
      data.smallFont(Localization::FormatText(U"map_edit.inspect.type", Localization::GetText(U"map_edit.kind.resource_point"))).draw(textX, panelRect.y + 54, Palette::White);
		data.smallFont(Localization::FormatText(U"map_edit.inspect.label", resourcePoint->label)).draw(textX, panelRect.y + 82, Palette::White);
		data.smallFont(Localization::FormatText(U"map_edit.inspect.owner", toOwnerDisplayString(resourcePoint->owner))).draw(textX, panelRect.y + 108, Palette::White);
		data.smallFont(Localization::FormatText(U"map_edit.inspect.position", resourcePoint->position.x, resourcePoint->position.y)).draw(textX, panelRect.y + 134, Palette::White);
		data.smallFont(Localization::FormatText(U"map_edit.inspect.radius", resourcePoint->radius)).draw(textX, panelRect.y + 160, Palette::White);
		data.smallFont(Localization::FormatText(U"map_edit.inspect.income", resourcePoint->incomeAmount)).draw(textX, panelRect.y + 186, Palette::White);
		data.smallFont(Localization::FormatText(U"map_edit.inspect.capture", resourcePoint->captureTime)).draw(textX, panelRect.y + 212, Palette::White);
		drawButton(RectF{ buttonX, panelRect.y + 246, buttonW, 30 }, Localization::GetText(U"map_edit.button.cycle_owner"), data.smallFont);
		drawButton(RectF{ buttonX, panelRect.y + 284, buttonW, 30 }, Localization::GetText(U"map_edit.button.radius_plus"), data.smallFont);
		drawButton(RectF{ buttonX, panelRect.y + 322, buttonW, 30 }, Localization::GetText(U"map_edit.button.radius_minus"), data.smallFont);
		drawButton(RectF{ buttonX, panelRect.y + 360, buttonW, 30 }, Localization::GetText(U"map_edit.button.income_plus"), data.smallFont);
		drawButton(RectF{ buttonX, panelRect.y + 398, buttonW, 30 }, Localization::GetText(U"map_edit.button.income_minus"), data.smallFont);
		drawButton(RectF{ buttonX, panelRect.y + 436, buttonW, 30 }, Localization::GetText(U"map_edit.button.capture_plus"), data.smallFont);
		drawButton(RectF{ buttonX, panelRect.y + 474, buttonW, 30 }, Localization::GetText(U"map_edit.button.capture_minus"), data.smallFont);
		drawButton(RectF{ buttonX, panelRect.y + 512, buttonW, 30 }, Localization::GetText(U"map_edit.button.delete"), data.smallFont);
	}
}

void MapEditScene::drawButton(const RectF& rect, const String& label, const Font& font, const bool selected)
{
	DrawMenuButton(rect, label, font, selected);
}
