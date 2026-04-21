# include "EffectsDetail.hpp"

namespace pe
{
    Effect MakeVignetteEffect()
    {
        struct State
        {
            PixelShader ps = LoadPS(U"vignette");
            ConstantBuffer<Float4> cb{ Float4{ 0.6f, 0.5f, 0.0f, 0.0f } };
            double intensity  = 0.6;
            double smoothness = 0.5;
            double roundness  = 0.0;
        };
        auto s = std::make_shared<State>();

        Effect e;
        e.name = U"Vignette";
        e.apply = [s](const Texture& src)
        {
            s->cb = Float4{
                static_cast<float>(s->intensity),
                static_cast<float>(s->smoothness),
                static_cast<float>(s->roundness),
                0.0f };
            Graphics2D::SetPSConstantBuffer(1, s->cb);

            const ScopedCustomShader2D shader{ s->ps };
            src.draw();
        };
        e.drawUI = [s](const Vec2& pos)
        {
            ui::SliderH(U"int: {:.2f}"_fmt(s->intensity),
                s->intensity, 0.0, 1.0, pos, 130, 200);
            ui::SliderH(U"smooth: {:.2f}"_fmt(s->smoothness),
                s->smoothness, 0.1, 1.5, pos + Vec2{ 0, 40 }, 130, 200);
            ui::SliderH(U"round: {:.2f}"_fmt(s->roundness),
                s->roundness, 0.0, 1.0, pos + Vec2{ 0, 80 }, 130, 200);
        };
        return e;
    }
}
