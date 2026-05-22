#pragma once
# include <Siv3D.hpp>
# include "../DefinitionStores.h"
# include "../TomlTextUtils.h"

namespace LT3
{
    namespace ResourceToml
    {
        inline constexpr auto KeyResources = U"Map001";
        inline constexpr auto KeyKind = U"kind";
        inline constexpr auto KeyId = U"id";
        inline constexpr auto KeyName = U"name";
        inline constexpr auto KeyIcon = U"icon";
    }

    inline FilePath ResolveResourceTomlPath()
    {
        return ResolveFirstExistingPath({
            U"000_Warehouse/000_DefaultGame/070_Scenario/InfoResource/aaa.toml",
            U"App/000_Warehouse/000_DefaultGame/070_Scenario/InfoResource/aaa.toml",
        });
    }

    inline ResourceKind ParseResourceKind(const String& value)
    {
        const String lowered = value.lowercased();
        if (lowered == U"trust")
        {
            return ResourceKind::Trust;
        }
        if (lowered == U"food")
        {
            return ResourceKind::Food;
        }
        return ResourceKind::Gold;
    }

    inline ColorF ResolveResourceColor(ResourceKind kind)
    {
        switch (kind)
        {
        case ResourceKind::Trust:
            return Palette::Violet;
        case ResourceKind::Food:
            return Palette::Yellowgreen;
        default:
            return Palette::Gold;
        }
    }

    inline String ResolveResourceIcon(ResourceKind kind, const String& configuredIcon)
    {
        if (!configuredIcon.isEmpty())
        {
            return configuredIcon;
        }

        switch (kind)
        {
        case ResourceKind::Trust:
            return U"trust.png";
        case ResourceKind::Food:
            return U"food.png";
        default:
            return U"gold.png";
        }
    }

    inline void LoadResourceDefinitions(DefinitionStores& defs)
    {
        defs.resources.clear();
        defs.resourceByTag.clear();

        const FilePath resourcePath = ResolveResourceTomlPath();
        const TOMLReader toml{ resourcePath };
        if (!toml)
        {
            defs.addResource({ U"gold", U"000", U"Gold", U"resource-gold.png", ResourceKind::Gold, Palette::Gold, 110, 10 });
            defs.addResource({ U"trust", U"002", U"Trust", U"resource-trust.png", ResourceKind::Trust, Palette::Violet, 0, 0 });
            defs.addResource({ U"food", U"004", U"Food", U"resource-food.png", ResourceKind::Food, Palette::Yellowgreen, 0, 0 });
            return;
        }

        HashSet<String> loadedTags;
        for (const auto resourceValue : toml[ResourceToml::KeyResources].tableArrayView())
        {
            const String tag = resourceValue[ResourceToml::KeyKind].getOr<String>(U"").lowercased();
            if (tag.isEmpty() || loadedTags.contains(tag))
            {
                continue;
            }

            const ResourceKind kind = ParseResourceKind(tag);
            const int32 passiveIncomePerSec = (kind == ResourceKind::Gold) ? 10 : 0;
            defs.addResource({
                tag,
                resourceValue[ResourceToml::KeyId].getOr<String>(U""),
                resourceValue[ResourceToml::KeyName].getOr<String>(tag),
                ResolveResourceIcon(kind, resourceValue[ResourceToml::KeyIcon].getOr<String>(U"")),
                kind,
                ResolveResourceColor(kind),
                (kind == ResourceKind::Gold) ? 110 : 0,
                passiveIncomePerSec,
            });
            loadedTags.insert(tag);
        }
    }
}
