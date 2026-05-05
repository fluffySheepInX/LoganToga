# 3D_Pi Architecture Review

## 実施した改善

- `Main.cpp` に直接埋め込まれていた階段生成機能を `Procedural/StairGenerator.hpp` に分離した。
- 階段生成の状態、入力処理、3D 描画、UI 描画を `procedural::StairGenerator` に集約した。
- `Main.cpp` はアプリケーション起動、カメラ、各サブシステムの更新順序、シーン合成に集中する構成へ寄せた。
- 既存の Rect ベース UI 方針に合わせ、分離後も `UI/RectUI.hpp` のコントロールを利用した。

## 改善すべき点・懸念点

1. **Main.cpp の責務がまだ広い**
   - カメラ操作、アセットロード、Pi3D アドオン制御、プレビュー状態、エディタ統合が混在している。
   - 次段階では `AppController`、`SceneAssets`、`CameraController` などに分けると変更影響を抑えやすい。

2. **ヘッダーオンリー実装が増えている**
   - `RoadEditor.hpp`、`TextureEditor.hpp`、`StairGenerator.hpp` などに実装が集中している。
   - ビルド時間、依存関係、差分レビューの観点では `.cpp` へ実装を移す余地がある。

3. **UI とドメインロジックの結合が強い**
   - エディタクラスがデータ、入力、UI、描画を同時に持っている。
   - 保存対象モデル、操作コマンド、UI 表示を分離するとテストや将来の別 UI 対応が容易になる。

4. **入力キャプチャの調停が中央集権的でない**
   - `Main.cpp` が各エディタの `wantsMouseCapture()` を直接問い合わせている。
   - `InputCaptureRegistry` のような小さい調停層を用意すると、エディタ追加時の競合を避けやすい。

5. **保存パスとアセットパスがコードに直書きされている**
   - `data/roads.toml`、`data/ground_layers.toml`、`example/obj/...` などが各所に散っている。
   - TOML ベースの設定ファイルに寄せると環境差分や将来のプリセット切替に強くなる。

6. **生成オブジェクトの永続化方針が未整理**
   - 階段生成データは現状メモリ上のみで、Road/Ground のような TOML 保存がない。
   - `GeneratedStair` の TOML serializer を追加し、ロード・保存・Undo 方針をそろえるべき。

7. **シーン描画順序が Main.cpp の手続き順に依存している**
   - Ground、TextureEditor、RoadEditor、Procedural、モデル、Environment の描画順が固定コード化されている。
   - レイヤーまたは render pass の概念を導入すると、透明オブジェクトやポストエフェクト追加時の破綻を防ぎやすい。

8. **エディタ間のショートカット衝突リスクがある**
   - RoadEditor と TextureEditor がどちらも `S` / `L` を使う。
   - アクティブツール制、または入力コンテキストを導入し、同時有効時の意図しない保存・ロードを防ぐ必要がある。

9. **Visual Studio フィルター定義が重複している**
   - `3D_Pi.vcxproj.filters` に `Effects\\*.cpp` の重複エントリが多数存在する。
   - IDE 上の見通しと保守性を下げるため、明示ファイル列挙または重複削除が望ましい。

10. **依存方向のルールが明文化されていない**
    - `Addons`、`Road`、`Ground`、`Procedural`、`UI` の依存可能範囲がコード規約として見えない。
    - 例: UI はドメインに依存しない、各エディタは Pi3D のグローバル状態へ直接依存しない、などのルールを設けると拡張時に崩れにくい。

## 推奨する次の分割案

- `Application/AppController.hpp/.cpp`
  - Main ループ、各サブシステムの update/draw 呼び出しを担当。
- `Application/CameraController.hpp/.cpp`
  - プリセット、ホイールズーム、ドラッグパン、UI ブロック判定を担当。
- `Application/SceneAssets.hpp/.cpp`
  - ground mesh、texture、model のロードと描画補助を担当。
- `Procedural/StairTypes.hpp`
  - `GeneratedStair` など保存可能なデータ型を分離。
- `Procedural/StairSerializer.hpp`
  - TOML による階段生成データの永続化。
