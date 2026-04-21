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

    // 既定の効果セットを構築 (新シェーダ追加時はここに 1 行追加する)
    [[nodiscard]]
    Array<Effect> CreateDefaultEffects();
}
