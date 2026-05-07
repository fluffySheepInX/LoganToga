# include "../stdafx.h"
# include "RoadEditor.hpp"

const road::PlacementAsset* RoadEditor::findActivePlacementAsset() const{
        const road::PlacementCategory activeCategory = road::PlacementCategoryFromIndex(m_placementSettings.activeCategoryIndex);

        for (const auto& asset : m_placementAssets)
        {
            if ((asset.category == activeCategory) && (asset.id == m_placementSettings.activeAssetId))
            {
                return &asset;
            }
        }

        for (const auto& asset : m_placementAssets)
        {
            if (asset.category == activeCategory)
            {
                return &asset;
            }
        }

        return (m_placementAssets.isEmpty() ? nullptr : &m_placementAssets.front());
    }



Optional<size_t> RoadEditor::findHoveredScatterItemIndex() const{
        if (not m_hoverPoint)
        {
            return none;
        }

        Optional<size_t> result;
        double nearestDistance = 0.65;

        for (size_t i = 0; i < m_scatterItems.size(); ++i)
        {
            const double distance = m_scatterItems[i].position.distanceFrom(*m_hoverPoint);
            if (distance <= nearestDistance)
            {
                nearestDistance = distance;
                result = i;
            }
        }

        return result;
    }



    void RoadEditor::updateScatterInteraction(){
        m_hoverScatterItemIndex = findHoveredScatterItemIndex();

        if (isCursorOnUI() || (not m_hoverPoint))
        {
            return;
        }

        const auto mode = road::PlacementModeFromIndex(m_placementSettings.activeModeIndex);
        if (not MouseL.down())
        {
            return;
        }

        if (mode == road::PlacementMode::Erase)
        {
            if (m_hoverScatterItemIndex)
            {
                m_scatterItems.remove_at(*m_hoverScatterItemIndex);
                m_hoverScatterItemIndex.reset();
                m_statusMessage = U"Scatter item erased";
            }
            return;
        }

        if (mode != road::PlacementMode::Single)
        {
            m_statusMessage = U"Brush/Select not implemented yet";
            return;
        }

        if (const auto context = evaluateBoundaryContext(*m_hoverPoint))
        {
            const auto profile = road::EvaluatePlacementDensityProfile(*context, m_placementSettings);
            const road::PlacementCategory category = road::PlacementCategoryFromIndex(m_placementSettings.activeCategoryIndex);
            double acceptance = 0.0;
            switch (category)
            {
            case road::PlacementCategory::Grass:
                acceptance = profile.grassDensity;
                break;
            case road::PlacementCategory::Pebble:
                acceptance = profile.pebbleDensity;
                break;
            case road::PlacementCategory::DirtDecal:
                acceptance = profile.mudDensity;
                break;
            case road::PlacementCategory::Prop:
                acceptance = Max(profile.pebbleDensity * 0.5, profile.grassDensity * 0.35);
                break;
            }

            if (acceptance < 0.08)
            {
                m_statusMessage = U"Placement rejected: low boundary density";
                return;
            }
        }

        const road::PlacementAsset* asset = findActivePlacementAsset();
        if (not asset)
        {
            m_statusMessage = U"No placement asset";
            return;
        }

        const double scale = Random(asset->defaultScaleMin, asset->defaultScaleMax);
        const double yaw = (asset->randomYaw ? Random(Math::TwoPi) : 0.0);
        m_scatterItems << road::PlacedScatterItem{
            .assetId = asset->id,
            .category = asset->category,
            .position = *m_hoverPoint,
            .yawRadians = yaw,
            .scale = scale,
            .tint = ColorF{ 1.0 }
        };

        m_statusMessage = U"Scatter item placed";
    }



    void RoadEditor::drawScatterItems3D() const{
        for (size_t i = 0; i < m_scatterItems.size(); ++i)
        {
            const auto& item = m_scatterItems[i];
            const bool hovered = (m_hoverScatterItemIndex && (*m_hoverScatterItemIndex == i));
            const double radius = 0.09 * item.scale;

            if (const auto modelIt = m_assetModelCache.find(item.assetId); modelIt != m_assetModelCache.end())
            {
                const Quaternion rot = Quaternion::RotateY(item.yawRadians);
                modelIt->second.draw(item.position, rot);
                continue;
            }

            if (const auto texIt = m_assetTextureCache.find(item.assetId); texIt != m_assetTextureCache.end())
            {
                Plane{ item.position + Vec3{ 0, 0.004, 0 }, radius * 3.0 }.draw(texIt->second, hovered ? ColorF{ 1.0, 1.0, 1.0, 0.9 } : ColorF{ 1.0, 1.0, 1.0, 0.7 });
                continue;
            }

            switch (item.category)
            {
            case road::PlacementCategory::Grass:
                Cylinder{ item.position + Vec3{ 0, radius * 0.9, 0 }, radius * 0.45, radius * 2.2 }.draw(hovered ? ColorF{ 0.42, 0.88, 0.45 } : ColorF{ 0.34, 0.72, 0.31 });
                break;
            case road::PlacementCategory::Pebble:
                Sphere{ item.position + Vec3{ 0, radius * 0.55, 0 }, radius * 0.75 }.draw(hovered ? ColorF{ 0.82, 0.78, 0.66 } : ColorF{ 0.66, 0.61, 0.52 });
                break;
            case road::PlacementCategory::DirtDecal:
                Cylinder{ item.position + Vec3{ 0, 0.004, 0 }, radius * 1.7, 0.008 }.draw(hovered ? ColorF{ 0.34, 0.23, 0.16, 0.80 } : ColorF{ 0.26, 0.20, 0.15, 0.56 });
                break;
            case road::PlacementCategory::Prop:
                Box{ item.position + Vec3{ 0, radius, 0 }, radius * 1.4 }.draw(hovered ? ColorF{ 0.72, 0.72, 0.76 } : ColorF{ 0.52, 0.52, 0.57 });
                break;
            }
        }
    }



    void RoadEditor::drawScatterHoverGuide3D() const{
        if (not m_hoverPoint)
        {
            return;
        }

        const auto mode = road::PlacementModeFromIndex(m_placementSettings.activeModeIndex);
        const ColorF ringColor = (mode == road::PlacementMode::Erase ? ColorF{ 0.92, 0.34, 0.34, 0.75 } : ColorF{ 0.30, 0.80, 0.95, 0.75 });
        Cylinder{ *m_hoverPoint + Vec3{ 0, 0.004, 0 }, 0.35, 0.008 }.draw(ringColor);

        if (m_hoverScatterItemIndex)
        {
            const auto& item = m_scatterItems[*m_hoverScatterItemIndex];
            Sphere{ item.position + Vec3{ 0, 0.20, 0 }, 0.15 }.draw(ColorF{ 1.0, 0.92, 0.28, 0.85 });
        }
    }



Optional<road::RoadBoundaryContext> RoadEditor::evaluateBoundaryContext(const Vec3& point) const{
        return road::EvaluateRoadBoundaryContext(point, m_roads, m_intersectionClusters);
    }



    void RoadEditor::loadPlacementAssets(){
        road::LoadPlacementAssetsFromToml(m_assetCatalogPath, m_placementAssets);
        m_assetModelCache.clear();
        m_assetTextureCache.clear();

        for (const auto& asset : m_placementAssets)
        {
            if (asset.resourcePath.isEmpty())
            {
                continue;
            }

            if (asset.renderType == road::PlacementRenderType::Model)
            {
                if (const Model model{ asset.resourcePath })
                {
                    Model::RegisterDiffuseTextures(model, TextureDesc::MippedSRGB);
                    m_assetModelCache.emplace(asset.id, model);
                }
            }
            else
            {
                if (const Texture texture{ asset.resourcePath, TextureDesc::MippedSRGB })
                {
                    m_assetTextureCache.emplace(asset.id, texture);
                }
            }
        }
    }
