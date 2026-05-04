//-----------------------------------------------
//
//	Effect.hpp
//	2D ポストエフェクトの基本構造
//
//	規約:
//	- HLSL/GLSL の constant buffer は b0 = Siv3D の PSConstants2D で固定。
//	  ユーザー領域は b1 (PSEffectParams) を使用する。
//	- ConstantBuffer<T> の T は 16 byte (Float4) 単位でアラインすること。
//	- apply は「現在バインドされているレンダーターゲット」へ src を描画する。
//	  呼び出し側が ScopedRenderTarget2D で中間 RT に切り替えれば自動的にチェインになる。
//
//-----------------------------------------------

# pragma once
# include <Siv3D.hpp>

namespace pe // post-effect
{
    struct EffectContext
    {
        Optional<Texture> sceneDepthTexture;
        Size sourceSize{ 0, 0 };
        double time = 0.0;
    };

    // ひとつの 2D ポストエフェクト
    struct Effect
    {
        String name;
        String tooltip;
        int32 uiRowCount = 0;
        bool requiresSceneDepth = false;

        // src を入力にして「現在の」レンダーターゲットへ描画する
        std::function<void(const Texture&, const EffectContext&)> apply;

        // 選択中だけ呼ばれるパラメータ UI 描画 (任意 / null 可)
        std::function<void(const Vec2& topLeft)> drawUI;

        // パラメータを初期値に戻す (任意 / null 可)
        std::function<void()> reset;
    };
}
