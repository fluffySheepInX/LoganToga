# pragma once
# include <Siv3D.hpp>

namespace building
{
	enum class BuildingRoofType
	{
		Flat,
		Gable,
		Hip,
	};

	enum class BuildingFacadeStyle
	{
		Simple,
		OldTown,
		Warehouse,
		Haunted,
	};

	enum class BuildingWallSide
	{
		Front,
		Back,
		Left,
		Right,
	};

	enum class BuildingOpeningType
	{
		Window,
		Door,
	};

	struct BuildingMaterialSet
	{
		ColorF wallColor{ 0.72, 0.68, 0.60, 1.0 };
		ColorF roofColor{ 0.28, 0.12, 0.10, 1.0 };
		ColorF trimColor{ 0.86, 0.82, 0.72, 1.0 };
		String wallMaterialKey = U"plaster_old";
		String roofMaterialKey = U"roof_dark";
	};

	struct BuildingOpening
	{
		BuildingWallSide wallSide = BuildingWallSide::Front;
		int32 floorIndex = 0;
		double normalizedX = 0.5;
		BuildingOpeningType type = BuildingOpeningType::Window;
		Vec2 size{ 0.8, 1.0 };
		int32 repeatCount = 1;
		double repeatSpacing = 1.6;
	};

	struct GeneratedBuilding
	{
		String id = U"building_001";
		String name = U"Building";
		Vec3 origin{ 0, 0, 0 };
		double rotation01 = 0.0;
		Vec2 footprintSize{ 6.0, 8.0 };
		int32 floors = 2;
		double floorHeight = 3.0;
		double wallThickness = 0.18;
		BuildingRoofType roofType = BuildingRoofType::Gable;
		double roofHeight = 1.4;
		BuildingFacadeStyle facadeStyle = BuildingFacadeStyle::OldTown;
		BuildingMaterialSet materialSet;
		Array<BuildingOpening> openings;
		uint32 variationSeed = 1;
	};

	[[nodiscard]] inline String ToString(const BuildingRoofType type)
	{
		switch (type)
		{
		case BuildingRoofType::Flat:
			return U"Flat";
		case BuildingRoofType::Hip:
			return U"Hip";
		case BuildingRoofType::Gable:
		default:
			return U"Gable";
		}
	}

	[[nodiscard]] inline BuildingRoofType RoofTypeFromString(const String& value)
	{
		if (value == U"Flat")
		{
			return BuildingRoofType::Flat;
		}
		if (value == U"Hip")
		{
			return BuildingRoofType::Hip;
		}
		return BuildingRoofType::Gable;
	}

	[[nodiscard]] inline String ToString(const BuildingFacadeStyle style)
	{
		switch (style)
		{
		case BuildingFacadeStyle::Simple:
			return U"Simple";
		case BuildingFacadeStyle::Warehouse:
			return U"Warehouse";
		case BuildingFacadeStyle::Haunted:
			return U"Haunted";
		case BuildingFacadeStyle::OldTown:
		default:
			return U"OldTown";
		}
	}

	[[nodiscard]] inline BuildingFacadeStyle FacadeStyleFromString(const String& value)
	{
		if (value == U"Simple")
		{
			return BuildingFacadeStyle::Simple;
		}
		if (value == U"Warehouse")
		{
			return BuildingFacadeStyle::Warehouse;
		}
		if (value == U"Haunted")
		{
			return BuildingFacadeStyle::Haunted;
		}
		return BuildingFacadeStyle::OldTown;
	}

	[[nodiscard]] inline String ToString(const BuildingWallSide side)
	{
		switch (side)
		{
		case BuildingWallSide::Back:
			return U"Back";
		case BuildingWallSide::Left:
			return U"Left";
		case BuildingWallSide::Right:
			return U"Right";
		case BuildingWallSide::Front:
		default:
			return U"Front";
		}
	}

	[[nodiscard]] inline BuildingWallSide WallSideFromString(const String& value)
	{
		if (value == U"Back")
		{
			return BuildingWallSide::Back;
		}
		if (value == U"Left")
		{
			return BuildingWallSide::Left;
		}
		if (value == U"Right")
		{
			return BuildingWallSide::Right;
		}
		return BuildingWallSide::Front;
	}

	[[nodiscard]] inline String ToString(const BuildingOpeningType type)
	{
		return (type == BuildingOpeningType::Door) ? U"Door" : U"Window";
	}

	[[nodiscard]] inline BuildingOpeningType OpeningTypeFromString(const String& value)
	{
		return (value == U"Door") ? BuildingOpeningType::Door : BuildingOpeningType::Window;
	}

	inline void Sanitize(GeneratedBuilding& building)
	{
		building.footprintSize.x = Clamp(building.footprintSize.x, 1.0, 80.0);
		building.footprintSize.y = Clamp(building.footprintSize.y, 1.0, 80.0);
		building.floors = Clamp(building.floors, 1, 12);
		building.floorHeight = Clamp(building.floorHeight, 1.5, 8.0);
		building.wallThickness = Clamp(building.wallThickness, 0.02, 1.0);
		building.roofHeight = Clamp(building.roofHeight, 0.0, 8.0);
		building.rotation01 = Math::Fmod(building.rotation01, 1.0);
		if (building.rotation01 < 0.0)
		{
			building.rotation01 += 1.0;
		}
		building.materialSet.wallColor.a = 1.0;
		building.materialSet.roofColor.a = 1.0;
		building.materialSet.trimColor.a = 1.0;
	}

	[[nodiscard]] inline GeneratedBuilding MakeDefaultBuilding(const Vec3& origin, const uint32 serial)
	{
		GeneratedBuilding building;
		building.id = U"building_{:03}"_fmt(serial);
		building.name = U"Building {}"_fmt(serial);
		building.origin = origin;
		building.variationSeed = serial * 0x9E3779B9u;
		building.openings = {
			BuildingOpening{ .wallSide = BuildingWallSide::Front, .floorIndex = 0, .normalizedX = 0.5, .type = BuildingOpeningType::Door, .size = Vec2{ 1.0, 2.2 }, .repeatCount = 1, .repeatSpacing = 0.0 },
			BuildingOpening{ .wallSide = BuildingWallSide::Front, .floorIndex = 1, .normalizedX = 0.25, .type = BuildingOpeningType::Window, .size = Vec2{ 0.8, 1.0 }, .repeatCount = 3, .repeatSpacing = 1.6 },
		};
		return building;
	}
}
