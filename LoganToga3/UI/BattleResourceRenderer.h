#pragma once
# include <Siv3D.hpp>
# include "../Systems/BattleSystems.h"
# include "../Data/BattleAssetPaths.h"
# include "BattleUnitRenderer.h"
# include "QuarterView.h"
# include "MapEditorTypes.h"
# include "MapEditorLayoutRects.h"
# include "../App/AppRuntimeState.h"

namespace LT3
{
    inline constexpr double ResourceFlagRaiseDurationRenderSec = 1.2;

    inline void DrawResourcePanelUiLayoutDragHandle(const RectF& panelRect, bool active)
    {
        if (!active)
        {
            return;
        }

        const RectF handle{ panelRect.x + panelRect.w - 24.0, panelRect.y + 6.0, 18.0, 18.0 };
        handle.draw(ColorF{ 1.0, 0.84, 0.0, 0.18 }).drawFrame(2.0, ColorF{ 1.0, 0.84, 0.0, 0.90 });
    }

    inline void DrawResourcePanelTopAnchorToggle(const RectF& panelRect, bool active, bool topAnchor)
    {
        if (!active)
        {
            return;
        }

        const RectF toggleRect{ panelRect.x + panelRect.w - 48.0, panelRect.y + 6.0, 18.0, 18.0 };
        toggleRect.draw(topAnchor ? ColorF{ 0.16, 0.24, 0.18, 0.95 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
            .drawFrame(2.0, toggleRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.18 });

        const Vec2 center = toggleRect.center();
        Line{ center.x - 5.0, center.y - 4.0, center.x + 5.0, center.y - 4.0 }.draw(2.0, topAnchor ? Palette::White : Palette::Lightgray);
        Triangle{ Vec2{ center.x, center.y - 7.0 }, Vec2{ center.x - 4.0, center.y }, Vec2{ center.x + 4.0, center.y } }.draw(topAnchor ? Palette::White : Palette::Lightgray);
    }

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

    inline FilePath ResolveResourceFlagPolePath()
    {
        return ResolveSystemImagePath(U"flagP.png");
    }

    inline FilePath ResolveResourceFlagGifPath(Faction faction)
    {
        switch (faction)
        {
        case Faction::Player:
            return ResolveSystemImagePath(U"flagP.gif");
        case Faction::Enemy:
            return ResolveSystemImagePath(U"enemy_flagP.gif");
        default:
            return FilePath{};
        }
    }

    inline bool HasResourceFlagAnimationAsset(Faction faction)
    {
        const FilePath gifPath = ResolveResourceFlagGifPath(faction);
        return (!gifPath.isEmpty() && FileSystem::Exists(gifPath));
    }

    inline const Texture* ResolveResourceFlagPoleTexture(const BattleRenderAssets& assets)
    {
        const FilePath polePath = ResolveResourceFlagPolePath();
        if (polePath.isEmpty() || !FileSystem::Exists(polePath))
        {
            return nullptr;
        }

        if (!assets.resourceTextureCache.contains(polePath))
        {
            assets.resourceTextureCache.emplace(polePath, Texture{ polePath });
        }

        return &assets.resourceTextureCache.at(polePath);
    }

    inline const Texture* ResolveResourceFlagGifFrame(const BattleRenderAssets& assets, Faction faction, bool animateGif)
    {
        const FilePath gifPath = ResolveResourceFlagGifPath(faction);
        if (gifPath.isEmpty() || !FileSystem::Exists(gifPath))
        {
            return nullptr;
        }

        if (!assets.unitGifDurationMillisecCache.contains(gifPath))
        {
            Array<Texture> frames;
            Array<int32> delaysMillisec;
            int32 durationMillisec = 0;
            AnimatedGIFReader reader{ gifPath };
            Array<Image> images;
            if (reader && reader.read(images, delaysMillisec, durationMillisec) && !images.isEmpty())
            {
                frames.reserve(images.size());
                for (const auto& image : images)
                {
                    frames << Texture{ image };
                }
            }

            assets.unitGifFrameCache.emplace(gifPath, std::move(frames));
            assets.unitGifFrameDelaysMillisecCache.emplace(gifPath, std::move(delaysMillisec));
            assets.unitGifDurationMillisecCache.emplace(gifPath, Max(durationMillisec, 1));
        }

        if (!assets.unitGifFrameCache.contains(gifPath))
        {
            return nullptr;
        }

        const Array<Texture>& frames = assets.unitGifFrameCache.at(gifPath);
        if (frames.isEmpty())
        {
            return nullptr;
        }

        if (!animateGif)
        {
            return &frames.front();
        }

        if (assets.unitGifFrameDelaysMillisecCache.contains(gifPath)
            && assets.unitGifDurationMillisecCache.contains(gifPath))
        {
            const Array<int32>& delaysMillisec = assets.unitGifFrameDelaysMillisecCache.at(gifPath);
            const int32 durationMillisec = assets.unitGifDurationMillisecCache.at(gifPath);
            if (!delaysMillisec.isEmpty() && durationMillisec > 0)
            {
                const size_t frameIndex = AnimatedGIFReader::GetFrameIndex(Scene::Time(), delaysMillisec, durationMillisec);
                return &frames[Min(frameIndex, frames.size() - 1)];
            }
        }

        return &frames.front();
    }

    inline void DrawResourceNodeFlag(const Vec2& pos, const ResourceNodeFlagState& flagState, const BattleRenderAssets& assets)
    {
        if (flagState.displayFaction == Faction::Neutral || !HasResourceFlagAnimationAsset(flagState.displayFaction))
        {
            return;
        }

        const Texture* poleTexture = ResolveResourceFlagPoleTexture(assets);
        const Texture* flagTexture = ResolveResourceFlagGifFrame(assets, flagState.displayFaction, true);
        if (!poleTexture || !flagTexture)
        {
            return;
        }

        const double poleHeight = poleTexture->height() * 3.0;
        const double poleWidth = static_cast<double>(poleTexture->width());
        const Vec2 poleBottom{ pos.x + 20.0, pos.y + 18.0 };
        const Vec2 poleCenter{ poleBottom.x, poleBottom.y - (poleHeight * 0.5) };
        poleTexture->resized(poleWidth, poleHeight).drawAt(poleCenter);

        const double flagScale = 1.0;
        const double flagWidth = Max(1.0, static_cast<double>(flagTexture->width()) * flagScale);
        const double flagHeight = Max(1.0, static_cast<double>(flagTexture->height()) * flagScale);
        const double topY = poleBottom.y - poleHeight + (flagHeight * 0.62);
        const double bottomY = poleBottom.y - flagHeight * 0.35;
        const double t = flagState.raising
            ? Clamp(flagState.raiseTimerSec / ResourceFlagRaiseDurationRenderSec, 0.0, 1.0)
            : 1.0;
        const double currentY = Math::Lerp(bottomY, topY, t);
        const Vec2 flagCenter{ poleCenter.x, currentY };
        flagTexture->resized(flagWidth, flagHeight).drawAt(flagCenter);
    }

    inline void DrawResourceNodeOverlay(const BattleWorld& world, const DefinitionStores& defs, const BattleRenderAssets& assets, const Font& uiFont, size_t index, const ResourceFlagRuntimeState* resourceFlags = nullptr)
    {
        const Vec2 pos = QuarterTileFaceCenterScreen(world.resourceNodes.position[index]);
        const bool depleted = world.resourceNodes.amount[index] <= 0;
        const ResourceDef& def = defs.resources[world.resourceNodes.defId[index]];
        ColorF textColor{ 1.0, 1.0, 1.0 };
        if (depleted)
        {
            textColor = ColorF{ 0.5, 0.5, 0.5 };
        }

        if (!depleted)
        {
            DrawResourceIcon(def, assets, pos, 30.0);
        }

        if (resourceFlags && index < resourceFlags->nodes.size())
        {
            const ResourceNodeFlagState& flagState = resourceFlags->nodes[index];
            if (flagState.raising || flagState.visible)
            {
                DrawResourceNodeFlag(pos, flagState, assets);
            }
        }

        const double captureRate = Clamp(world.resourceNodes.captureProgress[index], 0.0, 1.0);
        if (!depleted && captureRate > 0.0 && captureRate < 1.0)
        {
            const RectF gaugeBack{ Arg::center = pos + Vec2{ 0, -32 }, 44.0, 6.0 };
            gaugeBack.draw(ColorF{ 0.04, 0.04, 0.04, 0.85 });

            ColorF gaugeColor{ 0.78, 0.78, 0.78 };
            if (world.resourceNodes.owner[index] == Faction::Player)
            {
                gaugeColor = ColorF{ 0.30, 0.80, 1.0 };
            }
            else if (world.resourceNodes.owner[index] == Faction::Enemy)
            {
                gaugeColor = ColorF{ 1.0, 0.42, 0.42 };
            }

            RectF{ gaugeBack.pos, gaugeBack.w * captureRate, gaugeBack.h }.draw(gaugeColor);
            gaugeBack.drawFrame(1.0, ColorF{ 1.0, 1.0, 1.0, 0.22 });

            const double remainSec = Max(0.0, (1.0 - captureRate) * 1.5);
            uiFont(U"{:.1f}s"_fmt(remainSec)).drawAt(11, pos + Vec2{ 0, -44 }, Palette::White);
        }

        uiFont(U"{}"_fmt(world.resourceNodes.amount[index])).drawAt(13, pos + Vec2{ 0, 34 }, textColor);
    }

    inline void DrawResourceNodes(const BattleWorld& world, const DefinitionStores& defs, const BattleRenderAssets& assets, const Font& uiFont, const ResourceFlagRuntimeState* resourceFlags = nullptr)
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
        }
    }

    inline void DrawResourceNodeOverlays(const BattleWorld& world, const DefinitionStores& defs, const BattleRenderAssets& assets, const Font& uiFont, const ResourceFlagRuntimeState* resourceFlags = nullptr)
    {
        for (size_t i = 0; i < world.resourceNodes.position.size(); ++i)
        {
            DrawResourceNodeOverlay(world, defs, assets, uiFont, i, resourceFlags);
        }
    }

    inline void DrawResourcePanel(const BattleWorld& world, const DefinitionStores& defs, const BattleRenderAssets& assets, const Font& uiFont)
    {
        const RectF panel{ 1288.0, 72.0, 282.0, 42.0 + 22.0 * Max<size_t>(1, defs.resources.size()) };
        panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.82 }).drawFrame(1, ColorF{ 1, 1, 1, 0.18 });

        uiFont(U"資源").draw(16, panel.x + 16.0, panel.y + 10.0, Palette::White);

        for (ResourceDefId resourceId = 0; resourceId < defs.resources.size(); ++resourceId)
        {
            const ResourceDef& def = defs.resources[resourceId];
            const double rowY = panel.y + 38.0 + static_cast<double>(resourceId) * 22.0;
            const int32 amount = GetPlayerResourceAmount(world, resourceId);

            const Vec2 iconCenter{ panel.x + 30.0, rowY + 10.0 };
            Circle{ iconCenter, 9.0 }.draw(def.color).drawFrame(1.0, ColorF{ 0.25, 0.18, 0.02 });
            DrawResourceIcon(def, assets, iconCenter, 18.0);
            uiFont(U"{}: {}"_fmt(def.name, amount)).draw(16, panel.x + 48.0, rowY, Palette::White);
        }
    }

    inline void DrawResourcePanel(const BattleWorld& world, const DefinitionStores& defs, const BattleRenderAssets& assets, const Font& uiFont, const MapEditorState& mapEditor)
    {
        const RectF panel = BattleResourcePanelRect(mapEditor, defs.resources.size());
        panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.82 }).drawFrame(1, ColorF{ 1, 1, 1, 0.18 });
        DrawResourcePanelUiLayoutDragHandle(panel, mapEditor.uiLayoutEditEnabled);
        DrawResourcePanelTopAnchorToggle(panel, mapEditor.uiLayoutEditEnabled, mapEditor.uiResourcePanelTopAnchor);

        uiFont(U"資源").draw(16, panel.x + 16.0, panel.y + 10.0, Palette::White);

        for (ResourceDefId resourceId = 0; resourceId < defs.resources.size(); ++resourceId)
        {
            const ResourceDef& def = defs.resources[resourceId];
            const double rowY = panel.y + 38.0 + static_cast<double>(resourceId) * 22.0;
            const int32 amount = GetPlayerResourceAmount(world, resourceId);

            const Vec2 iconCenter{ panel.x + 30.0, rowY + 10.0 };
            Circle{ iconCenter, 9.0 }.draw(def.color).drawFrame(1.0, ColorF{ 0.25, 0.18, 0.02 });
            DrawResourceIcon(def, assets, iconCenter, 18.0);
            uiFont(U"{}: {}"_fmt(def.name, amount)).draw(16, panel.x + 48.0, rowY, Palette::White);
        }
    }
}
