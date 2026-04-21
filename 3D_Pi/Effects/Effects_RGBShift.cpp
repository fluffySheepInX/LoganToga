# include "EffectsDetail.hpp"

namespace pe
{
    Effect MakeRGBShiftEffect()
    {
        struct State { PixelShader ps = LoadPS(U"rgb_shift", false); };
        auto s = std::make_shared<State>();

        Effect e;
        e.name = U"RGB シフト";
        e.apply = [s](const Texture& src)
        {
            const ScopedCustomShader2D shader{ s->ps };
            src.draw();
        };
        e.drawUI = nullptr;
        return e;
    }
}
