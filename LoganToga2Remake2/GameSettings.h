#pragma once

#include "ContinueRunSave.h"

struct PersistentGameSettings
{
	DisplaySettings displaySettings;
	bool fullscreen = false;
	double masterVolume = 1.0;
	double bgmVolume = 1.0;
	double seVolume = 1.0;
};

namespace GameSettings
{
	inline constexpr int32 SchemaVersion = 1;

	[[nodiscard]] inline String GetSettingsDirectoryPath()
	{
		return FileSystem::PathAppend(FileSystem::GetFolderPath(SpecialFolder::LocalAppData), U"LoganToga2Remake2");
	}

	[[nodiscard]] inline String GetLocalSettingsPath()
	{
		return U"save/settings.toml";
	}

	[[nodiscard]] inline String GetAppDataSettingsPath()
	{
		return FileSystem::PathAppend(GetSettingsDirectoryPath(), U"save/settings.toml");
	}

	[[nodiscard]] inline String GetSettingsPathForLocation(const ContinueRunSaveLocation location)
	{
		switch (location)
		{
		case ContinueRunSaveLocation::AppData:
			return GetAppDataSettingsPath();
		case ContinueRunSaveLocation::Local:
		default:
			return GetLocalSettingsPath();
		}
	}

	[[nodiscard]] inline String GetSettingsPath()
	{
		return GetSettingsPathForLocation(GetContinueRunSaveLocation());
	}

	[[nodiscard]] inline String EscapeTomlString(String value)
	{
		value.replace(U"\\", U"\\\\");
		value.replace(U"\"", U"\\\"");
		return value;
	}

	[[nodiscard]] inline String QuoteTomlString(const String& value)
	{
		return (U'"' + EscapeTomlString(value) + U'"');
	}

	[[nodiscard]] inline String GetResolutionPresetPersistenceLabel(const WindowResolutionPreset preset)
	{
		switch (preset)
		{
		case WindowResolutionPreset::Small:
			return U"Small";
		case WindowResolutionPreset::Large:
			return U"Large";
		case WindowResolutionPreset::Medium:
		default:
			return U"Medium";
		}
	}

	[[nodiscard]] inline WindowResolutionPreset ParseResolutionPresetPersistenceLabel(const String& label)
	{
		if (label == U"Small")
		{
			return WindowResolutionPreset::Small;
		}

		if (label == U"Large")
		{
			return WindowResolutionPreset::Large;
		}

		return WindowResolutionPreset::Medium;
	}

	[[nodiscard]] inline PersistentGameSettings MakeDefaultGameSettings()
	{
		return {};
	}

	[[nodiscard]] inline String BuildGameSettingsTomlContent(const PersistentGameSettings& settings)
	{
		String content;
		content += U"schemaVersion = 1\n";
		content += U"resolutionPreset = " + QuoteTomlString(GetResolutionPresetPersistenceLabel(settings.displaySettings.resolutionPreset)) + U"\n";
		content += U"fullscreen = " + String{ settings.fullscreen ? U"true" : U"false" } + U"\n";
		content += U"masterVolume = " + Format(Clamp(settings.masterVolume, 0.0, 1.0)) + U"\n";
		content += U"bgmVolume = " + Format(Clamp(settings.bgmVolume, 0.0, 1.0)) + U"\n";
		content += U"seVolume = " + Format(Clamp(settings.seVolume, 0.0, 1.0)) + U"\n";
		return content;
	}

	[[nodiscard]] inline PersistentGameSettings LoadGameSettingsFromDisk()
	{
		PersistentGameSettings settings = MakeDefaultGameSettings();
		const TOMLReader toml{ GetSettingsPath() };
		if (!toml)
		{
			return settings;
		}

		try
		{
			if (toml[U"schemaVersion"].get<int32>() != SchemaVersion)
			{
				return MakeDefaultGameSettings();
			}
		}
		catch (const std::exception&)
		{
			return MakeDefaultGameSettings();
		}

		try
		{
			settings.displaySettings.resolutionPreset = ParseResolutionPresetPersistenceLabel(toml[U"resolutionPreset"].get<String>());
		}
		catch (const std::exception&)
		{
		}

		try
		{
			settings.fullscreen = toml[U"fullscreen"].get<bool>();
		}
		catch (const std::exception&)
		{
		}

		try
		{
			settings.masterVolume = Clamp(toml[U"masterVolume"].get<double>(), 0.0, 1.0);
		}
		catch (const std::exception&)
		{
		}

		try
		{
			settings.bgmVolume = Clamp(toml[U"bgmVolume"].get<double>(), 0.0, 1.0);
		}
		catch (const std::exception&)
		{
		}

		try
		{
			settings.seVolume = Clamp(toml[U"seVolume"].get<double>(), 0.0, 1.0);
		}
		catch (const std::exception&)
		{
		}

		return settings;
	}

	[[nodiscard]] inline PersistentGameSettings& GetGameSettingsStorage()
	{
		static PersistentGameSettings settings = LoadGameSettingsFromDisk();
		return settings;
	}

	[[nodiscard]] inline PersistentGameSettings GetGameSettings()
	{
		return GetGameSettingsStorage();
	}

	[[nodiscard]] inline bool SaveGameSettings(const PersistentGameSettings& settings)
	{
		const String settingsPath = GetSettingsPath();
		FileSystem::CreateDirectories(FileSystem::ParentPath(settingsPath));

		TextWriter writer{ settingsPath };
		if (!writer)
		{
			return false;
		}

		writer.write(BuildGameSettingsTomlContent(settings));
		GetGameSettingsStorage() = settings;
		return true;
	}

	[[nodiscard]] inline bool ClearGameSettings()
	{
		const String settingsPath = GetSettingsPath();
		if (FileSystem::Exists(settingsPath) && !FileSystem::Remove(settingsPath))
		{
			return false;
		}

		GetGameSettingsStorage() = MakeDefaultGameSettings();
		return true;
	}

	[[nodiscard]] inline bool SetPersistedDisplaySettings(const DisplaySettings& displaySettings)
	{
		auto settings = GetGameSettings();
		settings.displaySettings = displaySettings;
		return SaveGameSettings(settings);
	}

	[[nodiscard]] inline bool SetPersistedWindowAndAudioSettings(const bool fullscreen, const double masterVolume, const double bgmVolume, const double seVolume)
	{
		auto settings = GetGameSettings();
		settings.fullscreen = fullscreen;
		settings.masterVolume = Clamp(masterVolume, 0.0, 1.0);
		settings.bgmVolume = Clamp(bgmVolume, 0.0, 1.0);
		settings.seVolume = Clamp(seVolume, 0.0, 1.0);
		return SaveGameSettings(settings);
	}

	[[nodiscard]] inline bool MoveGameSettingsToLocation(const ContinueRunSaveLocation currentLocation, const ContinueRunSaveLocation nextLocation)
	{
		if (currentLocation == nextLocation)
		{
			return true;
		}

		const String currentPath = GetSettingsPathForLocation(currentLocation);
		const String nextPath = GetSettingsPathForLocation(nextLocation);
		FileSystem::CreateDirectories(FileSystem::ParentPath(nextPath));

		const auto settings = GetGameSettings();

		TextWriter writer{ nextPath };
		if (!writer)
		{
			return false;
		}

		writer.write(BuildGameSettingsTomlContent(settings));
		if (FileSystem::Exists(currentPath) && (currentPath != nextPath))
		{
			FileSystem::Remove(currentPath);
		}

		return true;
	}
}
