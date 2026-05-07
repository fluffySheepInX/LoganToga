# pragma once
# include <Siv3D.hpp>
# include "GroundLayer.hpp"

struct GroundLayerDocument
{
    bool autoYOffset = true;
    double autoYOffsetStep = 0.003;
    double baseYOffset = 0.002;
    Array<GroundLayer> layers;
};
