
# include "SkyAppLoopInternal.hpp"
# include "MainScene.hpp"
# include "MainSettings.hpp"

using namespace MainSupport;
using namespace SkyAppSupport;

namespace SkyAppFlow
{
	namespace Detail
	{
		SkyAppFrameState BuildFrameState(SkyAppState& state)
		{
			SkyAppFrameState frame;
        frame.panels = SkyAppPanels{ state.uiLayoutSettings, state.skySettingsExpanded, state.cameraSettingsExpanded, state.miniMapExpanded, state.showResourceAdjustUi };
			frame.isEditorMode = (state.appMode == AppMode::EditMap);
			frame.showEscMenu = state.showEscMenu;
			state.mapEditor.enabled = frame.isEditorMode;
			if (not IsValidMillIndex(state, state.selectedMillIndex))
			{
				state.selectedMillIndex.reset();
			}
			frame.showSapperMenu = ((state.selectedSapperIndices.size() == 1) && (not state.playerWon));
			frame.showMillStatusEditor = ((not frame.isEditorMode) && IsValidMillIndex(state, state.selectedMillIndex));
            frame.showUnitEditor = (state.showUI && state.unitEditorMode && (not frame.isEditorMode) && (not state.playerWon));
          frame.isHoveringUI = frame.panels.isHoveringUi(state.showUI, state.skySettingsExpanded, state.cameraSettingsExpanded, frame.isEditorMode, state.showBlacksmithMenu, frame.showSapperMenu, frame.showMillStatusEditor, state.modelHeightEditMode, frame.showUnitEditor);
			frame.birdRenderPosition = BirdDisplayPosition.movedBy(0, state.modelHeightSettings.birdOffsetY, 0);
			frame.ashigaruRenderPosition = AshigaruDisplayPosition.movedBy(0, state.modelHeightSettings.ashigaruOffsetY, 0);
           frame.sugoiCarRenderPosition = SugoiCarDisplayPosition.movedBy(0, state.modelHeightSettings.sugoiCarOffsetY, 0);
			return frame;
		}

		[[nodiscard]] Point& GetUiPanelPosition(UiLayoutSettings& settings, const UiLayoutPanel panel)
		{
            switch (panel)
			{
			case UiLayoutPanel::MiniMap:
				return settings.miniMapPosition;

			case UiLayoutPanel::ModelHeight:
				return settings.modelHeightPosition;

			case UiLayoutPanel::UnitEditor:
				return settings.unitEditorPosition;

			case UiLayoutPanel::ResourcePanel:
			default:
				return settings.resourcePanelPosition;
			}
		}

