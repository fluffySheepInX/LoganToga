# include "MapEditorInternal.hpp"

using namespace MapEditorDetail;

namespace
{
	constexpr int32 ResourceAreaRingSegments = 28;
	constexpr double WallPreviewThickness = 1.0;
	constexpr double WallPreviewHeight = 2.0;
	constexpr double WallPreviewMarkerSpacing = 1.2;
   constexpr double RoadHandleRadius = 0.34;
	constexpr double RoadRotationHandleRadius = 0.42;

	void DrawTerrainCell(const TerrainCell& terrainCell, const double yOffset = 0.01, const double height = 0.02)
	{
		const Vec3 center = ToTerrainCellCenter(terrainCell.cell).movedBy(0, yOffset, 0);
		const ColorF cellColor = GetTerrainCellDrawColor(terrainCell);
		Box{ center, TerrainCellSize, height, TerrainCellSize }.draw(cellColor.removeSRGBCurve());
	}

	void DrawTerrainCellPreview(const Point& cell, const ColorF& color)
	{
		const Vec3 center = ToTerrainCellCenter(cell).movedBy(0, 0.03, 0);
		Box{ center, TerrainCellSize, 0.035, TerrainCellSize }.draw(color.removeSRGBCurve());
	}

	void DrawResourceAreaRing(const Vec3& position, const double radius, const ColorF& ringColor)
	{
		const double markerRadius = Clamp((radius * 0.06), 0.10, 0.24);
		const ColorF shadowColor{ 0.02, 0.03, 0.05, 0.72 };

		for (int32 i = 0; i < ResourceAreaRingSegments; ++i)
		{
			const double angle = (Math::TwoPi * i / ResourceAreaRingSegments);
			const Vec3 offset{ Math::Cos(angle) * radius, 0.0, Math::Sin(angle) * radius };
			Cylinder{ position.movedBy(offset).movedBy(0, 0.03, 0), (markerRadius + 0.03), 0.05 }.draw(shadowColor.removeSRGBCurve());
			Cylinder{ position.movedBy(offset).movedBy(0, 0.035, 0), markerRadius, 0.04 }.draw(ringColor.removeSRGBCurve());
		}

		Cylinder{ position.movedBy(0, 0.05, 0), 0.15, 0.10 }.draw(ColorF{ 0.06, 0.08, 0.11, 0.92 }.removeSRGBCurve());
		Sphere{ position.movedBy(0, 0.22, 0), 0.12 }.draw(ringColor.removeSRGBCurve());
	}

	Vec3 GetWallDirection(const PlacedModel& placedModel)
	{
		return Vec3{ Math::Sin(placedModel.yaw), 0.0, Math::Cos(placedModel.yaw) };
	}

	void DrawWallPreview(const PlacedModel& placedModel, const ColorF& wallColor)
	{
		const double wallLength = Clamp(placedModel.wallLength, 2.0, 80.0);
		const Vec3 halfDirection = (GetWallDirection(placedModel) * (wallLength * 0.5));
		const Vec3 start = (placedModel.position - halfDirection);
		const Vec3 goal = (placedModel.position + halfDirection);
		Line3D{ start.movedBy(0, 0.12, 0), goal.movedBy(0, 0.12, 0) }.draw(wallColor.removeSRGBCurve());
		Cylinder{ start.movedBy(0, 0.5, 0), 0.28, WallPreviewHeight }.draw(wallColor.removeSRGBCurve());
		Cylinder{ goal.movedBy(0, 0.5, 0), 0.28, WallPreviewHeight }.draw(wallColor.removeSRGBCurve());

		const int32 segmentCount = Max(1, static_cast<int32>(Math::Ceil(wallLength / WallPreviewMarkerSpacing)));
		for (int32 i = 0; i <= segmentCount; ++i)
		{
			const double t = (static_cast<double>(i) / segmentCount);
			const Vec3 position = start.lerp(goal, t);
			Cylinder{ position.movedBy(0, 0.4, 0), (WallPreviewThickness * 0.34), 0.8 }.draw(ColorF{ wallColor, 0.85 }.removeSRGBCurve());
		}
	}

