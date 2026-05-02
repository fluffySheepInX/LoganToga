# include "EffectsDetail.hpp"

namespace pe
{
    Effect MakePixelArtEffect()
    {
        struct State
        {
            PixelShader ps = LoadPS(U"pixelart");
            ConstantBuffer<Float4> cb{ Float4{ 5.0f, 0.0f, 0.0f, 0.0f } };
            double levels = 5.0;
        };
        auto s = std::make_shared<State>();

        Effect e;
        e.name = U"ピクセルアート";
        e.apply = [s](const Texture& src)
        {
            s->cb = Float4{ static_cast<float>(s->levels), 0.0f, 0.0f, 0.0f };
            Graphics2D::SetPSConstantBuffer(1, s->cb);

            const ScopedCustomShader2D shader{ s->ps };
            src.draw();
        };
        e.drawUI = [s](const Vec2& pos)
        {
            ui::SliderH(U"levels: {:.0f}"_fmt(s->levels),
                s->levels, 2.0, 16.0, pos, 130, 200);
            s->levels = std::round(s->levels);
        };
        e.reset = [s]()
        {
            s->levels = 5.0;
        };
        return e;
    }
}