		[[nodiscard]] bool HandleUiEditInput(SkyAppState& state, SkyAppFrameState& frame)
		{
			if ((not state.uiEditMode) || frame.showEscMenu)
			{
				state.uiPanelDrag.reset();
				return false;
			}

			auto beginDrag = [&](const UiLayoutPanel panel, const Rect& panelRect)
				{
					const Point panelPosition{ panelRect.x, panelRect.y };
					state.uiPanelDrag = UiPanelDragState{
						.panel = panel,
						.grabOffset = (Cursor::Pos() - panelPosition),
						.startPosition = panelPosition,
                     .layoutAtDragStart = state.uiLayoutSettings,
						.moved = false,
					};
				};

			if ((not state.uiPanelDrag) && MouseL.down())
			{
				if (frame.panels.resourcePanel.mouseOver())
				{
					beginDrag(UiLayoutPanel::ResourcePanel, frame.panels.resourcePanel);
				}
				else if (frame.panels.miniMap.mouseOver())
				{
					beginDrag(UiLayoutPanel::MiniMap, frame.panels.miniMap);
				}
                else if (state.modelHeightEditMode && frame.panels.modelHeight.mouseOver())
				{
					beginDrag(UiLayoutPanel::ModelHeight, frame.panels.modelHeight);
				}
               else if (frame.panels.unitEditor.mouseOver())
				{
					beginDrag(UiLayoutPanel::UnitEditor, frame.panels.unitEditor);
				}
				else if (frame.panels.unitEditorList.mouseOver())
				{
					beginDrag(UiLayoutPanel::UnitEditor, frame.panels.unitEditorList);
				}
			}

			bool layoutChanged = false;
			bool movedThisFrame = false;

			if (state.uiPanelDrag && MouseL.pressed())
			{
				Point& panelPosition = GetUiPanelPosition(state.uiLayoutSettings, state.uiPanelDrag->panel);
				const Point previousPosition = panelPosition;
                const Point requestedPosition = SkyAppUiLayout::SnapToUiEditGrid(Cursor::Pos() - state.uiPanelDrag->grabOffset);

				if (state.uiPanelDrag->panel == UiLayoutPanel::MiniMap)
				{
					const Rect rect = SkyAppUiLayout::MiniMap(Scene::Width(), Scene::Height(), requestedPosition, state.miniMapExpanded);
					panelPosition = Point{ rect.x, rect.y };
                   movedThisFrame = (panelPosition != state.uiPanelDrag->startPosition);
				}
             else if (state.uiPanelDrag->panel == UiLayoutPanel::ModelHeight)
				{
					const Rect rect = SkyAppUiLayout::ModelHeight(Scene::Width(), Scene::Height(), requestedPosition);
					panelPosition = Point{ rect.x, rect.y };
					movedThisFrame = (panelPosition != state.uiPanelDrag->startPosition);
				}
                else if (state.uiPanelDrag->panel == UiLayoutPanel::UnitEditor)
				{
					const Point delta = (requestedPosition - state.uiPanelDrag->startPosition);
					const Rect startDetailRect = SkyAppUiLayout::UnitEditor(Scene::Width(), Scene::Height(), state.uiPanelDrag->layoutAtDragStart.unitEditorPosition);
					const Rect startListRect = SkyAppUiLayout::UnitEditorList(Scene::Width(), Scene::Height(), state.uiPanelDrag->layoutAtDragStart.unitEditorListPosition);
					const int32 clampedDeltaX = Clamp(delta.x,
						Max(-startDetailRect.x, -startListRect.x),
						Min(Scene::Width() - startDetailRect.rightX(), Scene::Width() - startListRect.rightX()));
					const int32 clampedDeltaY = Clamp(delta.y,
						Max(-startDetailRect.y, -startListRect.y),
						Min(Scene::Height() - startDetailRect.bottomY(), Scene::Height() - startListRect.bottomY()));
					state.uiLayoutSettings.unitEditorPosition = (state.uiPanelDrag->layoutAtDragStart.unitEditorPosition + Point{ clampedDeltaX, clampedDeltaY });
					state.uiLayoutSettings.unitEditorListPosition = (state.uiPanelDrag->layoutAtDragStart.unitEditorListPosition + Point{ clampedDeltaX, clampedDeltaY });
					panelPosition = state.uiLayoutSettings.unitEditorPosition;
                   movedThisFrame = ((clampedDeltaX != 0) || (clampedDeltaY != 0));
				}
				else
				{
					const Rect rect = SkyAppUiLayout::ResourcePanel(Scene::Width(), Scene::Height(), requestedPosition);
					panelPosition = Point{ rect.x, rect.y };
                   movedThisFrame = (panelPosition != state.uiPanelDrag->startPosition);
				}

                state.uiPanelDrag->moved = state.uiPanelDrag->moved || movedThisFrame;
				layoutChanged = (panelPosition != previousPosition)
					|| ((state.uiPanelDrag->panel == UiLayoutPanel::UnitEditor)
						&& (state.uiLayoutSettings.unitEditorListPosition != state.uiPanelDrag->layoutAtDragStart.unitEditorListPosition));
			}

			if (state.uiPanelDrag && MouseL.up())
			{
				const bool moved = state.uiPanelDrag->moved;
				state.uiPanelDrag.reset();

				if (moved)
				{
					state.uiLayoutMessage.show(SaveUiLayoutSettings(state.uiLayoutSettings)
						? U"Saved: {}"_fmt(UiLayoutSettingsPath)
						: U"UI layout save failed");
				}
			}

			if (layoutChanged)
			{
				frame = BuildFrameState(state);
			}

			return layoutChanged;
		}
	}

	namespace
	{
		inline constexpr StringView GrassPatchModelFileName = U"grass2.obj";
		inline constexpr StringView RoadTextureFileName = U"road.jpg";
		inline constexpr StringView SugoiCarModelBaseName = U"sugoiCar";

		[[nodiscard]] FilePath ResolveGrassPatchModelPath()
		{
			const Array<FilePath> candidates{
				FilePath{ U"example/obj/" } + GrassPatchModelFileName,
				FilePath{ GrassPatchModelFileName },
				FilePath{ U"qqq/App/example/obj/" } + GrassPatchModelFileName,
			};

			for (const auto& candidate : candidates)
			{
				if (FileSystem::Exists(candidate))
				{
					return candidate;
				}
			}

			return FilePath{ U"example/obj/" } + GrassPatchModelFileName;
		}

		[[nodiscard]] FilePath ResolveRoadTexturePath()
		{
			const Array<FilePath> candidates{
				FilePath{ U"example/" } + RoadTextureFileName,
				FilePath{ U"App/example/" } + RoadTextureFileName,
				FilePath{ U"SKY/App/example/" } + RoadTextureFileName,
				FilePath{ RoadTextureFileName },
			};

			for (const auto& candidate : candidates)
			{
				if (FileSystem::Exists(candidate))
				{
					return candidate;
				}
			}

			return FilePath{ U"example/" } + RoadTextureFileName;
		}

