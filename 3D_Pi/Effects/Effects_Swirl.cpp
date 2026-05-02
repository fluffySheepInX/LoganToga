# include "EffectsDetail.hpp"

namespace pe
{
    Effect MakeSwirlEffect()
    {
        struct State
        {
            PixelShader ps = LoadPS(U"swirl");
            ConstantBuffer<Float4> cb{ Float4{ 4.0f, 0.0f, 0.0f, 0.0f } };
            double angle = 4.0;
        };
        auto s = std::make_shared<State>();

        Effect e;
        e.name = U"スワール";
        e.apply = [s](const Texture& src)
        {
            s->cb = Float4{ static_cast<float>(s->angle), 0.0f, 0.0f, 0.0f };
            Graphics2D::SetPSConstantBuffer(1, s->cb);

            const ScopedCustomShader2D shader{ s->ps };
            src.draw();
        };
        e.drawUI = [s](const Vec2& pos)
        {
            ui::SliderH(U"angle: {:.2f}"_fmt(s->angle),
                s->angle, -8.0, 8.0, pos, 130, 200);
        };
        e.reset = [s]()
        {
            s->angle = 4.0;
        };
        return e;
    }
}
