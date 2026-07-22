# Pi3D Consumer Smoke Build

`Addons/Pi3D` が既存の `3D_Pi` アプリ固有コードへ依存せず、別の OpenSiv3D プロジェクトからコンパイルできることを検証します。

## 検証内容

- `<Pi3D/Pi3D.hpp>` から公開 API 一式をコンパイル
- `Addons/Pi3D/PostEffects/*.cpp` を独立した Consumer へ登録
- addon 内の引用 include がパッケージ境界外へ出ていないことを検査
- 必須 HLSL / GLSL、Editor 画像、TOML の存在を検査
- Debug / Release x64 の双方に対応

StaticLibrary のため、OpenSiv3D の実行環境やウィンドウ初期化は必要ありません。実行時の描画確認ではなく、コピー移植時のコード境界と配布物欠落を検出するテストです。

## 実行

Visual Studio で `Pi3DConsumerSmoke` プロジェクトをビルドします。コマンドラインでは Developer PowerShell から次を実行します。

- Debug: `msbuild 3D_Pi\Tests\Pi3DConsumerSmoke\Pi3DConsumerSmoke.vcxproj /p:Configuration=Debug /p:Platform=x64`
- Release: `msbuild 3D_Pi\Tests\Pi3DConsumerSmoke\Pi3DConsumerSmoke.vcxproj /p:Configuration=Release /p:Platform=x64`

パッケージ境界とリソースだけを確認する場合は、`Validate-Pi3DPackage.ps1` を単独実行できます。
