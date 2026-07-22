# include "../stdafx.h"
# include "BuildingPlacementTool.hpp"

namespace building
{
	void BuildingPlacementTool::update(BuildingDocument& document, const BasicCamera3D& camera, const bool cursorBlockedByUI)
	{
		const Optional<Vec3> groundPoint = cursorGroundPoint(camera);
		if (m_placementGhost && groundPoint)
		{
			m_placementGhost->origin = KeyShift.pressed() ? snapPoint(*groundPoint) : *groundPoint;
		}

		if (cursorBlockedByUI)
		{
			m_draggingSelected = false;
			return;
		}

		if (MouseL.down() && groundPoint)
		{
			if (m_placementGhost)
			{
				GeneratedBuilding building = *m_placementGhost;
				building.id = document.makeNextId();
				building.name = U"Building {}"_fmt(document.nextSerial - 1);
				building.variationSeed = nextVariationSeed();
				Sanitize(building);
				document.buildings << building;
				m_selectedBuildingId = building.id;
				m_placementGhost.reset();
				return;
			}

			m_selectedBuildingId = findBuildingAt(document, *groundPoint);
			m_draggingSelected = m_selectedBuildingId.has_value();
		}

		if (not MouseL.pressed())
		{
			m_draggingSelected = false;
		}

		if (m_draggingSelected && m_selectedBuildingId && groundPoint)
		{
			if (GeneratedBuilding* selected = document.findById(*m_selectedBuildingId))
			{
				selected->origin = KeyShift.pressed() ? snapPoint(*groundPoint) : *groundPoint;
			}
		}
	}

	void BuildingPlacementTool::beginPlacement(const Vec3& fallbackOrigin)
	{
		m_placementGhost = MakeDefaultBuilding(fallbackOrigin, m_seedSerial++);
	}

	void BuildingPlacementTool::cancelTransientTool()
	{
		m_placementGhost.reset();
		m_draggingSelected = false;
	}

	void BuildingPlacementTool::duplicateSelected(BuildingDocument& document)
	{
		if (not m_selectedBuildingId)
		{
			return;
		}

		if (const GeneratedBuilding* source = document.findById(*m_selectedBuildingId))
		{
			GeneratedBuilding copy = *source;
			copy.id = document.makeNextId();
			copy.name = U"{} Copy"_fmt(source->name);
			copy.origin += Vec3{ 1.0, 0.0, 1.0 };
			copy.variationSeed = nextVariationSeed();
			document.buildings << copy;
			m_selectedBuildingId = copy.id;
		}
	}

	void BuildingPlacementTool::deleteSelected(BuildingDocument& document)
	{
		if (not m_selectedBuildingId)
		{
			return;
		}

		if (const auto index = document.indexOf(*m_selectedBuildingId))
		{
			document.buildings.erase(document.buildings.begin() + *index);
		}
		m_selectedBuildingId.reset();
	}

	void BuildingPlacementTool::rotateSelected(BuildingDocument& document, const double deltaRotation01)
	{
		if (not m_selectedBuildingId)
		{
			return;
		}

		if (GeneratedBuilding* selected = document.findById(*m_selectedBuildingId))
		{
			selected->rotation01 += deltaRotation01;
			Sanitize(*selected);
		}
	}

	Optional<String> BuildingPlacementTool::selectedBuildingId() const
	{
		return m_selectedBuildingId;
	}

	void BuildingPlacementTool::setSelectedBuildingId(const Optional<String>& id)
	{
		m_selectedBuildingId = id;
	}

	Optional<GeneratedBuilding> BuildingPlacementTool::placementGhost() const
	{
		return m_placementGhost;
	}

	bool BuildingPlacementTool::isPlacing() const noexcept
	{
		return m_placementGhost.has_value();
	}

	Optional<Vec3> BuildingPlacementTool::cursorGroundPoint(const BasicCamera3D& camera) const
	{
		const Ray ray = camera.screenToRay(Cursor::PosF());
		const InfinitePlane groundPlane{ Float3{ 0, 0, 0 }, Float3{ 0, 1, 0 } };
		if (const auto distance = ray.intersects(groundPlane))
		{
			return ray.point_at(*distance);
		}
		return none;
	}

	Optional<String> BuildingPlacementTool::findBuildingAt(const BuildingDocument& document, const Vec3& point) const
	{
		for (auto it = document.buildings.rbegin(); it != document.buildings.rend(); ++it)
		{
			if (containsGroundPoint(*it, point))
			{
				return it->id;
			}
		}
		return none;
	}

	bool BuildingPlacementTool::containsGroundPoint(const GeneratedBuilding& building, const Vec3& point)
	{
		const Vec3 delta = point - building.origin;
		const double angle = -(building.rotation01 * Math::TwoPi);
		const double c = Math::Cos(angle);
		const double s = Math::Sin(angle);
		const Vec2 local{
			delta.x * c - delta.z * s,
			delta.x * s + delta.z * c
		};
		return (Abs(local.x) <= building.footprintSize.x * 0.5) && (Abs(local.y) <= building.footprintSize.y * 0.5);
	}

	Vec3 BuildingPlacementTool::snapPoint(Vec3 point)
	{
		point.x = Math::Round(point.x);
		point.z = Math::Round(point.z);
		return point;
	}

	uint32 BuildingPlacementTool::nextVariationSeed() const
	{
		const uint64 timeSeed = static_cast<uint64>(Time::GetMillisec());
		return static_cast<uint32>((timeSeed ^ (timeSeed >> 32)) * 0x9E3779B9u);
	}
}
