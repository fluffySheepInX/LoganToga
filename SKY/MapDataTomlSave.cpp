# include "MapDataTomlInternal.hpp"

using namespace MapDataTomlDetail;

namespace
{
	void WriteTomlColor(TextWriter& writer, const String& key, const Color& value)
	{
		writer.writeln(U"{} = [{}, {}, {}, {}]"_fmt(key, value.r, value.g, value.b, value.a));
	}
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

	for (const auto& terrainCell : mapData.terrainCells)
	{
		writer.writeln(U"[[terrainCells]]");
		writer.writeln(U"x = {}"_fmt(terrainCell.cell.x));
		writer.writeln(U"z = {}"_fmt(terrainCell.cell.y));
		writer.writeln(U"type = \"{}\""_fmt(ToString(terrainCell.type)));
		WriteTomlColor(writer, U"color", terrainCell.color);
		writer.writeln(U"");
	}

	for (const auto& navPoint : mapData.navPoints)
	{
		writer.writeln(U"[[navPoints]]");
		WriteTomlVec3(writer, U"position", navPoint.position);
		writer.writeln(U"radius = {:.3f}"_fmt(SanitizeNavPointRadius(navPoint.radius)));
		writer.writeln(U"");
	}

	for (const auto& navLink : mapData.navLinks)
	{
		if ((mapData.navPoints.size() <= navLink.fromIndex) || (mapData.navPoints.size() <= navLink.toIndex)
			|| (navLink.fromIndex == navLink.toIndex))
		{
			continue;
		}

		writer.writeln(U"[[navLinks]]");
		writer.writeln(U"from = {}"_fmt(navLink.fromIndex));
		writer.writeln(U"to = {}"_fmt(navLink.toIndex));
		writer.writeln(U"bidirectional = {}"_fmt(navLink.bidirectional ? U"true" : U"false"));
		writer.writeln(U"costMultiplier = {:.3f}"_fmt(SanitizeNavLinkCostMultiplier(navLink.costMultiplier)));
		writer.writeln(U"");
	}

	for (const auto& object : mapData.placedModels)
	{
		writer.writeln(U"[[objects]]");
		writer.writeln(U"type = \"{}\""_fmt(ToString(object.type)));
		WriteTomlVec3(writer, U"position", object.position);

        if (object.type == PlaceableModelType::Wall)
		{
			writer.writeln(U"yaw = {:.6f}"_fmt(object.yaw));
			writer.writeln(U"wallLength = {:.3f}"_fmt(SanitizeWallLength(object.wallLength)));
		}

		if (object.type == PlaceableModelType::Road)
		{
			writer.writeln(U"yaw = {:.6f}"_fmt(object.yaw));
			writer.writeln(U"roadLength = {:.3f}"_fmt(SanitizeRoadSpan(object.roadLength)));
			writer.writeln(U"roadWidth = {:.3f}"_fmt(SanitizeRoadSpan(object.roadWidth)));
		}

		if (object.type == PlaceableModelType::TireTrackDecal)
		{
			writer.writeln(U"yaw = {:.6f}"_fmt(object.yaw));
			writer.writeln(U"roadLength = {:.3f}"_fmt(SanitizeRoadSpan(object.roadLength)));
			writer.writeln(U"roadWidth = {:.3f}"_fmt(SanitizeRoadSpan(object.roadWidth)));
		}

		if (object.type == PlaceableModelType::Mill)
		{
			writer.writeln(U"attackRange = {:.3f}"_fmt(SanitizeMillAttackRange(object.attackRange)));
			writer.writeln(U"attackDamage = {:.3f}"_fmt(SanitizeMillAttackDamage(object.attackDamage)));
			writer.writeln(U"attackInterval = {:.3f}"_fmt(SanitizeMillAttackInterval(object.attackInterval)));
           writer.writeln(U"attackTargetCount = {}"_fmt(SanitizeMillAttackTargetCount(object.attackTargetCount)));
			writer.writeln(U"suppressionDuration = {:.3f}"_fmt(SanitizeMillSuppressionDuration(object.suppressionDuration)));
			writer.writeln(U"suppressionMoveSpeedMultiplier = {:.3f}"_fmt(SanitizeMillSuppressionMoveSpeedMultiplier(object.suppressionMoveSpeedMultiplier)));
			writer.writeln(U"suppressionAttackDamageMultiplier = {:.3f}"_fmt(SanitizeMillSuppressionAttackDamageMultiplier(object.suppressionAttackDamageMultiplier)));
			writer.writeln(U"suppressionAttackIntervalMultiplier = {:.3f}"_fmt(SanitizeMillSuppressionAttackIntervalMultiplier(object.suppressionAttackIntervalMultiplier)));
		}

		writer.writeln(U"");
	}

	return true;
}
