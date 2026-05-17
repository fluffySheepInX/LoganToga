#pragma once
# include <Siv3D.hpp>
# include "DefinitionStores.h"

namespace LT3
{
	struct BuildLineIconOverride
	{
		String actionTag;
		String iconHorizontal;
		String iconDiagUpRight;
		String iconDiagUpLeft;
	};

	inline FilePath ResolveBuildLineIconOverridesTomlPath()
	{
		const FilePath fromApp = U"000_Warehouse/000_DefaultGame/070_Scenario/InfoBuildMenu/BuildLineIcons.toml";
		if (FileSystem::Exists(fromApp))
		{
			return fromApp;
		}

		const FilePath fromRepo = U"App/000_Warehouse/000_DefaultGame/070_Scenario/InfoBuildMenu/BuildLineIcons.toml";
		if (FileSystem::Exists(fromRepo))
		{
			return fromRepo;
		}

		return fromApp;
	}

	inline HashTable<String, BuildLineIconOverride> LoadBuildLineIconOverrideMap()
	{
		HashTable<String, BuildLineIconOverride> result;
		const FilePath path = ResolveBuildLineIconOverridesTomlPath();
		const TOMLReader toml{ path };
		if (!toml)
		{
			return result;
		}

		for (const auto overrideValue : toml[U"overrides"].tableArrayView())
		{
			const String actionTag = overrideValue[U"action_tag"].getOr<String>(U"");
			if (actionTag.isEmpty())
			{
				continue;
			}

			result[actionTag] = BuildLineIconOverride{
				actionTag,
				overrideValue[U"icon_horizontal"].getOr<String>(U""),
				overrideValue[U"icon_diag_up_right"].getOr<String>(U""),
				overrideValue[U"icon_diag_up_left"].getOr<String>(U"")
			};
		}

		return result;
	}

	inline void ApplyBuildLineIconOverrides(Array<BuildActionDef>& actions)
	{
		const HashTable<String, BuildLineIconOverride> overrides = LoadBuildLineIconOverrideMap();
		if (overrides.empty())
		{
			return;
		}

		for (auto& action : actions)
		{
			if (!overrides.contains(action.tag))
			{
				continue;
			}

			const BuildLineIconOverride& overrideValue = overrides.at(action.tag);
			if (!overrideValue.iconHorizontal.isEmpty())
			{
				action.lineIconHorizontal = overrideValue.iconHorizontal;
			}
			if (!overrideValue.iconDiagUpRight.isEmpty())
			{
				action.lineIconDiagUpRight = overrideValue.iconDiagUpRight;
			}
			if (!overrideValue.iconDiagUpLeft.isEmpty())
			{
				action.lineIconDiagUpLeft = overrideValue.iconDiagUpLeft;
			}
		}
	}

	inline bool SaveBuildLineIconOverride(const String& actionTag, const String& iconHorizontal, const String& iconDiagUpRight, const String& iconDiagUpLeft, String& statusText)
	{
		if (actionTag.isEmpty())
		{
			statusText = U"Build line icon save failed: action tag is empty";
			return false;
		}

		HashTable<String, BuildLineIconOverride> overrides = LoadBuildLineIconOverrideMap();
		overrides[actionTag] = BuildLineIconOverride{ actionTag, iconHorizontal, iconDiagUpRight, iconDiagUpLeft };

		const FilePath path = ResolveBuildLineIconOverridesTomlPath();
		FileSystem::CreateDirectories(FileSystem::ParentPath(path));
		TextWriter writer{ path, OpenMode::Trunc, TextEncoding::UTF8_NO_BOM };
		if (!writer)
		{
			statusText = U"Build line icon save failed: {}"_fmt(path);
			return false;
		}

		Array<String> tags;
		tags.reserve(overrides.size());
		for (const auto& [tag, _] : overrides)
		{
			tags << tag;
		}
		tags.sort();

		for (const auto& tag : tags)
		{
			const BuildLineIconOverride& value = overrides.at(tag);
			writer << U"[[overrides]]\n";
			writer << U"action_tag = \"" << value.actionTag << U"\"\n";
			writer << U"icon_horizontal = \"" << value.iconHorizontal << U"\"\n";
			writer << U"icon_diag_up_right = \"" << value.iconDiagUpRight << U"\"\n";
			writer << U"icon_diag_up_left = \"" << value.iconDiagUpLeft << U"\"\n\n";
		}

		statusText = U"Saved build line icon overrides: {}"_fmt(path);
		return true;
	}
}
