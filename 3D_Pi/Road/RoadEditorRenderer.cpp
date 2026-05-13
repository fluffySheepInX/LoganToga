# include "../stdafx.h"
# include "RoadEditorRenderer.hpp"
# include "RoadEditor.hpp"

void RoadEditorRenderer::Draw(const RoadEditor& editor)
{
    editor.drawRenderer3D();
}
