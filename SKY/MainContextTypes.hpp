# pragma once

# include <Siv3D.hpp>

namespace MainSupport
{
  struct TerrainVisualSettings
	{
		bool noiseEnabled = true;
        double materialBlendStrength = 0.28;
      double placementImprintStrength = 1.0;
		double wearStrength = 1.0;
		double ambientOcclusionStrength = 1.0;
		double macroNoiseStrength = 0.65;
		double macroNoiseScale = 0.05;
		double noiseStrength = 0.55;
		double noiseScale = 0.18;
	};

	struct EditorTextColorSettings
	{
		ColorF darkPrimary{ 0.96, 0.98, 1.0, 1.0 };
		ColorF darkSecondary{ 0.84, 0.90, 0.98, 0.92 };
		ColorF darkAccent{ 1.0, 0.94, 0.72, 0.96 };
		ColorF lightPrimary{ 0.14, 0.14, 0.16, 1.0 };
		ColorF lightSecondary{ 0.28, 0.42, 0.58, 0.92 };
		ColorF lightAccent{ 0.22, 0.40, 0.64, 0.96 };
        ColorF cardPrimary{ 0.14, 0.14, 0.16, 1.0 };
		ColorF cardSecondary{ 0.28, 0.42, 0.58, 0.92 };
		ColorF selectedPrimary{ 0.98, 0.99, 1.0, 1.0 };
		ColorF selectedSecondary{ 0.94, 0.97, 1.0, 0.96 };
		ColorF warning{ 1.0, 0.88, 0.76, 0.96 };
		ColorF error{ 0.75, 0.20, 0.20, 1.0 };
	};
}