    void DrawRoadOutline(const PlacedModel& placedModel, const ColorF& outlineColor, const Optional<int32>& highlightedCorner = none, const bool highlightedRotationHandle = false)
		{
			const Array<Vec3> corners = GetRoadCorners(placedModel);
			if (corners.size() < 4)
			{
				return;
			}

			for (size_t i = 0; i < corners.size(); ++i)
			{
				const Vec3 start = corners[i].movedBy(0, 0.08, 0);
				const Vec3 end = corners[(i + 1) % corners.size()].movedBy(0, 0.08, 0);
				Line3D{ start, end }.draw(outlineColor.removeSRGBCurve());
			}

			for (size_t i = 0; i < corners.size(); ++i)
			{
				const bool isHighlighted = (highlightedCorner && (*highlightedCorner == static_cast<int32>(i)));
				const ColorF handleColor = isHighlighted ? ColorF{ 1.0, 0.94, 0.34, 0.95 } : outlineColor;
				Cylinder{ corners[i].movedBy(0, 0.18, 0), (RoadHandleRadius + (isHighlighted ? 0.06 : 0.0)), 0.18 }.draw(handleColor.removeSRGBCurve());
			}

			const Vec3 rotationHandle = GetRoadRotationHandlePosition(placedModel);
			Line3D{ placedModel.position.movedBy(0, 0.10, 0), rotationHandle.movedBy(0, 0.10, 0) }.draw(outlineColor.removeSRGBCurve());
			const ColorF rotationHandleColor = highlightedRotationHandle ? ColorF{ 1.0, 0.94, 0.34, 0.95 } : outlineColor;
			Cylinder{ rotationHandle.movedBy(0, 0.20, 0), (RoadRotationHandleRadius + (highlightedRotationHandle ? 0.06 : 0.0)), 0.18 }.draw(rotationHandleColor.removeSRGBCurve());
		}
}

