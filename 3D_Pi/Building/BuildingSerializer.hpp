# pragma once
# include <Siv3D.hpp>
# include <fstream>
# include "BuildingDocument.hpp"

namespace building
{
	namespace detail
	{
		[[nodiscard]] inline bool HasTomlTableArraySection(FilePathView path, const String& key)
		{
			std::ifstream reader{ Unicode::ToUTF8(FileSystem::FullPath(path)) };
			if (not reader)
			{
				return false;
			}

			const std::string sectionHeader = "[[" + Unicode::ToUTF8(key) + "]]";
			std::string line;
			while (std::getline(reader, line))
			{
				if ((3 <= line.size())
					&& (static_cast<unsigned char>(line[0]) == 0xEF)
					&& (static_cast<unsigned char>(line[1]) == 0xBB)
					&& (static_cast<unsigned char>(line[2]) == 0xBF))
				{
					line.erase(0, 3);
				}

				const size_t start = line.find_first_not_of(" \t");
				if (start == std::string::npos)
				{
					continue;
				}

				const size_t end = line.find_last_not_of(" \t");
				if (line.substr(start, end - start + 1) == sectionHeader)
				{
					return true;
				}
			}

			return false;
		}

		[[nodiscard]] inline String EscapeTomlString(const String& value)
		{
			String escaped;
			for (const auto ch : value)
			{
				if (ch == U'\\')
				{
					escaped += U"\\\\";
				}
				else if (ch == U'\"')
				{
					escaped += U"\\\"";
				}
				else
				{
					escaped += ch;
				}
			}
			return escaped;
		}

		[[nodiscard]] inline Vec3 ReadVec3(const TOMLValue& node, const Vec3& fallback)
		{
			Array<double> values;
			for (const auto& axis : node.arrayView())
			{
				if (const auto value = axis.getOpt<double>())
				{
					values << *value;
				}
			}
			if (values.size() >= 3)
			{
				return Vec3{ values[0], values[1], values[2] };
			}
			return fallback;
		}

		[[nodiscard]] inline Vec2 ReadVec2(const TOMLValue& node, const Vec2& fallback)
		{
			Array<double> values;
			for (const auto& axis : node.arrayView())
			{
				if (const auto value = axis.getOpt<double>())
				{
					values << *value;
				}
			}
			if (values.size() >= 2)
			{
				return Vec2{ values[0], values[1] };
			}
			return fallback;
		}

		[[nodiscard]] inline ColorF ReadColor(const TOMLValue& node, const ColorF& fallback)
		{
			Array<double> values;
			for (const auto& component : node.arrayView())
			{
				if (const auto value = component.getOpt<double>())
				{
					values << *value;
				}
			}
			if (values.size() >= 3)
			{
				return ColorF{ values[0], values[1], values[2], values.size() >= 4 ? values[3] : 1.0 };
			}
			return fallback;
		}
	}

	inline void SaveBuildingDocument(const FilePath& savePath, const BuildingDocument& document)
	{
		const FilePath directory = FileSystem::ParentPath(savePath);
		if (not directory.isEmpty())
		{
			FileSystem::CreateDirectories(directory);
		}

		TextWriter writer{ savePath };
		if (not writer)
		{
			return;
		}

		writer.writeln(U"nextSerial = {}"_fmt(document.nextSerial));
		writer.writeln(U"");

		for (const auto& building : document.buildings)
		{
			writer.writeln(U"[[buildings]]");
			writer.writeln(U"id = \"{}\""_fmt(detail::EscapeTomlString(building.id)));
			writer.writeln(U"name = \"{}\""_fmt(detail::EscapeTomlString(building.name)));
			writer.writeln(U"origin = [{:.3f}, {:.3f}, {:.3f}]"_fmt(building.origin.x, building.origin.y, building.origin.z));
			writer.writeln(U"rotation01 = {:.5f}"_fmt(building.rotation01));
			writer.writeln(U"footprintSize = [{:.3f}, {:.3f}]"_fmt(building.footprintSize.x, building.footprintSize.y));
			writer.writeln(U"floors = {}"_fmt(building.floors));
			writer.writeln(U"floorHeight = {:.4f}"_fmt(building.floorHeight));
			writer.writeln(U"wallThickness = {:.4f}"_fmt(building.wallThickness));
			writer.writeln(U"roofType = \"{}\""_fmt(ToString(building.roofType)));
			writer.writeln(U"roofHeight = {:.4f}"_fmt(building.roofHeight));
			writer.writeln(U"facadeStyle = \"{}\""_fmt(ToString(building.facadeStyle)));
			writer.writeln(U"variationSeed = {}"_fmt(building.variationSeed));
			writer.writeln(U"wallColor = [{:.4f}, {:.4f}, {:.4f}, {:.4f}]"_fmt(building.materialSet.wallColor.r, building.materialSet.wallColor.g, building.materialSet.wallColor.b, building.materialSet.wallColor.a));
			writer.writeln(U"roofColor = [{:.4f}, {:.4f}, {:.4f}, {:.4f}]"_fmt(building.materialSet.roofColor.r, building.materialSet.roofColor.g, building.materialSet.roofColor.b, building.materialSet.roofColor.a));
			writer.writeln(U"trimColor = [{:.4f}, {:.4f}, {:.4f}, {:.4f}]"_fmt(building.materialSet.trimColor.r, building.materialSet.trimColor.g, building.materialSet.trimColor.b, building.materialSet.trimColor.a));
			writer.writeln(U"wallMaterialKey = \"{}\""_fmt(detail::EscapeTomlString(building.materialSet.wallMaterialKey)));
			writer.writeln(U"roofMaterialKey = \"{}\""_fmt(detail::EscapeTomlString(building.materialSet.roofMaterialKey)));

			for (const auto& opening : building.openings)
			{
				writer.writeln(U"[[buildings.openings]]");
				writer.writeln(U"wallSide = \"{}\""_fmt(ToString(opening.wallSide)));
				writer.writeln(U"floorIndex = {}"_fmt(opening.floorIndex));
				writer.writeln(U"normalizedX = {:.4f}"_fmt(opening.normalizedX));
				writer.writeln(U"type = \"{}\""_fmt(ToString(opening.type)));
				writer.writeln(U"size = [{:.3f}, {:.3f}]"_fmt(opening.size.x, opening.size.y));
				writer.writeln(U"repeatCount = {}"_fmt(opening.repeatCount));
				writer.writeln(U"repeatSpacing = {:.4f}"_fmt(opening.repeatSpacing));
			}
			writer.writeln(U"");
		}
	}

