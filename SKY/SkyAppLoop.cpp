# include "SkyAppLoopInternal.hpp"
# include "MainScene.hpp"
# include "MainSettings.hpp"

using namespace MainSupport;
using namespace SkyAppSupport;

namespace SkyAppFlow
{
	namespace
	{
		constexpr double SelectionDragThreshold = 12.0;
		constexpr double MillSelectionRadius = 4.5;
		constexpr double MaxReasonableWorldCoordinate = 5000.0;
			constexpr double CameraKeyboardMoveSpeedMin = 4.0;
			constexpr double CameraKeyboardMoveSpeedMax = 28.0;
			constexpr double CameraKeyboardMoveSpeedDistanceFactor = 0.7;

		[[nodiscard]] bool IsFiniteVec3Value(const Vec3& value)
		{
			return std::isfinite(value.x)
				&& std::isfinite(value.y)
				&& std::isfinite(value.z);
		}

		[[nodiscard]] bool IsReasonableWorldPosition(const Vec3& position)
		{
			return IsFiniteVec3Value(position)
				&& (Abs(position.x) <= MaxReasonableWorldCoordinate)
				&& (Abs(position.y) <= MaxReasonableWorldCoordinate)
				&& (Abs(position.z) <= MaxReasonableWorldCoordinate);
		}

		[[nodiscard]] bool IsSafeCameraPair(const Vec3& eye, const Vec3& focus)
		{
			if ((not IsFiniteVec3Value(eye)) || (not IsFiniteVec3Value(focus)))
			{
				return false;
			}

			const Vec3 direction = (focus - eye);
			const double distanceSq = direction.lengthSq();
            if ((not std::isfinite(distanceSq)) || (distanceSq < Square(MinimumCameraEyeFocusDistance * 0.5)))
			{
				return false;
			}

			const double upAlignment = Abs((direction / std::sqrt(distanceSq)).dot(Vec3::Up()));
			return std::isfinite(distanceSq)
             && (upAlignment < 0.995);
		}

			void UpdateCameraKeyboardMovement(AppCamera3D& camera, CameraSettings& cameraSettings)
			{
				const double forwardInput = (KeyW.pressed() ? 1.0 : 0.0) - (KeyS.pressed() ? 1.0 : 0.0);
				const double rightInput = (KeyD.pressed() ? 1.0 : 0.0) - (KeyA.pressed() ? 1.0 : 0.0);

				if ((forwardInput == 0.0) && (rightInput == 0.0))
				{
					return;
				}

				const Vec3 viewDirection = (cameraSettings.focus - cameraSettings.eye);
				Vec3 forward{ viewDirection.x, 0.0, viewDirection.z };
				const double forwardLengthSq = forward.lengthSq();

				if ((not std::isfinite(forwardLengthSq)) || (forwardLengthSq <= 0.0001))
				{
					return;
				}

				forward /= std::sqrt(forwardLengthSq);
				const Vec3 right{ forward.z, 0.0, -forward.x };
				Vec3 movement = ((forward * forwardInput) + (right * rightInput));
				const double movementLengthSq = movement.lengthSq();

				if ((not std::isfinite(movementLengthSq)) || (movementLengthSq <= 0.0))
				{
					return;
				}

				movement /= std::sqrt(movementLengthSq);
				const double cameraDistance = viewDirection.length();
				const double moveSpeed = Clamp((cameraDistance * CameraKeyboardMoveSpeedDistanceFactor), CameraKeyboardMoveSpeedMin, CameraKeyboardMoveSpeedMax);
				const Vec3 delta = (movement * (moveSpeed * Scene::DeltaTime()));
				cameraSettings.eye += delta;
				cameraSettings.focus += delta;
				EnsureValidCameraSettings(cameraSettings);
				ThrowIfInvalidCameraPair(cameraSettings.eye, cameraSettings.focus, U"UpdateCameraKeyboardMovement");
				camera.setView(cameraSettings.eye, cameraSettings.focus);
			}

		void ApplyCameraFallback(SkyAppState& state)
		{
			state.cameraSettings.eye = DefaultCameraEye;
			state.cameraSettings.focus = DefaultCameraFocus;
			EnsureValidCameraSettings(state.cameraSettings);
			ThrowIfInvalidCameraPair(state.cameraSettings.eye, state.cameraSettings.focus, U"ApplyCameraFallback");
			state.camera = AppCamera3D{ Graphics3D::GetRenderTargetSize(), 40_deg, state.cameraSettings.eye, state.cameraSettings.focus };
		}

