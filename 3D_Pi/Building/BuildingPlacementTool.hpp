# pragma once
# include <Siv3D.hpp>
# include "BuildingDocument.hpp"

namespace building
{
	class BuildingPlacementTool
	{
	public:
		void update(BuildingDocument& document, const BasicCamera3D& camera, bool cursorBlockedByUI);

		void beginPlacement(const Vec3& fallbackOrigin = Vec3{ 0, 0, 0 });
		void cancelTransientTool();
		void duplicateSelected(BuildingDocument& document);
		void deleteSelected(BuildingDocument& document);
		void rotateSelected(BuildingDocument& document, double deltaRotation01);

		[[nodiscard]] Optional<String> selectedBuildingId() const;
		void setSelectedBuildingId(const Optional<String>& id);
		[[nodiscard]] Optional<GeneratedBuilding> placementGhost() const;
		[[nodiscard]] bool isPlacing() const noexcept;

	private:
		[[nodiscard]] Optional<Vec3> cursorGroundPoint(const BasicCamera3D& camera) const;
		[[nodiscard]] Optional<String> findBuildingAt(const BuildingDocument& document, const Vec3& point) const;
		[[nodiscard]] static bool containsGroundPoint(const GeneratedBuilding& building, const Vec3& point);
		[[nodiscard]] static Vec3 snapPoint(Vec3 point);
		[[nodiscard]] uint32 nextVariationSeed() const;

		Optional<String> m_selectedBuildingId;
		Optional<GeneratedBuilding> m_placementGhost;
		bool m_draggingSelected = false;
		uint32 m_seedSerial = 1;
	};
}
