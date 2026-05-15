#pragma once
# include <Siv3D.hpp>
# include "../Systems/BattleSystems.h"
# include "../Data/BattleAssetPaths.h"
# include "BattleUnitRenderer.h"
# include "QuarterView.h"

namespace LT3
{
    inline bool DrawResourceIcon(const ResourceDef& def, const BattleRenderAssets& assets, const Vec2& center, double size)
    {
        if (def.icon.isEmpty())
        {
            return false;
        }

        const FilePath iconPath = ResolveSystemImagePath(def.icon);
        if (!FileSystem::Exists(iconPath))
        {
            return false;
        }

        if (!assets.resourceTextureCache.contains(iconPath))
        {
            assets.resourceTextureCache.emplace(iconPath, Texture{ iconPath });
        }

        assets.resourceTextureCache.at(iconPath).resized(size, size).drawAt(center);
        return true;
    }

    inline void DrawResourceNodes(const BattleWorld& world, const DefinitionStores& defs, const BattleRenderAssets& assets, const Font& uiFont)
    {
        for (size_t i = 0; i < world.resourceNodes.position.size(); ++i)
        {
            const Vec2 pos = QuarterTileFaceCenterScreen(world.resourceNodes.position[i]);
            const bool depleted = world.resourceNodes.amount[i] <= 0;
            const ResourceDef& def = defs.resources[world.resourceNodes.defId[i]];
            ColorF color = def.color;
            ColorF textColor{ 1.0, 1.0, 1.0 };
            if (depleted)
            {
                color = ColorF{ 0.3, 0.25, 0.08 };
                textColor = ColorF{ 0.5, 0.5, 0.5 };
            }
            Ellipse{ pos + Vec2{ 0, 6 }, 30, 15 }.draw(ColorF{ 0, 0, 0, 0.25 });
            Circle{ pos, 22 }.draw(color);
            Circle{ pos, 22 }.drawFrame(2, ColorF{ 0.25, 0.18, 0.02 });
            if (!depleted)
            {
                DrawResourceIcon(def, assets, pos, 30.0);
            }

            const double captureRate = Clamp(world.resourceNodes.captureProgress[i], 0.0, 1.0);
            if (!depleted && captureRate > 0.0 && captureRate < 1.0)
            {
                const RectF gaugeBack{ Arg::center = pos + Vec2{ 0, -32 }, 44.0, 6.0 };
                gaugeBack.draw(ColorF{ 0.04, 0.04, 0.04, 0.85 });

                ColorF gaugeColor{ 0.78, 0.78, 0.78 };
                if (world.resourceNodes.owner[i] == Faction::Player)
                {
                    gaugeColor = ColorF{ 0.30, 0.80, 1.0 };
                }
                else if (world.resourceNodes.owner[i] == Faction::Enemy)
                {
                    gaugeColor = ColorF{ 1.0, 0.42, 0.42 };
                }

                RectF{ gaugeBack.pos, gaugeBack.w * captureRate, gaugeBack.h }.draw(gaugeColor);
                gaugeBack.drawFrame(1.0, ColorF{ 1.0, 1.0, 1.0, 0.22 });

                const double remainSec = Max(0.0, (1.0 - captureRate) * 1.5);
                uiFont(U"{:.1f}s"_fmt(remainSec)).drawAt(11, pos + Vec2{ 0, -44 }, Palette::White);
            }

            uiFont(U"{}"_fmt(world.resourceNodes.amount[i])).drawAt(13, pos + Vec2{ 0, 34 }, textColor);
        }
    }

    inline void DrawResourcePanel(const BattleWorld& world, const DefinitionStores& defs, const BattleRenderAssets& assets, const Font& uiFont)
    {
        const double rowHeight = 22.0;
        const double panelHeight = 42.0 + rowHeight * Max<size_t>(1, defs.resources.size());
        const RectF panel{ 1288.0, 72.0, 282.0, panelHeight };
        panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.82 }).drawFrame(1, ColorF{ 1, 1, 1, 0.18 });

        uiFont(U"資源").draw(16, panel.x + 16.0, panel.y + 10.0, Palette::White);

        for (ResourceDefId resourceId = 0; resourceId < defs.resources.size(); ++resourceId)
        {
            const ResourceDef& def = defs.resources[resourceId];
            const double rowY = panel.y + 38.0 + static_cast<double>(resourceId) * rowHeight;
            const int32 amount = GetPlayerResourceAmount(world, resourceId);

            const Vec2 iconCenter{ panel.x + 30.0, rowY + 10.0 };
            Circle{ iconCenter, 9.0 }.draw(def.color).drawFrame(1.0, ColorF{ 0.25, 0.18, 0.02 });
            DrawResourceIcon(def, assets, iconCenter, 18.0);
            uiFont(U"{}: {}"_fmt(def.name, amount)).draw(16, panel.x + 48.0, rowY, Palette::White);
        }
    }
}
