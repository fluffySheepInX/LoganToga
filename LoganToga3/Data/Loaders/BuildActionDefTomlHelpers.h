#pragma once
# include <Siv3D.hpp>
# include "../DefinitionStores.h"

namespace LT3
{
		namespace BuildActionToml
	{
		inline constexpr auto KeyCommands = U"commands";
		inline constexpr auto KeyOwnerTag = U"owner_tag";
		inline constexpr auto KeyOwnerTags = U"owner_tags";
		inline constexpr auto KeyId = U"id";
		inline constexpr auto KeyResult = U"result";
		inline constexpr auto KeyType = U"type";
		inline constexpr auto KeySpawn = U"spawn";
		inline constexpr auto KeySpawns = U"spawns";
		inline constexpr auto KeyName = U"name";
		inline constexpr auto KeyDescription = U"description";
		inline constexpr auto KeyIcon = U"icon";
		inline constexpr auto KeyIcons = U"icons";
		inline constexpr auto KeyIconHorizontal = U"icon_horizontal";
		inline constexpr auto KeyIconDiagUpRight = U"icon_diag_up_right";
		inline constexpr auto KeyIconDiagUpLeft = U"icon_diag_up_left";
		inline constexpr auto KeyCategory = U"category";
		inline constexpr auto KeyRequires = U"requires";
		inline constexpr auto KeyCreateCount = U"create_count";
		inline constexpr auto KeyCost = U"cost";
		inline constexpr auto KeyWood = U"wood";
		inline constexpr auto KeyGold = U"gold";
		inline constexpr auto KeyTrust = U"trust";
		inline constexpr auto KeyFood = U"food";
		inline constexpr auto KeyBuildTime = U"build_time";
		inline constexpr auto KeyIsMove = U"is_move";
		inline constexpr auto KeyPlacementMode = U"placement_mode";
		inline constexpr auto KeyLineAxisMode = U"line_axis_mode";
		inline constexpr auto KeyLineThicknessCells = U"line_thickness_cells";
		inline constexpr auto KeyMaxLineCells = U"max_line_cells";
		inline constexpr auto KeyUseRightDragPlacement = U"use_right_drag_placement";
	}

	inline bool EqualsIgnoreCaseOwnerTag(const String& a, const String& b)
	{
		return !a.isEmpty() && !b.isEmpty() && a.lowercased() == b.lowercased();
	}

	inline Array<String> NormalizeOwnerTags(const Array<String>& source)
	{
		Array<String> normalized;
		for (const auto& tag : source)
		{
			if (tag.isEmpty())
			{
				continue;
			}

			bool alreadyExists = false;
			for (const auto& existing : normalized)
			{
				if (EqualsIgnoreCaseOwnerTag(existing, tag))
				{
					alreadyExists = true;
					break;
				}
			}

			if (!alreadyExists)
			{
				normalized << tag;
			}
		}

		return normalized;
	}

	inline String BuildActionTomlEscape(StringView text)
	{
		String result;
		for (const char32 ch : text)
		{
			if (ch == U'\\')
			{
				result += U"\\\\";
			}
			else if (ch == U'\"')
			{
				result += U"\\\"";
			}
			else
			{
				result += ch;
			}
		}

		return result;
	}

	inline void WriteTomlStringArrayValue(TextWriter& writer, const Array<String>& values)
	{
		writer << U"[";
		for (size_t i = 0; i < values.size(); ++i)
		{
			if (i > 0)
			{
				writer << U", ";
			}
			writer << U"\"" << BuildActionTomlEscape(values[i]) << U"\"";
		}
		writer << U"]";
	}

	inline String BuildTomlStringArrayValue(const Array<String>& values)
	{
		String result = U"[";
		for (size_t i = 0; i < values.size(); ++i)
		{
			if (i > 0)
			{
				result += U", ";
			}

			result += U"\"" + BuildActionTomlEscape(values[i]) + U"\"";
		}

		result += U"]";
		return result;
	}

	inline Array<String> ReadTomlStringArrayValue(const TOMLValue& value)
	{
		Array<String> result;
		if (!value.isArray())
		{
			return result;
		}

		for (const auto item : value.arrayView())
		{
			if (const Optional<String> text = item.getOpt<String>())
			{
				result << *text;
			}
		}

		return result;
	}

	inline Array<String> ReadOwnerTags(const TOMLValue& ownerTagsValue, const String& fallbackOwnerTag)
	{
		Array<String> ownerTags = NormalizeOwnerTags(ReadTomlStringArrayValue(ownerTagsValue));
		if (ownerTags.isEmpty() && !fallbackOwnerTag.isEmpty())
		{
			ownerTags << fallbackOwnerTag;
		}

		return NormalizeOwnerTags(ownerTags);
	}

	inline Array<String> NormalizeSpawnTags(const Array<String>& source)
	{
		return NormalizeOwnerTags(source);
	}

