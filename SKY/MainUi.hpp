# pragma once
# include "BirdModel.hpp"
# include "MainContext.hpp"

namespace MainSupport
{
    bool DrawTextButton(const Rect& rect, StringView label);
	bool DrawCheckBox(const Rect& rect, bool& checked, StringView label, bool enabled = true);
	void DrawAnimationClipSelector(UnitModel& model, StringView title, int32 x, int32 y, int32 width);
	void DrawModelHeightEditor(ModelHeightSettings& modelHeightSettings,
     size_t& activePreviewModelIndex,
     bool& textureMode,
		TireTrackTextureSegment& activeTextureSegment,
        UnitModelAnimationRole& previewAnimationRole,
        UnitModel& activeModel,
        const Array<FilePath>& previewModelPaths,
        const Array<String>& previewModelLabels,
		String& modelHeightMessage,
		double& modelHeightMessageUntil,
		const Rect& panelRect,
     const Array<Vec3>& previewRenderPositions);
}
