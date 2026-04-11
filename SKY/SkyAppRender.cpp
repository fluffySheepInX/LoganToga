# include "SkyAppLoopInternal.hpp"
# include "SkyAppSupport.hpp"
# include "MainScene.hpp"

using namespace MainSupport;
using namespace SkyAppSupport;

namespace SkyAppFlow

{
	namespace
	{
      constexpr int32 ResourceAreaRingSegments = 36;

		void DrawTerrainCellOverlay(const TerrainCell& terrainCell)
		{
			const Vec3 center = ToTerrainCellCenter(terrainCell.cell).movedBy(0, 0.01, 0);
			Box{ center, TerrainCellSize, 0.02, TerrainCellSize }.draw(GetTerrainCellDrawColor(terrainCell).removeSRGBCurve());
		}

		[[nodiscard]] ColorF GetResourceAreaColor(const ResourceType type)
		{
			switch (type)
			{
			case ResourceType::Budget:
				return ColorF{ 0.96, 0.82, 0.22, 0.28 };

			case ResourceType::Gunpowder:
				return ColorF{ 0.92, 0.42, 0.26, 0.28 };

			case ResourceType::Mana:
				return ColorF{ 0.42, 0.60, 0.98, 0.28 };

			default:
				return ColorF{ 0.72, 0.72, 0.72, 0.28 };
			}
		}

		void DrawResourceAreaRing(const Vec3& position, const double radius, const ColorF& ringColor)
		{
			const double markerRadius = Clamp((radius * 0.055), 0.10, 0.22);
			const ColorF shadowColor{ 0.02, 0.03, 0.05, 0.56 };

			for (int32 i = 0; i < ResourceAreaRingSegments; ++i)
			{
				const double angle = (Math::TwoPi * i / ResourceAreaRingSegments);
				const Vec3 offset{ Math::Cos(angle) * radius, 0.0, Math::Sin(angle) * radius };
				Cylinder{ position.movedBy(offset).movedBy(0, 0.02, 0), (markerRadius + 0.03), 0.04 }.draw(shadowColor.removeSRGBCurve());
				Cylinder{ position.movedBy(offset).movedBy(0, 0.025, 0), markerRadius, 0.03 }.draw(ringColor.removeSRGBCurve());
			}
		}

		[[nodiscard]] ColorF GetTeamColor(const Optional<UnitTeam>& team)
		{
			if (team && (*team == UnitTeam::Player))
			{
				return ColorF{ 0.92, 0.95, 1.0, 0.95 };
			}

			if (team && (*team == UnitTeam::Enemy))
			{
				return ColorF{ 1.0, 0.78, 0.74, 0.95 };
			}

			return ColorF{ 0.72, 0.78, 0.84, 0.90 };
		}

		void DrawPreviewUnitRenderModel(const SkyAppResources& resources, const SkyAppState& state, const SkyAppFrameState& frame, const UnitRenderModel renderModel)
		{
          const UnitModel& previewModel = resources.GetUnitRenderModel(renderModel);
			if (not previewModel.isLoaded())
			{
				return;
			}

         previewModel.draw(frame.GetPreviewRenderPosition(renderModel),
				BirdDisplayYaw,
                GetUnitRenderModelPreviewColor(renderModel).removeSRGBCurve(),
                GetModelScale(state.modelHeightSettings, renderModel));
		}
	}

 void RenderWorld(const SkyAppResources& resources, const SkyAppState& state, const SkyAppFrameState& frame)
	{
		resources.groundPlane.draw(resources.groundTexture);
        for (const auto& terrainCell : state.mapData.terrainCells)
		{
			DrawTerrainCellOverlay(terrainCell);
		}
		Sphere{ { 0, 1, 0 }, 1 }.draw(ColorF{ 0.75 }.removeSRGBCurve());

		for (size_t i = 0; i < state.mapData.resourceAreas.size(); ++i)
		{
			const ResourceArea& area = state.mapData.resourceAreas[i];
			const ResourceAreaState resourceState = ((i < state.resourceAreaStates.size()) ? state.resourceAreaStates[i] : ResourceAreaState{});
           DrawResourceAreaRing(area.position, area.radius, GetResourceAreaColor(area.type));
			Cylinder{ area.position.movedBy(0, 0.06, 0), Max(0.35, area.radius * 0.16), 0.12 }.draw(GetTeamColor(resourceState.ownerTeam).removeSRGBCurve());
		}

		resources.blacksmithModel.draw(state.mapData.playerBasePosition);
		resources.blacksmithModel.draw(state.mapData.enemyBasePosition);
     for (const UnitRenderModel renderModel : GetUnitRenderModels())
		{
         DrawPreviewUnitRenderModel(resources, state, frame, renderModel);
		}

		if (frame.showUnitEditor)
		{
			const UnitEditorSelection& previewSelection = state.unitEditorSelection;
			const UnitParameters& previewParameters = GetUnitParameters(state.unitEditorSettings, previewSelection.team, previewSelection.unitType);
			const UnitRenderModel previewRenderModel = GetUnitRenderModel(previewSelection.team, previewSelection.unitType);
           const Vec3 previewPosition = frame.GetPreviewRenderPosition(previewRenderModel);
			const ColorF previewColor = ((previewSelection.team == UnitTeam::Enemy)
				? ColorF{ 1.0, 0.76, 0.68, 0.72 }
				: ColorF{ 0.86, 0.94, 1.0, 0.72 });
			DrawUnitFootprintPreview(previewPosition, BirdDisplayYaw, previewParameters, previewColor);
		}

		for (const auto& placedModel : state.mapData.placedModels)
		{
         DrawPlacedModel(placedModel, resources.millModel, resources.treeModel, resources.pineModel, resources.grassPatchModel, resources.roadTexture);
		}

		if (IsValidMillIndex(state, state.selectedMillIndex))
		{
			const PlacedModel& selectedMill = state.mapData.placedModels[*state.selectedMillIndex];
			const double attackRange = Clamp(selectedMill.attackRange, 1.0, 20.0);
			Cylinder{ selectedMill.position.movedBy(0, 0.03, 0), attackRange, 0.06 }.draw(ColorF{ 1.0, 0.92, 0.30, 0.28 }.removeSRGBCurve());
			Cylinder{ selectedMill.position.movedBy(0, 0.16, 0), 0.65, 0.18 }.draw(ColorF{ 1.0, 0.92, 0.30, 0.70 }.removeSRGBCurve());
		}

       const UnitRenderModelRegistryView renderModels = resources.GetUnitRenderModelRegistry();
		DrawSpawnedSappers(state.spawnedSappers, renderModels, state.modelHeightSettings, ColorF{ 0.92, 0.95, 1.0 });
		DrawSpawnedSappers(state.enemySappers, renderModels, state.modelHeightSettings, ColorF{ 1.0, 0.78, 0.74 });
		DrawMapEditorScene(state.mapEditor, state.mapData);
		state.sky.draw();
	}
}
