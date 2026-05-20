#pragma once
# include <Siv3D.hpp>

namespace LT3
{
    namespace UnitCatalogToml
    {
        inline constexpr int32 SchemaVersion = 1;

        inline constexpr const char32* KeySchemaVersion = U"schema_version";
        inline constexpr const char32* KeyUnits = U"units";
        inline constexpr const char32* KeyId = U"id";
        inline constexpr const char32* KeyUnitId = U"unit_id";
        inline constexpr const char32* KeyName = U"name";
        inline constexpr const char32* KeyKind = U"kind";
        inline constexpr const char32* KeyRace = U"race";
        inline constexpr const char32* KeyImage = U"image";
        inline constexpr const char32* KeyBuildingCategory = U"building_category";
        inline constexpr const char32* KeyUnitFamily = U"unit_family";
        inline constexpr const char32* KeySkills = U"skills";
        inline constexpr const char32* KeyLevel = U"level";
        inline constexpr const char32* KeyLevelMax = U"level_max";
        inline constexpr const char32* KeyVisionRadius = U"vision_radius";
        inline constexpr const char32* KeyGoldCost = U"gold_cost";
        inline constexpr const char32* KeyCost = U"cost";
        inline constexpr const char32* KeyTrustCost = U"trust_cost";
        inline constexpr const char32* KeyPrice = U"price";
        inline constexpr const char32* KeyFoodCost = U"food_cost";
        inline constexpr const char32* KeyHp = U"hp";
        inline constexpr const char32* KeyBuildingHp = U"building_hp";
        inline constexpr const char32* KeyMp = U"mp";
        inline constexpr const char32* KeyAttack = U"attack";
        inline constexpr const char32* KeyDefense = U"defense";
        inline constexpr const char32* KeyMagic = U"magic";
        inline constexpr const char32* KeyMagicDefense = U"magic_defense";
        inline constexpr const char32* KeySpeed = U"speed";
        inline constexpr const char32* KeyMove = U"move";
        inline constexpr const char32* KeyMaintainRange = U"maintain_range";
        inline constexpr const char32* KeyVisualScale = U"visual_scale";
        inline constexpr const char32* KeyScale = U"scale";
        inline constexpr const char32* KeyVisualOffset = U"visual_offset";
        inline constexpr const char32* KeyShadowOffset = U"shadow_offset";
        inline constexpr const char32* KeyPlacementAnchor = U"placement_anchor";
        inline constexpr const char32* KeyRenderSizeMode = U"render_size_mode";
        inline constexpr const char32* KeyGameplaySizeMul = U"gameplay_size_mul";
        inline constexpr const char32* KeyArtWidthRef = U"art_width_ref";
        inline constexpr const char32* KeyArtWidthValue = U"art_width_value";
        inline constexpr const char32* KeyArtWidthValueLineHorizontal = U"art_width_value_line_horizontal";
        inline constexpr const char32* KeyArtWidthValueLineDiagUpRight = U"art_width_value_line_diag_up_right";
        inline constexpr const char32* KeyArtWidthValueLineDiagUpLeft = U"art_width_value_line_diag_up_left";
        inline constexpr const char32* KeyArtKeepAspect = U"art_keep_aspect";
        inline constexpr const char32* KeyLineIconHorizontalOffset = U"line_icon_horizontal_offset";
        inline constexpr const char32* KeyLineIconDiagUpRightOffset = U"line_icon_diag_up_right_offset";
        inline constexpr const char32* KeyLineIconDiagUpLeftOffset = U"line_icon_diag_up_left_offset";
    }

    enum class UnitPlacementAnchor
    {
        Center,
        BottomCenter,
    };

    enum class UnitRenderSizeMode
    {
        Gameplay,
        Art,
    };

    enum class UnitArtWidthReference
    {
        Cell,
        Pixel,
    };

