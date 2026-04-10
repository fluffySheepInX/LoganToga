# include "MainScene.hpp"

namespace
{
	[[nodiscard]] bool DrawProjectedLine3D(const MainSupport::AppCamera3D& camera, const Vec3& start, const Vec3& end, const double thickness, const ColorF& color)
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
