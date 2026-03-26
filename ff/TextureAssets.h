# pragma once
# include "GameConstants.h"

namespace ff
{
	ColorF BlendColor(const ColorF& a, const ColorF& b, double t);
	Texture MakeTileTexture(const ColorF& baseColor, const ColorF& accentColor);
	Array<Texture> LoadTerrainTextures();
}
