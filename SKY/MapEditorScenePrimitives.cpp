# include "MapEditorSceneInternal.hpp"

using namespace MapEditorDetail;

namespace
{
	constexpr int32 ResourceAreaRingSegments = 28;
	constexpr double WallPreviewThickness = 1.0;
	constexpr double WallPreviewHeight = 2.0;
	constexpr double WallPreviewMarkerSpacing = 1.2;
	constexpr double RoadHandleRadius = 0.34;
	constexpr double RoadRotationHandleRadius = 0.42;

	[[nodiscard]] Vec3 GetWallDirection(const PlacedModel& placedModel)
	{
		return Vec3{ Math::Sin(placedModel.yaw), 0.0, Math::Cos(placedModel.yaw) };
	}
}

namespace MapEditorSceneDetail
{
	void DrawTerrainCell(const TerrainCell& terrainCell, const double yOffset, const double height)
	{
		const Vec3 center = ToTerrainCellCenter(terrainCell.cell).movedBy(0, yOffset, 0);
		const ColorF cellColor = GetTerrainCellDrawColor(terrainCell);
		Box{ center, TerrainCellSize, height, TerrainCellSize }.draw(cellColor.removeSRGBCurve());
	}

	void DrawTerrainCellPreview(const Point& cell, const ColorF& color)
	{
		const Vec3 center = ToTerrainCellCenter(cell).movedBy(0, 0.03, 0);
		Box{ center, TerrainCellSize, 0.035, TerrainCellSize }.draw(color.removeSRGBCurve());
	}

	void DrawTerrainCellPreviewRange(const Point& startCell, const Point& endCell, const ColorF& color)
	{
		const int32 minX = Min(startCell.x, endCell.x);
		const int32 maxX = Max(startCell.x, endCell.x);
		const int32 minY = Min(startCell.y, endCell.y);
		const int32 maxY = Max(startCell.y, endCell.y);

		for (int32 y = minY; y <= maxY; ++y)
		{
			for (int32 x = minX; x <= maxX; ++x)
			{
				DrawTerrainCellPreview(Point{ x, y }, color);
			}
		}
	}

	void DrawTireTrackOutline(const PlacedModel& placedModel, const ColorF& outlineColor)
	{
		const double decalLength = Clamp(placedModel.roadLength, 2.0, 80.0);
		const double decalWidth = Clamp(placedModel.roadWidth, 2.0, 80.0);
		const Vec3 direction{ Math::Sin(placedModel.yaw), 0.0, Math::Cos(placedModel.yaw) };
		const Vec3 side{ direction.z, 0.0, -direction.x };
		const Vec3 halfForward = (direction * (decalLength * 0.5));
		const Vec3 halfSide = (side * (decalWidth * 0.5));
		const Array<Vec3> corners{
			placedModel.position - halfForward - halfSide,
			placedModel.position - halfForward + halfSide,
			placedModel.position + halfForward + halfSide,
			placedModel.position + halfForward - halfSide,
		};

		for (size_t i = 0; i < corners.size(); ++i)
		{
			Line3D{ corners[i].movedBy(0, 0.06, 0), corners[(i + 1) % corners.size()].movedBy(0, 0.06, 0) }.draw(outlineColor.removeSRGBCurve());
		}

		const Vec3 startMarker = (placedModel.position - halfForward).movedBy(0, 0.10, 0);
		const Vec3 endMarker = (placedModel.position + halfForward).movedBy(0, 0.10, 0);
		Cylinder{ startMarker, 0.12, 0.14 }.draw(ColorF{ 0.24, 0.82, 1.0, outlineColor.a }.removeSRGBCurve());
		Cylinder{ endMarker, 0.12, 0.14 }.draw(ColorF{ 1.0, 0.86, 0.24, outlineColor.a }.removeSRGBCurve());
	}

	void DrawResourceAreaRing(const Vec3& position, const double radius, const ColorF& ringColor)
	{
		const double markerRadius = Clamp((radius * 0.06), 0.10, 0.24);
		const ColorF shadowColor{ 0.02, 0.03, 0.05, 0.72 };

		for (int32 i = 0; i < ResourceAreaRingSegments; ++i)
		{
			const double angle = (Math::TwoPi * i / ResourceAreaRingSegments);
			const Vec3 offset{ Math::Cos(angle) * radius, 0.0, Math::Sin(angle) * radius };
			Cylinder{ position.movedBy(offset).movedBy(0, 0.03, 0), (markerRadius + 0.03), 0.05 }.draw(shadowColor.removeSRGBCurve());
			Cylinder{ position.movedBy(offset).movedBy(0, 0.035, 0), markerRadius, 0.04 }.draw(ringColor.removeSRGBCurve());
		}

		Cylinder{ position.movedBy(0, 0.05, 0), 0.15, 0.10 }.draw(ColorF{ 0.06, 0.08, 0.11, 0.92 }.removeSRGBCurve());
		Sphere{ position.movedBy(0, 0.22, 0), 0.12 }.draw(ringColor.removeSRGBCurve());
	}

