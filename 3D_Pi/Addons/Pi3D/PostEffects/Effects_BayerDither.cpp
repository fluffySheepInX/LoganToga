# include "EffectsDetail.hpp"

namespace pe
{
    Effect MakeBayerDitherEffect()
    {
        struct State
        {
            PixelShader ps = LoadPS(U"bayer_dither");
            ConstantBuffer<Float4> cb{ Float4{ 0.5f, 2.0f, 0.35f, 0.0f } };
            double threshold = 0.5;
            double scale = 2.0;
            double strength = 0.35;
        };
        auto s = std::make_shared<State>();

        Effect e;
        e.apply = [s](const Texture& src, const EffectContext&)
        {
            ApplyFullscreenEffect(s->ps, s->cb,
                Float4{
                    static_cast<float>(s->threshold),
                    static_cast<float>(s->scale),
                    static_cast<float>(s->strength),
                    0.0f }, src);
        };
        e.drawUI = [s](const Vec2& pos)
        {
            ui::SliderH(U"threshold: {:.2f}"_fmt(s->threshold),
                s->threshold, 0.0, 1.0, pos, 130, 200);
            ui::SliderH(U"scale: {:.0f}px"_fmt(s->scale),
                s->scale, 1.0, 8.0, pos + Vec2{ 0, 40 }, 130, 200);
            ui::SliderH(U"strength: {:.2f}"_fmt(s->strength),
                s->strength, 0.0, 1.0, pos + Vec2{ 0, 80 }, 130, 200);
            s->scale = std::round(s->scale);
        };
        e.reset = [s]()
        {
            s->threshold = 0.5;
            s->scale = 2.0;
            s->strength = 0.35;
        };
        return e;
    }
}
