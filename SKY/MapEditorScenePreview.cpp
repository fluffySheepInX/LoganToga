# include "MapEditorSceneInternal.hpp"

using namespace MapEditorDetail;

namespace MapEditorSceneDetail
{
	void DrawToolHoverPreview(const MapEditorState& state, const MapData& mapData, const Vec3& hoverPosition)
	{
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
         if (not state.showNavPoints)
			{
				return;
			}

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
			const PlacedModel previewRoad = BuildRoadFromStartAndEnd(
				state.pendingRoadPlacementStartPosition.value_or(hoverPosition),
				hoverPosition,
				8.0,
				4.0,
				0.0);
			DrawRoadOutline(previewRoad, ColorF{ 0.42, 0.86, 1.0, 0.70 });
			return;
		}

		if (state.selectedTool == MapEditorTool::PlaceTireTrackDecal)
		{
			const PlacedModel previewDecal = BuildTireTrackDecalFromStartAndEnd(
				state.pendingTireTrackPlacementStartPosition.value_or(hoverPosition),
				hoverPosition,
				6.0,
				2.0,
				0.0);
			DrawTireTrackOutline(previewDecal, ColorF{ 0.42, 0.86, 1.0, 0.72 });
			return;
		}

		if (const auto terrainType = ToTerrainCellType(state.selectedTool))
		{
			const TerrainCell previewCell{
				.cell = Point{ 0, 0 },
				.type = *terrainType,
				.color = state.selectedTerrainColor,
			};
			const ColorF terrainColor = GetTerrainCellDrawColor(previewCell);
			const ColorF terrainPreviewColor{ terrainColor.r, terrainColor.g, terrainColor.b, 0.74 };
			if ((state.terrainPaintMode == MapEditorTerrainPaintMode::Area) && state.pendingTerrainPaintRangeStartCell)
			{
				DrawTerrainCellPreviewRange(*state.pendingTerrainPaintRangeStartCell, ToTerrainCell(hoverPosition), terrainPreviewColor);
			}
			else
			{
				DrawTerrainCellPreview(ToTerrainCell(hoverPosition), terrainPreviewColor);
			}
			return;
		}

		if (state.selectedTool == MapEditorTool::EraseTerrain)
		{
			if ((state.terrainPaintMode == MapEditorTerrainPaintMode::Area) && state.pendingTerrainPaintRangeStartCell)
			{
				DrawTerrainCellPreviewRange(*state.pendingTerrainPaintRangeStartCell, ToTerrainCell(hoverPosition), ColorF{ 0.95, 0.26, 0.22, 0.44 });
			}
			else
			{
				DrawTerrainCellPreview(ToTerrainCell(hoverPosition), ColorF{ 0.95, 0.26, 0.22, 0.44 });
			}
			return;
		}

		Cylinder{ hoverPosition.movedBy(0, 0.06, 0), 0.35, 0.12 }.draw(previewColor.removeSRGBCurve());
		Sphere{ hoverPosition.movedBy(0, 0.26, 0), 0.12 }.draw(previewColor.removeSRGBCurve());
	}
}
