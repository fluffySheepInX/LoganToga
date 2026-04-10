# include "MainScene.hpp"
# include <cmath>

namespace
{
   constexpr double MaxAbsCameraCoordinate = 100000.0;
	constexpr double MaxCameraEyeFocusDistance = 50000.0;
	constexpr double MaxCameraViewUpAlignment = 0.995;

   [[nodiscard]] bool IsFiniteVec3(const Vec3& value)
	{
		return std::isfinite(value.x)
			&& std::isfinite(value.y)
			&& std::isfinite(value.z);
	}

	[[nodiscard]] Vec3 ClampCameraPosition(const Vec3& position)
	{
		return Vec3{
			Clamp(position.x, -MaxAbsCameraCoordinate, MaxAbsCameraCoordinate),
			Clamp(position.y, -MaxAbsCameraCoordinate, MaxAbsCameraCoordinate),
			Clamp(position.z, -MaxAbsCameraCoordinate, MaxAbsCameraCoordinate),
		};
	}

	[[nodiscard]] bool IsSafeCameraDirection(const Vec3& direction)
	{
		if (not IsFiniteVec3(direction))
		{
			return false;
		}

		const double distanceSq = direction.lengthSq();
		if ((not std::isfinite(distanceSq)) || (distanceSq < Square(MainSupport::MinimumCameraEyeFocusDistance * 0.5)))
		{
			return false;
		}

		const Vec3 normalizedDirection = (direction / std::sqrt(distanceSq));
		const double upAlignment = Abs(normalizedDirection.dot(Vec3::Up()));
		return std::isfinite(upAlignment)
			&& (upAlignment < MaxCameraViewUpAlignment);
	}

	[[nodiscard]] String BuildInvalidCameraMessage(const Vec3& eye, const Vec3& focus, const StringView context)
	{
		const Vec3 direction = (focus - eye);
		const double distanceSq = direction.lengthSq();
       const double minimumDistanceSq = Square(MainSupport::MinimumCameraEyeFocusDistance * 0.5);
		const double upAlignment = (((0.0 < distanceSq) && std::isfinite(distanceSq))
			? Abs((direction / std::sqrt(distanceSq)).dot(Vec3::Up()))
			: Math::Inf);

		return Format(
			U"Camera validation failed: ", context,
			U"\n eye = ", eye,
			U"\n focus = ", focus,
			U"\n direction = ", direction,
			U"\n distanceSq = ", distanceSq,
			U"\n minimumAllowedDistanceSq = ", minimumDistanceSq,
          U"\n upAlignment = ", upAlignment,
			U"\n maxAllowedUpAlignment = ", MaxCameraViewUpAlignment,
			U"\n eyeFinite = ", IsFiniteVec3(eye),
			U"\n focusFinite = ", IsFiniteVec3(focus),
			U"\n directionFinite = ", IsFiniteVec3(direction),
			U"\n eyeEqualsFocus = ", (eye == focus));
	}
}

namespace MainSupport
{
    void EnsureValidCameraSettings(CameraSettings& cameraSettings)
	{
		if ((not IsFiniteVec3(cameraSettings.eye)) || (not IsFiniteVec3(cameraSettings.focus)))
		{
			cameraSettings.eye = DefaultCameraEye;
			cameraSettings.focus = DefaultCameraFocus;
		}

		cameraSettings.eye = ClampCameraPosition(cameraSettings.eye);
		cameraSettings.focus = ClampCameraPosition(cameraSettings.focus);

		Vec3 viewDirection = (cameraSettings.focus - cameraSettings.eye);
      double safeDistanceSq = viewDirection.lengthSq();

        if (not IsSafeCameraDirection(viewDirection))
		{
			viewDirection = (DefaultCameraFocus - DefaultCameraEye);
           safeDistanceSq = viewDirection.lengthSq();
		}

     if (not IsSafeCameraDirection(viewDirection))
		{
			viewDirection = Vec3{ 0.0, -0.2, 1.0 };
           safeDistanceSq = viewDirection.lengthSq();
		}

		const Vec3 safeDirection = viewDirection.normalized();
       const double distance = Clamp(std::sqrt(Max(safeDistanceSq, 0.0)), MinimumCameraEyeFocusDistance, MaxCameraEyeFocusDistance);
		cameraSettings.focus = ClampCameraPosition(cameraSettings.eye + safeDirection * distance);

		if ((not IsFiniteVec3(cameraSettings.focus))
           || (not IsSafeCameraDirection(cameraSettings.focus - cameraSettings.eye)))
		{
			cameraSettings.eye = DefaultCameraEye;
			cameraSettings.focus = DefaultCameraFocus;
		}
	}

	void ThrowIfInvalidCameraPair(const Vec3& eye, const Vec3& focus, const StringView context)
	{
		const Vec3 direction = (focus - eye);
		const double distanceSq = direction.lengthSq();

		if ((not IsFiniteVec3(eye))
			|| (not IsFiniteVec3(focus))
			|| (not IsFiniteVec3(direction))
			|| (not std::isfinite(distanceSq))
          || (not IsSafeCameraDirection(direction)))
		{
			throw Error{ BuildInvalidCameraMessage(eye, focus, context) };
		}
	}

	void ThrowIfInvalidCameraState(const AppCamera3D& camera, const StringView context)
	{
		ThrowIfInvalidCameraPair(camera.getEyePosition(), camera.getFocusPosition(), context);
	}

    Optional<Ray> TryScreenToRay(const AppCamera3D& camera, const Vec2& screenPosition)
	{
       ThrowIfInvalidCameraState(camera, U"TryScreenToRay");

		const Vec3 eye = camera.getEyePosition();
		const Vec3 focus = camera.getFocusPosition();

		if ((not IsFiniteVec3(eye)) || (not IsFiniteVec3(focus)))
		{
			return none;
		}

        if (not IsSafeCameraDirection(focus - eye))
		{
			return none;
		}

		return camera.screenToRay(screenPosition);
	}

 Optional<Vec3> GetWheelZoomFocusPosition(const AppCamera3D& camera, const Vec3& playerBasePosition)
	{
     const Optional<Ray> centerRay = TryScreenToRay(camera, Scene::CenterF());
		if (not centerRay)
		{
			return none;
		}

		double nearestDistance = Math::Inf;
		Optional<Vec3> focusPosition;
		const Sphere playerBaseInteractionSphere{ playerBasePosition + Vec3{ 0, 4.0, 0 }, 4.5 };

        if (const auto distance = centerRay->intersects(playerBaseInteractionSphere))
		{
			nearestDistance = *distance;
          focusPosition = centerRay->point_at(*distance);
		}

		const InfinitePlane groundPlane{ Float3{ 0, 0, 0 }, Float3{ 0, 1, 0 } };

        if (const auto distance = centerRay->intersects(groundPlane))
		{
			if (*distance < nearestDistance)
			{
				nearestDistance = *distance;
              focusPosition = centerRay->point_at(*distance);
			}
		}

		return focusPosition;
	}

	Vec3 GetSapperPopTargetPosition(const Vec3& rallyPoint, const size_t sapperIndex)
	{
		const int32 columns = 3;
		const double spacing = 1.9;
		const int32 row = static_cast<int32>(sapperIndex / columns);
		const int32 column = static_cast<int32>(sapperIndex % columns);
		const double xOffset = ((column - 1) * spacing);
		const double zOffset = (row * spacing);
		return rallyPoint.movedBy(xOffset, 0, zOffset);
	}
}
