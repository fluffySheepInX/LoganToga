# include "EffectsDetail.hpp"

namespace pe
{
    Effect MakeExtractBrightEffect()
    {
        struct State { PixelShader ps = LoadPS(U"extract_bright_linear", false); };
        auto s = std::make_shared<State>();

        Effect e;
        e.name = U"ブライトパス抽出";
        e.apply = [s](const Texture& src)
        {
            const ScopedCustomShader2D shader{ s->ps };
            src.draw();
        };
        e.drawUI = nullptr;
        return e;
    }
}
