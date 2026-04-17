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
		constexpr double TerrainCellOverlayY = 0.01;
		constexpr double TerrainCellOverlayHeight = 0.02;
		constexpr double TerrainFadeOverlayY = 0.016;
		constexpr double TerrainFadeOverlayHeight = 0.014;
		constexpr double TerrainFadeStripWidth = (TerrainCellSize * 0.34);
		constexpr double TerrainFadeInset = (TerrainCellSize * 0.08);
           constexpr double FogExploredAlphaScale = 0.36;

		[[nodiscard]] int64 ToTerrainCellKey(const Point& cell)
		{
			return (static_cast<int64>(cell.x) << 32)
				^ static_cast<uint32>(cell.y);
		}

		[[nodiscard]] bool ShouldDrawTerrainFade(const ColorF& a, const ColorF& b)
		{
			return ((Abs(a.r - b.r) + Abs(a.g - b.g) + Abs(a.b - b.b) + Abs(a.a - b.a)) > 0.08);
		}

		void DrawTerrainEdgeFade(const Vec3& center,
			const Point& direction,
			const ColorF& fromColor,
			const ColorF& toColor)
		{
			const ColorF outerColor = ColorF{ fromColor.lerp(toColor, 0.32), 0.34 };
			const ColorF innerColor = ColorF{ fromColor.lerp(toColor, 0.62), 0.56 };
			const double stripCenterOffset = ((TerrainCellSize - TerrainFadeStripWidth) * 0.5);

			if (direction.x != 0)
			{
				const double offsetX = (stripCenterOffset * direction.x);
				Box{ center.movedBy(offsetX, TerrainFadeOverlayY, 0), TerrainFadeStripWidth, TerrainFadeOverlayHeight, (TerrainCellSize - TerrainFadeInset) }.draw(outerColor.removeSRGBCurve());
				Box{ center.movedBy(offsetX, TerrainFadeOverlayY + 0.002, 0), (TerrainFadeStripWidth * 0.58), TerrainFadeOverlayHeight * 0.72, (TerrainCellSize - TerrainFadeInset * 1.6) }.draw(innerColor.removeSRGBCurve());
				return;
			}

			const double offsetZ = (stripCenterOffset * direction.y);
			Box{ center.movedBy(0, TerrainFadeOverlayY, offsetZ), (TerrainCellSize - TerrainFadeInset), TerrainFadeOverlayHeight, TerrainFadeStripWidth }.draw(outerColor.removeSRGBCurve());
			Box{ center.movedBy(0, TerrainFadeOverlayY + 0.002, offsetZ), (TerrainCellSize - TerrainFadeInset * 1.6), TerrainFadeOverlayHeight * 0.72, (TerrainFadeStripWidth * 0.58) }.draw(innerColor.removeSRGBCurve());
		}

		[[nodiscard]] double GetFogOverlayAlpha(const FogOfWarSettings& settings)
		{
			const double darkness = Clamp(settings.overlayDarkness, 0.0, 1.0);

			switch (settings.overlayDarknessCurve)
			{
			case FogOverlayDarknessCurve::Soft:
				return Pow(darkness, 1.5);

			case FogOverlayDarknessCurve::Strong:
				return Pow(darkness, 2.2);

			case FogOverlayDarknessCurve::Linear:
			default:
				return darkness;
			}
		}

         [[nodiscard]] ColorF GetFogOverlayColor(const FogVisibility visibility, const FogOfWarSettings& settings)
			{
              const double alpha = GetFogOverlayAlpha(settings);
				switch (visibility)
				{
				case FogVisibility::Explored:
                      return ColorF{ 0.03, 0.05, 0.08, (alpha * FogExploredAlphaScale) };

				case FogVisibility::Hidden:
                        return ColorF{ 0.01, 0.02, 0.04, alpha };

				case FogVisibility::Visible:
				default:
					return ColorF{ 0.0, 0.0, 0.0, 0.0 };
				}
			}

		[[nodiscard]] ColorF ApplyFogToTerrainColor(const ColorF& terrainColor, const FogVisibility visibility, const FogOfWarSettings& settings)
		{
			if (visibility == FogVisibility::Visible)
			{
				return terrainColor;
			}

			const ColorF fogColor = GetFogOverlayColor(visibility, settings);
			const double blend = Math::Saturate(fogColor.a);
			return ColorF{
				terrainColor.r + (fogColor.r - terrainColor.r) * blend,
				terrainColor.g + (fogColor.g - terrainColor.g) * blend,
				terrainColor.b + (fogColor.b - terrainColor.b) * blend,
				terrainColor.a,
			};
		}

		[[nodiscard]] bool IsWorldObjectVisibleAt(const FogOfWarState& fogOfWar, const Vec3& position)
		{
			return (not fogOfWar.enabled) || IsFogVisibleAt(fogOfWar, position);
		}

		[[nodiscard]] bool ShouldDrawPlacedModelInFog(const FogOfWarState& fogOfWar, const PlacedModel& placedModel)
		{
			if (not fogOfWar.enabled)
			{
				return true;
			}

			const FogVisibility visibility = GetFogVisibilityAt(fogOfWar, placedModel.position);
			if (visibility == FogVisibility::Visible)
			{
				return true;
			}

			if (visibility == FogVisibility::Explored)
			{
               return true;
			}

			if (visibility == FogVisibility::Hidden)
			{
               return ((placedModel.type == PlaceableModelType::Wall)
					|| (placedModel.type == PlaceableModelType::Rock));
			}

			return false;
		}

		[[nodiscard]] MainSupport::PlacedModelRenderResources BuildPlacedModelRenderResources(const SkyAppResources& resources)
		{
			return MainSupport::PlacedModelRenderResources{
				.millModel = resources.millModel,
				.treeModel = resources.treeModel,
				.pineModel = resources.pineModel,
				.grassPatchModel = resources.grassPatchModel,
				.roadTexture = resources.roadTexture,
				.tireTrackStartTexture = resources.tireTrackStartTexture,
				.tireTrackMiddleTexture = resources.tireTrackMiddleTexture,
				.tireTrackEndTexture = resources.tireTrackEndTexture,
			};
		}

          void DrawTerrainCellOverlay(const TerrainSurfaceCell& terrainCell,
			const HashTable<int64, const TerrainSurfaceCell*>& terrainLookup,
			const FogOfWarState& fogOfWar,
			const FogOfWarSettings& fogSettings)
		{
          const Vec3 center = ToTerrainCellCenter(terrainCell.cell).movedBy(0, TerrainCellOverlayY, 0);
                const FogVisibility visibility = GetFogVisibilityAt(fogOfWar, terrainCell.cell);
				const ColorF cellColor = ApplyFogToTerrainColor(terrainCell.finalColor, visibility, fogSettings);
			if (cellColor.a <= 0.01)
			{
				return;
			}

			Box{ center, TerrainCellSize, TerrainCellOverlayHeight, TerrainCellSize }.draw(cellColor.removeSRGBCurve());

			for (const Point direction : { Point{ 1, 0 }, Point{ -1, 0 }, Point{ 0, 1 }, Point{ 0, -1 } })
			{
				const Point neighborCellPosition = (terrainCell.cell + direction);
				const auto it = terrainLookup.find(ToTerrainCellKey(neighborCellPosition));

				if ((it == terrainLookup.end()) || (it->second == nullptr))
				{
					continue;
				}

              const TerrainSurfaceCell& neighborCell = *it->second;
				const FogVisibility neighborVisibility = GetFogVisibilityAt(fogOfWar, neighborCell.cell);
				const ColorF neighborColor = ApplyFogToTerrainColor(neighborCell.finalColor, neighborVisibility, fogSettings);
				if (not ShouldDrawTerrainFade(cellColor, neighborColor))
				{
					continue;
				}

                DrawTerrainEdgeFade(center, direction, cellColor, neighborColor);
			}
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
            const UnitModelAnimationRole previewRole = ((state.modelHeightEditMode && (state.modelHeightTarget == renderModel))
				? state.modelHeightPreviewAnimationRole
				: UnitModelAnimationRole::Idle);
			  const UnitModel& previewModel = resources.GetUnitRenderModel(renderModel, previewRole);
			if (not previewModel.isLoaded())
			{
				return;
			}

         previewModel.draw(frame.GetPreviewRenderPosition(renderModel),
				BirdDisplayYaw,
                GetUnitRenderModelPreviewColor(renderModel).removeSRGBCurve(),
                GetModelScale(state.modelHeightSettings, renderModel));
		}

		using TerrainCellLookup = HashTable<int64, const TerrainSurfaceCell*>;

		template <class TerrainCells>
		[[nodiscard]] TerrainCellLookup BuildTerrainLookup(const TerrainCells& terrainCells)
		{
			TerrainCellLookup terrainLookup;
			terrainLookup.reserve(terrainCells.size());

			for (const auto& terrainCell : terrainCells)
			{
				terrainLookup.emplace(ToTerrainCellKey(terrainCell.cell), &terrainCell);
			}

			return terrainLookup;
		}

		[[nodiscard]] bool ShouldDrawWorldObject(const SkyAppFrameState& frame, const FogOfWarState& fogOfWar, const Vec3& position)
		{
			return frame.isEditorMode || IsWorldObjectVisibleAt(fogOfWar, position);
		}

		void DrawTerrainSurface(const SkyAppState& state)
		{
			const TerrainCellLookup terrainLookup = BuildTerrainLookup(state.terrainSurface.cells);

			for (const auto& terrainCell : state.terrainSurface.cells)
			{
				DrawTerrainCellOverlay(terrainCell, terrainLookup, state.fogOfWar, state.fogOfWarSettings);
			}
		}

		void DrawResourceAreas(const SkyAppState& state, const SkyAppFrameState& frame)
		{
			for (size_t i = 0; i < state.mapData.resourceAreas.size(); ++i)
			{
				const ResourceArea& area = state.mapData.resourceAreas[i];
				if (not ShouldDrawWorldObject(frame, state.fogOfWar, area.position))
				{
					continue;
				}

				const ResourceAreaState resourceState = ((i < state.resourceAreaStates.size()) ? state.resourceAreaStates[i] : ResourceAreaState{});
				DrawResourceAreaRing(area.position, area.radius, GetResourceAreaColor(area.type));
				Cylinder{ area.position.movedBy(0, 0.06, 0), Max(0.35, area.radius * 0.16), 0.12 }.draw(GetTeamColor(resourceState.ownerTeam).removeSRGBCurve());
			}
		}

		void DrawBases(const SkyAppResources& resources, const SkyAppState& state, const SkyAppFrameState& frame)
		{
			resources.blacksmithModel.draw(state.mapData.playerBasePosition);

			if (ShouldDrawWorldObject(frame, state.fogOfWar, state.mapData.enemyBasePosition))
			{
				resources.blacksmithModel.draw(state.mapData.enemyBasePosition);
			}
		}

		void DrawPreviewUnits(const SkyAppResources& resources, const SkyAppState& state, const SkyAppFrameState& frame)
		{
			for (const UnitRenderModel renderModel : GetUnitRenderModels())
			{
				DrawPreviewUnitRenderModel(resources, state, frame, renderModel);
			}
		}

		void DrawUnitEditorPreview(const SkyAppState& state, const SkyAppFrameState& frame)
		{
			if (not frame.showUnitEditor)
			{
				return;
			}

			const UnitEditorSelection& previewSelection = state.unitEditorSelection;
			const UnitParameters& previewParameters = GetUnitParameters(state.unitEditorSettings, previewSelection.team, previewSelection.unitType);
			const UnitRenderModel previewRenderModel = GetUnitRenderModel(previewSelection.team, previewSelection.unitType);
			const Vec3 previewPosition = frame.GetPreviewRenderPosition(previewRenderModel);
			const ColorF previewColor = ((previewSelection.team == UnitTeam::Enemy)
				? ColorF{ 1.0, 0.76, 0.68, 0.72 }
				: ColorF{ 0.86, 0.94, 1.0, 0.72 });
			DrawUnitFootprintPreview(previewPosition, BirdDisplayYaw, previewParameters, previewColor);
		}

		void DrawPlacedModels(const SkyAppResources& resources, const SkyAppState& state, const SkyAppFrameState& frame)
		{
          const MainSupport::PlacedModelRenderResources renderResources = BuildPlacedModelRenderResources(resources);

			for (const auto& placedModel : state.mapData.placedModels)
			{
				if ((not frame.isEditorMode) && (not ShouldDrawPlacedModelInFog(state.fogOfWar, placedModel)))
				{
					continue;
				}

             DrawPlacedModel(placedModel, state.modelHeightSettings, renderResources);
			}
		}

      void DrawPlacedModelSelectionOverlay(const PlacedModel& placedModel)
		{
			if (not SupportsMillDefenseParameters(placedModel.type))
			{
				return;
			}

			const MillDefenseParameters defenseParameters = GetMillDefenseParameters(placedModel);
			const double attackRange = Clamp(defenseParameters.attackRange, 1.0, 20.0);
			Cylinder{ placedModel.position.movedBy(0, 0.03, 0), attackRange, 0.06 }.draw(ColorF{ 1.0, 0.92, 0.30, 0.28 }.removeSRGBCurve());
			Cylinder{ placedModel.position.movedBy(0, 0.16, 0), 0.65, 0.18 }.draw(ColorF{ 1.0, 0.92, 0.30, 0.70 }.removeSRGBCurve());
		}

		void DrawSelectedPlacedModelOverlay(const SkyAppState& state)
		{
			if (not IsValidMillIndex(state, state.selectedMillIndex))
			{
				return;
			}

         DrawPlacedModelSelectionOverlay(state.mapData.placedModels[*state.selectedMillIndex]);
		}

		void DrawSpawnedUnits(const SkyAppResources& resources, const SkyAppState& state)
		{
			const UnitRenderModelRegistryView renderModels = resources.GetUnitRenderModelRegistry();
           DrawSpawnedSappers(state.spawnedSappers, state.unitEditorSettings, renderModels, state.modelHeightSettings, ColorF{ 0.92, 0.95, 1.0 });
			DrawSpawnedSappers(state.enemySappers, state.unitEditorSettings, renderModels, state.modelHeightSettings, ColorF{ 1.0, 0.78, 0.74 }, &state.fogOfWar, true);
		}

	}

 void RenderWorld(const SkyAppResources& resources, const SkyAppState& state, const SkyAppFrameState& frame)
	{
		resources.groundPlane.draw(resources.groundTexture);
     DrawTerrainSurface(state);
		Sphere{ { 0, 1, 0 }, 1 }.draw(ColorF{ 0.75 }.removeSRGBCurve());
		DrawResourceAreas(state, frame);
		DrawBases(resources, state, frame);
		DrawPreviewUnits(resources, state, frame);
		DrawUnitEditorPreview(state, frame);
		DrawPlacedModels(resources, state, frame);
     DrawSelectedPlacedModelOverlay(state);
		DrawSpawnedUnits(resources, state);

		DrawMapEditorScene(state.mapEditor, state.mapData);
		state.sky.draw();
	}
}