    inline UnitPlacementAnchor ParseUnitPlacementAnchor(const String& value, const String& kind)
    {
        const String lowered = value.lowercased();
        if (lowered == U"bottom" || lowered == U"bottom_center")
        {
            return UnitPlacementAnchor::BottomCenter;
        }
        if (lowered == U"center")
        {
            return UnitPlacementAnchor::Center;
        }

        return (kind.lowercased() == U"building") ? UnitPlacementAnchor::BottomCenter : UnitPlacementAnchor::Center;
    }

    inline String UnitPlacementAnchorToTomlValue(UnitPlacementAnchor anchor)
    {
        return (anchor == UnitPlacementAnchor::BottomCenter) ? U"bottom_center" : U"center";
    }

    inline UnitRenderSizeMode ParseUnitRenderSizeMode(const String& value)
    {
        return (value.lowercased() == U"art") ? UnitRenderSizeMode::Art : UnitRenderSizeMode::Gameplay;
    }

    inline String UnitRenderSizeModeToTomlValue(UnitRenderSizeMode mode)
    {
        return (mode == UnitRenderSizeMode::Art) ? U"art" : U"gameplay";
    }

    inline UnitArtWidthReference ParseUnitArtWidthReference(const String& value)
    {
        return (value.lowercased() == U"pixel") ? UnitArtWidthReference::Pixel : UnitArtWidthReference::Cell;
    }

    inline String UnitArtWidthReferenceToTomlValue(UnitArtWidthReference reference)
    {
        return (reference == UnitArtWidthReference::Pixel) ? U"pixel" : U"cell";
    }

    struct UnitCatalogEntry
    {
        int32 id = 0;
        String unit_id;
        String name;
        String kind;
        String race;
        String image;
        String building_category;
        String unit_family;
        Array<String> skills;
        int32 level = 1;
        int32 levelMax = 1;
        int32 visionRadius = 0;
        int32 goldCost = 0;
        int32 trustCost = 0;
        int32 foodCost = 0;
        int32 hp = 0;
        int32 buildingHp = 0;
        int32 mp = 0;
        int32 attack = 0;
        int32 defense = 0;
        int32 magic = 0;
        int32 magicDefense = 0;
        int32 speed = 0;
        int32 move = 0;
        int32 maintainRange = 0;
        double visualScale = 1.0;
        Point visualOffset{ 0, 0 };
        Point shadowOffset{ 0, 0 };
        UnitPlacementAnchor placementAnchor = UnitPlacementAnchor::Center;
        UnitRenderSizeMode renderSizeMode = UnitRenderSizeMode::Gameplay;
        double gameplaySizeMul = 2.2;
        UnitArtWidthReference artWidthReference = UnitArtWidthReference::Cell;
        double artWidthValue = 1.0;
        double artWidthValueLineHorizontal = 1.0;
        double artWidthValueLineDiagUpRight = 1.0;
        double artWidthValueLineDiagUpLeft = 1.0;
        bool artKeepAspect = true;
        Point lineIconHorizontalOffset{ 0, 0 };
        Point lineIconDiagUpRightOffset{ 0, 0 };
        Point lineIconDiagUpLeftOffset{ 0, 0 };
    };

    struct UnitCatalog
    {
        Array<UnitCatalogEntry> entries;
        FilePath sourcePath;
        String statusText = U"Unit catalog not loaded";
    };

    inline FilePath ResolveUnitCatalogTomlPath()
    {
        const Array<FilePath> candidates = {
            U"000_Warehouse/000_DefaultGame/070_Scenario/InfoUnit/UnitSample.toml",
            U"App/000_Warehouse/000_DefaultGame/070_Scenario/InfoUnit/UnitSample.toml",
            U"LoganToga3/App/000_Warehouse/000_DefaultGame/070_Scenario/InfoUnit/UnitSample.toml",
        };

        for (const auto& path : candidates)
        {
            if (FileSystem::Exists(path))
            {
                return path;
            }
        }

        return candidates.back();
    }

    inline Array<String> ReadTomlStringArray(const TOMLValue& value)
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

    inline String UnitCatalogTomlEscape(StringView text)
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