		[[nodiscard]] FilePath ResolveSugoiCarModelPath()
		{
			const Array<FilePath> candidates{
              FilePath{ SugoiCarModelPath },
				FilePath{ U"App/model/" } + SugoiCarModelBaseName + U".glb",
				FilePath{ U"SKY/App/model/" } + SugoiCarModelBaseName + U".glb",
				FilePath{ U"example/obj/" } + SugoiCarModelBaseName + U".glb",
				FilePath{ U"example/obj/" } + SugoiCarModelBaseName + U".obj",
				FilePath{ U"App/example/obj/" } + SugoiCarModelBaseName + U".glb",
				FilePath{ U"App/example/obj/" } + SugoiCarModelBaseName + U".obj",
				FilePath{ U"SKY/App/example/obj/" } + SugoiCarModelBaseName + U".glb",
				FilePath{ U"SKY/App/example/obj/" } + SugoiCarModelBaseName + U".obj",
				FilePath{ SugoiCarModelBaseName } + U".glb",
				FilePath{ SugoiCarModelBaseName } + U".obj",
			};

			for (const auto& candidate : candidates)
			{
				if (FileSystem::Exists(candidate))
				{
					return candidate;
				}
			}

           return FilePath{ U"SKY/App/model/" } + SugoiCarModelBaseName + U".glb";
		}
	}

	SkyAppResources::SkyAppResources()
		: groundPlane{ MeshData::OneSidedPlane(2000, { 400, 400 }) }
		, groundTexture{ U"example/texture/ground.jpg", TextureDesc::MippedSRGB }
      , roadTexture{ ResolveRoadTexturePath(), TextureDesc::MippedSRGB }
		, blacksmithModel{ U"example/obj/blacksmith.obj" }
		, millModel{ U"example/obj/mill.obj" }
		, treeModel{ U"example/obj/tree.obj" }
		, pineModel{ U"example/obj/pine.obj" }
     , grassPatchModel{ ResolveGrassPatchModelPath() }
		, birdModel{ BirdModelPath, BirdDisplayHeight }
		, ashigaruModel{ AshigaruModelPath, BirdDisplayHeight }
     , sugoiCarModel{ ResolveSugoiCarModelPath(), BirdDisplayHeight }
		, renderTexture{ Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes }
	{
		Model::RegisterDiffuseTextures(treeModel, TextureDesc::MippedSRGB);
		Model::RegisterDiffuseTextures(pineModel, TextureDesc::MippedSRGB);
       Model::RegisterDiffuseTextures(grassPatchModel, TextureDesc::MippedSRGB);
	}

	void InitializeSkyAppState(SkyAppState& state)
	{
		state.cameraSettings = LoadCameraSettings();
		EnsureValidCameraSettings(state.cameraSettings);
		ThrowIfInvalidCameraPair(state.cameraSettings.eye, state.cameraSettings.focus, U"InitializeSkyAppState");
		state.modelHeightSettings = LoadModelHeightSettings();
        state.uiLayoutSettings = LoadUiLayoutSettings(Scene::Width(), Scene::Height());
        state.unitEditorSettings = LoadUnitEditorSettings();
        state.currentMapPath = MainSupport::MapDataPath;
		const MapDataLoadResult initialMapDataLoad = LoadMapDataWithStatus(MapDataPath);
		state.mapData = initialMapDataLoad.mapData;
		state.camera = AppCamera3D{ Graphics3D::GetRenderTargetSize(), 40_deg, state.cameraSettings.eye, state.cameraSettings.focus };
		ResetMatch(state);

		if (not initialMapDataLoad.message.isEmpty())
		{
			state.mapDataMessage.show(initialMapDataLoad.message, 4.0);
		}
	}

	void RunSkyAppFrame(SkyAppResources& resources, SkyAppState& state)
	{
		resources.birdModel.update(Scene::DeltaTime());
		resources.ashigaruModel.update(Scene::DeltaTime());
     resources.sugoiCarModel.update(Scene::DeltaTime());
     UpdateSpawnedSappers(state.spawnedSappers, state.mapData, state.modelHeightSettings);
		UpdateSpawnedSappers(state.enemySappers, state.mapData, state.modelHeightSettings);

		if (not state.playerWon)
		{
			UpdateBattleState(state);
		}

		UpdateAttackEffects(state);

		state.selectedSapperIndices.remove_if([&state](const size_t index)
			{
				return (state.spawnedSappers.size() <= index);
			});

		if (KeyEscape.down())
		{
			state.showEscMenu = not state.showEscMenu;
			state.selectionDragStart.reset();
           state.uiPanelDrag.reset();
		}

          SkyAppFrameState frame = Detail::BuildFrameState(state);
		Detail::HandleUiEditInput(state, frame);

		{
			const ScopedRenderTarget3D target{ resources.renderTexture.clear(ColorF{ 0.0 }) };
            Detail::UpdateCameraAndEditor(state, frame);
			Detail::HandleSelectionInput(state, frame);
			RenderWorld(resources, state, frame);
		}

		DrawOverlay(resources, state, frame);
		DrawHudUi(resources, state, frame);
	}
}
