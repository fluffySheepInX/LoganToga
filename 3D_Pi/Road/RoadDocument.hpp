# pragma once
# include <Siv3D.hpp>
# include "RoadTypes.hpp"

struct RoadDocument
{
    RoadMaterialSettings material = road::DefaultRoadMaterialSettings();
    Array<RoadPath> roads;
};
