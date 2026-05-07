# include "../stdafx.h"
# include "TextureEditor.hpp"
# include "GroundLayerSerializer.hpp"

void TextureEditor::save() const{
        ground::SaveGroundLayerDocument(m_savePath, m_document);
    }



    void TextureEditor::load(){
        m_layerTextures.clear();
        m_layerDirty.clear();
        m_selectedLayerIndex = 0;
        m_layerListScroll = 0.0;
        m_textureListScroll = 0.0;

        if (not ground::LoadGroundLayerDocument(m_savePath, m_document))
        {
            loadDefaults();
            return;
        }

        const Array<GroundLayer> loadedLayers = m_document.layers;
        m_document.layers.clear();
        for (const auto& layer : loadedLayers)
        {
            addLayer(layer);
        }

        m_statusMessage = U"Loaded {} layers"_fmt(m_layers.size());
    }



    void TextureEditor::loadDefaults(){
        m_selectedLayerIndex = 0;
        m_layerListScroll = 0.0;
        m_textureListScroll = 0.0;

        m_document = ground::BuildDefaultGroundLayerDocument();

        m_layerTextures.clear();
        m_layerDirty.clear();

        const Array<GroundLayer> defaults = m_document.layers;
        m_document.layers.clear();
        for (const auto& layer : defaults)
        {
            addLayer(layer);
        }

        m_statusMessage = U"Defaults loaded";
    }
