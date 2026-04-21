# include "EffectsDetail.hpp"

namespace pe
{
    Effect MakeOutlineEffect()
    {
        struct State
        {
            PixelShader ps = LoadPS(U"outline");
            ConstantBuffer<Float4> cb{ Float4{ 0.15f, 1.0f, 1.0f, 0.0f } };
            double threshold   = 0.15;
            double thickness   = 1.0;
            double inkStrength = 1.0;
        };
        auto s = std::make_shared<State>();

        Effect e;
        e.name = U"アウトライン";
        e.apply = [s](const Texture& src)
        {
            s->cb = Float4{
                static_cast<float>(s->threshold),
                static_cast<float>(s->thickness),
                static_cast<float>(s->inkStrength),
                0.0f };
            Graphics2D::SetPSConstantBuffer(1, s->cb);

            const ScopedCustomShader2D shader{ s->ps };
            src.draw();
        };
        e.drawUI = [s](const Vec2& pos)
        {
            ui::SliderH(U"threshold: {:.2f}"_fmt(s->threshold),
                s->threshold, 0.02, 0.8, pos, 130, 200);
            ui::SliderH(U"thickness: {:.1f}"_fmt(s->thickness),
                s->thickness, 0.5, 3.0, pos + Vec2{ 0, 40 }, 130, 200);
            ui::SliderH(U"ink: {:.2f}"_fmt(s->inkStrength),
                s->inkStrength, 0.0, 1.0, pos + Vec2{ 0, 80 }, 130, 200);
        };
        return e;
    }
}
