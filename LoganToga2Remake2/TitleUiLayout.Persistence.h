#pragma once

#include "GameSettings.h"
#include "TitleUiLayout.Serialization.h"

namespace TitleUi
{
	[[nodiscard]] inline String GetLocalLayoutPath()
	{
		return U"save/title_ui_layout.toml";
	}

	[[nodiscard]] inline String GetAppDataLayoutPath()
	{
		return FileSystem::PathAppend(GameSettings::GetSettingsDirectoryPath(), U"save/title_ui_layout.toml");
	}

	[[nodiscard]] inline String GetLayoutPathForLocation(const ContinueRunSaveLocation location)
	{
		switch (location)
		{
		case ContinueRunSaveLocation::AppData:
			return GetAppDataLayoutPath();
		case ContinueRunSaveLocation::Local:
		default:
			return GetLocalLayoutPath();
		}
	}

	[[nodiscard]] inline String GetLayoutPath()
	{
		return GetLayoutPathForLocation(GetContinueRunSaveLocation());
	}

	[[nodiscard]] inline TitleUiLayout LoadTitleUiLayoutFromDisk()
	{
		const TOMLReader toml{ GetLayoutPath() };
		if (!toml)
		{
			return MakeDefaultTitleUiLayout();
		}

		return LoadTitleUiLayoutFromToml(toml);
	}

	[[nodiscard]] inline TitleUiLayout& GetTitleUiLayoutStorage()
	{
		static TitleUiLayout layout = LoadTitleUiLayoutFromDisk();
		return layout;
	}

	[[nodiscard]] inline TitleUiLayout GetTitleUiLayout()
	{
		return GetTitleUiLayoutStorage();
	}

	[[nodiscard]] inline TitleUiLayout ReloadTitleUiLayout()
	{
		GetTitleUiLayoutStorage() = LoadTitleUiLayoutFromDisk();
		return GetTitleUiLayoutStorage();
	}

	[[nodiscard]] inline bool SaveTitleUiLayout(const TitleUiLayout& layout)
	{
		const String layoutPath = GetLayoutPath();
		FileSystem::CreateDirectories(FileSystem::ParentPath(layoutPath));

		TextWriter writer{ layoutPath };
		if (!writer)
		{
			return false;
		}

		writer.write(BuildTitleUiLayoutTomlContent(layout));
		GetTitleUiLayoutStorage() = layout;
		return true;
	}

	[[nodiscard]] inline bool ClearTitleUiLayout()
	{
		const String layoutPath = GetLayoutPath();
		if (FileSystem::Exists(layoutPath) && !FileSystem::Remove(layoutPath))
		{
			return false;
		}

		GetTitleUiLayoutStorage() = MakeDefaultTitleUiLayout();
		return true;
	}

	[[nodiscard]] inline bool MoveTitleUiLayoutToLocation(const ContinueRunSaveLocation currentLocation, const ContinueRunSaveLocation nextLocation)
	{
		if (currentLocation == nextLocation)
		{
			return true;
		}

		const String currentPath = GetLayoutPathForLocation(currentLocation);
		const String nextPath = GetLayoutPathForLocation(nextLocation);
		FileSystem::CreateDirectories(FileSystem::ParentPath(nextPath));

		const auto layout = GetTitleUiLayout();

		TextWriter writer{ nextPath };
		if (!writer)
		{
			return false;
		}

		writer.write(BuildTitleUiLayoutTomlContent(layout));
		if (FileSystem::Exists(currentPath) && (currentPath != nextPath))
		{
			FileSystem::Remove(currentPath);
		}

		return true;
	}
}
