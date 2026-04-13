# include "MainSettingsInternal.hpp"

namespace MainSupport
{
	EditorTextColorSettings LoadEditorTextColorSettings()
	{
		EditorTextColorSettings settings;
		const TOMLReader toml{ EditorTextColorSettingsPath };

		if (not toml)
		{
			return settings;
		}

		settings.darkPrimary = SettingsDetail::ReadTomlColorF(toml, U"darkPrimary", settings.darkPrimary);
		settings.darkSecondary = SettingsDetail::ReadTomlColorF(toml, U"darkSecondary", settings.darkSecondary);
		settings.darkAccent = SettingsDetail::ReadTomlColorF(toml, U"darkAccent", settings.darkAccent);
		settings.lightPrimary = SettingsDetail::ReadTomlColorF(toml, U"lightPrimary", settings.lightPrimary);
		settings.lightSecondary = SettingsDetail::ReadTomlColorF(toml, U"lightSecondary", settings.lightSecondary);
		settings.lightAccent = SettingsDetail::ReadTomlColorF(toml, U"lightAccent", settings.lightAccent);
      settings.cardPrimary = SettingsDetail::ReadTomlColorF(toml, U"cardPrimary", settings.cardPrimary);
		settings.cardSecondary = SettingsDetail::ReadTomlColorF(toml, U"cardSecondary", settings.cardSecondary);
		settings.selectedPrimary = SettingsDetail::ReadTomlColorF(toml, U"selectedPrimary", settings.selectedPrimary);
		settings.selectedSecondary = SettingsDetail::ReadTomlColorF(toml, U"selectedSecondary", settings.selectedSecondary);
		settings.warning = SettingsDetail::ReadTomlColorF(toml, U"warning", settings.warning);
		settings.error = SettingsDetail::ReadTomlColorF(toml, U"error", settings.error);
		return settings;
	}

	bool SaveEditorTextColorSettings(const EditorTextColorSettings& settings)
	{
		const String directoryPath = FileSystem::ParentPath(EditorTextColorSettingsPath);
		if (not directoryPath.isEmpty())
		{
			FileSystem::CreateDirectories(directoryPath);
		}

		TextWriter writer{ EditorTextColorSettingsPath };

		if (not writer)
		{
			return false;
		}

		SettingsDetail::WriteTomlColorF(writer, U"darkPrimary", settings.darkPrimary);
		SettingsDetail::WriteTomlColorF(writer, U"darkSecondary", settings.darkSecondary);
		SettingsDetail::WriteTomlColorF(writer, U"darkAccent", settings.darkAccent);
		SettingsDetail::WriteTomlColorF(writer, U"lightPrimary", settings.lightPrimary);
		SettingsDetail::WriteTomlColorF(writer, U"lightSecondary", settings.lightSecondary);
		SettingsDetail::WriteTomlColorF(writer, U"lightAccent", settings.lightAccent);
      SettingsDetail::WriteTomlColorF(writer, U"cardPrimary", settings.cardPrimary);
		SettingsDetail::WriteTomlColorF(writer, U"cardSecondary", settings.cardSecondary);
		SettingsDetail::WriteTomlColorF(writer, U"selectedPrimary", settings.selectedPrimary);
		SettingsDetail::WriteTomlColorF(writer, U"selectedSecondary", settings.selectedSecondary);
		SettingsDetail::WriteTomlColorF(writer, U"warning", settings.warning);
		SettingsDetail::WriteTomlColorF(writer, U"error", settings.error);
		return true;
	}

	const EditorTextColorSettings& GetEditorTextColorSettings()
	{
		return SettingsDetail::CachedEditorTextColorSettings();
	}

	EditorTextColorSettings& GetMutableEditorTextColorSettings()
	{
		return SettingsDetail::CachedEditorTextColorSettings();
	}

	void ResetEditorTextColorSettings()
	{
		SettingsDetail::CachedEditorTextColorSettings() = EditorTextColorSettings{};
	}
}

namespace MainSupport::SettingsDetail
{
	EditorTextColorSettings& CachedEditorTextColorSettings()
	{
		static EditorTextColorSettings settings = MainSupport::LoadEditorTextColorSettings();
		return settings;
	}
}
