# include "EffectsDetail.hpp"

namespace pe
{
    Effect MakeCRTEffect()
    {
        struct State
        {
            PixelShader ps = LoadPS(U"crt");
            ConstantBuffer<Float4> cb{ Float4{ 0.10f, 0.45f, 0.35f, 0.55f } };
            double curvature = 0.10;
            double scanline  = 0.45;
            double mask      = 0.35;
            double vignette  = 0.55;
        };
        auto s = std::make_shared<State>();

        Effect e;
        e.apply = [s](const Texture& src, const EffectContext&)
        {
            ApplyFullscreenEffect(s->ps, s->cb,
                Float4{
                    static_cast<float>(s->curvature),
                    static_cast<float>(s->scanline),
                    static_cast<float>(s->mask),
                    static_cast<float>(s->vignette) }, src);
        };
        e.drawUI = [s](const Vec2& pos)
        {
            ui::SliderH(U"curve: {:.2f}"_fmt(s->curvature),
                s->curvature, 0.0, 0.5, pos, 130, 200);
            ui::SliderH(U"scan: {:.2f}"_fmt(s->scanline),
                s->scanline, 0.0, 1.0, pos + Vec2{ 0, 40 }, 130, 200);
            ui::SliderH(U"mask: {:.2f}"_fmt(s->mask),
                s->mask, 0.0, 1.0, pos + Vec2{ 0, 80 }, 130, 200);
            ui::SliderH(U"vig: {:.2f}"_fmt(s->vignette),
                s->vignette, 0.0, 1.0, pos + Vec2{ 0, 120 }, 130, 200);
        };
        e.reset = [s]()
        {
            s->curvature = 0.10;
            s->scanline = 0.45;
            s->mask = 0.35;
            s->vignette = 0.55;
        };
        return e;
    }
}
