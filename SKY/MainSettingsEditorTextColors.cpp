# include "MainSettingsInternal.hpp"

namespace
{
	[[nodiscard]] String EscapeTomlString(const StringView value)
	{
		return String{ value }.replaced(U"\\", U"\\\\").replaced(U"\"", U"\\\"");
	}

  [[nodiscard]] String ToPanelSkinTomlKey(const MainSupport::PanelSkinTarget target)
	{
		switch (target)
		{
     case MainSupport::PanelSkinTarget::UnitEditor:
			return U"unitEditorPanelNinePatchPath";

		case MainSupport::PanelSkinTarget::ToolModal:
			return U"toolModalPanelNinePatchPath";

       case MainSupport::PanelSkinTarget::Settings:
			return U"settingsPanelNinePatchPath";

       case MainSupport::PanelSkinTarget::CameraSettings:
			return U"cameraSettingsPanelNinePatchPath";

		case MainSupport::PanelSkinTarget::Hud:
			return U"hudPanelNinePatchPath";

		case MainSupport::PanelSkinTarget::MapEditor:
          return U"mapEditorPanelNinePatchPath";

		case MainSupport::PanelSkinTarget::Default:
		default:
            return U"defaultPanelNinePatchPath";
		}
	}

	[[nodiscard]] FilePath ReadPanelNinePatchPath(const TOMLReader& toml, const MainSupport::PanelSkinTarget target)
	{
		if (const auto path = toml[ToPanelSkinTomlKey(target)].getOpt<String>())
		{
			return *path;
		}

		if (target == MainSupport::PanelSkinTarget::Default)
		{
			if (const auto legacyPath = toml[U"panelNinePatchPath"].getOpt<String>())
			{
				return *legacyPath;
			}
		}

		return{};
	}
}

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

   FilePath LoadPanelNinePatchPath()
	{
		return LoadPanelNinePatchPath(PanelSkinTarget::Default);
	}

	FilePath LoadPanelNinePatchPath(const PanelSkinTarget target)
	{
     if (not FileSystem::Exists(PanelSkinSettingsPath))
		{
			return{};
		}

		const TOMLReader toml{ PanelSkinSettingsPath };

		if (not toml)
		{
			return{};
		}

     return ReadPanelNinePatchPath(toml, target);
	}

	bool SavePanelNinePatchPath(const FilePathView path)
   {
		return SavePanelNinePatchPath(PanelSkinTarget::Default, path);
	}

	bool SavePanelNinePatchPath(const PanelSkinTarget target, const FilePathView path)
	{
		const String directoryPath = FileSystem::ParentPath(PanelSkinSettingsPath);
		if (not directoryPath.isEmpty())
		{
			FileSystem::CreateDirectories(directoryPath);
		}

		TextWriter writer{ PanelSkinSettingsPath };

		if (not writer)
		{
			return false;
		}

     const FilePath defaultPath = (target == PanelSkinTarget::Default)
			? String{ path }
			: SettingsDetail::CachedPanelNinePatchPath();
      const FilePath settingsPath = (target == PanelSkinTarget::Settings)
			? String{ path }
			: SettingsDetail::CachedSettingsPanelNinePatchPath();
       const FilePath cameraSettingsPath = (target == PanelSkinTarget::CameraSettings)
			? String{ path }
			: SettingsDetail::CachedCameraSettingsPanelNinePatchPath();
		const FilePath hudPath = (target == PanelSkinTarget::Hud)
			? String{ path }
			: SettingsDetail::CachedHudPanelNinePatchPath();
		const FilePath mapEditorPath = (target == PanelSkinTarget::MapEditor)
			? String{ path }
			: SettingsDetail::CachedMapEditorPanelNinePatchPath();
		const FilePath unitEditorPath = (target == PanelSkinTarget::UnitEditor)
			? String{ path }
			: SettingsDetail::CachedUnitEditorPanelNinePatchPath();
		const FilePath toolModalPath = (target == PanelSkinTarget::ToolModal)
			? String{ path }
			: SettingsDetail::CachedToolModalPanelNinePatchPath();

		if (not defaultPath.isEmpty())
		{
           writer.writeln(U"defaultPanelNinePatchPath = \"{}\""_fmt(EscapeTomlString(defaultPath)));
		}

		if (not settingsPath.isEmpty())
		{
			writer.writeln(U"settingsPanelNinePatchPath = \"{}\""_fmt(EscapeTomlString(settingsPath)));
		}

		if (not cameraSettingsPath.isEmpty())
		{
			writer.writeln(U"cameraSettingsPanelNinePatchPath = \"{}\""_fmt(EscapeTomlString(cameraSettingsPath)));
		}

		if (not hudPath.isEmpty())
		{
			writer.writeln(U"hudPanelNinePatchPath = \"{}\""_fmt(EscapeTomlString(hudPath)));
		}

		if (not mapEditorPath.isEmpty())
		{
			writer.writeln(U"mapEditorPanelNinePatchPath = \"{}\""_fmt(EscapeTomlString(mapEditorPath)));
		}

		if (not unitEditorPath.isEmpty())
		{
			writer.writeln(U"unitEditorPanelNinePatchPath = \"{}\""_fmt(EscapeTomlString(unitEditorPath)));
		}

		if (not toolModalPath.isEmpty())
		{
			writer.writeln(U"toolModalPanelNinePatchPath = \"{}\""_fmt(EscapeTomlString(toolModalPath)));
		}

		return true;
	}

	const FilePath& GetPanelNinePatchPath()
	{
		return SettingsDetail::CachedPanelNinePatchPath();
	}

	const FilePath& GetConfiguredPanelNinePatchPath(const PanelSkinTarget target)
	{
		switch (target)
		{
      case PanelSkinTarget::UnitEditor:
			return SettingsDetail::CachedUnitEditorPanelNinePatchPath();

		case PanelSkinTarget::ToolModal:
			return SettingsDetail::CachedToolModalPanelNinePatchPath();

       case PanelSkinTarget::Settings:
			return SettingsDetail::CachedSettingsPanelNinePatchPath();

        case PanelSkinTarget::CameraSettings:
			return SettingsDetail::CachedCameraSettingsPanelNinePatchPath();

		case PanelSkinTarget::Hud:
			return SettingsDetail::CachedHudPanelNinePatchPath();

		case PanelSkinTarget::MapEditor:
			return SettingsDetail::CachedMapEditorPanelNinePatchPath();

		case PanelSkinTarget::Default:
		default:
			return SettingsDetail::CachedPanelNinePatchPath();
		}
	}

	FilePath GetEffectivePanelNinePatchPath(const PanelSkinTarget target)
	{
		const FilePath configuredPath = GetConfiguredPanelNinePatchPath(target);
		if ((target != PanelSkinTarget::Default) && (not configuredPath.isEmpty()))
		{
			return configuredPath;
		}

		return GetPanelNinePatchPath();
	}

	void SetPanelNinePatchPath(const FilePathView path)
   {
		SetPanelNinePatchPath(PanelSkinTarget::Default, path);
	}

	void SetPanelNinePatchPath(const PanelSkinTarget target, const FilePathView path)
	{
        switch (target)
		{
      case PanelSkinTarget::UnitEditor:
			SettingsDetail::CachedUnitEditorPanelNinePatchPath() = String{ path };
			return;

		case PanelSkinTarget::ToolModal:
			SettingsDetail::CachedToolModalPanelNinePatchPath() = String{ path };
			return;

       case PanelSkinTarget::Settings:
			SettingsDetail::CachedSettingsPanelNinePatchPath() = String{ path };
			return;

        case PanelSkinTarget::CameraSettings:
			SettingsDetail::CachedCameraSettingsPanelNinePatchPath() = String{ path };
			return;

		case PanelSkinTarget::Hud:
			SettingsDetail::CachedHudPanelNinePatchPath() = String{ path };
			return;

		case PanelSkinTarget::MapEditor:
			SettingsDetail::CachedMapEditorPanelNinePatchPath() = String{ path };
			return;

		case PanelSkinTarget::Default:
		default:
			SettingsDetail::CachedPanelNinePatchPath() = String{ path };
			return;
		}
	}

	void ResetPanelNinePatchPath()
   {
		ResetPanelNinePatchPath(PanelSkinTarget::Default);
	}

	void ResetPanelNinePatchPath(const PanelSkinTarget target)
	{
     SetPanelNinePatchPath(target, U"");
	}
}