	inline bool LoadBuildingDocument(const FilePath& savePath, BuildingDocument& document)
	{
		if (not FileSystem::Exists(savePath))
		{
			return false;
		}

		const TOMLReader toml{ savePath };
		if (not toml)
		{
			return false;
		}

		BuildingDocument loaded;
		loaded.nextSerial = static_cast<uint32>(toml[U"nextSerial"].getOr<int64>(1));

		if (detail::HasTomlTableArraySection(savePath, U"buildings"))
		{
			for (const auto& table : toml[U"buildings"].tableArrayView())
			{
				GeneratedBuilding building;
				building.id = table[U"id"].getOr<String>(U"building_{:03}"_fmt(loaded.nextSerial++));
				building.name = table[U"name"].getOr<String>(U"Building");
				building.origin = detail::ReadVec3(table[U"origin"], building.origin);
				building.rotation01 = table[U"rotation01"].getOr<double>(building.rotation01);
				building.footprintSize = detail::ReadVec2(table[U"footprintSize"], building.footprintSize);
				building.floors = table[U"floors"].getOr<int32>(building.floors);
				building.floorHeight = table[U"floorHeight"].getOr<double>(building.floorHeight);
				building.wallThickness = table[U"wallThickness"].getOr<double>(building.wallThickness);
				building.roofType = RoofTypeFromString(table[U"roofType"].getOr<String>(ToString(building.roofType)));
				building.roofHeight = table[U"roofHeight"].getOr<double>(building.roofHeight);
				building.facadeStyle = FacadeStyleFromString(table[U"facadeStyle"].getOr<String>(ToString(building.facadeStyle)));
				building.variationSeed = static_cast<uint32>(table[U"variationSeed"].getOr<int64>(building.variationSeed));
				building.materialSet.wallColor = detail::ReadColor(table[U"wallColor"], building.materialSet.wallColor);
				building.materialSet.roofColor = detail::ReadColor(table[U"roofColor"], building.materialSet.roofColor);
				building.materialSet.trimColor = detail::ReadColor(table[U"trimColor"], building.materialSet.trimColor);
				building.materialSet.wallMaterialKey = table[U"wallMaterialKey"].getOr<String>(building.materialSet.wallMaterialKey);
				building.materialSet.roofMaterialKey = table[U"roofMaterialKey"].getOr<String>(building.materialSet.roofMaterialKey);
				building.openings.clear();

				for (const auto& openingTable : table[U"openings"].tableArrayView())
				{
					BuildingOpening opening;
					opening.wallSide = WallSideFromString(openingTable[U"wallSide"].getOr<String>(ToString(opening.wallSide)));
					opening.floorIndex = openingTable[U"floorIndex"].getOr<int32>(opening.floorIndex);
					opening.normalizedX = openingTable[U"normalizedX"].getOr<double>(opening.normalizedX);
					opening.type = OpeningTypeFromString(openingTable[U"type"].getOr<String>(ToString(opening.type)));
					opening.size = detail::ReadVec2(openingTable[U"size"], opening.size);
					opening.repeatCount = openingTable[U"repeatCount"].getOr<int32>(opening.repeatCount);
					opening.repeatSpacing = openingTable[U"repeatSpacing"].getOr<double>(opening.repeatSpacing);
					building.openings << opening;
				}

				Sanitize(building);
				loaded.buildings << building;
			}
		}

		document = loaded;
		return true;
	}
}
