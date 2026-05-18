#pragma once
# include <Siv3D.hpp>

namespace LT3
{
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
        String tag;
        String name;
        String kind;
        String race;
        String image;
        String classBuild;
        String classTag;
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

        for (const auto unitValue : toml[U"units"].tableArrayView())
        {
            UnitCatalogEntry entry;
            entry.tag = unitValue[U"tag"].getOr<String>(U"");
            entry.name = unitValue[U"name"].getOr<String>(entry.tag);
            entry.kind = unitValue[U"kind"].getOr<String>(U"unit");
            entry.race = unitValue[U"race"].getOr<String>(U"");
            entry.image = unitValue[U"image"].getOr<String>(U"");
            entry.classBuild = unitValue[U"class_build"].getOr<String>(U"");
            entry.classTag = unitValue[U"class_tag"].getOr<String>(U"");
            entry.skills = ReadTomlStringArray(unitValue[U"skills"]);
            entry.level = unitValue[U"level"].getOr<int32>(1);
            entry.levelMax = unitValue[U"level_max"].getOr<int32>(1);
            entry.visionRadius = unitValue[U"vision_radius"].getOr<int32>(0);
            entry.goldCost = unitValue[U"gold_cost"].getOr<int32>(unitValue[U"cost"].getOr<int32>(0));
            entry.trustCost = unitValue[U"trust_cost"].getOr<int32>(unitValue[U"price"].getOr<int32>(0));
            entry.foodCost = unitValue[U"food_cost"].getOr<int32>(0);
            entry.hp = unitValue[U"hp"].getOr<int32>(0);
            entry.buildingHp = unitValue[U"building_hp"].getOr<int32>(0);
            entry.mp = unitValue[U"mp"].getOr<int32>(0);
            entry.attack = unitValue[U"attack"].getOr<int32>(0);
            entry.defense = unitValue[U"defense"].getOr<int32>(0);
            entry.magic = unitValue[U"magic"].getOr<int32>(0);
            entry.magicDefense = unitValue[U"magic_defense"].getOr<int32>(0);
            entry.speed = unitValue[U"speed"].getOr<int32>(0);
            entry.move = unitValue[U"move"].getOr<int32>(0);
            entry.maintainRange = unitValue[U"maintain_range"].getOr<int32>(0);
            entry.visualScale = Clamp(unitValue[U"visual_scale"].getOr<double>(unitValue[U"scale"].getOr<double>(1.0)), 0.25, 3.0);
            entry.visualOffset = ReadTomlPointArray(unitValue[U"visual_offset"]);
            entry.shadowOffset = ReadTomlPointArray(unitValue[U"shadow_offset"]);
            entry.placementAnchor = ParseUnitPlacementAnchor(unitValue[U"placement_anchor"].getOr<String>(U""), entry.kind);
            entry.renderSizeMode = ParseUnitRenderSizeMode(unitValue[U"render_size_mode"].getOr<String>(U"gameplay"));
            if (const Optional<double> gameplaySizeMul = unitValue[U"gameplay_size_mul"].getOpt<double>())
            {
                entry.gameplaySizeMul = Clamp(*gameplaySizeMul, 0.2, 8.0);
            }
            else
            {
                entry.gameplaySizeMul = (entry.kind.lowercased() == U"building") ? 2.2 : 2.0;
            }
            entry.artWidthReference = ParseUnitArtWidthReference(unitValue[U"art_width_ref"].getOr<String>(U"cell"));
            entry.artWidthValue = Clamp(unitValue[U"art_width_value"].getOr<double>(1.0), 0.1, 8.0);
            entry.artWidthValueLineHorizontal = Clamp(unitValue[U"art_width_value_line_horizontal"].getOr<double>(entry.artWidthValue), 0.1, 8.0);
            entry.artWidthValueLineDiagUpRight = Clamp(unitValue[U"art_width_value_line_diag_up_right"].getOr<double>(entry.artWidthValue), 0.1, 8.0);
            entry.artWidthValueLineDiagUpLeft = Clamp(unitValue[U"art_width_value_line_diag_up_left"].getOr<double>(entry.artWidthValue), 0.1, 8.0);
            entry.artKeepAspect = unitValue[U"art_keep_aspect"].getOr<bool>(true);
            entry.lineIconHorizontalOffset = ReadTomlPointArray(unitValue[U"line_icon_horizontal_offset"]);
            entry.lineIconDiagUpRightOffset = ReadTomlPointArray(unitValue[U"line_icon_diag_up_right_offset"]);
            entry.lineIconDiagUpLeftOffset = ReadTomlPointArray(unitValue[U"line_icon_diag_up_left_offset"]);

            if (!entry.tag.isEmpty())
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

        for (const auto& entry : catalog.entries)
        {
            String block;
            block += U"[[units]]\n";
            block += U"tag = \"{}\"\n"_fmt(UnitCatalogTomlEscape(entry.tag));
            block += U"name = \"{}\"\n"_fmt(UnitCatalogTomlEscape(entry.name));
            block += U"kind = \"{}\"\n"_fmt(UnitCatalogTomlEscape(entry.kind));
            block += U"race = \"{}\"\n"_fmt(UnitCatalogTomlEscape(entry.race));
            block += U"image = \"{}\"\n"_fmt(UnitCatalogTomlEscape(entry.image));
            block += U"class_build = \"{}\"\n"_fmt(UnitCatalogTomlEscape(entry.classBuild));
            block += U"class_tag = \"{}\"\n"_fmt(UnitCatalogTomlEscape(entry.classTag));
            block += U"skills = {}\n"_fmt(BuildTomlStringArray(entry.skills));
            block += U"level = {}\n"_fmt(entry.level);
            block += U"level_max = {}\n"_fmt(entry.levelMax);
            block += U"vision_radius = {}\n"_fmt(entry.visionRadius);
            block += U"gold_cost = {}\n"_fmt(entry.goldCost);
            block += U"trust_cost = {}\n"_fmt(entry.trustCost);
            block += U"food_cost = {}\n"_fmt(entry.foodCost);
            block += U"hp = {}\n"_fmt(entry.hp);
            block += U"building_hp = {}\n"_fmt(entry.buildingHp);
            block += U"mp = {}\n"_fmt(entry.mp);
            block += U"attack = {}\n"_fmt(entry.attack);
            block += U"defense = {}\n"_fmt(entry.defense);
            block += U"magic = {}\n"_fmt(entry.magic);
            block += U"magic_defense = {}\n"_fmt(entry.magicDefense);
            block += U"speed = {}\n"_fmt(entry.speed);
            block += U"move = {}\n"_fmt(entry.move);
            block += U"maintain_range = {}\n"_fmt(entry.maintainRange);
            block += U"visual_scale = {}\n"_fmt(entry.visualScale);
            block += U"visual_offset = [{}, {}]\n"_fmt(entry.visualOffset.x, entry.visualOffset.y);
            block += U"shadow_offset = [{}, {}]\n"_fmt(entry.shadowOffset.x, entry.shadowOffset.y);
            block += U"placement_anchor = \"{}\"\n"_fmt(UnitPlacementAnchorToTomlValue(entry.placementAnchor));
            block += U"render_size_mode = \"{}\"\n"_fmt(UnitRenderSizeModeToTomlValue(entry.renderSizeMode));
            block += U"gameplay_size_mul = {}\n"_fmt(entry.gameplaySizeMul);
            block += U"art_width_ref = \"{}\"\n"_fmt(UnitArtWidthReferenceToTomlValue(entry.artWidthReference));
            block += U"art_width_value = {}\n"_fmt(entry.artWidthValue);
            block += U"art_width_value_line_horizontal = {}\n"_fmt(entry.artWidthValueLineHorizontal);
            block += U"art_width_value_line_diag_up_right = {}\n"_fmt(entry.artWidthValueLineDiagUpRight);
            block += U"art_width_value_line_diag_up_left = {}\n"_fmt(entry.artWidthValueLineDiagUpLeft);
            block += U"art_keep_aspect = {}\n"_fmt(entry.artKeepAspect ? U"true" : U"false");
            block += U"line_icon_horizontal_offset = [{}, {}]\n"_fmt(entry.lineIconHorizontalOffset.x, entry.lineIconHorizontalOffset.y);
            block += U"line_icon_diag_up_right_offset = [{}, {}]\n"_fmt(entry.lineIconDiagUpRightOffset.x, entry.lineIconDiagUpRightOffset.y);
            block += U"line_icon_diag_up_left_offset = [{}, {}]\n\n"_fmt(entry.lineIconDiagUpLeftOffset.x, entry.lineIconDiagUpLeftOffset.y);
            writer.write(block);
        }

        catalog.statusText = U"Saved unit catalog: {}"_fmt(catalog.sourcePath);
        statusText = catalog.statusText;
        return true;
    }
}
