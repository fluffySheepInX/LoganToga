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
        const FilePath fromApp = U"000_Warehouse/000_DefaultGame/070_Scenario/InfoUnit/UnitSample.toml";
        if (FileSystem::Exists(fromApp))
        {
            return fromApp;
        }

        const FilePath fromRepo = U"App/000_Warehouse/000_DefaultGame/070_Scenario/InfoUnit/UnitSample.toml";
        if (FileSystem::Exists(fromRepo))
        {
            return fromRepo;
        }

        return fromApp;
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

    inline UnitCatalog LoadUnitCatalog()
    {
        UnitCatalog catalog;
        catalog.sourcePath = ResolveUnitCatalogTomlPath();

        const TOMLReader toml{ catalog.sourcePath };
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
        TextWriter writer{ catalog.sourcePath };
        if (!writer)
        {
            statusText = U"Unit catalog save failed: {}"_fmt(catalog.sourcePath);
            return false;
        }

        for (const auto& entry : catalog.entries)
        {
            writer << U"[[units]]\n";
            writer << U"tag = \"" << UnitCatalogTomlEscape(entry.tag) << U"\"\n";
            writer << U"name = \"" << UnitCatalogTomlEscape(entry.name) << U"\"\n";
            writer << U"kind = \"" << UnitCatalogTomlEscape(entry.kind) << U"\"\n";
            writer << U"race = \"" << UnitCatalogTomlEscape(entry.race) << U"\"\n";
            writer << U"image = \"" << UnitCatalogTomlEscape(entry.image) << U"\"\n";
            writer << U"class_build = \"" << UnitCatalogTomlEscape(entry.classBuild) << U"\"\n";
            writer << U"class_tag = \"" << UnitCatalogTomlEscape(entry.classTag) << U"\"\n";
            writer << U"skills = ";
            WriteTomlStringArray(writer, entry.skills);
            writer << U"\n";
            writer << U"level = " << entry.level << U"\n";
            writer << U"level_max = " << entry.levelMax << U"\n";
            writer << U"vision_radius = " << entry.visionRadius << U"\n";
            writer << U"price = " << entry.price << U"\n";
            writer << U"cost = " << entry.cost << U"\n";
            writer << U"hp = " << entry.hp << U"\n";
            writer << U"building_hp = " << entry.buildingHp << U"\n";
            writer << U"mp = " << entry.mp << U"\n";
            writer << U"attack = " << entry.attack << U"\n";
            writer << U"defense = " << entry.defense << U"\n";
            writer << U"magic = " << entry.magic << U"\n";
            writer << U"magic_defense = " << entry.magicDefense << U"\n";
            writer << U"speed = " << entry.speed << U"\n";
            writer << U"move = " << entry.move << U"\n";
            writer << U"maintain_range = " << entry.maintainRange << U"\n";
            writer << U"visual_scale = " << entry.visualScale << U"\n\n";
        }

        catalog.statusText = U"Saved unit catalog: {}"_fmt(catalog.sourcePath);
        statusText = catalog.statusText;
        return true;
    }
}
