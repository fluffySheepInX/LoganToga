# include "SkyAppLoopInternal.hpp"
# include "SkyAppSupport.hpp"
# include "MainScene.hpp"

using namespace MainSupport;
using namespace SkyAppSupport;

namespace SkyAppFlow

{
	namespace
	{
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
	}

	void RenderWorld(SkyAppResources& resources, SkyAppState& state, const SkyAppFrameState& frame)
	{
		resources.groundPlane.draw(resources.groundTexture);
		Sphere{ { 0, 1, 0 }, 1 }.draw(ColorF{ 0.75 }.removeSRGBCurve());

		for (size_t i = 0; i < state.mapData.resourceAreas.size(); ++i)
		{
			const ResourceArea& area = state.mapData.resourceAreas[i];
			const ResourceAreaState resourceState = ((i < state.resourceAreaStates.size()) ? state.resourceAreaStates[i] : ResourceAreaState{});
			Cylinder{ area.position.movedBy(0, 0.02, 0), area.radius, 0.04 }.draw(GetResourceAreaColor(area.type).removeSRGBCurve());
			Cylinder{ area.position.movedBy(0, 0.06, 0), Max(0.35, area.radius * 0.16), 0.12 }.draw(GetTeamColor(resourceState.ownerTeam).removeSRGBCurve());
		}

		resources.blacksmithModel.draw(state.mapData.playerBasePosition);
		resources.blacksmithModel.draw(state.mapData.enemyBasePosition);
		if (resources.birdModel.isLoaded())
		{
			resources.birdModel.draw(frame.birdRenderPosition, BirdDisplayYaw, ColorF{ 0.92, 0.95, 1.0 }.removeSRGBCurve());
		}
		if (resources.ashigaruModel.isLoaded())
		{
			resources.ashigaruModel.draw(frame.ashigaruRenderPosition, BirdDisplayYaw, ColorF{ 0.95, 0.92, 0.90 }.removeSRGBCurve());
		}
		for (const auto& placedModel : state.mapData.placedModels)
		{
			DrawPlacedModel(placedModel, resources.millModel, resources.treeModel, resources.pineModel);
		}

		if (IsValidMillIndex(state, state.selectedMillIndex))
		{
			const PlacedModel& selectedMill = state.mapData.placedModels[*state.selectedMillIndex];
			const double attackRange = Clamp(selectedMill.attackRange, 1.0, 20.0);
			Cylinder{ selectedMill.position.movedBy(0, 0.03, 0), attackRange, 0.06 }.draw(ColorF{ 1.0, 0.92, 0.30, 0.28 }.removeSRGBCurve());
			Cylinder{ selectedMill.position.movedBy(0, 0.16, 0), 0.65, 0.18 }.draw(ColorF{ 1.0, 0.92, 0.30, 0.70 }.removeSRGBCurve());
		}

		DrawSpawnedSappers(state.spawnedSappers, resources.birdModel, ColorF{ 0.92, 0.95, 1.0 });
		DrawSpawnedSappers(state.enemySappers, resources.ashigaruModel, ColorF{ 1.0, 0.78, 0.74 });
		DrawMapEditorScene(state.mapEditor, state.mapData);
		UpdateSkyFromTime(state.sky, state.skyTime);
		state.sky.draw();
	}
}
