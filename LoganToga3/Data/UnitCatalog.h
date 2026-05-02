#pragma once
# include <Siv3D.hpp>

namespace LT3
{
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
        int32 price = 0;
        int32 cost = 0;
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
            entry.price = unitValue[U"price"].getOr<int32>(0);
            entry.cost = unitValue[U"cost"].getOr<int32>(0);
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
            block += U"price = {}\n"_fmt(entry.price);
            block += U"cost = {}\n"_fmt(entry.cost);
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
            block += U"visual_scale = {}\n\n"_fmt(entry.visualScale);
            writer.write(block);
        }

        catalog.statusText = U"Saved unit catalog: {}"_fmt(catalog.sourcePath);
        statusText = catalog.statusText;
        return true;
    }
}
