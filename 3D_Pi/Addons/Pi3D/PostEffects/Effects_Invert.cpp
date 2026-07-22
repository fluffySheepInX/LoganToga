# include "EffectsDetail.hpp"

namespace pe
{
    Effect MakeInvertEffect()
    {
        return MakeSimpleShaderEffect(U"invert");
    }
}
