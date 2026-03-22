#include "ContinueRunSave.h"

#include "Localization.h"

namespace
{
    [[nodiscard]] String GetContinueRunSaveLocationPersistenceLabel(const ContinueRunSaveLocation location)
	{
		switch (location)
		{
		case ContinueRunSaveLocation::AppData:
			return U"appdata";
		case ContinueRunSaveLocation::Local:
		default:
			return U"local";
		}
	}

	[[nodiscard]] String GetContinueRunSaveLocationConfigDirectoryPath()
	{
		return FileSystem::PathAppend(FileSystem::GetFolderPath(SpecialFolder::LocalAppData), U"LoganToga2Remake2");
	}

	[[nodiscard]] String GetContinueRunSaveLocationConfigPath()
	{
		return FileSystem::PathAppend(GetContinueRunSaveLocationConfigDirectoryPath(), U"continue_run_settings.toml");
	}

	[[nodiscard]] String GetLocalContinueRunSavePath()
	{
		return U"save/continue_run.toml";
	}

	[[nodiscard]] String GetAppDataContinueRunSavePath()
	{
		return FileSystem::PathAppend(GetContinueRunSaveLocationConfigDirectoryPath(), U"save/continue_run.toml");
	}

	[[nodiscard]] ContinueRunSaveLocation ParseContinueRunSaveLocation(const String& label)
	{
        const String normalized = label.lowercased();
		if ((normalized == U"appdata") || (label == U"AppData"))
		{
			return ContinueRunSaveLocation::AppData;
		}

		return ContinueRunSaveLocation::Local;
	}

	[[nodiscard]] ContinueRunSaveLocation LoadContinueRunSaveLocationSetting()
	{
		const TOMLReader toml{ GetContinueRunSaveLocationConfigPath() };
		if (!toml)
		{
			return ContinueRunSaveLocation::Local;
		}

		try
		{
			if (toml[U"schemaVersion"].get<int32>() != 1)
			{
				return ContinueRunSaveLocation::Local;
			}

			return ParseContinueRunSaveLocation(toml[U"location"].get<String>());
		}
		catch (const std::exception&)
		{
			return ContinueRunSaveLocation::Local;
		}
	}

	[[nodiscard]] ContinueRunSaveLocation& GetContinueRunSaveLocationStorage()
	{
		static ContinueRunSaveLocation location = LoadContinueRunSaveLocationSetting();
		return location;
	}

	[[nodiscard]] String GetContinueRunSavePathForLocation(const ContinueRunSaveLocation location)
	{
		switch (location)
		{
		case ContinueRunSaveLocation::AppData:
			return GetAppDataContinueRunSavePath();
		case ContinueRunSaveLocation::Local:
		default:
			return GetLocalContinueRunSavePath();
		}
	}

	[[nodiscard]] bool SaveContinueRunSaveLocationSetting(const ContinueRunSaveLocation location)
	{
		FileSystem::CreateDirectories(GetContinueRunSaveLocationConfigDirectoryPath());

		String content;
		AppendTomlLine(content, U"schemaVersion", U"1");
       AppendTomlLine(content, U"location", QuoteTomlString(GetContinueRunSaveLocationPersistenceLabel(location)));

		TextWriter writer{ GetContinueRunSaveLocationConfigPath() };
		if (!writer)
		{
			return false;
		}

		writer.write(content);
		return true;
	}
}

String GetContinueRunSavePath()
{
	return GetContinueRunSavePathForLocation(GetContinueRunSaveLocation());
}

bool HasContinueRunSave()
{
	return FileSystem::Exists(GetContinueRunSavePath());
}

ContinueRunSaveLocation GetContinueRunSaveLocation()
{
	return GetContinueRunSaveLocationStorage();
}

String GetContinueRunSaveLocationLabel(const ContinueRunSaveLocation location)
{
	switch (location)
	{
	case ContinueRunSaveLocation::AppData:
      return Localization::GetText(U"common.save_location.appdata");
	case ContinueRunSaveLocation::Local:
	default:
        return Localization::GetText(U"common.save_location.local");
	}
}

ContinueRunSaveLocation CycleContinueRunSaveLocation(const ContinueRunSaveLocation location)
{
	return (location == ContinueRunSaveLocation::Local)
		? ContinueRunSaveLocation::AppData
		: ContinueRunSaveLocation::Local;
}

bool SetContinueRunSaveLocation(const ContinueRunSaveLocation location)
{
	auto& currentLocation = GetContinueRunSaveLocationStorage();
	if (currentLocation == location)
	{
		return true;
	}

	const String currentSavePath = GetContinueRunSavePathForLocation(currentLocation);
	const String nextSavePath = GetContinueRunSavePathForLocation(location);
	const bool hasCurrentSave = FileSystem::Exists(currentSavePath);
	if (hasCurrentSave)
	{
		FileSystem::CreateDirectories(FileSystem::ParentPath(nextSavePath));
		if (!FileSystem::Copy(currentSavePath, nextSavePath, CopyOption::OverwriteExisting))
		{
			return false;
		}
	}

	if (!SaveContinueRunSaveLocationSetting(location))
	{
		if (hasCurrentSave && FileSystem::Exists(nextSavePath))
		{
			FileSystem::Remove(nextSavePath);
		}
		return false;
	}

	if (hasCurrentSave)
	{
		FileSystem::Remove(currentSavePath);
	}

	currentLocation = location;
	return true;
}