	void DrawWallPreview(const PlacedModel& placedModel, const ColorF& wallColor)
	{
		const double wallLength = Clamp(placedModel.wallLength, 2.0, 80.0);
		const Vec3 halfDirection = (GetWallDirection(placedModel) * (wallLength * 0.5));
		const Vec3 start = (placedModel.position - halfDirection);
		const Vec3 goal = (placedModel.position + halfDirection);
		Line3D{ start.movedBy(0, 0.12, 0), goal.movedBy(0, 0.12, 0) }.draw(wallColor.removeSRGBCurve());
		Cylinder{ start.movedBy(0, 0.5, 0), 0.28, WallPreviewHeight }.draw(wallColor.removeSRGBCurve());
		Cylinder{ goal.movedBy(0, 0.5, 0), 0.28, WallPreviewHeight }.draw(wallColor.removeSRGBCurve());

		const int32 segmentCount = Max(1, static_cast<int32>(Math::Ceil(wallLength / WallPreviewMarkerSpacing)));
		for (int32 i = 0; i <= segmentCount; ++i)
		{
			const double t = (static_cast<double>(i) / segmentCount);
			const Vec3 position = start.lerp(goal, t);
			Cylinder{ position.movedBy(0, 0.4, 0), (WallPreviewThickness * 0.34), 0.8 }.draw(ColorF{ wallColor, 0.85 }.removeSRGBCurve());
		}
	}

	void DrawRoadOutline(const PlacedModel& placedModel, const ColorF& outlineColor, const Optional<int32>& highlightedCorner, const bool highlightedRotationHandle)
	{
		const Array<Vec3> corners = GetRoadCorners(placedModel);
		if (corners.size() < 4)
		{
			return;
		}

		for (size_t i = 0; i < corners.size(); ++i)
		{
			const Vec3 start = corners[i].movedBy(0, 0.08, 0);
			const Vec3 end = corners[(i + 1) % corners.size()].movedBy(0, 0.08, 0);
			Line3D{ start, end }.draw(outlineColor.removeSRGBCurve());
		}

		for (size_t i = 0; i < corners.size(); ++i)
		{
			const bool isHighlighted = (highlightedCorner && (*highlightedCorner == static_cast<int32>(i)));
			const ColorF handleColor = isHighlighted ? ColorF{ 1.0, 0.94, 0.34, 0.95 } : outlineColor;
			Cylinder{ corners[i].movedBy(0, 0.18, 0), (RoadHandleRadius + (isHighlighted ? 0.06 : 0.0)), 0.18 }.draw(handleColor.removeSRGBCurve());
		}

		const Vec3 rotationHandle = GetRoadRotationHandlePosition(placedModel);
		Line3D{ placedModel.position.movedBy(0, 0.10, 0), rotationHandle.movedBy(0, 0.10, 0) }.draw(outlineColor.removeSRGBCurve());
		const ColorF rotationHandleColor = highlightedRotationHandle ? ColorF{ 1.0, 0.94, 0.34, 0.95 } : outlineColor;
		Cylinder{ rotationHandle.movedBy(0, 0.20, 0), (RoadRotationHandleRadius + (highlightedRotationHandle ? 0.06 : 0.0)), 0.18 }.draw(rotationHandleColor.removeSRGBCurve());
	}

	RoadOutlineHighlightState GetRoadOutlineHighlightState(const MapEditorState& state, const PlacedModel& placedModel)
	{
		return RoadOutlineHighlightState{
			.hoveredCorner = state.roadResizeDrag
				? Optional<int32>{ state.roadResizeDrag->draggedCornerIndex }
				: HitTestRoadCornerHandle(placedModel, state.hoveredGroundPosition),
			.hoveredRotationHandle = (state.roadRotateDrag
				|| HitTestRoadRotationHandle(placedModel, state.hoveredGroundPosition)),
		};
	}
}