    inline void WriteTomlStringArray(TextWriter& writer, const Array<String>& values)
    {
        writer << U"[";
        for (size_t i = 0; i < values.size(); ++i)
        {
            if (i > 0)
            {
                writer << U", ";
            }
            writer << U"\"" << UnitCatalogTomlEscape(values[i]) << U"\"";
        }
        writer << U"]";
    }

    inline String BuildTomlStringArray(const Array<String>& values)
    {
        String result = U"[";
        for (size_t i = 0; i < values.size(); ++i)
        {
            if (i > 0)
            {
                result += U", ";
            }

            result += U"\"" + UnitCatalogTomlEscape(values[i]) + U"\"";
        }

        result += U"]";
        return result;
    }

    inline Array<int32> SortedUnitCatalogEntryIndicesById(const UnitCatalog& catalog)
    {
        Array<int32> indices;
        indices.reserve(catalog.entries.size());
        for (int32 i = 0; i < static_cast<int32>(catalog.entries.size()); ++i)
        {
            indices << i;
        }

        std::sort(indices.begin(), indices.end(), [&](const int32 a, const int32 b)
        {
            const UnitCatalogEntry& left = catalog.entries[a];
            const UnitCatalogEntry& right = catalog.entries[b];
            if (left.id != right.id)
            {
                return left.id < right.id;
            }

            return left.unit_id < right.unit_id;
        });

        return indices;
    }

    inline Point ReadTomlPointArray(const TOMLValue& value)
    {
        Point result{ 0, 0 };
        if (!value.isArray())
        {
            return result;
        }

        int32 index = 0;
        for (const auto item : value.arrayView())
        {
            if (const Optional<int32> coordinate = item.getOpt<int32>())
            {
                if (index == 0)
                {
                    result.x = *coordinate;
                }
                else if (index == 1)
                {
                    result.y = *coordinate;
                    break;
                }
                ++index;
            }
        }

        return result;
    }

    inline TOMLReader OpenUnitCatalogToml(FilePathView path)
    {
        TOMLReader toml{ path };
        if (toml)
        {
            return toml;
        }

        if (Unicode::GetTextEncoding(path) != TextEncoding::UTF8_WITH_BOM)
        {
            return toml;
        }

        Blob blob;
        if (!blob.createFromFile(path))
        {
            return toml;
        }

        constexpr size_t BomSize = 3;
        if (blob.size() <= BomSize)
        {
            return toml;
        }

        Array<Byte> bytes = blob.asArray();
        bytes.erase(bytes.begin(), bytes.begin() + BomSize);
        return TOMLReader{ MemoryReader{ Blob{ std::move(bytes) } } };
    }

