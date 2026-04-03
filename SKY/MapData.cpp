# include "MapData.hpp"

namespace
{
	[[nodiscard]] Vec3 ReadTomlVec3(const TOMLReader& toml, const String& key, const Vec3& fallback)
	{
		try
		{
			Array<double> values;
			values.reserve(3);

			for (const auto& value : toml[key].arrayView())
			{
				values << value.get<double>();

				if (values.size() >= 3)
				{
					break;
				}
			}

			if (values.size() < 3)
			{
				return fallback;
			}

			return Vec3{ values[0], values[1], values[2] };
		}
		catch (const std::exception&)
		{
			return fallback;
		}
	}

	void WriteTomlVec3(TextWriter& writer, const String& key, const Vec3& value)
	{
		writer.writeln(U"{} = [{:.3f}, {:.3f}, {:.3f}]"_fmt(key, value.x, value.y, value.z));
	}

	[[nodiscard]] Optional<PlaceableModelType> ParsePlaceableModelType(const String& value)
	{
		if (value == U"Mill")
		{
			return PlaceableModelType::Mill;
		}

		if (value == U"Tree")
		{
			return PlaceableModelType::Tree;
		}

		if (value == U"Pine")
		{
			return PlaceableModelType::Pine;
		}

		return none;
	}
}

MapData MakeDefaultMapData()
{
	MapData mapData;
	mapData.placedModels = {
		PlacedModel{ .type = PlaceableModelType::Mill, .position = Vec3{ -8, 0, 4 } },
		PlacedModel{ .type = PlaceableModelType::Tree, .position = Vec3{ 16, 0, 4 } },
		PlacedModel{ .type = PlaceableModelType::Pine, .position = Vec3{ 16, 0, 0 } },
	};
	return mapData;
}

MapData LoadMapData(FilePathView path)
{
	const TOMLReader toml{ path };

	if (not toml)
	{
		return MakeDefaultMapData();
	}

	MapData mapData;
	mapData.sapperRallyPoint = ReadTomlVec3(toml, U"sapperRallyPoint", mapData.sapperRallyPoint);
	mapData.placedModels.clear();

	try
	{
		for (const auto& objectTable : toml[U"objects"].tableArrayView())
		{
			const String typeString = objectTable[U"type"].get<String>();
			const Optional<PlaceableModelType> type = ParsePlaceableModelType(typeString);

			if (not type)
			{
				continue;
			}

			Array<double> positionValues;
			for (const auto& value : objectTable[U"position"].arrayView())
			{
				positionValues << value.get<double>();
			}

			if (positionValues.size() < 3)
			{
				continue;
			}

			mapData.placedModels << PlacedModel{
				.type = *type,
				.position = Vec3{ positionValues[0], positionValues[1], positionValues[2] },
			};
		}
	}
	catch (const std::exception&)
	{
		return MakeDefaultMapData();
	}

	return mapData;
}

bool SaveMapData(const MapData& mapData, FilePathView path)
{
	const FilePath directory = FileSystem::ParentPath(path);
	if (not directory.isEmpty())
	{
		FileSystem::CreateDirectories(directory);
	}

	TextWriter writer{ path };

	if (not writer)
	{
		return false;
	}

	WriteTomlVec3(writer, U"sapperRallyPoint", mapData.sapperRallyPoint);
	writer.writeln(U"");

	for (const auto& object : mapData.placedModels)
	{
		writer.writeln(U"[[objects]]");
		writer.writeln(U"type = \"{}\""_fmt(ToString(object.type)));
		WriteTomlVec3(writer, U"position", object.position);
		writer.writeln(U"");
	}

	return true;
}

StringView ToString(const PlaceableModelType type)
{
	switch (type)
	{
	case PlaceableModelType::Mill:
		return U"Mill";

	case PlaceableModelType::Tree:
		return U"Tree";

	case PlaceableModelType::Pine:
		return U"Pine";

	default:
		return U"Tree";
	}
}
