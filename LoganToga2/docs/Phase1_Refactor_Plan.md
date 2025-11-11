# フェーズ1 リファクタリング計画（Service 化＋Strategy 抽出）

本ドキュメントは、`Battle001` 周辺の戦闘処理に対して低リスクで効果の高いリファクタリングを行うための概要・メリット/デメリット・手順をまとめたものです。

##目的
- 巨大関数と分岐の集中を解消し、変更影響を局所化
- 並列処理／スレッド安全性の改善ポイントを明確化
- 単体テスト容易化（ロジックを疎結合に分離）

## スコープ（フェーズ1）
- Service 化（Facade）
 - `gSkillReadyAtSec` → `CooldownService`
 - `gSkillUsesLeft` → `UsageService`
 - `gHitIntervalState` → `ProjectileHitService`
 -霧更新（差分計算） → `FogService`（既存ロジックの囲い込み）
- Strategy 抽出
 - 弾道（`MoveType::thr`/直進） → `ITrajectory`（`ArcTrajectory` / `LinearTrajectory`）
 - ダメージモデル（`SkillStrKind`） → `IDamageModel`（物理/魔法/混合など）
 - ターゲティング → `ITargetingStrategy`（最近傍/最優先/回復優先など）
- Factory/Builder
 - `createSkillExecution` の生成手順を `ProjectileFactory`へ移譲

## メリット
-変更容易性
 - 新しい弾道/ダメージ/ターゲティングの追加がクラス追加で完結
- テスト容易性
 - Service/Strategy をモック差し替え可能
- 可読性/責務分離
 - `Battle001` の肥大化を抑制し、役割ごとに把握可能に
- 並列化に向けた基盤
 -共有状態を Service に集約し、排他/スナップショット境界を明確化

## デメリット/リスク
- 短期的なクラス数増加による認知コスト
- インターフェース分割に伴う初期配線コスト
- 性能回帰リスク（間接化によるオーバーヘッド）
 - 緩和策: Strategy の選択は初期化時に固定、ホットループでは関数ポインタ/参照を再利用

## 設計概要
- Service 層（ステート集約/排他管理）
 - `CooldownService`
 - `bool isReady(UnitID, SkillTag)`
 - `void commit(UnitID, SkillTag, seconds)`
 - `UsageService`
 - `bool hasUsesLeft(UnitID, SkillTag)`
 - `void setUses(UnitID, SkillTag, count)` / `void consume(UnitID, SkillTag)`
 - `ProjectileHitService`
 - `bool shouldCheck(uint64 bulletKey, Vec2 newPos, double threshold)`
 - `void forgetByBulletNo(int32 bulletNo)`
 - `FogService`
 - `Grid<Visibility> compute(const Snapshot& units)` / 差分適用

- Strategy 層（分岐排除）
 - `ITrajectory::eval(start, end, t)`
 - 実装: `LinearTrajectory`, `ArcTrajectory(heightRatio)`
 - `IDamageModel::calc(attacker, target, skill)`
 - 実装:物理/魔法/混合（`SkillStrKind` 対応）
 - `ITargetingStrategy::pick(attacker, candidates, skill)`
 - 実装: 最近傍/最優先/回復優先 etc.

- Factory
 - `ProjectileFactory::create(attacker, target, skill)`
 - 弾生成・寿命・速度計算の一元化

## 実装手順（推奨順）
1) 下準備
 - `LoganToga2/Services/` `LoganToga2/Combat/` ディレクトリを作成
 -既存グローバル（`gSkillReadyAtSec`, `gSkillUsesLeft`, `gHitIntervalState`）の用途を洗い出し

2) Service の導入（置き換えは段階的）
 - `CooldownService` を導入し、`isSkillReady`/`commitCooldown` 内部を差し替え
 - `UsageService` を導入し、`hasUsesLeft`/`consumeUse`/`setSkillUses` を委譲
 - `ProjectileHitService` を導入し、`updateAndCheckCollisions` の間引き判定と掃除を委譲
 -霧更新を `FogService` に囲い込み（スレッド間共有データの集約）

3) Strategy 抽出
 - 弾道: `MoveType` 分岐を `ITrajectory` 実装に移動
 - `thr` → `ArcTrajectory(heightRatio)`、それ以外 → `LinearTrajectory`
 - ダメージ: `CalucDamage` の `switch(SkillStrKind)` を `IDamageModel` に移譲
 - ターゲティング: `tryActivateSkillOnTargetGroup` 内のフィルタ/優先ロジックを `ITargetingStrategy` に分離

4) Factory/Builder
 - `createSkillExecution`から弾の生成・速度・寿命の計算を `ProjectileFactory`へ切り出し
 - 呼び出し側は Factory に依存し、詳細計算を知らない構造へ

5) 差し替えポイントの固定
 - 初期化時に `Strategy` を選択して `Battle001` に注入（あるいは `CommonConfig`から決定）
 - フレーム中は参照を再利用して間接呼び出しを最小化

6) テスト
 - 各 Service/Strategy のユニットテスト（正常/境界/異常）
 - `Battle001` はモックを注入した結合テストで最小限の回帰確認

##影響範囲
- `Battle001.cpp` の以下の関数の内部実装が置き換え対象
 - `isSkillReady` / `commitCooldown` / `hasUsesLeft` / `consumeUse` / `setSkillUses`
 - `updateAndCheckCollisions`（ヒット判定間引き/掃除）
 - 弾道計算（`MoveType::thr` 分岐）
 - `CalucDamage`（ダメージモデル）
 - `tryActivateSkillOnTargetGroup`（ターゲティング）
 - `createSkillExecution`（生成手順）

## 完了の受け入れ基準（DoD）
-主要グローバル状態が Service 経由でのみアクセスされる
- 弾道/ダメージ/ターゲティングの分岐が Strategy 実装に移動
- 挙動回帰がない（既存データで同等の結果になる）
- 単体テストが追加され、CIで実行可能

## リスクと緩和策
- 回帰バグ
 - 比較テスト（Before/After のログ一致、リプレイテスト）
- パフォーマンス劣化
 - Strategy参照のキャッシュ、`noexcept`/`inline` 適用、構造体の移動/参照渡し徹底
-競合/デッドロック
 - Service 内で排他ポリシーを明示（`std::shared_mutex` 等）

## ロールバック方針
- 段階的に `#if` ガード/ブランチで旧実装へ切り替え可能に
- Service/Strategy を差し替え前の関数にフォールバックするアダプタを保持

## 作業見積（目安）
- Service 化:0.5?1.0 人日
- Strategy 抽出（3種）:1.0?1.5 人日
- Factory 抽出:0.5 人日
- テスト/回帰確認:0.5?1.0 人日
