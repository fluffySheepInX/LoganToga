# pragma once
# include <Siv3D.hpp>
# include "BuildingDocument.hpp"

namespace building
{
	class BuildingRenderer
	{
	public:
		void draw(const BuildingDocument& document, const Optional<String>& selectedBuildingId, const Optional<GeneratedBuilding>& placementGhost = none) const;

	private:
		void drawBuilding(const GeneratedBuilding& building, bool selected, bool ghost) const;
		void drawOpenings(const GeneratedBuilding& building, double wallHeight, bool ghost) const;
		void drawOpening(const GeneratedBuilding& building, const BuildingOpening& opening, int32 repeatIndex, double wallHeight, bool ghost) const;
		void drawSelectionBounds(const GeneratedBuilding& building) const;
	};
}
