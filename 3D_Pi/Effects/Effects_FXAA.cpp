# include "EffectsDetail.hpp"

namespace pe
{
    Effect MakeFXAAEffect()
    {
        struct State
        {
            PixelShader ps = LoadPS(U"fxaa");
            ConstantBuffer<Float4> cb{ Float4{ 0.001f, 0.001f, 1.0f, 0.0f } };
            double strength = 1.0;
        };
        auto s = std::make_shared<State>();

        Effect e;
        e.name = U"FXAA";
        e.apply = [s](const Texture& src)
        {
            const Size sz = src.size();
            s->cb = Float4{
                1.0f / static_cast<float>(Max(1, sz.x)),
                1.0f / static_cast<float>(Max(1, sz.y)),
                static_cast<float>(s->strength),
                0.0f };
            Graphics2D::SetPSConstantBuffer(1, s->cb);

            const ScopedCustomShader2D shader{ s->ps };
            src.draw();
        };
        e.drawUI = [s](const Vec2& pos)
        {
            ui::SliderH(U"strength: {:.2f}"_fmt(s->strength),
                s->strength, 0.0, 1.0, pos, 130, 200);
        };
        return e;
    }
}
