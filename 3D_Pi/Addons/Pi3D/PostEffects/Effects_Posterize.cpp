# include "EffectsDetail.hpp"

namespace pe
{
    Effect MakePosterizeEffect()
    {
        return MakeSimpleShaderEffect(U"posterize");
    }
}
