# include "EffectsDetail.hpp"

namespace pe
{
    Effect MakeToonEffect()
    {
        struct State
        {
            PixelShader ps = LoadPS(U"toon");
            ConstantBuffer<Float4> cb{ Float4{ 4.0f, 0.35f, 0.0f, 0.0f } };
            double bands = 4.0;
            double floorV = 0.35;
        };
        auto s = std::make_shared<State>();

        Effect e;
        e.name = U"トゥーン";
        e.apply = [s](const Texture& src)
        {
            s->cb = Float4{ static_cast<float>(s->bands), static_cast<float>(s->floorV), 0.0f, 0.0f };
            Graphics2D::SetPSConstantBuffer(1, s->cb);

            const ScopedCustomShader2D shader{ s->ps };
            src.draw();
        };
        e.drawUI = [s](const Vec2& pos)
        {
            ui::SliderH(U"bands: {:.0f}"_fmt(s->bands),
                s->bands, 2.0, 8.0, pos, 130, 200);
            s->bands = std::round(s->bands);

            ui::SliderH(U"shadow: {:.2f}"_fmt(s->floorV),
                s->floorV, 0.0, 0.6, pos + Vec2{ 0, 40 }, 130, 200);
        };
        return e;
    }
}
