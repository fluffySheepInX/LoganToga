# include "../stdafx.h"
# include "TextureEditor.hpp"

Optional<Vec2> TextureEditor::cursorToGround(const BasicCamera3D& camera) const{
        const Ray ray = camera.screenToRay(Cursor::PosF());
        const InfinitePlane groundPlane3D{ Float3{ 0, 0, 0 }, Float3{ 0, 1, 0 } };

        if (const auto distance = ray.intersects(groundPlane3D))
        {
            const Vec3 hitPos = ray.point_at(*distance);
            return Vec2{ hitPos.x, hitPos.z };
        }

        return none;
    }



bool TextureEditor::hasSelectedLayer() const{
        return (not m_layers.isEmpty()) && (m_selectedLayerIndex < m_layers.size());
    }



    void TextureEditor::applyPlacement(const Vec2& groundPos, const StringView source){
        if (not hasSelectedLayer())
        {
            m_lastPlacementReason = U"blocked: no selected layer";
            return;
        }

        m_layers[m_selectedLayerIndex].position = groundPos;
        markLayerDirty(m_selectedLayerIndex);
        m_placeAtClickRequested = false;
        m_lastPlacementApplied = true;
        m_lastPlacementReason = U"applied via {}"_fmt(source);
        m_statusMessage = U"Placed texture at ({:.2f}, {:.2f}) via {}"_fmt(groundPos.x, groundPos.y, source);
    }



String TextureEditor::buildPlacementDiagnostics() const{
        const String selectedLayer = hasSelectedLayer()
            ? U"{} (# {})"_fmt(m_layers[m_selectedLayerIndex].label, m_selectedLayerIndex)
            : U"(none)";
        const String groundText = m_lastCursorGroundPos
            ? U"({:.3f}, {:.3f})"_fmt(m_lastCursorGroundPos->x, m_lastCursorGroundPos->y)
            : U"none";
        const String layerPosText = hasSelectedLayer()
            ? U"({:.3f}, {:.3f})"_fmt(m_layers[m_selectedLayerIndex].position.x, m_layers[m_selectedLayerIndex].position.y)
            : U"none";

        return U"[TextureEditor.PlaceAtClick]\n"
            U"updateEnabled = {}\n"_fmt(m_enabled ? U"true" : U"false")
            + U"requestArmed = {}\n"_fmt(m_placeAtClickRequested ? U"true" : U"false")
            + U"selectedLayer = {}\n"_fmt(selectedLayer)
            + U"cursorScreen = ({}, {})\n"_fmt(m_lastCursorScreenPos.x, m_lastCursorScreenPos.y)
            + U"cursorGround = {}\n"_fmt(groundText)
            + U"panelHover = {}\n"_fmt(m_lastPlacementPanelHover ? U"true" : U"false")
            + U"leftClickDown = {}\n"_fmt(m_lastPlacementClickSeen ? U"true" : U"false")
            + U"placementApplied = {}\n"_fmt(m_lastPlacementApplied ? U"true" : U"false")
            + U"layerPosition = {}\n"_fmt(layerPosText)
            + U"reason = {}\n"_fmt(m_lastPlacementReason)
            + U"status = {}"_fmt(m_statusMessage);
    }



String TextureEditor::categoryName(const int32 index){
        switch (index)
        {
        case 0: return U"Grass";
        case 1: return U"Dirt";
        case 2: return U"Plaza";
        case 3: return U"Brick";
        case 4: return U"Stone";
        case 5: return U"Decal";
        default: return U"Other";
        }
    }



    void TextureEditor::scanAvailableTextures(){
        m_availableTextures.clear();
        for (const auto& dirPath : { String{ U"example/texture/" }, String{ U"texture/" } })
        {
            if (not FileSystem::IsDirectory(dirPath))
            {
                continue;
            }
            for (const auto& path : FileSystem::DirectoryContents(dirPath, Recursive::No))
            {
                const String ext = FileSystem::Extension(path).lowercased();
                if (ext == U"jpg" || ext == U"png")
                {
                    m_availableTextures << path;
                }
            }
        }
    }



const Image& TextureEditor::getOrLoadSourceImage(const FilePath& path){
        if (const auto it = m_sourceImageCache.find(path); it != m_sourceImageCache.end())
        {
            return it->second;
        }
        m_sourceImageCache.emplace(path, Image{ path });
        return m_sourceImageCache[path];
    }



