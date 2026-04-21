# include "EffectsDetail.hpp"

namespace pe
{
    Effect MakeNoneEffect()
    {
        Effect e;
        e.name = U"なし";
        e.apply = [](const Texture& src)
        {
            src.draw();
        };
        e.drawUI = nullptr;
        return e;
    }
}