namespace MainSupport::SettingsDetail
{
	EditorTextColorSettings& CachedEditorTextColorSettings()
	{
		static EditorTextColorSettings settings = MainSupport::LoadEditorTextColorSettings();
		return settings;
	}

	FilePath& CachedPanelNinePatchPath()
	{
		static FilePath path = MainSupport::LoadPanelNinePatchPath();
		return path;
	}

	FilePath& CachedSettingsPanelNinePatchPath()
	{
		static FilePath path = MainSupport::LoadPanelNinePatchPath(MainSupport::PanelSkinTarget::Settings);
		return path;
	}

	FilePath& CachedCameraSettingsPanelNinePatchPath()
	{
		static FilePath path = MainSupport::LoadPanelNinePatchPath(MainSupport::PanelSkinTarget::CameraSettings);
		return path;
	}

	FilePath& CachedHudPanelNinePatchPath()
	{
		static FilePath path = MainSupport::LoadPanelNinePatchPath(MainSupport::PanelSkinTarget::Hud);
		return path;
	}

	FilePath& CachedMapEditorPanelNinePatchPath()
	{
		static FilePath path = MainSupport::LoadPanelNinePatchPath(MainSupport::PanelSkinTarget::MapEditor);
		return path;
	}

	FilePath& CachedUnitEditorPanelNinePatchPath()
	{
		static FilePath path = MainSupport::LoadPanelNinePatchPath(MainSupport::PanelSkinTarget::UnitEditor);
		return path;
	}

	FilePath& CachedToolModalPanelNinePatchPath()
	{
		static FilePath path = MainSupport::LoadPanelNinePatchPath(MainSupport::PanelSkinTarget::ToolModal);
		return path;
	}
}
