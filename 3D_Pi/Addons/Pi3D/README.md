# Pi3D Addon

Pi3D は OpenSiv3D 向けの 3D レンダリング・環境・照明・ポストエフェクト実験 Addon です。

## 現在の状態

`Addons/Pi3D` は自己完結です。addon 内の include は addon 内部と `<Siv3D.hpp>` のみで、ホストプロジェクトのコードへの依存はありません。シェーダー・テクスチャ・プリセット TOML は `Resources/` に同梱しています。

他プロジェクトへの展開手順:

1. `Addons/Pi3D` フォルダを丸ごとコピーする
2. `PostEffects/*.cpp` をプロジェクトに登録する
3. `Pi3D::Config` でリソースパスを設定し `Pi3D::RegisterAddon(config)` を呼ぶ

分離状態は `Tests/Pi3DConsumerSmoke` のビルドで検証できます。必要なファイルと既定配置は `manifest.toml` に記載しています。

## 初期化

`Pi3D.hpp` を include し、最初の `Pi3D::Instance()` または引数なし `Pi3D::RegisterAddon()` より前に構成を登録します。

```cpp
Pi3D::Config config;
config.shaderDirectory = U"example/shader";
config.textureDirectory = U"texture";
config.effectPresetsPath = U"../Addons/Pi3D/Resources/toml/effect_presets.toml";
config.lightingPresetsPath = U"../Addons/Pi3D/Resources/toml/lighting_presets.toml";
config.settingsPath = U"save/pi3d_settings.toml";
config.editorUIEnabled = true;
config.persistenceEnabled = true;

if (not Pi3D::RegisterAddon(config))
{
	throw Error{ U"Pi3D configuration must be registered before System construction" };
}
```

引数なし `Pi3D::RegisterAddon()` は既存コードとの互換性のため維持されています。

## フレーム呼び出し順

1. `Pi3D::Update(cameraEye, cameraFocus)`
2. `Pi3D::Begin3D(drawScene)`
3. `Pi3D::End3D(drawSceneForDepth)`
4. 必要な場合だけ `Pi3D::DrawUI()`

カメラのホイール操作を制御する場合は `Pi3D::WantsMouseWheelCapture()` を確認します。

## UI と永続化を使わない場合

`editorUIEnabled = false` で Editor UI の描画と入力捕捉を停止できます。`persistenceEnabled = false` で設定ファイルの読み書きを停止できます。

`PI3D_ENABLE_EDITOR_UI=0` を定義すると、エディタパネル層（EditorIconLayout、各 UI 実装ファイル、アイコン画像の読み込み）がコンパイルから除外され、`drawUI()` は no-op、`WantsMouseWheelCapture()` は常に false になります。なお `UI/RectUI.hpp` / `UI/Layout.hpp` はエフェクトパラメータ記述子が使うコア依存として常時コンパイルされます。

## パス構成の規則

- `shaderDirectory` は `hlsl/` と `glsl/` を含むディレクトリです。
- `textureDirectory` は Pi3D Editor 用画像を含むディレクトリです。
- preset の明示パスが空の場合だけ、既存プロジェクト互換の候補を探索します。
- Config は System 構築時に固定されます。構築後の `SetConfig()` は失敗します。

## 次段階

1. 必要に応じて PiLighting の UI 実装を専用 .ipp へ物理分割する
