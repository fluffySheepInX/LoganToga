#pragma once
# include <Siv3D.hpp>
# include "../DefinitionStores.h"
# include "../TomlTextUtils.h"

namespace LT3
{
    namespace ResourceToml
    {
        inline constexpr auto KeyResources = U"resources";
        inline constexpr auto KeyLegacyResources = U"Map001";
        inline constexpr auto KeyTag = U"tag";
        inline constexpr auto KeyKind = U"kind";
        inline constexpr auto KeyId = U"id";
        inline constexpr auto KeyName = U"name";
        inline constexpr auto KeyIcon = U"icon";
        inline constexpr auto KeyInitialAmount = U"initial_amount";
        inline constexpr auto KeyPassiveIncomePerSec = U"passive_income_per_sec";
    }

    inline FilePath ResolveResourceTomlPath()
    {
        return ResolveFirstExistingPath({
            U"000_Warehouse/000_DefaultGame/070_Scenario/InfoResource/Resources.toml",
            U"App/000_Warehouse/000_DefaultGame/070_Scenario/InfoResource/Resources.toml",
        });
    }

    inline int32 DefaultInitialResourceAmount(ResourceKind kind)
    {
        return (kind == ResourceKind::Gold) ? 110 : 0;
    }

    inline int32 DefaultPassiveIncomePerSec(ResourceKind kind)
    {
        return (kind == ResourceKind::Gold) ? 10 : 0;
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

        const TOMLValue resourcesValue = toml[ResourceToml::KeyResources].isEmpty()
            ? toml[ResourceToml::KeyLegacyResources]
            : toml[ResourceToml::KeyResources];

        HashSet<String> loadedTags;
        for (const auto resourceValue : resourcesValue.tableArrayView())
        {
            const String tag = resourceValue[ResourceToml::KeyTag].getOr<String>(
                resourceValue[ResourceToml::KeyKind].getOr<String>(U""))
                .lowercased();
            if (tag.isEmpty() || loadedTags.contains(tag))
            {
                continue;
            }

            const String kindText = resourceValue[ResourceToml::KeyKind].getOr<String>(tag).lowercased();
            const ResourceKind kind = ParseResourceKind(kindText);
            defs.addResource({
                tag,
                resourceValue[ResourceToml::KeyId].getOr<String>(U""),
                resourceValue[ResourceToml::KeyName].getOr<String>(tag),
                ResolveResourceIcon(kind, resourceValue[ResourceToml::KeyIcon].getOr<String>(U"")),
                kind,
                ResolveResourceColor(kind),
                Max(0, resourceValue[ResourceToml::KeyInitialAmount].getOr<int32>(DefaultInitialResourceAmount(kind))),
                Max(0, resourceValue[ResourceToml::KeyPassiveIncomePerSec].getOr<int32>(DefaultPassiveIncomePerSec(kind))),
            });
            loadedTags.insert(tag);
        }
    }
}
