# include "EffectsDetail.hpp"

namespace pe
{
    Effect MakeMosaicEffect()
    {
        struct State
        {
            PixelShader ps = LoadPS(U"mosaic");
            ConstantBuffer<Float4> cb{ Float4{ 12.0f, 0.0f, 0.0f, 0.0f } };
            double blockSize = 12.0;
        };
        auto s = std::make_shared<State>();

        Effect e;
        e.apply = [s](const Texture& src, const EffectContext&)
        {
            ApplyFullscreenEffect(s->ps, s->cb,
                Float4{ static_cast<float>(s->blockSize), 0.0f, 0.0f, 0.0f }, src);
        };
        e.drawUI = [s](const Vec2& pos)
        {
            ui::SliderH(U"block: {:.0f}px"_fmt(s->blockSize),
                s->blockSize, 2.0, 64.0, pos, 130, 200);
            s->blockSize = std::round(s->blockSize);
        };
        e.reset = [s]()
        {
            s->blockSize = 12.0;
        };
        return e;
    }
}
