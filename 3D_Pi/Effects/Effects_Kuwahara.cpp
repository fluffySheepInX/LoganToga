# include "EffectsDetail.hpp"

namespace pe
{
    Effect MakeKuwaharaEffect()
    {
        struct State
        {
            PixelShader ps = LoadPS(U"kuwahara");
            ConstantBuffer<Float4> cb{ Float4{ 3.0f, 0.0f, 0.0f, 0.0f } };
            double radius = 3.0;
        };
        auto s = std::make_shared<State>();

        Effect e;
        e.apply = [s](const Texture& src, const EffectContext&)
        {
            ApplyFullscreenEffect(s->ps, s->cb,
                Float4{ static_cast<float>(s->radius), 0.0f, 0.0f, 0.0f }, src);
        };
        e.drawUI = [s](const Vec2& pos)
        {
            ui::SliderH(U"radius: {:.0f}"_fmt(s->radius),
                s->radius, 1.0, 6.0, pos, 130, 200);
            s->radius = std::round(s->radius);
        };
        e.reset = [s]()
        {
            s->radius = 3.0;
        };
        return e;
    }
}