void DrawMapEditorScene(const MapEditorState& state, const MapData& mapData)
{
  if (not state.enabled)
	{
		return;
	}

  for (const auto& terrainCell : mapData.terrainCells)
	{
		DrawTerrainCell(terrainCell);
	}

  if (state.showNavLinks)
	{
		for (const auto& navLink : mapData.navLinks)
		{
			if ((mapData.navPoints.size() <= navLink.fromIndex)
				|| (mapData.navPoints.size() <= navLink.toIndex)
				|| (navLink.fromIndex == navLink.toIndex))
			{
				continue;
			}

			const Vec3 from = mapData.navPoints[navLink.fromIndex].position.movedBy(0, 0.08, 0);
			const Vec3 to = mapData.navPoints[navLink.toIndex].position.movedBy(0, 0.08, 0);
			const ColorF linkColor = navLink.bidirectional
				? ColorF{ 0.18, 0.95, 0.72, 0.72 }
				: ColorF{ 0.95, 0.72, 0.22, 0.72 };
			Line3D{ from, to }.draw(linkColor.removeSRGBCurve());
		}
	}

	if (state.showNavPoints)
	{
		for (size_t i = 0; i < mapData.navPoints.size(); ++i)
		{
			const NavPoint& navPoint = mapData.navPoints[i];
			const bool selected = (state.selectedNavPointIndex && (*state.selectedNavPointIndex == i));
			const bool pending = (state.pendingNavLinkStartIndex && (*state.pendingNavLinkStartIndex == i));
			const ColorF pointColor = selected
				? ColorF{ 1.0, 0.92, 0.32, 0.95 }
				: pending
					? ColorF{ 0.25, 0.92, 0.98, 0.90 }
					: ColorF{ 0.22, 0.90, 0.48, 0.78 };
			Cylinder{ navPoint.position.movedBy(0, 0.06, 0), 0.22, 0.12 }.draw(pointColor.removeSRGBCurve());
			Sphere{ navPoint.position.movedBy(0, 0.28, 0), 0.14 }.draw(pointColor.removeSRGBCurve());
			Cylinder{ navPoint.position.movedBy(0, 0.015, 0), Max(0.35, navPoint.radius), 0.03 }.draw(ColorF{ pointColor, 0.22 }.removeSRGBCurve());
		}
	}

	Cylinder{ mapData.playerBasePosition.movedBy(0, 0.28, 0), 0.24, 0.56 }.draw(ColorF{ 0.22, 0.82, 0.98 }.removeSRGBCurve());
	Sphere{ mapData.playerBasePosition.movedBy(0, 0.74, 0), 0.22 }.draw(ColorF{ 0.40, 0.90, 1.0 }.removeSRGBCurve());
	Cylinder{ mapData.enemyBasePosition.movedBy(0, 0.28, 0), 0.24, 0.56 }.draw(ColorF{ 0.98, 0.32, 0.28 }.removeSRGBCurve());
	Sphere{ mapData.enemyBasePosition.movedBy(0, 0.74, 0), 0.22 }.draw(ColorF{ 1.0, 0.50, 0.45 }.removeSRGBCurve());
	Cylinder{ mapData.sapperRallyPoint.movedBy(0, 0.28, 0), 0.18, 0.56 }.draw(ColorF{ 0.95, 0.82, 0.12 }.removeSRGBCurve());
	Sphere{ mapData.sapperRallyPoint.movedBy(0, 0.68, 0), 0.20 }.draw(ColorF{ 0.98, 0.92, 0.35 }.removeSRGBCurve());

	for (const auto& resourceArea : mapData.resourceAreas)
	{
		const ColorF areaColor = GetResourceAreaColor(resourceArea.type);
     DrawResourceAreaRing(resourceArea.position, resourceArea.radius, areaColor);
	}

	if (IsValidPlacedModelIndex(mapData, state.selectedPlacedModelIndex))
	{
		const PlacedModel& placedModel = mapData.placedModels[*state.selectedPlacedModelIndex];
       if (placedModel.type == PlaceableModelType::Wall)
		{
			DrawWallPreview(placedModel, ColorF{ 1.0, 0.92, 0.28, 0.70 });
		}
        else if (placedModel.type == PlaceableModelType::Road)
		{
          const bool hoveredRotationHandle = state.roadRotateDrag
				|| HitTestRoadRotationHandle(placedModel, state.hoveredGroundPosition);
			const Optional<int32> hoveredCorner = state.roadResizeDrag
				? Optional<int32>{ state.roadResizeDrag->draggedCornerIndex }
				: HitTestRoadCornerHandle(placedModel, state.hoveredGroundPosition);
           DrawRoadOutline(placedModel, ColorF{ 1.0, 0.92, 0.28, 0.78 }, hoveredCorner, hoveredRotationHandle);
		}
		else
		{
			const double radius = GetPlacedModelSelectionRadius(placedModel);
			Cylinder{ placedModel.position.movedBy(0, 0.04, 0), radius, 0.08 }.draw(ColorF{ 1.0, 0.92, 0.28, 0.50 }.removeSRGBCurve());
			Sphere{ placedModel.position.movedBy(0, 0.42, 0), 0.22 }.draw(ColorF{ 1.0, 0.96, 0.55, 0.70 }.removeSRGBCurve());
		}
	}

	if (IsValidResourceAreaIndex(mapData, state.selectedResourceAreaIndex))
	{
		const ResourceArea& resourceArea = mapData.resourceAreas[*state.selectedResourceAreaIndex];
		DrawResourceAreaRing(resourceArea.position, resourceArea.radius, ColorF{ 1.0, 0.94, 0.38, 0.95 });
	}

	if (not state.enabled || not state.hoveredGroundPosition)
	{
		return;
	}

	const Vec3 hoverPosition = *state.hoveredGroundPosition;
	if (state.selectionMode)
	{
      if (state.showNavPoints && IsValidNavPointIndex(mapData, state.selectedNavPointIndex))
		{
			const NavPoint& navPoint = mapData.navPoints[*state.selectedNavPointIndex];
			Cylinder{ hoverPosition.movedBy(0, 0.015, 0), Max(0.35, navPoint.radius), 0.03 }.draw(ColorF{ 0.40, 0.96, 1.0, 0.42 }.removeSRGBCurve());
			Sphere{ hoverPosition.movedBy(0, 0.26, 0), 0.14 }.draw(ColorF{ 0.40, 0.96, 1.0, 0.64 }.removeSRGBCurve());
		}

		if (IsValidPlacedModelIndex(mapData, state.selectedPlacedModelIndex))
		{
			const PlacedModel& placedModel = mapData.placedModels[*state.selectedPlacedModelIndex];
           if (placedModel.type == PlaceableModelType::Wall)
			{
				PlacedModel previewWall = placedModel;
				previewWall.position = hoverPosition;
				DrawWallPreview(previewWall, ColorF{ 0.40, 0.96, 1.0, 0.72 });
			}
            else if (placedModel.type == PlaceableModelType::Road)
			{
              const bool hoveredRotationHandle = state.roadRotateDrag
					|| HitTestRoadRotationHandle(placedModel, state.hoveredGroundPosition);
				const Optional<int32> hoveredCorner = state.roadResizeDrag
					? Optional<int32>{ state.roadResizeDrag->draggedCornerIndex }
					: HitTestRoadCornerHandle(placedModel, state.hoveredGroundPosition);
              if (hoveredCorner || state.roadResizeDrag || hoveredRotationHandle)
				{
                   DrawRoadOutline(placedModel, ColorF{ 0.40, 0.96, 1.0, 0.84 }, hoveredCorner, hoveredRotationHandle);
				}
				else
				{
					PlacedModel previewRoad = placedModel;
					previewRoad.position = hoverPosition;
					DrawRoadOutline(previewRoad, ColorF{ 0.40, 0.96, 1.0, 0.72 });
				}
			}
			else
			{
				const double radius = GetPlacedModelSelectionRadius(placedModel);
				Cylinder{ hoverPosition.movedBy(0, 0.04, 0), radius, 0.08 }.draw(ColorF{ 0.25, 0.92, 0.98, 0.32 }.removeSRGBCurve());
				Sphere{ hoverPosition.movedBy(0, 0.28, 0), 0.16 }.draw(ColorF{ 0.40, 0.96, 1.0, 0.56 }.removeSRGBCurve());
			}
		}

		if (IsValidResourceAreaIndex(mapData, state.selectedResourceAreaIndex))
		{
			const ResourceArea& resourceArea = mapData.resourceAreas[*state.selectedResourceAreaIndex];
			DrawResourceAreaRing(hoverPosition, resourceArea.radius, ColorF{ 0.40, 0.96, 1.0, 0.72 });
		}

		return;
	}

	const Optional<MainSupport::ResourceType> resourceType = ToResourceType(state.selectedTool);
	const ColorF previewColor = (state.selectedTool == MapEditorTool::SetPlayerBasePosition)
		? ColorF{ 0.25, 0.85, 0.98, 0.60 }
		: (state.selectedTool == MapEditorTool::SetEnemyBasePosition)
			? ColorF{ 0.98, 0.35, 0.30, 0.60 }
			: (state.selectedTool == MapEditorTool::SetSapperRallyPoint)
				? ColorF{ 0.98, 0.85, 0.25, 0.65 }
				: resourceType
					? GetResourceAreaColor(*resourceType)
					: ColorF{ 0.25, 0.85, 0.98, 0.50 };

	if (resourceType)
	{
       DrawResourceAreaRing(hoverPosition, MainSupport::ResourceAreaDefaultRadius, previewColor);
		return;
	}

	if (state.selectedTool == MapEditorTool::PlaceNavPoint)
	{
		Cylinder{ hoverPosition.movedBy(0, 0.015, 0), 1.4, 0.03 }.draw(ColorF{ 0.22, 0.90, 0.48, 0.28 }.removeSRGBCurve());
		Cylinder{ hoverPosition.movedBy(0, 0.06, 0), 0.22, 0.12 }.draw(ColorF{ 0.22, 0.90, 0.48, 0.55 }.removeSRGBCurve());
		Sphere{ hoverPosition.movedBy(0, 0.28, 0), 0.14 }.draw(ColorF{ 0.22, 0.96, 0.52, 0.72 }.removeSRGBCurve());
		return;
	}

 if (state.selectedTool == MapEditorTool::LinkNavPoints)
	{
      if (state.showNavPoints && IsValidNavPointIndex(mapData, state.pendingNavLinkStartIndex))
		{
			const NavPoint& navPoint = mapData.navPoints[*state.pendingNavLinkStartIndex];
			Sphere{ navPoint.position.movedBy(0, 0.32, 0), 0.18 }.draw(ColorF{ 0.25, 0.92, 0.98, 0.95 }.removeSRGBCurve());
		}
		return;
	}

	if (state.selectedTool == MapEditorTool::PlaceWall)
	{
       PlacedModel previewWall = BuildWallFromStartAndEnd(hoverPosition, hoverPosition, 10.0, 0.0);
		if (state.pendingWallPlacementStartPosition)
		{
            previewWall = BuildWallFromStartAndEnd(*state.pendingWallPlacementStartPosition, hoverPosition, 10.0, 0.0);
		}
		DrawWallPreview(previewWall, ColorF{ 0.42, 0.86, 0.98, 0.68 });
		return;
	}

	if (state.selectedTool == MapEditorTool::PlaceRoad)
	{
        PlacedModel previewRoad = BuildRoadFromStartAndEnd(
			state.pendingRoadPlacementStartPosition.value_or(hoverPosition),
			hoverPosition,
			8.0,
			4.0,
			0.0);
		DrawRoadOutline(previewRoad, ColorF{ 0.42, 0.86, 1.0, 0.70 });
		return;
	}

	if (const auto terrainType = ToTerrainCellType(state.selectedTool))
	{
     const TerrainCell previewCell{
			.cell = Point{ 0, 0 },
			.type = *terrainType,
			.color = state.selectedTerrainColor,
		};
		const ColorF previewColor = GetTerrainCellDrawColor(previewCell);
		DrawTerrainCellPreview(ToTerrainCell(hoverPosition), ColorF{ previewColor.r, previewColor.g, previewColor.b, 0.74 });
		return;
	}

	if (state.selectedTool == MapEditorTool::EraseTerrain)
	{
		DrawTerrainCellPreview(ToTerrainCell(hoverPosition), ColorF{ 0.95, 0.26, 0.22, 0.44 });
		return;
	}

	Cylinder{ hoverPosition.movedBy(0, 0.06, 0), 0.35, 0.12 }.draw(previewColor.removeSRGBCurve());
	Sphere{ hoverPosition.movedBy(0, 0.26, 0), 0.12 }.draw(previewColor.removeSRGBCurve());
}