	inline Array<String> ReadSpawnTags(const TOMLValue& spawnTagsValue, const String& fallbackSpawnTag)
	{
		Array<String> spawnTags = NormalizeSpawnTags(ReadTomlStringArrayValue(spawnTagsValue));
		if (spawnTags.isEmpty() && !fallbackSpawnTag.isEmpty())
		{
			spawnTags << fallbackSpawnTag;
		}

		return NormalizeSpawnTags(spawnTags);
	}

	inline Array<String> NormalizeIconLayers(const Array<String>& source)
	{
		Array<String> normalized;
		for (const auto& icon : source)
		{
			if (icon.isEmpty())
			{
				continue;
			}

			normalized << icon;
		}

		return normalized;
	}

	inline Array<String> ReadIconLayers(const TOMLValue& iconsValue, const String& fallbackIcon)
	{
		Array<String> iconLayers = NormalizeIconLayers(ReadTomlStringArrayValue(iconsValue));
		if (iconLayers.isEmpty() && !fallbackIcon.isEmpty())
		{
			iconLayers << fallbackIcon;
		}

		return NormalizeIconLayers(iconLayers);
	}

	inline BuildActionResultType ParseBuildActionResultType(const String& value)
	{
		const String lowered = value.lowercased();
		if (lowered == U"unit")
		{
			return BuildActionResultType::Unit;
		}
		if (lowered == U"obj")
		{
			return BuildActionResultType::Object;
		}
		if (lowered == U"carrier")
		{
			return BuildActionResultType::Carrier;
		}

		return BuildActionResultType::None;
	}

	inline BuildPlacementMode ParseBuildPlacementMode(const String& value)
	{
		const String lowered = value.lowercased();
		if (lowered == U"line")
		{
			return BuildPlacementMode::Line;
		}

		return BuildPlacementMode::Point;
	}

	inline BuildLineAxisMode ParseBuildLineAxisMode(const String& value)
	{
		const String lowered = value.lowercased();
		if (lowered == U"horizontal" || lowered == U"horizontal_only")
		{
			return BuildLineAxisMode::HorizontalOnly;
		}
		if (lowered == U"vertical" || lowered == U"vertical_only")
		{
			return BuildLineAxisMode::VerticalOnly;
		}

		return BuildLineAxisMode::Auto;
	}

	inline bool TryReadBuildActionResult(const TOMLValue& resultValue, BuildActionResultType& resultType, String& resultTag, Array<String>& resultTags)
	{
		resultType = BuildActionResultType::None;
		resultTag.clear();
		resultTags.clear();

		for (const auto resultTable : resultValue.tableArrayView())
		{
			const BuildActionResultType parsedType = ParseBuildActionResultType(resultTable[BuildActionToml::KeyType].getOr<String>(U""));
			if (parsedType != BuildActionResultType::None)
			{
				resultTags = ReadSpawnTags(resultTable[BuildActionToml::KeySpawns], resultTable[BuildActionToml::KeySpawn].getOr<String>(U""));
				resultTag = resultTags.isEmpty() ? U"" : resultTags.front();
				resultType = parsedType;
				return true;
			}
		}

		if (resultValue.isTable())
		{
			const BuildActionResultType parsedType = ParseBuildActionResultType(resultValue[BuildActionToml::KeyType].getOr<String>(U""));
			if (parsedType != BuildActionResultType::None)
			{
				resultTags = ReadSpawnTags(resultValue[BuildActionToml::KeySpawns], resultValue[BuildActionToml::KeySpawn].getOr<String>(U""));
				resultTag = resultTags.isEmpty() ? U"" : resultTags.front();
				resultType = parsedType;
				return true;
			}
		}

		return false;
	}

	inline FilePath ResolveBuildActionTomlPath()
	{
		const FilePath fromApp = U"000_Warehouse/000_DefaultGame/070_Scenario/InfoBuildMenu/BuildMenu.toml";
		if (FileSystem::Exists(fromApp))
		{
			return fromApp;
		}

		const FilePath fromRepo = U"App/000_Warehouse/000_DefaultGame/070_Scenario/InfoBuildMenu/BuildMenu.toml";
		if (FileSystem::Exists(fromRepo))
		{
			return fromRepo;
		}

		return fromApp;
	}

	inline String BuildActionResultTypeToTomlValue(BuildActionResultType type)
	{
		switch (type)
		{
		case BuildActionResultType::Unit:
			return U"unit";
		case BuildActionResultType::Object:
			return U"obj";
		case BuildActionResultType::Carrier:
			return U"Carrier";
		default:
			return U"none";
		}
	}

	inline String BuildPlacementModeToTomlValue(BuildPlacementMode mode)
	{
		return (mode == BuildPlacementMode::Line) ? U"line" : U"point";
	}

	inline String BuildLineAxisModeToTomlValue(BuildLineAxisMode mode)
	{
		switch (mode)
		{
		case BuildLineAxisMode::HorizontalOnly:
			return U"horizontal_only";
		case BuildLineAxisMode::VerticalOnly:
			return U"vertical_only";
		default:
			return U"auto";
		}
	}
}
