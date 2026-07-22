//-----------------------------------------------
//
//	Effects_DoF.cpp
//	Depth of Field (被写界深度) ポストエフェクト
//
//-----------------------------------------------

# include "EffectsDetail.hpp"

namespace pe
{
    Effect MakeDepthOfFieldEffect()
    {
        struct State
        {
            PixelShader ps = LoadPS(U"dof_combine");
            ConstantBuffer<Float4> cb{ Float4{ 18.0f, 2.0f, 6.0f, 10.0f } };
            RenderTexture blurA;
            RenderTexture blurB;
            Size lastSize{ 0, 0 };

            double focusDistance  = 18.0;
            double focusRange     = 2.0;
            double nearTransition = 6.0;
            double farTransition  = 10.0;
            double blurPass       = 2.0;

            void ensureRT(const Size& srcSize)
            {
                const Size half = Size{ Max(1, srcSize.x / 2), Max(1, srcSize.y / 2) };
                if (lastSize != half)
                {
                    blurA = RenderTexture{ half, TextureFormat::R16G16B16A16_Float };
                    blurB = RenderTexture{ half, TextureFormat::R16G16B16A16_Float };
                    lastSize = half;
                }
            }
        };
        auto s = std::make_shared<State>();

        Effect e;
        e.apply = [s](const Texture& src, const EffectContext& context)
        {
            const Optional<Texture>& depthTex = context.sceneDepthTexture;
            if (not depthTex)
            {
                // 深度テクスチャ未設定時はスルー描画
                src.draw();
                return;
            }

            s->ensureRT(context.sourceSize);

            // 1) ハーフ解像度に縮小コピー
            {
                const ScopedRenderTarget2D rt{ s->blurA.clear(ColorF{ 0, 0, 0, 1 }) };
                const ScopedRenderStates2D blend{ BlendState::Opaque };
                src.resized(s->blurA.size()).draw();
            }
            Graphics2D::Flush();

            // 2) ガウシアンブラー
            const int passes = Max(1, static_cast<int>(std::round(s->blurPass)));
            for (int i = 0; i < passes; ++i)
            {
                Shader::GaussianBlur(s->blurA, s->blurB, s->blurA);
            }

            // 3) シャープ + 深度 + ブラーを合成
            s->cb = Float4{
                static_cast<float>(s->focusDistance),
                static_cast<float>(s->focusRange),
                static_cast<float>(s->nearTransition),
                static_cast<float>(s->farTransition) };
            Graphics2D::SetPSConstantBuffer(1, s->cb);
            Graphics2D::SetPSTexture(1, *depthTex);
            Graphics2D::SetPSTexture(2, s->blurA);

            {
                const ScopedCustomShader2D shader{ s->ps };
                src.draw();
            }
            Graphics2D::Flush();

            Graphics2D::SetPSTexture(1, none);
            Graphics2D::SetPSTexture(2, none);
        };
        e.drawUI = [s](const Vec2& pos)
        {
            ui::SliderH(U"focus: {:.1f}"_fmt(s->focusDistance),
                s->focusDistance, 0.0, 100.0, pos, 130, 200);
            ui::SliderH(U"range: {:.2f}"_fmt(s->focusRange),
                s->focusRange, 0.0, 20.0, pos + Vec2{ 0, 40 }, 130, 200);
            ui::SliderH(U"near: {:.2f}"_fmt(s->nearTransition),
                s->nearTransition, 0.1, 30.0, pos + Vec2{ 0, 80 }, 130, 200);
            ui::SliderH(U"far: {:.2f}"_fmt(s->farTransition),
                s->farTransition, 0.1, 60.0, pos + Vec2{ 0, 120 }, 130, 200);
            ui::SliderH(U"pass: {:.0f}"_fmt(s->blurPass),
                s->blurPass, 1.0, 6.0, pos + Vec2{ 0, 160 }, 130, 200);
            s->blurPass = std::round(s->blurPass);
        };
        e.reset = [s]()
        {
            s->focusDistance = 18.0;
            s->focusRange = 2.0;
            s->nearTransition = 6.0;
            s->farTransition = 10.0;
            s->blurPass = 2.0;
        };
        return e;
    }
}
