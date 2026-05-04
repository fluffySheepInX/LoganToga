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
    struct EffectDescriptor
    {
        StringView name;
        StringView category;
        StringView tooltip;
        int32 uiRowCount = 0;
        bool requiresSceneDepth = false;
        Effect(*factory)() = nullptr;
    };

    // HLSL/GLSL ペアを 1 行で読む共通ヘルパ
    //   useEffectParams = true で b1 (PSEffectParams) もバインドする
    [[nodiscard]]
    PixelShader LoadPS(StringView name, bool useEffectParams = true);

    // 既定のエフェクト記述子一覧
    [[nodiscard]]
    const Array<EffectDescriptor>& GetDefaultEffectDescriptors();

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
    [[nodiscard]] Effect MakeGlitchEffect();
    [[nodiscard]] Effect MakeSwirlEffect();
    [[nodiscard]] Effect MakeExtractBrightEffect();
    [[nodiscard]] Effect MakeBloomEffect();
    [[nodiscard]] Effect MakeDepthOfFieldEffect();
    [[nodiscard]] Effect MakeTonemapACESEffect();
    [[nodiscard]] Effect MakeVignetteEffect();
    [[nodiscard]] Effect MakeFilmGrainEffect();
    [[nodiscard]] Effect MakeFXAAEffect();
    [[nodiscard]] Effect MakeWarmGradeEffect();

    // Cinematic プリセット (Bloom -> Tonemap(ACES) -> Vignette -> FilmGrain -> FXAA)
    //   引数の descriptors は GetDefaultEffectDescriptors() の戻り値
    [[nodiscard]]
    Array<size_t> GetCinematicPresetChain(const Array<EffectDescriptor>& descriptors);

    // Dusty プリセット (2000s 海外ゲーム風: Bloom -> Tonemap -> WarmGrade -> Vignette -> FilmGrain -> FXAA)
    [[nodiscard]]
    Array<size_t> GetDustyPresetChain(const Array<EffectDescriptor>& descriptors);

    // 既定の効果セットを構築 (新規追加時は EffectDescriptor 一覧に 1 行追加する)
    [[nodiscard]]
    Array<Effect> CreateDefaultEffects();
}
