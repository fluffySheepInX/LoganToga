# include "MapEditorInternal.hpp"

using namespace MapEditorDetail;

void DrawMapEditorScene(const MapEditorState& state, const MapData& mapData)
{
	Cylinder{ mapData.playerBasePosition.movedBy(0, 0.28, 0), 0.24, 0.56 }.draw(ColorF{ 0.22, 0.82, 0.98 }.removeSRGBCurve());
	Sphere{ mapData.playerBasePosition.movedBy(0, 0.74, 0), 0.22 }.draw(ColorF{ 0.40, 0.90, 1.0 }.removeSRGBCurve());
	Cylinder{ mapData.enemyBasePosition.movedBy(0, 0.28, 0), 0.24, 0.56 }.draw(ColorF{ 0.98, 0.32, 0.28 }.removeSRGBCurve());
	Sphere{ mapData.enemyBasePosition.movedBy(0, 0.74, 0), 0.22 }.draw(ColorF{ 1.0, 0.50, 0.45 }.removeSRGBCurve());
	Cylinder{ mapData.sapperRallyPoint.movedBy(0, 0.28, 0), 0.18, 0.56 }.draw(ColorF{ 0.95, 0.82, 0.12 }.removeSRGBCurve());
	Sphere{ mapData.sapperRallyPoint.movedBy(0, 0.68, 0), 0.20 }.draw(ColorF{ 0.98, 0.92, 0.35 }.removeSRGBCurve());

	for (const auto& resourceArea : mapData.resourceAreas)
	{
		const ColorF areaColor = GetResourceAreaColor(resourceArea.type);
		Cylinder{ resourceArea.position.movedBy(0, 0.04, 0), resourceArea.radius, 0.08 }.draw(areaColor.removeSRGBCurve());
		Sphere{ resourceArea.position.movedBy(0, 0.28, 0), 0.18 }.draw(areaColor.removeSRGBCurve());
	}

	if (IsValidPlacedModelIndex(mapData, state.selectedPlacedModelIndex))
	{
		const PlacedModel& placedModel = mapData.placedModels[*state.selectedPlacedModelIndex];
		const double radius = GetPlacedModelSelectionRadius(placedModel);
		Cylinder{ placedModel.position.movedBy(0, 0.04, 0), radius, 0.08 }.draw(ColorF{ 1.0, 0.92, 0.28, 0.50 }.removeSRGBCurve());
		Sphere{ placedModel.position.movedBy(0, 0.42, 0), 0.22 }.draw(ColorF{ 1.0, 0.96, 0.55, 0.70 }.removeSRGBCurve());
	}

	if (not state.enabled || not state.hoveredGroundPosition)
	{
		return;
	}

	const Vec3 hoverPosition = *state.hoveredGroundPosition;
	if (state.selectionMode)
	{
		if (IsValidPlacedModelIndex(mapData, state.selectedPlacedModelIndex))
		{
			const PlacedModel& placedModel = mapData.placedModels[*state.selectedPlacedModelIndex];
			const double radius = GetPlacedModelSelectionRadius(placedModel);
			Cylinder{ hoverPosition.movedBy(0, 0.04, 0), radius, 0.08 }.draw(ColorF{ 0.25, 0.92, 0.98, 0.32 }.removeSRGBCurve());
			Sphere{ hoverPosition.movedBy(0, 0.28, 0), 0.16 }.draw(ColorF{ 0.40, 0.96, 1.0, 0.56 }.removeSRGBCurve());
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
		Cylinder{ hoverPosition.movedBy(0, 0.04, 0), MainSupport::ResourceAreaDefaultRadius, 0.08 }.draw(previewColor.removeSRGBCurve());
		Sphere{ hoverPosition.movedBy(0, 0.28, 0), 0.18 }.draw(previewColor.removeSRGBCurve());
		return;
	}

	Cylinder{ hoverPosition.movedBy(0, 0.06, 0), 0.35, 0.12 }.draw(previewColor.removeSRGBCurve());
	Sphere{ hoverPosition.movedBy(0, 0.26, 0), 0.12 }.draw(previewColor.removeSRGBCurve());
}