    inline UnitCatalog LoadUnitCatalog()
    {
        UnitCatalog catalog;
        catalog.sourcePath = ResolveUnitCatalogTomlPath();

        const TOMLReader toml = OpenUnitCatalogToml(catalog.sourcePath);
        if (!toml)
        {
            catalog.statusText = U"Unit TOML load failed: {}"_fmt(catalog.sourcePath);
            return catalog;
        }

        const int32 schemaVersion = toml[UnitCatalogToml::KeySchemaVersion].getOr<int32>(0);
        if (schemaVersion != UnitCatalogToml::SchemaVersion)
        {
            catalog.statusText = U"Unsupported unit schema_version {} in {}"_fmt(schemaVersion, catalog.sourcePath);
            return catalog;
        }

        for (const auto unitValue : toml[UnitCatalogToml::KeyUnits].tableArrayView())
        {
            UnitCatalogEntry entry;
            entry.id = unitValue[UnitCatalogToml::KeyId].getOr<int32>(0);
            entry.unit_id = unitValue[UnitCatalogToml::KeyUnitId].getOr<String>(U"");
            entry.name = unitValue[UnitCatalogToml::KeyName].getOr<String>(entry.unit_id);
            entry.kind = unitValue[UnitCatalogToml::KeyKind].getOr<String>(U"unit");
            entry.race = unitValue[UnitCatalogToml::KeyRace].getOr<String>(U"");
            entry.image = unitValue[UnitCatalogToml::KeyImage].getOr<String>(U"");
            entry.building_category = unitValue[UnitCatalogToml::KeyBuildingCategory].getOr<String>(U"");
            entry.unit_family = unitValue[UnitCatalogToml::KeyUnitFamily].getOr<String>(U"");
            entry.skills = ReadTomlStringArray(unitValue[UnitCatalogToml::KeySkills]);
            entry.level = unitValue[UnitCatalogToml::KeyLevel].getOr<int32>(1);
            entry.levelMax = unitValue[UnitCatalogToml::KeyLevelMax].getOr<int32>(1);
            entry.visionRadius = unitValue[UnitCatalogToml::KeyVisionRadius].getOr<int32>(0);
            entry.goldCost = unitValue[UnitCatalogToml::KeyGoldCost].getOr<int32>(unitValue[UnitCatalogToml::KeyCost].getOr<int32>(0));
            entry.trustCost = unitValue[UnitCatalogToml::KeyTrustCost].getOr<int32>(unitValue[UnitCatalogToml::KeyPrice].getOr<int32>(0));
            entry.foodCost = unitValue[UnitCatalogToml::KeyFoodCost].getOr<int32>(0);
            entry.hp = unitValue[UnitCatalogToml::KeyHp].getOr<int32>(0);
            entry.buildingHp = unitValue[UnitCatalogToml::KeyBuildingHp].getOr<int32>(0);
            entry.mp = unitValue[UnitCatalogToml::KeyMp].getOr<int32>(0);
            entry.attack = unitValue[UnitCatalogToml::KeyAttack].getOr<int32>(0);
            entry.defense = unitValue[UnitCatalogToml::KeyDefense].getOr<int32>(0);
            entry.magic = unitValue[UnitCatalogToml::KeyMagic].getOr<int32>(0);
            entry.magicDefense = unitValue[UnitCatalogToml::KeyMagicDefense].getOr<int32>(0);
            entry.speed = unitValue[UnitCatalogToml::KeySpeed].getOr<int32>(0);
            entry.move = unitValue[UnitCatalogToml::KeyMove].getOr<int32>(0);
            entry.maintainRange = unitValue[UnitCatalogToml::KeyMaintainRange].getOr<int32>(0);
            entry.visualScale = Clamp(unitValue[UnitCatalogToml::KeyVisualScale].getOr<double>(unitValue[UnitCatalogToml::KeyScale].getOr<double>(1.0)), 0.25, 3.0);
            entry.visualOffset = ReadTomlPointArray(unitValue[UnitCatalogToml::KeyVisualOffset]);
            entry.shadowOffset = ReadTomlPointArray(unitValue[UnitCatalogToml::KeyShadowOffset]);
            entry.placementAnchor = ParseUnitPlacementAnchor(unitValue[UnitCatalogToml::KeyPlacementAnchor].getOr<String>(U""), entry.kind);
            entry.renderSizeMode = ParseUnitRenderSizeMode(unitValue[UnitCatalogToml::KeyRenderSizeMode].getOr<String>(U"gameplay"));
            if (const Optional<double> gameplaySizeMul = unitValue[UnitCatalogToml::KeyGameplaySizeMul].getOpt<double>())
            {
                entry.gameplaySizeMul = Clamp(*gameplaySizeMul, 0.2, 8.0);
            }
            else
            {
                entry.gameplaySizeMul = (entry.kind.lowercased() == U"building") ? 2.2 : 2.0;
            }
            entry.artWidthReference = ParseUnitArtWidthReference(unitValue[UnitCatalogToml::KeyArtWidthRef].getOr<String>(U"cell"));
            entry.artWidthValue = Clamp(unitValue[UnitCatalogToml::KeyArtWidthValue].getOr<double>(1.0), 0.1, 8.0);
            entry.artWidthValueLineHorizontal = Clamp(unitValue[UnitCatalogToml::KeyArtWidthValueLineHorizontal].getOr<double>(entry.artWidthValue), 0.1, 8.0);
            entry.artWidthValueLineDiagUpRight = Clamp(unitValue[UnitCatalogToml::KeyArtWidthValueLineDiagUpRight].getOr<double>(entry.artWidthValue), 0.1, 8.0);
            entry.artWidthValueLineDiagUpLeft = Clamp(unitValue[UnitCatalogToml::KeyArtWidthValueLineDiagUpLeft].getOr<double>(entry.artWidthValue), 0.1, 8.0);
            entry.artKeepAspect = unitValue[UnitCatalogToml::KeyArtKeepAspect].getOr<bool>(true);
            entry.lineIconHorizontalOffset = ReadTomlPointArray(unitValue[UnitCatalogToml::KeyLineIconHorizontalOffset]);
            entry.lineIconDiagUpRightOffset = ReadTomlPointArray(unitValue[UnitCatalogToml::KeyLineIconDiagUpRightOffset]);
            entry.lineIconDiagUpLeftOffset = ReadTomlPointArray(unitValue[UnitCatalogToml::KeyLineIconDiagUpLeftOffset]);

            if (!entry.unit_id.isEmpty())
            {
                catalog.entries << entry;
            }
        }

        catalog.statusText = U"Loaded {} units/buildings from {}"_fmt(catalog.entries.size(), catalog.sourcePath);
        return catalog;
    }

