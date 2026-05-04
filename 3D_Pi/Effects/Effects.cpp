
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
        [[nodiscard]] Effect CreateEffectFromDescriptor(const EffectDescriptor& descriptor)
        {
            Effect effect = descriptor.factory ? descriptor.factory() : Effect{};
            effect.name = descriptor.name;
            effect.tooltip = descriptor.tooltip;
            effect.uiRowCount = descriptor.uiRowCount;
            effect.requiresSceneDepth = descriptor.requiresSceneDepth;
            return effect;
        }
    }

    const Array<EffectDescriptor>& GetDefaultEffectDescriptors()
    {
        static const Array<EffectDescriptor> descriptors = {
            { U"なし", U"Utility", U"画面に変化を加えません。比較用の基準です。\n使いどころ: 他のエフェクトとの差を見たい時、段ごとの効き具合確認時。", 0, false, &MakeNoneEffect },
            { U"トゥーン", U"Stylize", U"明暗の段数を減らして、セル画っぽい陰影にします。\n使いどころ: アニメ調、陰影を整理したい時、形をくっきり見せたい時。", 2, false, &MakeToonEffect },
            { U"ピクセルアート", U"Stylize", U"色の階調を整理して、簡略化されたレトロ画面寄りにします。\n使いどころ: レトロゲーム風、粗い質感を出したい時、情報量を減らしたい時。", 1, false, &MakePixelArtEffect },
            { U"アウトライン", U"Stylize", U"輪郭線を強調して、被写体の外形を見やすくします。\n使いどころ: トゥーン表現、読みやすさ重視、オブジェクトを目立たせたい時。", 3, false, &MakeOutlineEffect },
            { U"油絵 (Kuwahara)", U"Stylize", U"色のばらつきをならして、筆で塗ったような油絵風にします。\n使いどころ: 絵画調、夢っぽい画、現実感を少し崩したい時。", 1, false, &MakeKuwaharaEffect },
            { U"CRT", U"Display", U"湾曲、走査線、色マスクで古いモニタ風にします。\n使いどころ: レトロSF、ブラウン管演出、監視画面やゲーム内端末の表現。", 4, false, &MakeCRTEffect },
            { U"グレースケール", U"Color", U"色を抜いて白黒にします。\n使いどころ: 回想、監視カメラ、雰囲気確認、明暗だけで見たい時。", 0, false, &MakeGrayscaleEffect },
            { U"ネガポジ反転", U"Color", U"RGB を反転してネガフィルムのような見た目にします。\n使いどころ: 異常演出、ダメージ表現、強い違和感や反転世界の表現。", 0, false, &MakeInvertEffect },
            { U"モザイク", U"Stylize", U"画面をブロック単位で粗くして、低解像度化したように見せます。\n使いどころ: ぼかし隠し、検閲表現、レトロ演出、情報量を落としたい時。", 1, false, &MakeMosaicEffect },
            { U"2値化（ベイヤーディザー）", U"Color", U"ベイヤー行列で規則的な網点パターンを加えながら白黒2値化します。\n使いどころ: レトロ印刷風、ドット表現、階調を模様で残したい時。", 3, false, &MakeBayerDitherEffect },
            { U"ポスタライズ", U"Color", U"色数を減らして、ベタ塗り感の強い見た目にします。\n使いどころ: 印刷物風、グラフィック調、派手な記号化をしたい時。", 0, false, &MakePosterizeEffect },
            { U"RGB シフト", U"Distort", U"色チャンネルを少しずらして、色収差やズレを出します。\n使いどころ: 軽いグリッチ、酔った視界、異常感やサイバー感を足したい時。", 0, false, &MakeRGBShiftEffect },
            { U"Glitch", U"Distort", U"横ずれ、RGB分離、ブロックノイズでデジタル破綻風にします。\n使いどころ: SFハッキング、通信障害、異常演出、サイバー感を強く出したい時。", 3, false, &MakeGlitchEffect },
            { U"スワール", U"Distort", U"画面をねじるように歪ませます。\n使いどころ: ワープ、魔法、違和感演出、画面遷移のアクセント。", 1, false, &MakeSwirlEffect },
            { U"ブライトパス抽出", U"Utility", U"明るい部分だけを抜き出します。単体では地味ですが Bloom の素材になります。\n使いどころ: 発光部分の確認、Bloom 用の事前抽出、輝度マスクの確認。", 0, false, &MakeExtractBrightEffect },
            { U"Bloom", U"Light", U"明るい部分をにじませて、発光感を足します。\n使いどころ: 夜景、ネオン、魔法、映像を少しリッチに見せたい時。", 4, false, &MakeBloomEffect },
            { U"DoF", U"Camera", U"ピント面以外をぼかして、被写界深度を作ります。\n使いどころ: 被写体強調、ミニチュア風、シネマ風、視線誘導したい時。", 5, true, &MakeDepthOfFieldEffect },
            { U"Tonemap (ACES)", U"Color", U"明るさを整えて、白飛びを抑えつつ映画っぽい階調に寄せます。\n使いどころ: HDR感の整理、Bloom 後の画作り、全体を自然にまとめたい時。", 2, false, &MakeTonemapACESEffect },
            { U"Vignette", U"Camera", U"画面周辺を暗くして、中央へ視線を集めます。\n使いどころ: シネマ風、視線誘導、緊張感や閉塞感を足したい時。", 3, false, &MakeVignetteEffect },
            { U"Film Grain", U"Camera", U"細かな粒状ノイズを重ねて、フィルムや高感度撮影っぽくします。\n使いどころ: 映画風、アナログ感、無機質なCGを少し馴染ませたい時。", 2, false, &MakeFilmGrainEffect },
            { U"FXAA", U"Utility", U"輪郭のジャギーを軽くぼかして目立ちにくくします。\n使いどころ: 最終段でのギザギザ軽減、画面を少し滑らかに見せたい時。", 1, false, &MakeFXAAEffect },
            { U"Warm Grade", U"Color", U"全体を暖色寄りに寄せ、乾いた古い洋ゲー風の色味にします。\n使いどころ: 夕景、荒野、懐かしい海外ゲーム風、土っぽい空気感を出したい時。", 4, false, &MakeWarmGradeEffect },
        };

        return descriptors;
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

    //-----------------------------------------------
    // Cinematic プリセット
    //-----------------------------------------------
    Array<size_t> GetCinematicPresetChain(const Array<EffectDescriptor>& descriptors)
    {
        const auto findIndex = [&](StringView name) -> size_t
        {
            for (size_t i = 0; i < descriptors.size(); ++i)
            {
                if (descriptors[i].name == name) { return i; }
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
    Array<size_t> GetDustyPresetChain(const Array<EffectDescriptor>& descriptors)
    {
        const auto findIndex = [&](StringView name) -> size_t
        {
            for (size_t i = 0; i < descriptors.size(); ++i)
            {
                if (descriptors[i].name == name) { return i; }
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
    //   新規追加時は EffectDescriptor 一覧に 1 行足すだけ
    //-----------------------------------------------
    Array<Effect> CreateDefaultEffects()
    {
        const auto& descriptors = GetDefaultEffectDescriptors();
        Array<Effect> effects;
        effects.reserve(descriptors.size());

        for (const auto& descriptor : descriptors)
        {
            effects << CreateEffectFromDescriptor(descriptor);
        }

        return effects;
    }
}
