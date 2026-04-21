# include "EffectsDetail.hpp"

namespace pe
{
    Effect MakeGrayscaleEffect()
    {
        struct State { PixelShader ps = LoadPS(U"grayscale", false); };
        auto s = std::make_shared<State>();

        Effect e;
        e.name = U"グレースケール";
        e.apply = [s](const Texture& src)
        {
            const ScopedCustomShader2D shader{ s->ps };
            src.draw();
        };
        e.drawUI = nullptr;
        return e;
    }
}
