//-----------------------------------------------
//
//	Effects_WarmGrade.cpp
//	2000s 海外ゲーム風 セピア寄りカラーグレーディング
//
//-----------------------------------------------

# include "EffectsDetail.hpp"

namespace pe
{
    Effect MakeWarmGradeEffect()
    {
        struct State
        {
            PixelShader ps = LoadPS(U"warm_grade");
            ConstantBuffer<Float4> cb{ Float4{ 0.75f, 0.85f, 1.10f, 0.06f } };
            double warmth    = 0.75;
            double saturation = 0.85;
            double contrast  = 1.10;
            double lift      = 0.06;
        };
        auto s = std::make_shared<State>();

        Effect e;
        e.name = U"Warm Grade";
        e.apply = [s](const Texture& src)
        {
            s->cb = Float4{
                static_cast<float>(s->warmth),
                static_cast<float>(s->saturation),
                static_cast<float>(s->contrast),
                static_cast<float>(s->lift) };
            Graphics2D::SetPSConstantBuffer(1, s->cb);

            const ScopedCustomShader2D shader{ s->ps };
            src.draw();
        };
        e.drawUI = [s](const Vec2& pos)
        {
            ui::SliderH(U"warmth: {:.2f}"_fmt(s->warmth),
                s->warmth, 0.0, 1.0, pos, 130, 200);
            ui::SliderH(U"sat: {:.2f}"_fmt(s->saturation),
                s->saturation, 0.0, 1.5, pos + Vec2{ 0, 40 }, 130, 200);
            ui::SliderH(U"contrast: {:.2f}"_fmt(s->contrast),
                s->contrast, 0.5, 2.0, pos + Vec2{ 0, 80 }, 130, 200);
            ui::SliderH(U"lift: {:.2f}"_fmt(s->lift),
                s->lift, 0.0, 0.3, pos + Vec2{ 0, 120 }, 130, 200);
        };
        e.reset = [s]()
        {
            s->warmth = 0.75;
            s->saturation = 0.85;
            s->contrast = 1.10;
            s->lift = 0.06;
        };
        return e;
    }
}
