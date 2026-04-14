# include "MapEditorSceneInternal.hpp"

using namespace MapEditorDetail;

namespace MapEditorSceneDetail
{
	void DrawNavLinks(const MapEditorState& state, const MapData& mapData)
	{
		if (not state.showNavLinks)
		{
			return;
		}

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

	void DrawNavPoints(const MapEditorState& state, const MapData& mapData)
	{
		if (not state.showNavPoints)
		{
			return;
		}

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

	void DrawBaseMarkers(const MapData& mapData)
	{
		Cylinder{ mapData.playerBasePosition.movedBy(0, 0.28, 0), 0.24, 0.56 }.draw(ColorF{ 0.22, 0.82, 0.98 }.removeSRGBCurve());
		Sphere{ mapData.playerBasePosition.movedBy(0, 0.74, 0), 0.22 }.draw(ColorF{ 0.40, 0.90, 1.0 }.removeSRGBCurve());
		Cylinder{ mapData.enemyBasePosition.movedBy(0, 0.28, 0), 0.24, 0.56 }.draw(ColorF{ 0.98, 0.32, 0.28 }.removeSRGBCurve());
		Sphere{ mapData.enemyBasePosition.movedBy(0, 0.74, 0), 0.22 }.draw(ColorF{ 1.0, 0.50, 0.45 }.removeSRGBCurve());
		Cylinder{ mapData.sapperRallyPoint.movedBy(0, 0.28, 0), 0.18, 0.56 }.draw(ColorF{ 0.95, 0.82, 0.12 }.removeSRGBCurve());
		Sphere{ mapData.sapperRallyPoint.movedBy(0, 0.68, 0), 0.20 }.draw(ColorF{ 0.98, 0.92, 0.35 }.removeSRGBCurve());
	}

	void DrawResourceAreas(const MapData& mapData)
	{
		for (const auto& resourceArea : mapData.resourceAreas)
		{
			DrawResourceAreaRing(resourceArea.position, resourceArea.radius, GetResourceAreaColor(resourceArea.type));
		}
	}

	void DrawSelectedPlacedModelHighlight(const MapEditorState& state, const MapData& mapData)
	{
		if (not IsValidPlacedModelIndex(mapData, state.selectedPlacedModelIndex))
		{
			return;
		}

		const PlacedModel& placedModel = mapData.placedModels[*state.selectedPlacedModelIndex];
		if (placedModel.type == PlaceableModelType::Wall)
		{
			DrawWallPreview(placedModel, ColorF{ 1.0, 0.92, 0.28, 0.70 });
			return;
		}

		if (placedModel.type == PlaceableModelType::Road)
		{
			const auto highlightState = GetRoadOutlineHighlightState(state, placedModel);
			DrawRoadOutline(placedModel, ColorF{ 1.0, 0.92, 0.28, 0.78 }, highlightState.hoveredCorner, highlightState.hoveredRotationHandle);
			return;
		}

		if (placedModel.type == PlaceableModelType::TireTrackDecal)
		{
			DrawTireTrackOutline(placedModel, ColorF{ 1.0, 0.92, 0.28, 0.78 });
			return;
		}

		const double radius = GetPlacedModelSelectionRadius(placedModel);
		Cylinder{ placedModel.position.movedBy(0, 0.04, 0), radius, 0.08 }.draw(ColorF{ 1.0, 0.92, 0.28, 0.50 }.removeSRGBCurve());
		Sphere{ placedModel.position.movedBy(0, 0.42, 0), 0.22 }.draw(ColorF{ 1.0, 0.96, 0.55, 0.70 }.removeSRGBCurve());
	}

	void DrawSelectedResourceAreaHighlight(const MapEditorState& state, const MapData& mapData)
	{
		if (not IsValidResourceAreaIndex(mapData, state.selectedResourceAreaIndex))
		{
			return;
		}

		const ResourceArea& resourceArea = mapData.resourceAreas[*state.selectedResourceAreaIndex];
		DrawResourceAreaRing(resourceArea.position, resourceArea.radius, ColorF{ 1.0, 0.94, 0.38, 0.95 });
	}

	void DrawSelectionModePreview(const MapEditorState& state, const MapData& mapData, const Vec3& hoverPosition)
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
				const auto highlightState = GetRoadOutlineHighlightState(state, placedModel);
				if (highlightState.hoveredCorner || state.roadResizeDrag || highlightState.hoveredRotationHandle)
				{
					DrawRoadOutline(placedModel, ColorF{ 0.40, 0.96, 1.0, 0.84 }, highlightState.hoveredCorner, highlightState.hoveredRotationHandle);
				}
				else
				{
					PlacedModel previewRoad = placedModel;
					previewRoad.position = hoverPosition;
					DrawRoadOutline(previewRoad, ColorF{ 0.40, 0.96, 1.0, 0.72 });
				}
			}
			else if (placedModel.type == PlaceableModelType::TireTrackDecal)
			{
				PlacedModel previewDecal = placedModel;
				previewDecal.position = hoverPosition;
				DrawTireTrackOutline(previewDecal, ColorF{ 0.40, 0.96, 1.0, 0.78 });
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
	}
}
