# include "../stdafx.h"
# include "RoadEditor.hpp"

void RoadEditor::saveCurrentAsPreset(const String& name){
        RoadSceneSnapshot snapshot;
        snapshot.name     = name;
        snapshot.roads    = m_roads;
        snapshot.material = m_materialSettings;

        if (road::SavePreset(m_presetsDir, snapshot))
        {
            for (auto& p : m_presets)
            {
                if (p.name == name)
                {
                    p = snapshot;
                    m_statusMessage = U"Preset updated: " + name;
                    return;
                }
            }
            m_presets << snapshot;
            m_selectedPresetIndex = m_presets.size() - 1;
            m_statusMessage = U"Preset saved: " + name;
        }
        else
        {
            m_statusMessage = U"Preset save failed";
        }
    }



    void RoadEditor::startTraceSession(){
        if (m_presets.isEmpty())
        {
            m_statusMessage = U"No preset to trace from";
            return;
        }

        const auto& preset = m_presets[m_selectedPresetIndex];

        RoadSceneSnapshot restorePoint;
        restorePoint.name     = U"__restore__";
        restorePoint.roads    = m_roads;
        restorePoint.material = m_materialSettings;
        m_session.begin(restorePoint);

        m_roads.clear();
        m_roadMeshes.clear();
        m_roadShoulderMeshes.clear();
        m_connectionPatchMeshes.clear();
        m_editingRoad.reset();
        m_editingMesh.reset();
        m_intersectionClusters.clear();

        m_ghost.buildFrom(preset.roads, preset.material);
        m_ghost.visible = true;

        rebuildAllMeshes();
        m_statusMessage = U"Trace session started: " + preset.name;
    }



    void RoadEditor::restoreSession(){
        if (not m_session.canRestore())
        {
            return;
        }

        const auto& rp = *m_session.restorePoint;
        m_roads           = rp.roads;
        m_materialSettings = rp.material;
        m_materialDirty   = true;

        m_editingRoad.reset();
        m_editingMesh.reset();
        m_ghost.clear();
        m_session.end();

        rebuildAllMeshes();
        refreshRoadMaterialTextureIfDirty();
        m_statusMessage = U"Scene restored";
    }



    void RoadEditor::commitSession(){
        m_ghost.clear();
        m_session.end();
        m_statusMessage = U"Trace session committed";
    }



    void RoadEditor::toggleGhostVisible(){
        if (m_ghost.roads.isEmpty())
        {
            m_statusMessage = U"No ghost loaded";
            return;
        }

        m_ghost.visible = not m_ghost.visible;
        m_statusMessage = m_ghost.visible ? U"Ghost: ON" : U"Ghost: OFF";
    }



    void RoadEditor::save() const{
        road::SaveRoadData(m_savePath, m_document);
    }



    void RoadEditor::load(){
        m_roads.clear();
        m_roadMeshes.clear();
        m_roadShoulderMeshes.clear();
        m_connectionPatchMeshes.clear();
        m_editingRoad.reset();
        m_editingMesh.reset();
        m_document.material = road::DefaultRoadMaterialSettings();

        road::LoadRoadData(m_savePath, m_document, m_statusMessage);
        loadPlacementAssets();
        m_materialDirty = true;
        refreshRoadMaterialTextureIfDirty();
        rebuildAllMeshes();
    }
