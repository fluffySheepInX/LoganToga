//-----------------------------------------------
//
//	Effect.hpp
//	2D ポストエフェクトの基本構造
//
//	規約:
//	- HLSL/GLSL の constant buffer は b0 = Siv3D の PSConstants2D で固定。
//	  ユーザー領域は b1 (PSEffectParams) を使用する。
//	- ConstantBuffer<T> の T は 16 byte (Float4) 単位でアラインすること。
//	- 将来のシェーダ重ね掛けは、apply の入出力を中間 RT 経由に拡張する形で対応予定。
//
//-----------------------------------------------

# pragma once
# include <Siv3D.hpp>

namespace pe // post-effect
{
    // ひとつの 2D ポストエフェクト
    struct Effect
    {
        String name;

        // MSRenderTexture を入力としてシーンへ最終描画する
        std::function<void(const MSRenderTexture&)> apply;

        // 選択中だけ呼ばれるパラメータ UI 描画 (任意 / null 可)
        std::function<void(const Vec2& topLeft)> drawUI;
    };
}
