//-----------------------------------------------
//
//	Effects.cpp
//	効果ファクトリの実装
//
//-----------------------------------------------

# include "Effects.hpp"
# include "../UI/RectUI.hpp"

namespace pe
{
    PixelShader LoadPS(StringView name, bool useEffectParams)
    {
        Array<ConstantBufferBinding> bindings = { { U"PSConstants2D", 0 } };
        if (useEffectParams)
        {
            bindings.push_back({ U"PSEffectParams", 1 });
        }

        const FilePath hlslPath = U"example/shader/hlsl/" + String{ name } + U".hlsl";
        const FilePath glslPath = U"example/shader/glsl/" + String{ name } + U".frag";

        // engine_log とは別に、確実に書き出されるテキストログにも残す
        {
            TextWriter w{ U"shader_load_log.txt", OpenMode::Append };
            if (w)
            {
                w.writeln(U"[LoadPS] name={}  hlsl_exists={} glsl_exists={}"_fmt(
                    name, FileSystem::Exists(hlslPath), FileSystem::Exists(glslPath)));
                w.writeln(U"  hlsl_full={}"_fmt(FileSystem::FullPath(hlslPath)));
                w.writeln(U"  glsl_full={}"_fmt(FileSystem::FullPath(glslPath)));
            }
        }

        Logger << U"[pe::LoadPS] loading shader: {}"_fmt(name);
        Logger << U"  HLSL: {} (exists={})"_fmt(hlslPath, FileSystem::Exists(hlslPath));
        Logger << U"  GLSL: {} (exists={})"_fmt(glslPath, FileSystem::Exists(glslPath));

        const PixelShader ps =
            HLSL{ hlslPath, U"PS" }
          | GLSL{ glslPath, bindings };

        if (not ps)
        {
            {
                TextWriter w{ U"shader_load_log.txt", OpenMode::Append };
                if (w) { w.writeln(U"  *** FAILED to compile/link: {}"_fmt(name)); }
            }
            Logger << U"[pe::LoadPS] FAILED to compile/load shader: {}"_fmt(name);
            throw Error{ U"Failed to load shader: {}"_fmt(name) };
        }

        Logger << U"[pe::LoadPS] OK: {}"_fmt(name);
        {
            TextWriter w{ U"shader_load_log.txt", OpenMode::Append };
            if (w) { w.writeln(U"  OK: {}"_fmt(name)); }
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
        e.apply = [](const Texture& src)
        {
            src.draw();
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

    //-----------------------------------------------
    // 油絵風 (Kuwahara フィルタ)
    //   param0.x = radius (1..6)
    //-----------------------------------------------
    Effect MakeKuwaharaEffect()
    {
        struct State
        {
            PixelShader ps = LoadPS(U"kuwahara");
            ConstantBuffer<Float4> cb{ Float4{ 3.0f, 0.0f, 0.0f, 0.0f } };
            double radius = 3.0;
        };
        auto s = std::make_shared<State>();

        Effect e;
        e.name = U"油絵 (Kuwahara)";
        e.apply = [s](const Texture& src)
        {
            s->cb = Float4{ static_cast<float>(s->radius), 0.0f, 0.0f, 0.0f };
            Graphics2D::SetPSConstantBuffer(1, s->cb);

            const ScopedCustomShader2D shader{ s->ps };
            src.draw();
        };
        e.drawUI = [s](const Vec2& pos)
        {
            ui::SliderH(U"radius: {:.0f}"_fmt(s->radius),
                s->radius, 1.0, 6.0, pos, 130, 200);
            s->radius = std::round(s->radius);
        };
        return e;
    }

    //-----------------------------------------------
    // CRT / ブラウン管
    //   param0.x = curvature        (0..0.5)
    //   param0.y = scanlineStrength (0..1)
    //   param0.z = maskStrength     (0..1)
    //   param0.w = vignette         (0..1)
    //-----------------------------------------------
    Effect MakeCRTEffect()
    {
        struct State
        {
            PixelShader ps = LoadPS(U"crt");
            ConstantBuffer<Float4> cb{ Float4{ 0.10f, 0.45f, 0.35f, 0.55f } };
            double curvature = 0.10;
            double scanline  = 0.45;
            double mask      = 0.35;
            double vignette  = 0.55;
        };
        auto s = std::make_shared<State>();

        Effect e;
        e.name = U"CRT";
        e.apply = [s](const Texture& src)
        {
            s->cb = Float4{
                static_cast<float>(s->curvature),
                static_cast<float>(s->scanline),
                static_cast<float>(s->mask),
                static_cast<float>(s->vignette) };
            Graphics2D::SetPSConstantBuffer(1, s->cb);

            const ScopedCustomShader2D shader{ s->ps };
            src.draw();
        };
        e.drawUI = [s](const Vec2& pos)
        {
            ui::SliderH(U"curve: {:.2f}"_fmt(s->curvature),
                s->curvature, 0.0, 0.5, pos, 130, 200);
            ui::SliderH(U"scan: {:.2f}"_fmt(s->scanline),
                s->scanline, 0.0, 1.0, pos + Vec2{ 0, 40 }, 130, 200);
            ui::SliderH(U"mask: {:.2f}"_fmt(s->mask),
                s->mask, 0.0, 1.0, pos + Vec2{ 0, 80 }, 130, 200);
            ui::SliderH(U"vig: {:.2f}"_fmt(s->vignette),
                s->vignette, 0.0, 1.0, pos + Vec2{ 0, 120 }, 130, 200);
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
        effects.push_back(MakeKuwaharaEffect());
        effects.push_back(MakeCRTEffect());
        return effects;
    }
}
