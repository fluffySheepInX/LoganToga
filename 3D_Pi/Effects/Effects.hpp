//-----------------------------------------------
//
//	Effects.hpp
//	効果ファクトリの宣言と一覧構築
//
//-----------------------------------------------

# pragma once
# include "Effect.hpp"

namespace pe
{
    // HLSL/GLSL ペアを 1 行で読む共通ヘルパ
    //   useEffectParams = true で b1 (PSEffectParams) もバインドする
    [[nodiscard]]
    PixelShader LoadPS(StringView name, bool useEffectParams = true);

    // 各効果ファクトリ
    [[nodiscard]] Effect MakeNoneEffect();
    [[nodiscard]] Effect MakeToonEffect();
    [[nodiscard]] Effect MakePixelArtEffect();
    [[nodiscard]] Effect MakeOutlineEffect();
    [[nodiscard]] Effect MakeKuwaharaEffect();
    [[nodiscard]] Effect MakeCRTEffect();
    [[nodiscard]] Effect MakeGrayscaleEffect();
    [[nodiscard]] Effect MakePosterizeEffect();
    [[nodiscard]] Effect MakeRGBShiftEffect();
    [[nodiscard]] Effect MakeSwirlEffect();
    [[nodiscard]] Effect MakeExtractBrightEffect();
    [[nodiscard]] Effect MakeBloomEffect();
    [[nodiscard]] Effect MakeTonemapACESEffect();
    [[nodiscard]] Effect MakeVignetteEffect();
    [[nodiscard]] Effect MakeFilmGrainEffect();
    [[nodiscard]] Effect MakeFXAAEffect();

    // Cinematic プリセット (Bloom -> Tonemap(ACES) -> Vignette -> FilmGrain -> FXAA)
    //   引数の effects は CreateDefaultEffects() の戻り値 (順序依存しない、名前で解決)
    [[nodiscard]]
    Array<size_t> GetCinematicPresetChain(const Array<Effect>& effects);

    // 既定の効果セットを構築 (新シェーダ追加時はここに 1 行追加する)
    [[nodiscard]]
    Array<Effect> CreateDefaultEffects();
}