    inline bool SaveUnitCatalogToml(UnitCatalog& catalog, String& statusText)
    {
        FileSystem::CreateDirectories(FileSystem::ParentPath(catalog.sourcePath));
        TextWriter writer{ catalog.sourcePath, OpenMode::Trunc, TextEncoding::UTF8_NO_BOM };
        if (!writer)
        {
            statusText = U"Unit catalog save failed: {}"_fmt(catalog.sourcePath);
            return false;
        }

        writer << UnitCatalogToml::KeySchemaVersion << U" = " << UnitCatalogToml::SchemaVersion << U"\n\n";

        for (const auto& entry : catalog.entries)
        {
            String block;
            block += U"[[{}]]\n"_fmt(UnitCatalogToml::KeyUnits);
            block += U"{} = {}\n"_fmt(UnitCatalogToml::KeyId, entry.id);
            block += U"{} = \"{}\"\n"_fmt(UnitCatalogToml::KeyUnitId, UnitCatalogTomlEscape(entry.unit_id));
            block += U"{} = \"{}\"\n"_fmt(UnitCatalogToml::KeyName, UnitCatalogTomlEscape(entry.name));
            block += U"{} = \"{}\"\n"_fmt(UnitCatalogToml::KeyKind, UnitCatalogTomlEscape(entry.kind));
            block += U"{} = \"{}\"\n"_fmt(UnitCatalogToml::KeyRace, UnitCatalogTomlEscape(entry.race));
            block += U"{} = \"{}\"\n"_fmt(UnitCatalogToml::KeyImage, UnitCatalogTomlEscape(entry.image));
            block += U"{} = \"{}\"\n"_fmt(UnitCatalogToml::KeyBuildingCategory, UnitCatalogTomlEscape(entry.building_category));
            block += U"{} = \"{}\"\n"_fmt(UnitCatalogToml::KeyUnitFamily, UnitCatalogTomlEscape(entry.unit_family));
            block += U"{} = {}\n"_fmt(UnitCatalogToml::KeySkills, BuildTomlStringArray(entry.skills));
            block += U"{} = {}\n"_fmt(UnitCatalogToml::KeyLevel, entry.level);
            block += U"{} = {}\n"_fmt(UnitCatalogToml::KeyLevelMax, entry.levelMax);
            block += U"{} = {}\n"_fmt(UnitCatalogToml::KeyVisionRadius, entry.visionRadius);
            block += U"{} = {}\n"_fmt(UnitCatalogToml::KeyGoldCost, entry.goldCost);
            block += U"{} = {}\n"_fmt(UnitCatalogToml::KeyTrustCost, entry.trustCost);
            block += U"{} = {}\n"_fmt(UnitCatalogToml::KeyFoodCost, entry.foodCost);
            block += U"{} = {}\n"_fmt(UnitCatalogToml::KeyHp, entry.hp);
            block += U"{} = {}\n"_fmt(UnitCatalogToml::KeyBuildingHp, entry.buildingHp);
            block += U"{} = {}\n"_fmt(UnitCatalogToml::KeyMp, entry.mp);
            block += U"{} = {}\n"_fmt(UnitCatalogToml::KeyAttack, entry.attack);
            block += U"{} = {}\n"_fmt(UnitCatalogToml::KeyDefense, entry.defense);
            block += U"{} = {}\n"_fmt(UnitCatalogToml::KeyMagic, entry.magic);
            block += U"{} = {}\n"_fmt(UnitCatalogToml::KeyMagicDefense, entry.magicDefense);
            block += U"{} = {}\n"_fmt(UnitCatalogToml::KeySpeed, entry.speed);
            block += U"{} = {}\n"_fmt(UnitCatalogToml::KeyMove, entry.move);
            block += U"{} = {}\n"_fmt(UnitCatalogToml::KeyMaintainRange, entry.maintainRange);
            block += U"{} = {}\n"_fmt(UnitCatalogToml::KeyVisualScale, entry.visualScale);
            block += U"{} = [{}, {}]\n"_fmt(UnitCatalogToml::KeyVisualOffset, entry.visualOffset.x, entry.visualOffset.y);
            block += U"{} = [{}, {}]\n"_fmt(UnitCatalogToml::KeyShadowOffset, entry.shadowOffset.x, entry.shadowOffset.y);
            block += U"{} = \"{}\"\n"_fmt(UnitCatalogToml::KeyPlacementAnchor, UnitPlacementAnchorToTomlValue(entry.placementAnchor));
            block += U"{} = \"{}\"\n"_fmt(UnitCatalogToml::KeyRenderSizeMode, UnitRenderSizeModeToTomlValue(entry.renderSizeMode));
            block += U"{} = {}\n"_fmt(UnitCatalogToml::KeyGameplaySizeMul, entry.gameplaySizeMul);
            block += U"{} = \"{}\"\n"_fmt(UnitCatalogToml::KeyArtWidthRef, UnitArtWidthReferenceToTomlValue(entry.artWidthReference));
            block += U"{} = {}\n"_fmt(UnitCatalogToml::KeyArtWidthValue, entry.artWidthValue);
            block += U"{} = {}\n"_fmt(UnitCatalogToml::KeyArtWidthValueLineHorizontal, entry.artWidthValueLineHorizontal);
            block += U"{} = {}\n"_fmt(UnitCatalogToml::KeyArtWidthValueLineDiagUpRight, entry.artWidthValueLineDiagUpRight);
            block += U"{} = {}\n"_fmt(UnitCatalogToml::KeyArtWidthValueLineDiagUpLeft, entry.artWidthValueLineDiagUpLeft);
            block += U"{} = {}\n"_fmt(UnitCatalogToml::KeyArtKeepAspect, entry.artKeepAspect ? U"true" : U"false");
            block += U"{} = [{}, {}]\n"_fmt(UnitCatalogToml::KeyLineIconHorizontalOffset, entry.lineIconHorizontalOffset.x, entry.lineIconHorizontalOffset.y);
            block += U"{} = [{}, {}]\n"_fmt(UnitCatalogToml::KeyLineIconDiagUpRightOffset, entry.lineIconDiagUpRightOffset.x, entry.lineIconDiagUpRightOffset.y);
            block += U"{} = [{}, {}]\n\n"_fmt(UnitCatalogToml::KeyLineIconDiagUpLeftOffset, entry.lineIconDiagUpLeftOffset.x, entry.lineIconDiagUpLeftOffset.y);
            writer.write(block);
        }

        catalog.statusText = U"Saved unit catalog: {}"_fmt(catalog.sourcePath);
        statusText = catalog.statusText;
        return true;
    }
}
