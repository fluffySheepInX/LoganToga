
//-----------------------------------------------
//
//	Effects.cpp
//	共通処理と効果一覧の実装
//
//-----------------------------------------------

# include "Effects.hpp"
# include "../Addons/Pi3D/Shader/PiShaderLoader.hpp"

namespace pe
{
    namespace
    {
        Optional<Texture> g_sceneDepthTexture;
    }

    PixelShader LoadPS(StringView name, bool useEffectParams)
    {
        Array<ConstantBufferBinding> bindings = { { U"PSConstants2D", 0 } };
        if (useEffectParams)
        {
            bindings.push_back({ U"PSEffectParams", 1 });
        }

        const FilePath hlslPath = Pi3D::PiShaderLoader::HLSL(name);
        const FilePath glslPath = Pi3D::PiShaderLoader::GLSLFragment(name);

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

    void SetSceneDepthTexture(const Optional<Texture>& texture)
    {
        g_sceneDepthTexture = texture;
    }

    const Optional<Texture>& GetSceneDepthTexture()
    {
        return g_sceneDepthTexture;
    }

    //-----------------------------------------------
    // Cinematic プリセット
    //-----------------------------------------------
    Array<size_t> GetCinematicPresetChain(const Array<Effect>& effects)
    {
        const auto findIndex = [&](StringView name) -> size_t
        {
            for (size_t i = 0; i < effects.size(); ++i)
            {
                if (effects[i].name == name) { return i; }
            }
            return 0; // 見つからなければ「なし」
        };

        return {
            findIndex(U"Bloom"),
            findIndex(U"Tonemap (ACES)"),
            findIndex(U"Vignette"),
            findIndex(U"Film Grain"),
            findIndex(U"FXAA"),
        };
    }

    //-----------------------------------------------
    // Dusty プリセット (2000s 海外ゲーム風)
    //-----------------------------------------------
    Array<size_t> GetDustyPresetChain(const Array<Effect>& effects)
    {
        const auto findIndex = [&](StringView name) -> size_t
        {
            for (size_t i = 0; i < effects.size(); ++i)
            {
                if (effects[i].name == name) { return i; }
            }
            return 0;
        };

        return {
            findIndex(U"Bloom"),
            findIndex(U"Tonemap (ACES)"),
            findIndex(U"Warm Grade"),
            findIndex(U"Vignette"),
            findIndex(U"Film Grain"),
            findIndex(U"FXAA"),
        };
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
        effects.push_back(MakeGrayscaleEffect());
        effects.push_back(MakePosterizeEffect());
        effects.push_back(MakeRGBShiftEffect());
        effects.push_back(MakeGlitchEffect());
        effects.push_back(MakeSwirlEffect());
        effects.push_back(MakeExtractBrightEffect());
        effects.push_back(MakeBloomEffect());
        effects.push_back(MakeDepthOfFieldEffect());
        effects.push_back(MakeTonemapACESEffect());
        effects.push_back(MakeVignetteEffect());
        effects.push_back(MakeFilmGrainEffect());
        effects.push_back(MakeFXAAEffect());
        effects.push_back(MakeWarmGradeEffect());
        return effects;
    }
}
