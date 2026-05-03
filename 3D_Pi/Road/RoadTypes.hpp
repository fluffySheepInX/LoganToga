# pragma once
# include <Siv3D.hpp>

struct RoadPath
{
    Array<Vec3> points;
    double width = 3.0;
    double textureRepeat = 4.0;
};

struct RoadMaterialSettings
{
    double baseBrightness = 1.0;
    double baseWarmth = 0.0;
    double macroVariation = 0.42;
    double detailVariation = 0.32;
    double trackStrength = 0.58;
    double trackWidth = 0.08;
    double edgeMudStrength = 0.52;
    double pebbleStrength = 0.25;
    double sootStrength = 0.42;
    double shoulderWidthExpand = 1.6;
    double shoulderOpacity = 0.48;
    double shoulderBrightness = 1.28;
    double shoulderOuterFade = 0.62;
    bool shoulderUseColorFade = false;
};

namespace road
{
    [[nodiscard]] inline RoadMaterialSettings DefaultRoadMaterialSettings()
    {
        return RoadMaterialSettings{};
    }
}