		[[nodiscard]] Optional<size_t> HitTestMill(const Array<PlacedModel>& placedModels, const Optional<Ray>& cursorRay)
		{
			if (not cursorRay)
			{
				return none;
			}

			double nearestDistance = Math::Inf;
			Optional<size_t> nearestIndex;

			for (size_t i = 0; i < placedModels.size(); ++i)
			{
				if (placedModels[i].type != PlaceableModelType::Mill)
				{
					continue;
				}

				const Sphere interactionSphere{ placedModels[i].position.movedBy(0, 4.0, 0), MillSelectionRadius };
				if (const auto distance = cursorRay->intersects(interactionSphere))
				{
					if (*distance < nearestDistance)
					{
						nearestDistance = *distance;
						nearestIndex = i;
					}
				}
			}

			return nearestIndex;
		}

		SkyAppFrameState BuildFrameState(SkyAppState& state)
		{
			SkyAppFrameState frame;
			frame.isEditorMode = (state.appMode == AppMode::EditMap);
			state.mapEditor.enabled = frame.isEditorMode;
			if (not IsValidMillIndex(state, state.selectedMillIndex))
			{
				state.selectedMillIndex.reset();
			}
			frame.showSapperMenu = ((state.selectedSapperIndices.size() == 1) && (not state.playerWon));
			frame.showMillStatusEditor = ((not frame.isEditorMode) && IsValidMillIndex(state, state.selectedMillIndex));
			frame.isHoveringUI = frame.panels.isHoveringUi(state.showUI, frame.isEditorMode, state.showBlacksmithMenu, frame.showSapperMenu, frame.showMillStatusEditor, state.modelHeightEditMode);
			frame.birdRenderPosition = BirdDisplayPosition.movedBy(0, state.modelHeightSettings.birdOffsetY, 0);
			frame.ashigaruRenderPosition = AshigaruDisplayPosition.movedBy(0, state.modelHeightSettings.ashigaruOffsetY, 0);
			return frame;
		}

		Optional<Vec3> GetGroundIntersection(const AppCamera3D& camera)
		{
			const Optional<Ray> ray = TryScreenToRay(camera, Cursor::PosF());
			if (not ray)
			{
				return none;
			}

			const InfinitePlane groundPlane3D{ Float3{ 0, 0, 0 }, Float3{ 0, 1, 0 } };

			if (const auto distance = ray->intersects(groundPlane3D))
			{
				const Vec3 position = ray->point_at(*distance);
				return Vec3{ position.x, 0.0, position.z };
			}

			return none;
		}

		RectF GetSelectionRect(const Optional<Vec2>& selectionDragStart)
		{
			const Vec2 start = *selectionDragStart;
			const Vec2 end = Cursor::PosF();
			return RectF{ Min(start.x, end.x), Min(start.y, end.y), Abs(end.x - start.x), Abs(end.y - start.y) };
		}

		Array<size_t> CollectSappersInSelectionRect(const AppCamera3D& camera, const Array<SpawnedSapper>& spawnedSappers, const RectF& selectionRect)
		{
			Array<size_t> indices;

			for (size_t i = 0; i < spawnedSappers.size(); ++i)
			{
				const Vec3 renderPosition = GetSpawnedSapperRenderPosition(spawnedSappers[i]).movedBy(0, 1.4, 0);
				const Float3 screenPosition = camera.worldToScreenPoint(Float3{ static_cast<float>(renderPosition.x), static_cast<float>(renderPosition.y), static_cast<float>(renderPosition.z) });

				if (screenPosition.z <= 0.0f)
				{
					continue;
				}

				if (selectionRect.intersects(Circle{ Vec2{ screenPosition.x, screenPosition.y }, 12.0 }))
				{
					indices << i;
				}
			}

			return indices;
		}

