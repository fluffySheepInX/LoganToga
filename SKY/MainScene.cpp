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

	void DrawMillModel(const Model& model, const Mat4x4& mat)
	{
		const auto& materials = model.materials();

		for (const auto& object : model.objects())
		{
			Mat4x4 m = Mat4x4::Identity();

			if (object.name == U"Mill_Blades_Cube.007")
			{
				m *= Mat4x4::Rotate(Vec3{ 0, 0, -1 }, (Scene::Time() * -120_deg), Vec3{ 0, 9.37401, 0 });
			}

			const Transformer3D t{ (m * mat) };
			object.draw(materials);
		}
	}

   bool DrawProjectedLine3D(const MainSupport::AppCamera3D& camera, const Vec3& start, const Vec3& end, const double thickness, const ColorF& color)
	{
      MainSupport::ThrowIfInvalidCameraState(camera, U"DrawProjectedLine3D");

		const Float3 startScreen = camera.worldToScreenPoint(Float3{ static_cast<float>(start.x), static_cast<float>(start.y), static_cast<float>(start.z) });
		const Float3 endScreen = camera.worldToScreenPoint(Float3{ static_cast<float>(end.x), static_cast<float>(end.y), static_cast<float>(end.z) });

		if ((startScreen.z <= 0.0f) || (endScreen.z <= 0.0f))
		{
			return false;
		}

		Line{ Vec2{ startScreen.x, startScreen.y }, Vec2{ endScreen.x, endScreen.y } }.draw(thickness, color);
		return true;
	}

  void DrawArrowAccent3D(const MainSupport::AppCamera3D& camera, const Vec3& anchor, const Vec3& armA, const Vec3& armB, const double thickness, const ColorF& color)
	{
		DrawProjectedLine3D(camera, anchor, (anchor + armA), thickness, color);
		DrawProjectedLine3D(camera, anchor, (anchor + armB), thickness, color);
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

	void DrawPlacedModel(const PlacedModel& placedModel, const Model& millModel, const Model& treeModel, const Model& pineModel)
	{
		switch (placedModel.type)
		{
		case PlaceableModelType::Mill:
			DrawMillModel(millModel, Mat4x4::Translate(placedModel.position));
			return;

		case PlaceableModelType::Tree:
		{
			const ScopedRenderStates3D renderState{ BlendState::OpaqueAlphaToCoverage, RasterizerState::SolidCullNone };
			treeModel.draw(placedModel.position);
			return;
		}

		case PlaceableModelType::Pine:
		{
			const ScopedRenderStates3D renderState{ BlendState::OpaqueAlphaToCoverage, RasterizerState::SolidCullNone };
			pineModel.draw(placedModel.position);
			return;
		}

		case PlaceableModelType::Rock:
		{
			Cylinder{ placedModel.position.movedBy(0, 0.45, 0), 2.1, 0.9 }.draw(ColorF{ 0.36, 0.39, 0.42 }.removeSRGBCurve());
			Sphere{ placedModel.position.movedBy(0, 1.45, 0), 1.75 }.draw(ColorF{ 0.48, 0.50, 0.54 }.removeSRGBCurve());
			Sphere{ placedModel.position.movedBy(-0.8, 1.1, 0.6), 0.82 }.draw(ColorF{ 0.42, 0.45, 0.50 }.removeSRGBCurve());
			return;
		}

		default:
			return;
		}
	}

    void DrawSelectionIndicator(const AppCamera3D& camera, const Vec3& position)
	{
		const Vec3 selectionBoxSize = (BlacksmithSelectionBoxSize + BlacksmithSelectionBoxPadding);
		const double halfWidth = (selectionBoxSize.x * 0.5);
		const double halfDepth = (selectionBoxSize.z * 0.5);
		constexpr double GroundHeight = 0.06;
		const Vec3 topLeft = position.movedBy(-halfWidth, GroundHeight, -halfDepth);
		const Vec3 topRight = position.movedBy(halfWidth, GroundHeight, -halfDepth);
		const Vec3 bottomRight = position.movedBy(halfWidth, GroundHeight, halfDepth);
		const Vec3 bottomLeft = position.movedBy(-halfWidth, GroundHeight, halfDepth);
		const double pulse = (0.65 + (0.35 * Periodic::Sine1_1(1.6s)));
		const ColorF lineColor{ 1.0, 0.9, 0.15, pulse };
		constexpr double lineThickness = 4.5;
		const double accentSize = Max(1.1, Min(selectionBoxSize.x, selectionBoxSize.z) * 0.13);

		DrawProjectedLine3D(camera, topLeft, topRight, lineThickness, lineColor);
		DrawProjectedLine3D(camera, topRight, bottomRight, lineThickness, lineColor);
		DrawProjectedLine3D(camera, bottomRight, bottomLeft, lineThickness, lineColor);
		DrawProjectedLine3D(camera, bottomLeft, topLeft, lineThickness, lineColor);

		auto samplePerimeter = [&](double t)
			{
				t = Math::Fraction(t);

				if (t < 0.25)
				{
					const double edgeT = (t / 0.25);
					return std::pair{ topRight.lerp(topLeft, edgeT), Vec3{ -1, 0, 0 } };
				}
				else if (t < 0.50)
				{
					const double edgeT = ((t - 0.25) / 0.25);
					return std::pair{ topLeft.lerp(bottomLeft, edgeT), Vec3{ 0, 0, 1 } };
				}
				else if (t < 0.75)
				{
					const double edgeT = ((t - 0.50) / 0.25);
					return std::pair{ bottomLeft.lerp(bottomRight, edgeT), Vec3{ 1, 0, 0 } };
				}

				const double edgeT = ((t - 0.75) / 0.25);
				return std::pair{ bottomRight.lerp(topRight, edgeT), Vec3{ 0, 0, -1 } };
			};

		constexpr size_t ArrowCount = 6;
		constexpr double ArrowLoopSpeed = 0.12;

		for (size_t i = 0; i < ArrowCount; ++i)
		{
			const double progress = Math::Fraction((Scene::Time() * ArrowLoopSpeed) + (static_cast<double>(i) / ArrowCount));
			const auto [anchor, tangent] = samplePerimeter(progress);
			const Vec3 backward = (-tangent * accentSize);
			const Vec3 side{ -tangent.z, 0, tangent.x };
			const Vec3 armA = (backward + (side * (accentSize * 0.75)));
			const Vec3 armB = (backward - (side * (accentSize * 0.75)));
			DrawArrowAccent3D(camera, anchor, armA, armB, lineThickness, lineColor);
		}
	}

    void DrawGroundContactOverlay(const AppCamera3D& camera, const Vec3& position)
	{
		const Vec3 markerCenter = position.movedBy(0, 0.06, 0);
		const ColorF markerColor{ 1.0, 0.22, 0.18, 0.95 };
		constexpr double markerHalfSize = 0.28;
		constexpr double markerHeight = 0.9;
		constexpr double lineThickness = 3.0;

		DrawProjectedLine3D(camera, markerCenter.movedBy(-markerHalfSize, 0, 0), markerCenter.movedBy(markerHalfSize, 0, 0), lineThickness, markerColor);
		DrawProjectedLine3D(camera, markerCenter.movedBy(0, 0, -markerHalfSize), markerCenter.movedBy(0, 0, markerHalfSize), lineThickness, markerColor);
		DrawProjectedLine3D(camera, markerCenter, markerCenter.movedBy(0, markerHeight, 0), (lineThickness - 0.5), markerColor);
	}
}
