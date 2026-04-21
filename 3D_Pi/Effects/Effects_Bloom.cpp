# include "EffectsDetail.hpp"

namespace pe
{
    Effect MakeBloomEffect()
    {
        struct State
        {
            PixelShader ps = LoadPS(U"bloom_extract");
            ConstantBuffer<Float4> cb{ Float4{ 0.8f, 0.5f, 1.0f, 0.0f } };
            RenderTexture rtA;
            RenderTexture rtB;
            Size lastSize{ 0, 0 };

            double threshold = 0.8;
            double knee      = 0.5;
            double intensity = 1.0;
            double blurPass  = 2.0;

            void ensureRT(const Size& srcSize)
            {
                const Size half = Size{ Max(1, srcSize.x / 2), Max(1, srcSize.y / 2) };
                if (lastSize != half)
                {
                    rtA = RenderTexture{ half, TextureFormat::R16G16B16A16_Float };
                    rtB = RenderTexture{ half, TextureFormat::R16G16B16A16_Float };
                    lastSize = half;
                }
            }
        };
        auto s = std::make_shared<State>();

        Effect e;
        e.name = U"Bloom";
        e.apply = [s](const Texture& src)
        {
            s->ensureRT(src.size());

            src.draw();
            Graphics2D::Flush();

            {
                const ScopedRenderTarget2D rt{ s->rtA.clear(ColorF{ 0, 0, 0, 1 }) };
                const ScopedRenderStates2D blend{ BlendState::Opaque };

                s->cb = Float4{
                    static_cast<float>(s->threshold),
                    static_cast<float>(s->knee),
                    1.0f,
                    0.0f };
                Graphics2D::SetPSConstantBuffer(1, s->cb);

                const ScopedCustomShader2D shader{ s->ps };
                src.resized(s->rtA.size()).draw();
            }
            Graphics2D::Flush();

            const int passes = Max(1, static_cast<int>(std::round(s->blurPass)));
            for (int i = 0; i < passes; ++i)
            {
                Shader::GaussianBlur(s->rtA, s->rtB, s->rtA);
            }

            {
                const ScopedRenderStates2D blend{ BlendState::Additive };
                s->rtA.resized(src.size()).draw(ColorF{ s->intensity });
            }
        };
        e.drawUI = [s](const Vec2& pos)
        {
            ui::SliderH(U"thr: {:.2f}"_fmt(s->threshold),
                s->threshold, 0.0, 2.0, pos, 130, 200);
            ui::SliderH(U"knee: {:.2f}"_fmt(s->knee),
                s->knee, 0.0, 1.0, pos + Vec2{ 0, 40 }, 130, 200);
            ui::SliderH(U"int: {:.2f}"_fmt(s->intensity),
                s->intensity, 0.0, 3.0, pos + Vec2{ 0, 80 }, 130, 200);
            ui::SliderH(U"pass: {:.0f}"_fmt(s->blurPass),
                s->blurPass, 1.0, 6.0, pos + Vec2{ 0, 120 }, 130, 200);
            s->blurPass = std::round(s->blurPass);
        };
        return e;
    }
}
