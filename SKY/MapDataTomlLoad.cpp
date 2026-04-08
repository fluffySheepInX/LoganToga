# include "MapDataTomlInternal.hpp"

using namespace MapDataTomlDetail;

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
						.suppressionDuration = SanitizeMillSuppressionDuration(objectTable[U"suppressionDuration"].getOpt<double>().value_or(MainSupport::MillSuppressionDuration)),
						.suppressionMoveSpeedMultiplier = SanitizeMillSuppressionMoveSpeedMultiplier(objectTable[U"suppressionMoveSpeedMultiplier"].getOpt<double>().value_or(MainSupport::MillSuppressionMoveSpeedMultiplier)),
						.suppressionAttackDamageMultiplier = SanitizeMillSuppressionAttackDamageMultiplier(objectTable[U"suppressionAttackDamageMultiplier"].getOpt<double>().value_or(MainSupport::MillSuppressionAttackDamageMultiplier)),
						.suppressionAttackIntervalMultiplier = SanitizeMillSuppressionAttackIntervalMultiplier(objectTable[U"suppressionAttackIntervalMultiplier"].getOpt<double>().value_or(MainSupport::MillSuppressionAttackIntervalMultiplier)),
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

	if (HasTomlTableArraySection(path, U"navPoints"))
	{
		mapData.navPoints.clear();
		size_t skippedNavPointCount = 0;

		try
		{
			for (const auto& navPointTable : toml[U"navPoints"].tableArrayView())
			{
				try
				{
					mapData.navPoints << NavPoint{
						.position = ReadTomlVec3(navPointTable, U"position", Vec3{ 0, 0, 0 }),
						.radius = SanitizeNavPointRadius(navPointTable[U"radius"].getOpt<double>().value_or(1.4)),
					};
				}
				catch (const std::exception&)
				{
					++skippedNavPointCount;
				}
			}
		}
		catch (const std::exception&)
		{
			AppendLoadMessage(result.message, U"navPoints の読込に失敗したため既定経路点を使用");
		}

		if (0 < skippedNavPointCount)
		{
			AppendLoadMessage(result.message, U"navPoints の一部を読み飛ばし");
		}
	}

	if (HasTomlTableArraySection(path, U"navLinks"))
	{
		mapData.navLinks.clear();
		size_t skippedNavLinkCount = 0;

		try
		{
			for (const auto& navLinkTable : toml[U"navLinks"].tableArrayView())
			{
				try
				{
					const Optional<int64> fromIndexValue = navLinkTable[U"from"].getOpt<int64>();
					const Optional<int64> toIndexValue = navLinkTable[U"to"].getOpt<int64>();

					if ((not fromIndexValue) || (not toIndexValue)
						|| (*fromIndexValue < 0) || (*toIndexValue < 0)
						|| (*fromIndexValue == *toIndexValue))
					{
						++skippedNavLinkCount;
						continue;
					}

					const size_t fromIndex = static_cast<size_t>(*fromIndexValue);
					const size_t toIndex = static_cast<size_t>(*toIndexValue);
					if ((mapData.navPoints.size() <= fromIndex) || (mapData.navPoints.size() <= toIndex))
					{
						++skippedNavLinkCount;
						continue;
					}

					mapData.navLinks << NavLink{
						.fromIndex = fromIndex,
						.toIndex = toIndex,
						.bidirectional = navLinkTable[U"bidirectional"].getOpt<bool>().value_or(true),
						.costMultiplier = SanitizeNavLinkCostMultiplier(navLinkTable[U"costMultiplier"].getOpt<double>().value_or(1.0)),
					};
				}
				catch (const std::exception&)
				{
					++skippedNavLinkCount;
				}
			}
		}
		catch (const std::exception&)
		{
			AppendLoadMessage(result.message, U"navLinks の読込に失敗したため既定接続を使用");
		}

		if (0 < skippedNavLinkCount)
		{
			AppendLoadMessage(result.message, U"navLinks の一部を読み飛ばし");
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
