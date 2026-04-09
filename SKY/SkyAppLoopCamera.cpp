# include "SkyAppLoopInternal.hpp"
# include "MainScene.hpp"

using namespace MainSupport;
using namespace SkyAppSupport;

namespace SkyAppFlow
{
	namespace
	{
		constexpr double MaxReasonableWorldCoordinate = 5000.0;
		constexpr double CameraKeyboardMoveSpeedMin = 4.0;
		constexpr double CameraKeyboardMoveSpeedMax = 28.0;
		constexpr double CameraKeyboardMoveSpeedDistanceFactor = 0.7;
			constexpr double CameraMiddleDragPanPerPixelMin = 0.02;
			constexpr double CameraMiddleDragPanPerPixelMax = 0.35;
			constexpr double CameraMiddleDragPanPerPixelDistanceFactor = 0.0025;

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

			void UpdateCameraMiddleDragMovement(AppCamera3D& camera, CameraSettings& cameraSettings, const bool enabled)
			{
				static Optional<Vec2> previousCursorPos;

				if ((not enabled) || (not MouseM.pressed()))
				{
					previousCursorPos.reset();
					return;
				}

				const Vec2 currentCursorPos = Cursor::PosF();

				if (not previousCursorPos)
				{
					previousCursorPos = currentCursorPos;
					return;
				}

				const Vec2 cursorDelta = (currentCursorPos - *previousCursorPos);
				previousCursorPos = currentCursorPos;

				if ((cursorDelta.x == 0.0) && (cursorDelta.y == 0.0))
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
				const double cameraDistance = viewDirection.length();
				const double panPerPixel = Clamp((cameraDistance * CameraMiddleDragPanPerPixelDistanceFactor), CameraMiddleDragPanPerPixelMin, CameraMiddleDragPanPerPixelMax);
				const Vec3 delta = (((right * -cursorDelta.x) + (forward * cursorDelta.y)) * panPerPixel);

				cameraSettings.eye += delta;
				cameraSettings.focus += delta;
				EnsureValidCameraSettings(cameraSettings);
				ThrowIfInvalidCameraPair(cameraSettings.eye, cameraSettings.focus, U"UpdateCameraMiddleDragMovement");
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
	}

	namespace Detail
	{
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
			const bool allowCameraDragInput = ((not frame.showEscMenu) && (not frame.isHoveringUI));
			const bool allowCameraWheelInput = ((not frame.showEscMenu)
				&& ((not frame.isHoveringUI)
					|| ((Mouse::Wheel() != 0.0)
						&& (not MouseL.pressed())
						&& (not MouseR.pressed()))));

				UpdateCameraMiddleDragMovement(state.camera, state.cameraSettings, ((not freezeStartupCameraUpdate) && allowCameraDragInput));

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
			UpdateMapEditor(state.mapEditor, state.mapData, state.camera, (frame.isEditorMode && (not frame.isHoveringUI) && (not frame.showEscMenu)));
		}
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
}