Image TextureEditor::buildLayerImage(const GroundLayer& layer){
        Image result{ TexRes, TexRes };
        const Image& src = getOrLoadSourceImage(layer.texturePath);
        const bool hasSrc = (not src.isEmpty());
        PerlinNoise noise{ layer.edgeNoiseSeed };
        const double softness = Max(0.001, layer.edgeSoftness);

        for (int32 py = 0; py < TexRes; ++py)
        {
            for (int32 px = 0; px < TexRes; ++px)
            {
                const double u = (px + 0.5) / TexRes;
                const double v = (py + 0.5) / TexRes;

                const double edgeDist = Min(Min(u, 1.0 - u), Min(v, 1.0 - v));

                const double noiseVal = noise.noise1D(u * layer.edgeNoiseFrequency + v * layer.edgeNoiseFrequency * 1.31);
                const double warpedDist = edgeDist + noiseVal * layer.edgeNoiseAmount;

                const double t = Clamp(warpedDist / softness, 0.0, 1.0);
                const double alpha = t * t * (3.0 - 2.0 * t);

                ColorF srcColor{ 0.55, 0.55, 0.55 };
                if (hasSrc)
                {
                    double su = std::fmod(u * layer.tilingScale, 1.0);
                    double sv = std::fmod(v * layer.tilingScale, 1.0);
                    if (su < 0.0) su += 1.0;
                    if (sv < 0.0) sv += 1.0;
                    const int32 sx = static_cast<int32>(su * src.width()) % src.width();
                    const int32 sy = static_cast<int32>(sv * src.height()) % src.height();
                    srcColor = ColorF{ src[sy][sx] };
                }

                result[py][px] = Color{ ColorF{
                    srcColor.r * layer.tint.r,
                    srcColor.g * layer.tint.g,
                    srcColor.b * layer.tint.b,
                    alpha * layer.tint.a
                } };
            }
        }

        return result;
    }



    void TextureEditor::ensureAllLayerTextures(){
        for (size_t i = 0; i < m_layers.size(); ++i)
        {
            if (not m_layerDirty[i])
            {
                continue;
            }

            const Image img = buildLayerImage(m_layers[i]);
            if (m_layerTextures[i])
            {
                m_layerTextures[i].fill(img);
            }
            else
            {
                m_layerTextures[i] = DynamicTexture{ img };
            }
            m_layerDirty[i] = false;
        }
    }



    void TextureEditor::markLayerDirty(const size_t index){
        if (index < m_layerDirty.size())
        {
            m_layerDirty[index] = true;
        }
    }



    void TextureEditor::addLayer(GroundLayer layer){
        if (layer.id.isEmpty())
        {
            layer.id = U"layer_{}"_fmt(m_layers.size());
        }
        if (layer.label.isEmpty())
        {
            layer.label = U"Layer {}"_fmt(m_layers.size());
        }
        m_layers << layer;
        m_layerTextures.emplace_back();
        m_layerDirty << true;
        m_selectedLayerIndex = m_layers.size() - 1;
        m_layerListScroll = getLayerListMaxScroll();
    }



    void TextureEditor::removeLayer(const size_t index){
        if (index >= m_layers.size())
        {
            return;
        }
        m_layers.remove_at(index);
        m_layerTextures.remove_at(index);
        m_layerDirty.remove_at(index);
        if ((not m_layers.isEmpty()) && m_selectedLayerIndex >= m_layers.size())
        {
            m_selectedLayerIndex = m_layers.size() - 1;
        }
        m_layerListScroll = Clamp(m_layerListScroll, 0.0, getLayerListMaxScroll());
        m_statusMessage = U"Layer deleted";
    }



    void TextureEditor::clearAllLayers(){
        m_layers.clear();
        m_layerTextures.clear();
        m_layerDirty.clear();
        m_selectedLayerIndex = 0;
        m_placeAtClickRequested = false;
        m_layerListScroll = 0.0;
        m_statusMessage = U"All layers cleared";
    }



    void TextureEditor::undoLastLayer(){
        if (not m_layers.isEmpty())
        {
            removeLayer(m_layers.size() - 1);
            m_statusMessage = U"Undo: layer removed";
        }
    }



    void TextureEditor::duplicateSelectedLayer(){
        if (not hasSelectedLayer())
        {
            return;
        }
        GroundLayer copy = m_layers[m_selectedLayerIndex];
        copy.id = U"layer_{}"_fmt(m_layers.size());
        copy.label = copy.label + U" (copy)";
        copy.position.x += 2.0;
        addLayer(copy);
        m_statusMessage = U"Layer duplicated";
    }



    void TextureEditor::moveLayerUp(const size_t index){
        if (index == 0 || index >= m_layers.size())
        {
            return;
        }
        std::swap(m_layers[index], m_layers[index - 1]);
        std::swap(m_layerTextures[index], m_layerTextures[index - 1]);
        std::swap(m_layerDirty[index], m_layerDirty[index - 1]);
        m_selectedLayerIndex = index - 1;
    }



    void TextureEditor::moveLayerDown(const size_t index){
        if ((index + 1) >= m_layers.size())
        {
            return;
        }
        std::swap(m_layers[index], m_layers[index + 1]);
        std::swap(m_layerTextures[index], m_layerTextures[index + 1]);
        std::swap(m_layerDirty[index], m_layerDirty[index + 1]);
        m_selectedLayerIndex = index + 1;
    }



    void TextureEditor::cycleTexture(GroundLayer& layer, const int32 direction){
        if (m_availableTextures.isEmpty())
        {
            return;
        }

        size_t currentIndex = 0;
        bool found = false;
        for (size_t i = 0; i < m_availableTextures.size(); ++i)
        {
            if (m_availableTextures[i] == layer.texturePath)
            {
                currentIndex = i;
                found = true;
                break;
            }
        }

        const size_t n = m_availableTextures.size();
        currentIndex = found
            ? ((currentIndex + n + direction) % n)
            : 0;
        layer.texturePath = m_availableTextures[currentIndex];
    }
