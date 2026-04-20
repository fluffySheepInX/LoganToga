# include "MainSettingsInternal.hpp"
# include "SettingsRegistry.hpp"
# include "SettingsSchemas.hpp"

namespace
{
    using MainSupport::PanelSkinTarget;

    struct PanelSkinSlot
    {
        PanelSkinTarget target;
        const char32* tomlKey;
    };

    inline constexpr std::array<PanelSkinSlot, 7> PanelSkinSlots{ {
        { PanelSkinTarget::Default,        U"defaultPanelNinePatchPath"        },
        { PanelSkinTarget::Settings,       U"settingsPanelNinePatchPath"       },
        { PanelSkinTarget::CameraSettings, U"cameraSettingsPanelNinePatchPath" },
        { PanelSkinTarget::Hud,            U"hudPanelNinePatchPath"            },
        { PanelSkinTarget::MapEditor,      U"mapEditorPanelNinePatchPath"      },
        { PanelSkinTarget::UnitEditor,     U"unitEditorPanelNinePatchPath"     },
        { PanelSkinTarget::ToolModal,      U"toolModalPanelNinePatchPath"      },
    } };

    [[nodiscard]] size_t PanelTargetIndex(const PanelSkinTarget target)
    {
        for (size_t i = 0; i < PanelSkinSlots.size(); ++i)
        {
            if (PanelSkinSlots[i].target == target)
            {
                return i;
            }
        }
        return 0;
    }

    [[nodiscard]] String EscapeTomlString(const StringView value)
    {
        return String{ value }.replaced(U"\\", U"\\\\").replaced(U"\"", U"\\\"");
    }

    [[nodiscard]] FilePath ReadPanelNinePatchPath(const TOMLReader& toml, const PanelSkinTarget target)
    {
        if (const auto path = toml[String{ PanelSkinSlots[PanelTargetIndex(target)].tomlKey }].getOpt<String>())
        {
            return *path;
        }

        if (target == PanelSkinTarget::Default)
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
    namespace
    {
        constexpr SettingDescriptor<EditorTextColorSettings> EditorTextColorsDescriptor{
            .name = U"EditorTextColors",
            .path = EditorTextColorSettingsPath,
            .loadFn = [](const TOMLReader& toml, EditorTextColorSettings& value)
            {
                SettingsSchemas::VisitEditorTextColors(TomlSchema::LoadVisitor{ toml, U"" }, value);
            },
            .saveFn = [](TextWriter& writer, const EditorTextColorSettings& value)
            {
                SettingsSchemas::VisitEditorTextColors(TomlSchema::SaveVisitor{ writer, U"" }, value);
            },
        };

        using PanelPathArray = std::array<FilePath, PanelSkinSlots.size()>;

        PanelPathArray& CachedPanelPaths()
        {
            static PanelPathArray paths = []() {
                PanelPathArray result;
                for (size_t i = 0; i < PanelSkinSlots.size(); ++i)
                {
                    result[i] = MainSupport::LoadPanelNinePatchPath(PanelSkinSlots[i].target);
                }
                return result;
            }();
            return paths;
        }

        [[nodiscard]] FilePath& CachedPath(const PanelSkinTarget target)
        {
            return CachedPanelPaths()[PanelTargetIndex(target)];
        }
    }

    EditorTextColorSettings LoadEditorTextColorSettings()
    {
        return LoadSetting(EditorTextColorsDescriptor);
    }

    bool SaveEditorTextColorSettings(const EditorTextColorSettings& settings)
    {
        return SaveSetting(EditorTextColorsDescriptor, settings);
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

        for (const auto& slot : PanelSkinSlots)
        {
            const FilePath effective = (slot.target == target)
                ? String{ path }
                : CachedPath(slot.target);

            if (not effective.isEmpty())
            {
                writer.writeln(U"{} = \"{}\""_fmt(StringView{ slot.tomlKey }, EscapeTomlString(effective)));
            }
        }

        return true;
    }

    const FilePath& GetPanelNinePatchPath()
    {
        return CachedPath(PanelSkinTarget::Default);
    }

    const FilePath& GetConfiguredPanelNinePatchPath(const PanelSkinTarget target)
    {
        return CachedPath(target);
    }

    FilePath GetEffectivePanelNinePatchPath(const PanelSkinTarget target)
    {
        const FilePath& configuredPath = GetConfiguredPanelNinePatchPath(target);
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
        CachedPath(target) = String{ path };
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
}
