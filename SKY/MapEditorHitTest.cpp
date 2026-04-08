# include "MapEditorInternal.hpp"
# include "MainScene.hpp"

namespace MapEditorDetail
{
	Optional<size_t> HitTestPlacedModel(const Array<PlacedModel>& placedModels, const MainSupport::AppCamera3D& camera)
	{
		const Optional<Ray> cursorRay = MainSupport::TryScreenToRay(camera, Cursor::PosF());
		if (not cursorRay)
		{
			return none;
		}

		double nearestDistance = Math::Inf;
		Optional<size_t> nearestIndex;

		for (size_t i = 0; i < placedModels.size(); ++i)
		{
			const Sphere interactionSphere{ placedModels[i].position.movedBy(0, 2.2, 0), GetPlacedModelSelectionRadius(placedModels[i]) };
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
