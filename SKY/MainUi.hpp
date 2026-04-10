# pragma once
# include "BirdModel.hpp"
# include "MainContext.hpp"

namespace MainSupport
{
	bool DrawTextButton(const Rect& rect, StringView label);
	void DrawAnimationClipSelector(BirdModel& model, StringView title, int32 x, int32 y, int32 width);
	void DrawModelHeightEditor(ModelHeightSettings& modelHeightSettings,
     ModelHeightTarget& activeTarget,
		String& modelHeightMessage,
		double& modelHeightMessageUntil,
		const Rect& panelRect,
		const Vec3& birdRenderPosition,
        const Vec3& ashigaruRenderPosition,
		const Vec3& sugoiCarRenderPosition);
}
