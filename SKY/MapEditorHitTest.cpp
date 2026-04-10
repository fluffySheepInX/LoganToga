# include "MapEditorInternal.hpp"
# include "MainScene.hpp"

namespace MapEditorDetail
{
 namespace
	{
		constexpr double WallSelectionHalfWidth = 1.25;
		constexpr double RoadCornerHandleRadius = 1.35;
     constexpr double RoadRotationHandleRadius = 1.6;
		constexpr double RoadRotationHandleOffset = 2.4;
		constexpr double MinRoadSpan = 2.0;
		constexpr double MaxRoadSpan = 80.0;

		[[nodiscard]] Vec2 ToHorizontal(const Vec3& value)
		{
			return Vec2{ value.x, value.z };
		}

		[[nodiscard]] double DistanceSqPointToSegment(const Vec2& point, const Vec2& start, const Vec2& end)
		{
			const Vec2 segment = (end - start);
			const double segmentLengthSq = segment.lengthSq();

			if (segmentLengthSq <= 0.0001)
			{
				return point.distanceFromSq(start);
			}

			const double t = Clamp(((point - start).dot(segment) / segmentLengthSq), 0.0, 1.0);
			return point.distanceFromSq(start + (segment * t));
		}

		[[nodiscard]] std::pair<Vec2, Vec2> GetWallEndpoints(const PlacedModel& placedModel)
		{
			const Vec2 center = ToHorizontal(placedModel.position);
			const double wallLength = Clamp(placedModel.wallLength, 2.0, 80.0);
			const Vec2 direction{ Math::Sin(placedModel.yaw), Math::Cos(placedModel.yaw) };
			const Vec2 halfDirection = (direction * (wallLength * 0.5));
			return { (center - halfDirection), (center + halfDirection) };
		}

		[[nodiscard]] double ClampRoadSpan(const double value)
		{
			return Clamp(value, MinRoadSpan, MaxRoadSpan);
		}

		[[nodiscard]] Vec2 GetRoadForwardDirection(const PlacedModel& placedModel)
		{
			return Vec2{ Math::Sin(placedModel.yaw), Math::Cos(placedModel.yaw) };
		}

		[[nodiscard]] Vec2 GetRoadRightDirection(const PlacedModel& placedModel)
		{
			const Vec2 forward = GetRoadForwardDirection(placedModel);
			return Vec2{ forward.y, -forward.x };
		}

		[[nodiscard]] Vec2 ToRoadLocal(const PlacedModel& placedModel, const Vec3& position)
		{
			const Vec2 delta = (ToHorizontal(position) - ToHorizontal(placedModel.position));
			const Vec2 forward = GetRoadForwardDirection(placedModel);
			const Vec2 right = GetRoadRightDirection(placedModel);
			return Vec2{ delta.dot(forward), delta.dot(right) };
		}

		[[nodiscard]] std::pair<int32, int32> GetRoadCornerSigns(const int32 cornerIndex)
		{
			switch (cornerIndex)
			{
			case 0:
				return { 1, 1 };

			case 1:
				return { 1, -1 };

			case 2:
				return { -1, -1 };

			case 3:
			default:
				return { -1, 1 };
			}
		}

		[[nodiscard]] bool IsPointInsideRoad(const PlacedModel& placedModel, const Vec3& position)
		{
			const Vec2 local = ToRoadLocal(placedModel, position);
			return (Abs(local.x) <= (ClampRoadSpan(placedModel.roadLength) * 0.5))
				&& (Abs(local.y) <= (ClampRoadSpan(placedModel.roadWidth) * 0.5));
		}
	}

	Array<Vec3> GetRoadCorners(const PlacedModel& placedModel)
	{
		const Vec2 center = ToHorizontal(placedModel.position);
		const Vec2 forward = (GetRoadForwardDirection(placedModel) * (ClampRoadSpan(placedModel.roadLength) * 0.5));
		const Vec2 right = (GetRoadRightDirection(placedModel) * (ClampRoadSpan(placedModel.roadWidth) * 0.5));

		return {
			Vec3{ center.x + forward.x + right.x, 0.0, center.y + forward.y + right.y },
			Vec3{ center.x + forward.x - right.x, 0.0, center.y + forward.y - right.y },
			Vec3{ center.x - forward.x - right.x, 0.0, center.y - forward.y - right.y },
			Vec3{ center.x - forward.x + right.x, 0.0, center.y - forward.y + right.y },
		};
	}

