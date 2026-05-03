# pragma once
# include <Siv3D.hpp>
# include "RoadTypes.hpp"

struct RoadSceneSnapshot
{
    String name;
    Array<RoadPath> roads;
    RoadMaterialSettings material;
};
