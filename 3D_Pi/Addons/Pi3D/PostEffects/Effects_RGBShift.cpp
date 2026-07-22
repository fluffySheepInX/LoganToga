# include "EffectsDetail.hpp"

namespace pe
{
    Effect MakeRGBShiftEffect()
    {
        return MakeSimpleShaderEffect(U"rgb_shift");
    }
}
