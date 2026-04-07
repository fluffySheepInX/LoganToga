# include "MapData.hpp"
# include <fstream>

namespace
{
    [[nodiscard]] double SanitizeMillAttackRange(const double value)
	{
		return Clamp(value, 1.0, 20.0);
	}

	[[nodiscard]] double SanitizeMillAttackDamage(const double value)
	{
		return Clamp(value, 1.0, 80.0);
	}

	[[nodiscard]] double SanitizeMillAttackInterval(const double value)
	{
		return Clamp(value, 0.2, 5.0);
	}

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

	[[nodiscard]] Vec3 ReadTomlVec3(const TOMLValue& tomlValue, const String& key, const Vec3& fallback)
	{
		try
		{
			Array<double> values;
			values.reserve(3);

			for (const auto& value : tomlValue[key].arrayView())
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

	void AppendLoadMessage(String& message, StringView detail)
	{
		if (message.isEmpty())
		{
			message = detail;
			return;
		}

		message += U" / ";
		message += detail;
	}

	[[nodiscard]] bool HasTomlTableArraySection(FilePathView path, const String& key)
	{
		std::ifstream reader{ Unicode::ToUTF8(FileSystem::FullPath(path)) };

		if (not reader)
		{
			return false;
		}

		const std::string sectionHeader = ("[[" + Unicode::ToUTF8(key) + "]]" );
		std::string line;

		while (std::getline(reader, line))
		{
			if (not line.empty() && static_cast<unsigned char>(line.front()) == 0xEF)
			{
				if ((3 <= line.size())
					&& (static_cast<unsigned char>(line[0]) == 0xEF)
					&& (static_cast<unsigned char>(line[1]) == 0xBB)
					&& (static_cast<unsigned char>(line[2]) == 0xBF))
				{
					line.erase(0, 3);
				}
			}

			const size_t start = line.find_first_not_of(" \t");
			if (start == std::string::npos)
			{
				continue;
			}

			const size_t end = line.find_last_not_of(" \t");
			if (line.substr(start, (end - start + 1)) == sectionHeader)
			{
				return true;
			}
		}

		return false;
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

	[[nodiscard]] Optional<MainSupport::ResourceType> ParseResourceType(const String& value)
	{
		if (value == U"Budget")
		{
			return MainSupport::ResourceType::Budget;
		}

		if (value == U"Gunpowder")
		{
			return MainSupport::ResourceType::Gunpowder;
		}

		if (value == U"Mana")
		{
			return MainSupport::ResourceType::Mana;
		}

		return none;
	}
}

MapDataLoadResult LoadMapDataWithStatus(FilePathView path)
{
	MapDataLoadResult result;
	result.mapData = MakeDefaultMapData();
	const TOMLReader toml{ path };

	if (not toml)
	{
		result.message = U"map_data.toml の読込に失敗したため既定値を使用";
		return result;
	}

	MapData mapData = result.mapData;

	mapData.playerBasePosition = ReadTomlVec3(toml, U"playerBasePosition", mapData.playerBasePosition);
	mapData.enemyBasePosition = ReadTomlVec3(toml, U"enemyBasePosition", mapData.enemyBasePosition);
	mapData.sapperRallyPoint = ReadTomlVec3(toml, U"sapperRallyPoint", mapData.sapperRallyPoint);

	Array<ResourceArea> loadedResourceAreas;
	size_t skippedResourceAreaCount = 0;

	if (HasTomlTableArraySection(path, U"resourceAreas"))
	{
		try
		{
			for (const auto& resourceAreaTable : toml[U"resourceAreas"].tableArrayView())
			{
				try
				{
					const String typeString = resourceAreaTable[U"type"].get<String>();
					const Optional<MainSupport::ResourceType> type = ParseResourceType(typeString);

					if (not type)
					{
						++skippedResourceAreaCount;
						continue;
					}

					double radius = MainSupport::ResourceAreaDefaultRadius;
					if (const auto radiusValue = resourceAreaTable[U"radius"].getOpt<double>())
					{
						radius = Max(1.0, *radiusValue);
					}

					loadedResourceAreas << ResourceArea{
						.type = *type,
						.position = ReadTomlVec3(resourceAreaTable, U"position", Vec3{ 0, 0, 0 }),
						.radius = radius,
					};
				}
				catch (const std::exception&)
				{
					++skippedResourceAreaCount;
				}
			}
		}
		catch (const std::exception&)
		{
			AppendLoadMessage(result.message, U"resourceAreas の読込に失敗したため既定エリアを使用");
		}
	}

	if (HasTomlTableArraySection(path, U"objects"))
	{
		mapData.placedModels.clear();
		size_t skippedObjectCount = 0;

		try
		{
			for (const auto& objectTable : toml[U"objects"].tableArrayView())
			{
				try
				{
					const String typeString = objectTable[U"type"].get<String>();
					const Optional<PlaceableModelType> type = ParsePlaceableModelType(typeString);

					if (not type)
					{
						++skippedObjectCount;
						continue;
					}

					Array<double> positionValues;
					for (const auto& value : objectTable[U"position"].arrayView())
					{
						positionValues << value.get<double>();
					}

					if (positionValues.size() < 3)
					{
						++skippedObjectCount;
						continue;
					}

					mapData.placedModels << PlacedModel{
						.type = *type,
						.position = Vec3{ positionValues[0], positionValues[1], positionValues[2] },
                      .attackRange = SanitizeMillAttackRange(objectTable[U"attackRange"].getOpt<double>().value_or(MainSupport::MillDefenseRange)),
						.attackDamage = SanitizeMillAttackDamage(objectTable[U"attackDamage"].getOpt<double>().value_or(MainSupport::MillDefenseDamage)),
						.attackInterval = SanitizeMillAttackInterval(objectTable[U"attackInterval"].getOpt<double>().value_or(MainSupport::MillDefenseInterval)),
					};
				}
				catch (const std::exception&)
				{
					++skippedObjectCount;
				}
			}
		}
		catch (const std::exception&)
		{
			AppendLoadMessage(result.message, U"objects の読込に失敗したため拠点座標のみ反映");
		}

		if (0 < skippedObjectCount)
		{
			AppendLoadMessage(result.message, U"objects の一部を読み飛ばし");
		}
	}

	if (not loadedResourceAreas.isEmpty())
	{
		mapData.resourceAreas = loadedResourceAreas;
	}

	if (0 < skippedResourceAreaCount)
	{
		AppendLoadMessage(result.message, U"resourceAreas の一部を読み飛ばし");
	}

	result.mapData = mapData;
	return result;
}

MapData LoadMapData(FilePathView path)
{
	return LoadMapDataWithStatus(path).mapData;
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

	WriteTomlVec3(writer, U"playerBasePosition", mapData.playerBasePosition);
	WriteTomlVec3(writer, U"enemyBasePosition", mapData.enemyBasePosition);
	WriteTomlVec3(writer, U"sapperRallyPoint", mapData.sapperRallyPoint);
	writer.writeln(U"");

	for (const auto& resourceArea : mapData.resourceAreas)
	{
		writer.writeln(U"[[resourceAreas]]");
		writer.writeln(U"type = \"{}\""_fmt(ToString(resourceArea.type)));
		WriteTomlVec3(writer, U"position", resourceArea.position);
		writer.writeln(U"radius = {:.3f}"_fmt(resourceArea.radius));
		writer.writeln(U"");
	}

	for (const auto& object : mapData.placedModels)
	{
		writer.writeln(U"[[objects]]");
		writer.writeln(U"type = \"{}\""_fmt(ToString(object.type)));
		WriteTomlVec3(writer, U"position", object.position);

		if (object.type == PlaceableModelType::Mill)
		{
			writer.writeln(U"attackRange = {:.3f}"_fmt(SanitizeMillAttackRange(object.attackRange)));
			writer.writeln(U"attackDamage = {:.3f}"_fmt(SanitizeMillAttackDamage(object.attackDamage)));
			writer.writeln(U"attackInterval = {:.3f}"_fmt(SanitizeMillAttackInterval(object.attackInterval)));
		}

		writer.writeln(U"");
	}

	return true;
}
