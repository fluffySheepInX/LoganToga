# include "EffectsDetail.hpp"

namespace pe
{
    Effect MakeExtractBrightEffect()
    {
        return MakeSimpleShaderEffect(U"extract_bright_linear");
    }
}