	Vec3 GetRoadRotationHandlePosition(const PlacedModel& placedModel)
	{
		const Vec2 center = ToHorizontal(placedModel.position);
		const Vec2 forward = GetRoadForwardDirection(placedModel);
		const Vec2 offset = (forward * ((ClampRoadSpan(placedModel.roadLength) * 0.5) + RoadRotationHandleOffset));
		return Vec3{ center.x + offset.x, placedModel.position.y, center.y + offset.y };
	}

	Optional<int32> HitTestRoadCornerHandle(const PlacedModel& placedModel, const Optional<Vec3>& hoveredGroundPosition)
	{
		if ((placedModel.type != PlaceableModelType::Road) || (not hoveredGroundPosition))
		{
			return none;
		}

		double nearestDistanceSq = Square(RoadCornerHandleRadius);
		Optional<int32> nearestCornerIndex;
		const Array<Vec3> corners = GetRoadCorners(placedModel);

		for (int32 i = 0; i < static_cast<int32>(corners.size()); ++i)
		{
			const double distanceSq = corners[static_cast<size_t>(i)].distanceFromSq(*hoveredGroundPosition);
			if (distanceSq >= nearestDistanceSq)
			{
				continue;
			}

			nearestDistanceSq = distanceSq;
			nearestCornerIndex = i;
		}

		return nearestCornerIndex;
	}

	bool HitTestRoadRotationHandle(const PlacedModel& placedModel, const Optional<Vec3>& hoveredGroundPosition)
	{
		if ((placedModel.type != PlaceableModelType::Road) || (not hoveredGroundPosition))
		{
			return false;
		}

		return (GetRoadRotationHandlePosition(placedModel).distanceFromSq(*hoveredGroundPosition) <= Square(RoadRotationHandleRadius));
	}

	void ResizeRoadFromCorner(PlacedModel& placedModel, const int32 draggedCornerIndex, const Vec3& draggedPosition, const Vec3& fixedCornerPosition)
	{
		if (placedModel.type != PlaceableModelType::Road)
		{
			return;
		}

		const auto [forwardSign, rightSign] = GetRoadCornerSigns(draggedCornerIndex);
		const Vec2 diagonal = (ToHorizontal(draggedPosition) - ToHorizontal(fixedCornerPosition));
		const Vec2 forward = GetRoadForwardDirection(placedModel);
		const Vec2 right = GetRoadRightDirection(placedModel);
		const double roadLength = ClampRoadSpan(Abs(diagonal.dot(forward)));
		const double roadWidth = ClampRoadSpan(Abs(diagonal.dot(right)));
		const Vec2 center = ToHorizontal(fixedCornerPosition)
			+ (forward * (forwardSign * roadLength * 0.5))
			+ (right * (rightSign * roadWidth * 0.5));

		placedModel.position = Vec3{ center.x, fixedCornerPosition.y, center.y };
		placedModel.roadLength = roadLength;
		placedModel.roadWidth = roadWidth;
	}

	Optional<size_t> HitTestPlacedModel(const Array<PlacedModel>& placedModels, const MainSupport::AppCamera3D& camera, const Optional<Vec3>& hoveredGroundPosition)
	{
		const Optional<Ray> cursorRay = MainSupport::TryScreenToRay(camera, Cursor::PosF());
      if ((not cursorRay) && (not hoveredGroundPosition))
		{
			return none;
		}

		double nearestDistance = Math::Inf;
		Optional<size_t> nearestIndex;

		for (size_t i = 0; i < placedModels.size(); ++i)
		{
         if ((placedModels[i].type == PlaceableModelType::Wall)
				|| (placedModels[i].type == PlaceableModelType::Road))
			{
				continue;
			}

			const Sphere interactionSphere{ placedModels[i].position.movedBy(0, 2.2, 0), GetPlacedModelSelectionRadius(placedModels[i]) };
         if (cursorRay && cursorRay->intersects(interactionSphere))
			{
                const auto distance = cursorRay->intersects(interactionSphere);
				if (*distance < nearestDistance)
				{
					nearestDistance = *distance;
					nearestIndex = i;
				}
			}
		}

		if (nearestIndex)
		{
			return nearestIndex;
		}

		if (not hoveredGroundPosition)
		{
			return none;
		}

       double nearestGroundSelectionDistanceSq = Math::Inf;
		const Vec2 hoverPoint = ToHorizontal(*hoveredGroundPosition);

		for (size_t i = 0; i < placedModels.size(); ++i)
		{
           if (placedModels[i].type == PlaceableModelType::Wall)
			{
               const auto [start, end] = GetWallEndpoints(placedModels[i]);
				const double distanceSq = DistanceSqPointToSegment(hoverPoint, start, end);
				if ((Square(WallSelectionHalfWidth) < distanceSq) || (distanceSq >= nearestGroundSelectionDistanceSq))
				{
					continue;
				}

				nearestGroundSelectionDistanceSq = distanceSq;
				nearestIndex = i;
				continue;
			}

            if ((placedModels[i].type == PlaceableModelType::Road) && IsPointInsideRoad(placedModels[i], *hoveredGroundPosition))
			{
               const double distanceSq = ToRoadLocal(placedModels[i], *hoveredGroundPosition).lengthSq();
				if (distanceSq < nearestGroundSelectionDistanceSq)
				{
					nearestGroundSelectionDistanceSq = distanceSq;
					nearestIndex = i;
				}
			}
		}

		return nearestIndex;
	}

