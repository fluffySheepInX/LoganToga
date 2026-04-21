# include "EffectsDetail.hpp"

namespace pe
{
    Effect MakePosterizeEffect()
    {
        struct State { PixelShader ps = LoadPS(U"posterize", false); };
        auto s = std::make_shared<State>();

        Effect e;
        e.name = U"ポスタライズ";
        e.apply = [s](const Texture& src)
        {
            const ScopedCustomShader2D shader{ s->ps };
            src.draw();
        };
        e.drawUI = nullptr;
        return e;
    }
}
