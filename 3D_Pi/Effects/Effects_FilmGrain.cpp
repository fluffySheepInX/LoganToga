# include "EffectsDetail.hpp"

namespace pe
{
    Effect MakeFilmGrainEffect()
    {
        struct State
        {
            PixelShader ps = LoadPS(U"film_grain");
            ConstantBuffer<Float4> cb{ Float4{ 0.0f, 0.06f, 0.0f, 0.0f } };
            double strength = 0.06;
            bool   lumaOnly = false;
        };
        auto s = std::make_shared<State>();

        Effect e;
        e.apply = [s](const Texture& src, const EffectContext& context)
        {
            ApplyFullscreenEffect(s->ps, s->cb,
                Float4{
                    static_cast<float>(context.time),
                    static_cast<float>(s->strength),
                    s->lumaOnly ? 1.0f : 0.0f,
                    0.0f }, src);
        };
        e.drawUI = [s](const Vec2& pos)
        {
            ui::SliderH(U"strength: {:.3f}"_fmt(s->strength),
                s->strength, 0.0, 0.3, pos, 130, 200);
            double v = s->lumaOnly ? 1.0 : 0.0;
            ui::SliderH(U"luma only: {:.0f}"_fmt(v),
                v, 0.0, 1.0, pos + Vec2{ 0, 40 }, 130, 200);
            s->lumaOnly = (0.5 < v);
        };
        e.reset = [s]()
        {
            s->strength = 0.06;
            s->lumaOnly = false;
        };
        return e;
    }
}
