# pragma once
# include <Siv3D.hpp>
# include "ProceduralTypes.hpp"

namespace procedural
{
    struct ProceduralDocument
    {
        Array<GeneratedStair> stairs;
        Array<GeneratedNatureObject> natureObjects;
    };
}