	Optional<size_t> HitTestNavPoint(const Array<NavPoint>& navPoints, const MainSupport::AppCamera3D& camera)
	{
		const Optional<Ray> cursorRay = MainSupport::TryScreenToRay(camera, Cursor::PosF());
		if (not cursorRay)
		{
			return none;
		}

		double nearestDistance = Math::Inf;
		Optional<size_t> nearestIndex;

		for (size_t i = 0; i < navPoints.size(); ++i)
		{
			const Sphere interactionSphere{ navPoints[i].position.movedBy(0, 0.45, 0), GetNavPointSelectionRadius(navPoints[i]) };
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

	Optional<size_t> HitTestResourceArea(const Array<ResourceArea>& resourceAreas, const Optional<Vec3>& hoveredGroundPosition)
	{
		if (not hoveredGroundPosition)
		{
			return none;
		}

		double nearestDistanceSq = Math::Inf;
		Optional<size_t> nearestIndex;

		for (size_t i = 0; i < resourceAreas.size(); ++i)
		{
			const double distanceSq = hoveredGroundPosition->distanceFromSq(resourceAreas[i].position);

			if ((Square(resourceAreas[i].radius) < distanceSq) || (distanceSq >= nearestDistanceSq))
			{
				continue;
			}

			nearestDistanceSq = distanceSq;
			nearestIndex = i;
		}

		return nearestIndex;
	}

	size_t CountNavLinksForPoint(const MapData& mapData, const size_t navPointIndex)
	{
		size_t count = 0;
		for (const auto& navLink : mapData.navLinks)
		{
			if ((navLink.fromIndex == navPointIndex) || (navLink.toIndex == navPointIndex))
			{
				++count;
			}
		}

		return count;
	}

	void RemoveNavPointAt(MapData& mapData, const size_t navPointIndex)
	{
		if (mapData.navPoints.size() <= navPointIndex)
		{
			return;
		}

		mapData.navPoints.erase(mapData.navPoints.begin() + navPointIndex);
		Array<NavLink> adjustedLinks;
		adjustedLinks.reserve(mapData.navLinks.size());

		for (const auto& navLink : mapData.navLinks)
		{
			if ((navLink.fromIndex == navPointIndex) || (navLink.toIndex == navPointIndex))
			{
				continue;
			}

			adjustedLinks << NavLink{
				.fromIndex = (navPointIndex < navLink.fromIndex) ? (navLink.fromIndex - 1) : navLink.fromIndex,
				.toIndex = (navPointIndex < navLink.toIndex) ? (navLink.toIndex - 1) : navLink.toIndex,
				.bidirectional = navLink.bidirectional,
				.costMultiplier = navLink.costMultiplier,
			};
		}

		mapData.navLinks.swap(adjustedLinks);
	}

	bool ToggleNavLink(MapData& mapData, const size_t firstIndex, const size_t secondIndex)
	{
		if ((firstIndex == secondIndex)
			|| (mapData.navPoints.size() <= firstIndex)
			|| (mapData.navPoints.size() <= secondIndex))
		{
			return false;
		}

		for (size_t i = 0; i < mapData.navLinks.size(); ++i)
		{
			const NavLink& navLink = mapData.navLinks[i];
			const bool matchesForward = ((navLink.fromIndex == firstIndex) && (navLink.toIndex == secondIndex));
			const bool matchesReverse = ((navLink.fromIndex == secondIndex) && (navLink.toIndex == firstIndex));

			if (matchesForward || matchesReverse)
			{
				mapData.navLinks.erase(mapData.navLinks.begin() + i);
				return false;
			}
		}

		const size_t fromIndex = (firstIndex < secondIndex) ? firstIndex : secondIndex;
		const size_t toIndex = (firstIndex < secondIndex) ? secondIndex : firstIndex;
		mapData.navLinks << NavLink{ .fromIndex = fromIndex, .toIndex = toIndex, .bidirectional = true, .costMultiplier = 1.0 };
		return true;
	}
}
