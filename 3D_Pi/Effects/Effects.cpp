//-----------------------------------------------
//
//	Effects.cpp
//	効果ファクトリの実装
//
//-----------------------------------------------

# include "Effects.hpp"

namespace pe
{
    PixelShader LoadPS(StringView name, bool useEffectParams)
    {
        Array<ConstantBufferBinding> bindings = { { U"PSConstants2D", 0 } };
        if (useEffectParams)
        {
            bindings.push_back({ U"PSEffectParams", 1 });
        }

        const PixelShader ps =
            HLSL{ U"example/shader/hlsl/" + String{ name } + U".hlsl", U"PS" }
          | GLSL{ U"example/shader/glsl/" + String{ name } + U".frag", bindings };

        if (not ps)
        {
            throw Error{ U"Failed to load shader: {}"_fmt(name) };
        }
        return ps;
    }

    //-----------------------------------------------
    // 「なし」: そのまま画面へ
    //-----------------------------------------------
    Effect MakeNoneEffect()
    {
        Effect e;
        e.name = U"なし";
        e.apply = [](const MSRenderTexture& src)
        {
            Shader::LinearToScreen(src);
        };
        e.drawUI = nullptr;
        return e;
    }

    //-----------------------------------------------
    // トゥーン (セルシェーディング)
    //   param0.x = numBands     (2..8)
    //   param0.y = shadowFloor  (0..1)
    //-----------------------------------------------
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
        e.apply = [s](const MSRenderTexture& src)
        {
            s->cb = Float4{ static_cast<float>(s->bands), static_cast<float>(s->floorV), 0.0f, 0.0f };
            Graphics2D::SetPSConstantBuffer(1, s->cb);

            const ScopedCustomShader2D shader{ s->ps };
            src.draw();
        };
        e.drawUI = [s](const Vec2& pos)
        {
            SimpleGUI::Slider(U"bands: {:.0f}"_fmt(s->bands),
                s->bands, 2.0, 8.0, pos, 130, 200);
            s->bands = std::round(s->bands);

            SimpleGUI::Slider(U"shadow: {:.2f}"_fmt(s->floorV),
                s->floorV, 0.0, 0.6, pos + Vec2{ 0, 40 }, 130, 200);
        };
        return e;
    }

    //-----------------------------------------------
    // ピクセルアート (4x4 ブロック平均 + カラー量子化)
    //   param0.x = levels (2..16)
    //-----------------------------------------------
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
        e.apply = [s](const MSRenderTexture& src)
        {
            s->cb = Float4{ static_cast<float>(s->levels), 0.0f, 0.0f, 0.0f };
            Graphics2D::SetPSConstantBuffer(1, s->cb);

            const ScopedCustomShader2D shader{ s->ps };
            src.draw();
        };
        e.drawUI = [s](const Vec2& pos)
        {
            SimpleGUI::Slider(U"levels: {:.0f}"_fmt(s->levels),
                s->levels, 2.0, 16.0, pos, 130, 200);
            s->levels = std::round(s->levels);
        };
        return e;
    }

    //-----------------------------------------------
    // インクアウトライン (Sobel エッジ検出)
    //   param0.x = threshold    (0..1)
    //   param0.y = thickness    (0.5..3) テクセル間隔
    //   param0.z = inkStrength  (0..1)  線の濃さ
    //-----------------------------------------------
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
        e.apply = [s](const MSRenderTexture& src)
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
            SimpleGUI::Slider(U"threshold: {:.2f}"_fmt(s->threshold),
                s->threshold, 0.02, 0.8, pos, 130, 200);
            SimpleGUI::Slider(U"thickness: {:.1f}"_fmt(s->thickness),
                s->thickness, 0.5, 3.0, pos + Vec2{ 0, 40 }, 130, 200);
            SimpleGUI::Slider(U"ink: {:.2f}"_fmt(s->inkStrength),
                s->inkStrength, 0.0, 1.0, pos + Vec2{ 0, 80 }, 130, 200);
        };
        return e;
    }

    //-----------------------------------------------
    // 既定の効果セット
    //   新シェーダ追加時はこの関数に push_back を 1 行足すだけ
    //-----------------------------------------------
    Array<Effect> CreateDefaultEffects()
    {
        Array<Effect> effects;
        effects.push_back(MakeNoneEffect());
        effects.push_back(MakeToonEffect());
        effects.push_back(MakePixelArtEffect());
        effects.push_back(MakeOutlineEffect());
        return effects;
    }
}
