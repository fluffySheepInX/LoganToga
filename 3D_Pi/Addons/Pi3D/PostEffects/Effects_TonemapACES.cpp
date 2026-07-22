# include "EffectsDetail.hpp"

namespace pe
{
    Effect MakeTonemapACESEffect()
    {
        struct State
        {
            PixelShader ps = LoadPS(U"tonemap_aces");
            ConstantBuffer<Float4> cb{ Float4{ 1.0f, 1.0f, 0.0f, 0.0f } };
            double exposure = 1.0;
            double gamma    = 1.0;
        };
        auto s = std::make_shared<State>();

        Effect e;
        e.apply = [s](const Texture& src, const EffectContext&)
        {
            ApplyFullscreenEffect(s->ps, s->cb,
                Float4{
                    static_cast<float>(s->exposure),
                    static_cast<float>(s->gamma),
                    0.0f, 0.0f }, src);
        };
        e.drawUI = [s](const Vec2& pos)
        {
            ui::SliderH(U"exposure: {:.2f}"_fmt(s->exposure),
                s->exposure, 0.0, 4.0, pos, 130, 200);
            ui::SliderH(U"gamma: {:.2f}"_fmt(s->gamma),
                s->gamma, 1.0, 2.4, pos + Vec2{ 0, 40 }, 130, 200);
        };
        e.reset = [s]()
        {
            s->exposure = 1.0;
            s->gamma = 1.0;
        };
        return e;
    }
}
