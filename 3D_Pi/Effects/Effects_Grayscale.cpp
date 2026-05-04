# include "EffectsDetail.hpp"

namespace pe
{
    Effect MakeGrayscaleEffect()
    {
        return MakeSimpleShaderEffect(U"grayscale");
    }
}
