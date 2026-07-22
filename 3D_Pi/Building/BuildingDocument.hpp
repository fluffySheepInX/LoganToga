# pragma once
# include <Siv3D.hpp>
# include "BuildingTypes.hpp"

namespace building
{
	struct BuildingDocument
	{
		Array<GeneratedBuilding> buildings;
		uint32 nextSerial = 1;

		[[nodiscard]] GeneratedBuilding* findById(const String& id)
		{
			for (auto& building : buildings)
			{
				if (building.id == id)
				{
					return &building;
				}
			}
			return nullptr;
		}

		[[nodiscard]] const GeneratedBuilding* findById(const String& id) const
		{
			for (const auto& building : buildings)
			{
				if (building.id == id)
				{
					return &building;
				}
			}
			return nullptr;
		}

		[[nodiscard]] Optional<size_t> indexOf(const String& id) const
		{
			for (size_t i = 0; i < buildings.size(); ++i)
			{
				if (buildings[i].id == id)
				{
					return i;
				}
			}
			return none;
		}

		[[nodiscard]] String makeNextId()
		{
			return U"building_{:03}"_fmt(nextSerial++);
		}
	};
}