		void UpdateCameraAndEditor(SkyAppState& state, const SkyAppFrameState& frame)
		{
			EnsureValidCameraSettings(state.cameraSettings);
			if (not IsSafeCameraPair(state.cameraSettings.eye, state.cameraSettings.focus))
			{
				ThrowIfInvalidCameraPair(state.cameraSettings.eye, state.cameraSettings.focus, U"UpdateCameraAndEditor: pre-setView state.cameraSettings");
			}
			ThrowIfInvalidCameraPair(state.cameraSettings.eye, state.cameraSettings.focus, U"UpdateCameraAndEditor: state.camera.setView (initial)");
			state.camera.setView(state.cameraSettings.eye, state.cameraSettings.focus);
			const bool freezeStartupCameraUpdate = (0 < state.startupCameraFreezeFrames);
			const bool allowCameraDragInput = (not frame.isHoveringUI);
			const bool allowCameraWheelInput = ((not frame.isHoveringUI)
				|| ((Mouse::Wheel() != 0.0)
					&& (not MouseL.pressed())
					&& (not MouseR.pressed())));

			if ((not freezeStartupCameraUpdate) && allowCameraDragInput)
			{
               UpdateCameraKeyboardMovement(state.camera, state.cameraSettings);
			}
			state.cameraSettings.eye = state.camera.getEyePosition();
			state.cameraSettings.focus = state.camera.getFocusPosition();
			if (not IsSafeCameraPair(state.cameraSettings.eye, state.cameraSettings.focus))
			{
				ThrowIfInvalidCameraPair(state.cameraSettings.eye, state.cameraSettings.focus, U"UpdateCameraAndEditor: camera state after input");
			}
			else
			{
				EnsureValidCameraSettings(state.cameraSettings);
				if (not IsSafeCameraPair(state.cameraSettings.eye, state.cameraSettings.focus))
				{
					ThrowIfInvalidCameraPair(state.cameraSettings.eye, state.cameraSettings.focus, U"UpdateCameraAndEditor: post-sanitize state.cameraSettings");
				}
				else
				{
					ThrowIfInvalidCameraPair(state.cameraSettings.eye, state.cameraSettings.focus, U"UpdateCameraAndEditor: state.camera.setView (post-sanitize)");
					state.camera.setView(state.cameraSettings.eye, state.cameraSettings.focus);
				}
			}

			if (allowCameraWheelInput)
			{
				UpdateCameraWheelZoom(state.camera, state.cameraSettings, state.mapData.playerBasePosition);
				if (not IsSafeCameraPair(state.cameraSettings.eye, state.cameraSettings.focus))
				{
					ThrowIfInvalidCameraPair(state.cameraSettings.eye, state.cameraSettings.focus, U"UpdateCameraAndEditor: after wheel zoom");
				}
			}

			if (0 < state.startupCameraFreezeFrames)
			{
				--state.startupCameraFreezeFrames;
			}

			Graphics3D::SetCameraTransform(state.camera);
			UpdateMapEditor(state.mapEditor, state.mapData, state.camera, (frame.isEditorMode && (not frame.isHoveringUI)));
		}

		void HandleSelectionInput(SkyAppState& state, const SkyAppFrameState& frame)
		{
			const Sphere playerBaseInteractionSphere{ state.mapData.playerBasePosition + Vec3{ 0, 4.0, 0 }, 4.5 };
			const Optional<Ray> cursorRay = TryScreenToRay(state.camera, Cursor::PosF());
			const bool blacksmithHovered = (cursorRay && playerBaseInteractionSphere.intersects(*cursorRay).has_value());
			const Optional<size_t> hoveredSapperIndex = (frame.isEditorMode ? none : HitTestSpawnedSapper(state.spawnedSappers, state.camera));
			const Optional<size_t> hoveredMillIndex = (frame.isEditorMode ? none : HitTestMill(state.mapData.placedModels, cursorRay));

			if ((not frame.isEditorMode) && (not state.playerWon) && (not frame.isHoveringUI) && MouseL.down())
			{
				state.selectionDragStart = Cursor::PosF();
			}

			if ((not frame.isEditorMode) && (not state.playerWon) && state.selectionDragStart && MouseL.up())
			{
				const RectF selectionRect = GetSelectionRect(state.selectionDragStart);

				if ((SelectionDragThreshold <= selectionRect.w) || (SelectionDragThreshold <= selectionRect.h))
				{
					state.selectedSapperIndices = CollectSappersInSelectionRect(state.camera, state.spawnedSappers, selectionRect);
					state.selectedMillIndex.reset();
					state.showBlacksmithMenu = false;
				}
				else if (hoveredSapperIndex)
				{
					state.selectedSapperIndices = { *hoveredSapperIndex };
					state.selectedMillIndex.reset();
					state.showBlacksmithMenu = false;
				}
				else if (hoveredMillIndex)
				{
					state.selectedMillIndex = *hoveredMillIndex;
					state.selectedSapperIndices.clear();
					state.showBlacksmithMenu = false;
				}
				else if (blacksmithHovered)
				{
					state.showBlacksmithMenu = not state.showBlacksmithMenu;
					state.selectedMillIndex.reset();
					state.selectedSapperIndices.clear();
				}
				else
				{
					state.showBlacksmithMenu = false;
					state.selectedMillIndex.reset();
					state.selectedSapperIndices.clear();
				}

				state.selectionDragStart.reset();
			}

			if ((not frame.isEditorMode) && (not state.playerWon) && (not state.selectedSapperIndices.isEmpty()) && (not frame.isHoveringUI) && MouseR.down())
			{
				if (const auto targetPosition = GetGroundIntersection(state.camera))
				{
					Array<size_t> validSelectedSapperIndices;

					for (const size_t selectedIndex : state.selectedSapperIndices)
					{
						if (selectedIndex < state.spawnedSappers.size())
						{
							validSelectedSapperIndices << selectedIndex;
						}
					}

					for (size_t i = 0; i < validSelectedSapperIndices.size(); ++i)
					{
						SetSpawnedSapperTarget(state.spawnedSappers[validSelectedSapperIndices[i]], GetSapperPopTargetPosition(*targetPosition, i));
					}

					state.selectedSapperIndices = validSelectedSapperIndices;
					state.showBlacksmithMenu = false;
				}
			}
		}
	}

