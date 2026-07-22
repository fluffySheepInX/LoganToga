# include "EffectsDetail.hpp"

namespace pe
{
    Effect MakeGlitchEffect()
    {
        struct State
        {
            PixelShader ps = LoadPS(U"glitch");
            ConstantBuffer<Float4> cb{ Float4{ 0.0f, 0.55f, 0.40f, 0.018f } };
            double intensity = 0.55;
            double blockiness = 0.40;
            double rgbShift = 0.018;
        };
        auto s = std::make_shared<State>();

        Effect e;
        e.apply = [s](const Texture& src, const EffectContext& context)
        {
            ApplyFullscreenEffect(s->ps, s->cb,
                Float4{
                    static_cast<float>(context.time),
                    static_cast<float>(s->intensity),
                    static_cast<float>(s->blockiness),
                    static_cast<float>(s->rgbShift) }, src);
        };
        e.drawUI = [s](const Vec2& pos)
        {
            ui::SliderH(U"intensity: {:.2f}"_fmt(s->intensity),
                s->intensity, 0.0, 1.0, pos, 130, 200);
            ui::SliderH(U"block: {:.2f}"_fmt(s->blockiness),
                s->blockiness, 0.0, 1.0, pos + Vec2{ 0, 40 }, 130, 200);
            ui::SliderH(U"rgb: {:.3f}"_fmt(s->rgbShift),
                s->rgbShift, 0.0, 0.05, pos + Vec2{ 0, 80 }, 130, 200);
        };
        e.reset = [s]()
        {
            s->intensity = 0.55;
            s->blockiness = 0.40;
            s->rgbShift = 0.018;
        };
        return e;
    }
}