	SkyAppResources::SkyAppResources()
		: groundPlane{ MeshData::OneSidedPlane(2000, { 400, 400 }) }
		, groundTexture{ U"example/texture/ground.jpg", TextureDesc::MippedSRGB }
		, blacksmithModel{ U"example/obj/blacksmith.obj" }
		, millModel{ U"example/obj/mill.obj" }
		, treeModel{ U"example/obj/tree.obj" }
		, pineModel{ U"example/obj/pine.obj" }
		, birdModel{ BirdModelPath, BirdDisplayHeight }
		, ashigaruModel{ AshigaruModelPath, BirdDisplayHeight }
		, renderTexture{ Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes }
	{
		Model::RegisterDiffuseTextures(treeModel, TextureDesc::MippedSRGB);
		Model::RegisterDiffuseTextures(pineModel, TextureDesc::MippedSRGB);
	}

	void ResetCameraToPlayerBase(SkyAppState& state)
	{
		if (not IsReasonableWorldPosition(state.mapData.playerBasePosition))
		{
			ApplyCameraFallback(state);
			return;
		}

		const Vec3 cameraOffset = (DefaultCameraEye - DefaultCameraFocus);
		state.cameraSettings.focus = state.mapData.playerBasePosition;
		state.cameraSettings.eye = (state.cameraSettings.focus + cameraOffset);
		EnsureValidCameraSettings(state.cameraSettings);
		if (not IsSafeCameraPair(state.cameraSettings.eye, state.cameraSettings.focus))
		{
			ThrowIfInvalidCameraPair(state.cameraSettings.eye, state.cameraSettings.focus, U"ResetCameraToPlayerBase");
		}
		ThrowIfInvalidCameraPair(state.cameraSettings.eye, state.cameraSettings.focus, U"ResetCameraToPlayerBase: state.camera.setView");
		state.camera.setView(state.cameraSettings.eye, state.cameraSettings.focus);
	}

	void InitializeSkyAppState(SkyAppState& state)
	{
		state.cameraSettings = LoadCameraSettings();
		EnsureValidCameraSettings(state.cameraSettings);
		ThrowIfInvalidCameraPair(state.cameraSettings.eye, state.cameraSettings.focus, U"InitializeSkyAppState");
		state.modelHeightSettings = LoadModelHeightSettings();
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
		UpdateSpawnedSappers(state.spawnedSappers);
		UpdateSpawnedSappers(state.enemySappers);

		if (not state.playerWon)
		{
			UpdateBattleState(state);
		}

		state.selectedSapperIndices.remove_if([&state](const size_t index)
			{
				return (state.spawnedSappers.size() <= index);
			});

		const SkyAppFrameState frame = BuildFrameState(state);

		{
			const ScopedRenderTarget3D target{ resources.renderTexture.clear(ColorF{ 0.0 }) };
			UpdateCameraAndEditor(state, frame);
			HandleSelectionInput(state, frame);
			RenderWorld(resources, state, frame);
		}

		DrawOverlay(resources, state, frame);
		DrawHudUi(resources, state, frame);
	}
}
