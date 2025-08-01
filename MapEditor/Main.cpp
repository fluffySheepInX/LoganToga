# include <Siv3D.hpp> // OpenSiv3D v0.6.8をインクルード（3Dゲームエンジンの基本機能を使用するため）

// 戦闘におけるプレイヤーの陣営を識別するためのenum（侵攻・防衛・中立の3状態を管理）
enum class BattleWhichIsThePlayer {
	Sortie,  // 侵攻側（攻撃を仕掛ける側）
	Def,     // 防衛側（守備を行う側）
	None     // 中立（どちらでもない状態）
};

// タイルのグラフィック表示に関する定数（アイソメトリック（斜め視点）表示のため）
inline constexpr Vec2 TileOffset{ 50, 25 };   // タイルの横幅50px、縦幅25pxでダイヤモンド形状を作る
inline constexpr int32 TileThickness = 15;    // タイルに立体感を与えるための厚み

// マップ上に配置されるユニット（キャラクター・兵士等）の情報を格納するクラス
class Unit
{
public:
	String houkou = U"";              // ユニットの向き（北・南・東・西等）戦闘時の攻撃方向を決定
	String ID = U"";                  // ユニットの識別子（重複を避けるため）
	int32 x = 0;                      // マップ上のX座標（グリッド単位）
	int32 y = 0;                      // マップ上のY座標（グリッド単位）
	String BattleWhichIsThePlayer;    // このユニットの所属陣営（仲間・敵・中立）

	// リソース関連の追加
	bool isResourcePoint = false;
	String resourceKey = U"";           // Resource-Keyの値
	String resourceType = U"";          // 金、木材、石材など
	int32 resourceAmount = 0;           // 資源量
	String resourceDisplayName = U"";   // 表示名
	String resourceIcon = U"";          // アイコンファイル名
};

class ResourceManager {
private:
	HashTable<Point, Unit> resourcePoints;  // 座標→リソース情報のマッピング

public:
	// リソースポイントを追加
	void addResourcePoint(const Point& pos, const String& key, const String& type = U"Gold",
						 int32 amount = 10, const String& displayName = U"", const String& icon = U"");

	// リソースポイントを取得
	Optional<Unit> getResourcePoint(const Point& pos) const;

	// 全リソースポイントを取得
	const HashTable<Point, Unit>& getAllResourcePoints() const { return resourcePoints; }

	// リソースポイントを削除
	void removeResourcePoint(const Point& pos);

	// クリア
	void clear() { resourcePoints.clear(); }
};

// 複数のテキスト入力欄の状態を一元管理するクラス（UIの状態管理を簡素化するため）
class TextEditStateManager {
private:
	HashTable<String, TextEditState> states;  // キー名とテキスト入力状態のマッピング

public:
	// 指定したキーのテキスト入力状態を取得（存在しない場合は新規作成）
	TextEditState& get(const String& key) {
		if (!states.contains(key)) {          // キーが存在しない場合のみ新規作成（重複防止）
			states[key] = TextEditState{};
		}
		return states[key];
	}

	// 指定したキーのテキスト内容を設定（外部からの値設定用）
	void set(const String& key, const String& value) {
		get(key).text = value;               // getを通すことで存在チェック不要
	}

	// 指定したキーのテキスト内容を取得（読み取り専用アクセス）
	String getText(const String& key) const {
		if (states.contains(key)) {          // 存在チェックが必要（constなのでgetが使えない）
			return states.at(key).text;
		}
		return U"";                          // 存在しない場合は空文字を返す
	}

	// テキスト内容を整数として取得（座標値等の数値入力用）
	int32 getInt(const String& key, int32 defaultValue = 0) const {
		String text = getText(key);
		return text.isEmpty() ? defaultValue : Parse<int32>(text);  // 空の場合はデフォルト値使用
	}

	// X・Y座標のペアを取得（2つのテキストボックスから座標を取得するため）
	Optional<Point> getPoint(const String& xKey, const String& yKey) const {
		String xText = getText(xKey);
		String yText = getText(yKey);
		if (xText.isEmpty() || yText.isEmpty()) {  // どちらかが空の場合は無効
			return none;
		}
		return Point{ Parse<int32>(xText), Parse<int32>(yText) };
	}

	// 座標値を2つのテキストボックスに分けて設定（内部的に文字列に変換して保存）
	void setPoint(const String& xKey, const String& yKey, const Point& point) {
		set(xKey, Format(point.x));          // int32を文字列に変換
		set(yKey, Format(point.y));
	}
};

// ファイルパス処理を共通化するユーティリティクラス（ファイル名抽出等の共通処理）
class FilePathUtils {
public:
	// フルパスからファイル名（拡張子なし）を抽出（TOMLファイル出力時の要素名生成用）
	static String extractFileName(const String& filePath) {
		String fileNameWithExtension = FileSystem::FileName(filePath);  // パスからファイル名+拡張子を取得
		return FileSystem::BaseName(fileNameWithExtension);             // 拡張子を除いたベース名を取得
	}

	// TOML要素のキー名を生成（ele0, ele1, ele2...の形式）
	static String createElementKey(int32 counter) {
		return U"ele{}"_fmt(counter);        // 文字列フォーマットでele+番号を生成
	}
};

// 指定ディレクトリからテクスチャファイルを読み込むクラス（タイル・建物画像の一括読み込み用）
class TextureLoader {
public:
	static Array<std::pair<String, Texture>> loadFromDirectory(const String& directory) {
		Array<std::pair<String, Texture>> textures;

		// 何も無しチップを表現する為（index 0は常に空テクスチャとして使用）
		textures.push_back({ U"", Texture{} });

		// ディレクトリ内の全ファイルを走査
		for (const auto& filePath : FileSystem::DirectoryContents(directory)) {
			if (FileSystem::IsFile(filePath)) {          // ディレクトリではなくファイルのみ処理
				textures.push_back({ filePath, Texture{ filePath } });  // パスとテクスチャのペアで保存
			}
		}

		return textures;
	}
};

// TOML形式のマップファイル生成を抽象化するクラス（複雑なフォーマット処理を隠蔽）
class TomlBuilder {
private:
	String content;              // 生成中のTOML内容を蓄積
	String newLine = U"\r\n";    // Windows形式の改行コード
	String tab = U"\t";          // インデント用のタブ文字

public:
	// TOMLファイルのマップセクション開始（[[Map]]ブロックの生成）
	void startMap(const String& mapName = U"Map001") {
		content += U"[[Map]]" + newLine;                    // TOMLの配列テーブル記法
		content += tab + U"name = \"{}\"" + newLine;        // マップ名（現在未使用だが将来の拡張用）
	}

	// テクスチャ要素の定義行を追加（ele0 = "grass"のような形式）
	void addElement(int32 counter, const String& fileName) {
		content += tab + U"ele{} = \"{}\""_fmt(counter, fileName) + newLine;
	}

	// マップデータセクション開始（複数行文字列の開始）
	void startDataSection() {
		content += tab + U"data = \"\"\"" + newLine;       // TOML複数行文字列の開始
	}

	// マップデータの1行を追加
	void addDataRow(const String& rowData) {
		content += rowData + newLine;
	}

	// マップデータセクション終了（複数行文字列の終了）
	void endDataSection() {
		content += tab + U"\"\"\"" + newLine;             // TOML複数行文字列の終了
	}

	// 完成したTOML文字列を取得
	String build() const {
		return content;
	}

	// 内容をクリア（再利用時のリセット用）
	void clear() {
		content.clear();
	}
};

// アイソメトリック座標計算を抽象化するクラス（複雑な座標変換ロジックをカプセル化）
class TileCoordinateSystem {
private:
	int32 N;                     // マップサイズ（N×Nのグリッド）
	Vec2 tileOffset;             // タイル間の距離（アイソメトリック表示用）
	int32 tileThickness;         // タイルの厚み（立体表示用）

public:
	// コンストラクタ（デフォルト値でアイソメトリック表示設定）
	TileCoordinateSystem(int32 mapSize, Vec2 offset = TileOffset, int32 thickness = TileThickness)
		: N(mapSize), tileOffset(offset), tileThickness(thickness) {
	}

	// マップサイズを動的に変更（リサイズ機能用）
	void updateMapSize(int32 newSize) {
		N = newSize;
	}

	// 現在のマップサイズを取得
	int32 getMapSize() const { return N; }

	// グリッド座標からアイソメトリック表示座標への変換
	Vec2 toBottomCenter(const Point& index) const {
		const int32 i = index.manhattanLength();                // マンハッタン距離でソート順を決定
		const int32 xi = (i < (N - 1)) ? 0 : (i - (N - 1));     // アイソメトリック表示の開始X座標
		const int32 yi = (i < (N - 1)) ? i : (N - 1);           // アイソメトリック表示の開始Y座標
		const int32 k = (index.manhattanDistanceFrom(Point{ xi, yi }) / 2);  // タイル内でのオフセット
		const double posX = ((i < (N - 1)) ? (i * -tileOffset.x) : ((i - 2 * N + 2) * tileOffset.x));  // X座標計算
		const double posY = (i * tileOffset.y);                 // Y座標計算（単純な線形計算）
		return{ (posX + tileOffset.x * 2 * k), posY };           // 最終的な表示座標
	}

	// グリッド座標から描画用の四角形を生成
	Quad toTile(const Point& index) const {
		const Vec2 bottomCenter = toBottomCenter(index);         // 底面中央座標を取得
		return Quad{                                             // ダイヤモンド形状の4頂点を定義
			bottomCenter.movedBy(0, -tileThickness).movedBy(0, -tileOffset.y * 2),      // 上頂点
			bottomCenter.movedBy(0, -tileThickness).movedBy(tileOffset.x, -tileOffset.y), // 右頂点
			bottomCenter.movedBy(0, -tileThickness),                                     // 下頂点  
			bottomCenter.movedBy(0, -tileThickness).movedBy(-tileOffset.x, -tileOffset.y) // 左頂点
		};
	}

	// 列方向の判定用四角形を生成（マウスクリック判定用）
	Array<Quad> makeColumnQuads() const {
		Array<Quad> quads;
		for (int32 x = 0; x < N; ++x) {                         // 全列について処理
			quads << createColumnQuad(x);
		}
		return quads;
	}

	// 行方向の判定用四角形を生成（マウスクリック判定用）
	Array<Quad> makeRowQuads() const {
		Array<Quad> quads;
		for (int32 y = 0; y < N; ++y) {                         // 全行について処理
			quads << createRowQuad(y);
		}
		return quads;
	}

	// マウス座標からグリッド座標への逆変換（タイルクリック判定用）
	Optional<Point> getIndexAt(const Vec2& pos, const Array<Quad>& columnQuads, const Array<Quad>& rowQuads) const {
		int32 x = -1, y = -1;

		// タイルの列インデックスを調べる（どの列にマウスがあるか）
		for (int32 i = 0; i < columnQuads.size(); ++i) {
			if (columnQuads[i].intersects(pos)) {              // マウス座標と列四角形の交差判定
				x = i;
				break;
			}
		}

		// タイルの行インデックスを調べる（どの行にマウスがあるか）
		for (int32 i = 0; i < rowQuads.size(); ++i) {
			if (rowQuads[i].intersects(pos)) {                 // マウス座標と行四角形の交差判定
				y = i;
				break;
			}
		}

		if ((x == -1) || (y == -1)) {                          // どちらかが見つからない場合は無効
			return none;
		}

		return Point{ x, y };                     // 有効な座標を返す
	}

private:
	// 指定列の判定用四角形を生成（プライベートヘルパー関数）
	Quad createColumnQuad(int32 x) const {
		return{
			toBottomCenter(Point{ x, 0 }).movedBy(0, -tileThickness).movedBy(0, -tileOffset.y * 2),         // 列の開始点
			toBottomCenter(Point{ x, 0 }).movedBy(0, -tileThickness).movedBy(tileOffset.x, -tileOffset.y),  // 右上
			toBottomCenter(Point{ x, (N - 1) }).movedBy(0, -tileThickness).movedBy(0, 0),                  // 列の終了点
			toBottomCenter(Point{ x, (N - 1) }).movedBy(0, -tileThickness).movedBy(-tileOffset.x, -tileOffset.y) // 左下
		};
	}

	// 指定行の判定用四角形を生成（プライベートヘルパー関数）
	Quad createRowQuad(int32 y) const {
		return{
			toBottomCenter(Point{ 0, y }).movedBy(0, -tileThickness).movedBy(-tileOffset.x, -tileOffset.y), // 行の開始点
			toBottomCenter(Point{ 0, y }).movedBy(0, -tileThickness).movedBy(0, -tileOffset.y * 2),        // 上
			toBottomCenter(Point{ (N - 1), y }).movedBy(0, -tileThickness).movedBy(tileOffset.x, -tileOffset.y), // 行の終了点
			toBottomCenter(Point{ (N - 1), y }).movedBy(0, -tileThickness).movedBy(0, 0)                   // 右下
		};
	}
};

// UI コンポーネントの抽象化（チェックボックス・ボタン等のUI状態管理）
class MapEditorUI {
private:
	bool showGrid = false;                    // グリッド表示のON/OFF
	bool showIndex = false;                   // 座標インデックス表示のON/OFF
	bool showUnit = false;                    // ユニット表示のON/OFF
	bool setResource = false;
	bool showGridBuiWhichIsThePlayer = false; // 建物の所属表示のON/OFF
	bool nowMapTipIsTile = true;              // 現在タイルモードかビルドモードか
	bool setEnemy = false;                    // ユニット配置モードのON/OFF

	size_t index1 = 0;                        // 陣営選択のインデックス
	size_t indexUnit = 0;                     // ユニット種別選択のインデックス
	size_t indexHoukou = 0;                   // ユニット向き選択のインデックス

	// UI表示用の選択肢配列（日本語での表示用）
	const Array<String> options = { U"侵攻", U"防衛", U"中立" };                                         // 建物の陣営選択肢
	const Array<String> optionsUnit = { U"仲間", U"敵", U"中立" };                                       // ユニットの陣営選択肢
	const Array<String> optionsHoukou = { U"北", U"北東", U"東", U"東南", U"南", U"西南", U"西", U"北西" }; // ユニットの向き選択肢

public:
	void renderBasicCheckboxes();                                    // 基本チェックボックス群の描画
	BattleWhichIsThePlayer renderBattleTypeRadioButtons();          // 陣営選択ラジオボタンの描画
	Unit createUnitFromUI(const String& currentX, const String& currentY, const String& unitID); // UI設定からユニット作成
	void renderUnitInterface(const Font& font, TextEditState& te1, TextEditState& te2, TextEditState& teUnit); // ユニット設定UI描画

	// UI状態を外部から参照するためのGetterメソッド群
	bool shouldShowGrid() const { return showGrid; }                              // グリッド表示状態
	bool shouldShowIndex() const { return showIndex; }                            // インデックス表示状態
	bool shouldShowUnit() const { return showUnit; }                              // ユニット表示状態
	bool shouldShowGridBuiWhichIsThePlayer() const { return showGridBuiWhichIsThePlayer; } // 建物所属表示状態
	bool isTileMode() const { return nowMapTipIsTile; }                           // タイルモード判定
	bool isSettingUnit() const { return setEnemy; }                               // ユニット配置モード判定
	bool isSettingResource() const { return setResource; }
	size_t getSelectedIndex() const { return index1; }                            // 選択中の陣営インデックス
};

// 基本チェックボックス群の描画処理（UI状態を変更するチェックボックスを配置）
void MapEditorUI::renderBasicCheckboxes() {
	SimpleGUI::CheckBox(showGrid, U"Show grid", Vec2{ 20, 240 }, 160);                    // グリッド表示切り替え
	SimpleGUI::CheckBox(showIndex, U"Show index", Vec2{ 20, 280 }, 160);                  // 座標表示切り替え
	SimpleGUI::CheckBox(nowMapTipIsTile, nowMapTipIsTile ? U"Tile mode" : U"Bui mode", Vec2{ 20, 320 }, 160); // モード切り替え

	if (!nowMapTipIsTile) {                   // ビルドモードの時のみ陣営選択を表示
		renderBattleTypeRadioButtons();
	}

	SimpleGUI::CheckBox(showGridBuiWhichIsThePlayer, U"Show GBP", Vec2{ 20, 480 }, 160);  // 建物所属表示切り替え
	SimpleGUI::CheckBox(setEnemy, U"set Unit", Vec2{ 20, 520 }, 160);                     // ユニット配置モード切り替え
	SimpleGUI::CheckBox(showUnit, U"show Unit", Vec2{ 20, 560 }, 160);                    // ユニット表示切り替え
	SimpleGUI::CheckBox(setResource, U"setResource", Vec2{ 20,640 }, 160);

}

// 陣営選択ラジオボタンの描画と選択結果の返却
BattleWhichIsThePlayer MapEditorUI::renderBattleTypeRadioButtons() {
	if (SimpleGUI::RadioButtons(index1, options, Vec2{ 180, 320 })) {        // 選択が変更された場合
		switch (index1) {                     // インデックスに応じてenum値を返す
		case 0: return BattleWhichIsThePlayer::Sortie;
		case 1: return BattleWhichIsThePlayer::Def;
		case 2: return BattleWhichIsThePlayer::None;
		default: return BattleWhichIsThePlayer::None;               // 想定外の値の場合は中立
		}
	}
	return static_cast<BattleWhichIsThePlayer>(index1);            // 変更がない場合は現在の値を返す
}

// UI設定値からユニットオブジェクトを生成
Unit MapEditorUI::createUnitFromUI(const String& currentX, const String& currentY, const String& unitID) {
	Unit unit;

	// 方向の設定（インデックスから実際の方向文字列に変換）
	const Array<String> directions = { U"北", U"北東", U"東", U"東南", U"南", U"西南", U"西", U"北西" };
	if (indexHoukou < directions.size()) {    // 配列境界チェック
		unit.houkou = directions[indexHoukou];
	}

	// 陣営の設定（インデックスから陣営文字列に変換）
	switch (indexUnit) {
	case 0: unit.BattleWhichIsThePlayer = U"仲間"; break;
	case 1: unit.BattleWhichIsThePlayer = U"敵"; break;
	case 2: unit.BattleWhichIsThePlayer = U"中立"; break;
	default: unit.BattleWhichIsThePlayer = U"中立"; break;          // 想定外の場合は中立
	}

	unit.ID = unitID;                         // ユーザー入力のID設定
	unit.x = Parse<int32>(currentX);          // 文字列座標を数値に変換
	unit.y = Parse<int32>(currentY);

	return unit;
}

// ユニット設定UI群の描画（ユニット配置モード時に表示される詳細設定）
void MapEditorUI::renderUnitInterface(const Font& font, TextEditState& te1, TextEditState& te2, TextEditState& teUnit) {
	SimpleGUI::RadioButtons(indexUnit, optionsUnit, Vec2{ 1920 - 200 - 50, 400 });      // 陣営選択ラジオボタン
	SimpleGUI::RadioButtons(indexHoukou, optionsHoukou, Vec2{ 1920 - 200 - 50, 520 });  // 向き選択ラジオボタン

	font(U"x").draw(1920 - 120 - 20, 440, Palette::Black);                              // X座標ラベル
	SimpleGUI::TextBox(te1, Vec2{ 1920 - 120 - 20, 480 }, 120, 4);                      // X座標入力欄

	font(U"y").draw(1920 - 120 - 20, 520, Palette::Black);                              // Y座標ラベル
	SimpleGUI::TextBox(te2, Vec2{ 1920 - 120 - 20, 560 }, 120, 4);                      // Y座標入力欄

	font(U"ID").draw(1920 - 120 - 20, 600, Palette::Black);                             // IDラベル
	SimpleGUI::TextBox(teUnit, Vec2{ 1920 - 120 - 20, 660 }, 120, 64);                  // ID入力欄
}

// この関数は、Grid<int32> に基づいて Array<Texture> をグループ化します
HashSet<String> GroupTexturesByGrid(const Grid<int32>& grid, const Array<std::pair<String, Texture>>& textures)
{
	HashSet<String> groupedTextures;          // 重複を排除するためにHashSetを使用

	for (size_t i = 0; i < grid.height(); ++i)     // 全行をスキャン
	{
		for (size_t j = 0; j < grid.width(); j++)   // 全列をスキャン
		{
			int32 gridValue = grid[i][j];             // セルの値を取得
			if (gridValue >= 0 && gridValue < textures.size())  // 有効な範囲内かチェック
			{
				groupedTextures.emplace(textures[gridValue].first);  // テクスチャパスを追加
			}
		}
	}

	return groupedTextures;
}

// マップデータをTOMLファイルに書き出す関数（エディタの保存機能）
void WriteTomlFile(const FilePath& path,
					const Grid<int32>& grid,                                     // タイルマップデータ
					const Grid<int32>& gridBui,                                  // 建物マップデータ
					const Grid<BattleWhichIsThePlayer>& gridBuiWhichIsThePlayer, // 建物所属データ
					const Optional<Point>& sor,                                  // 侵攻開始位置
					const Optional<Point>& def,                                  // 防衛開始位置
					const Optional<Point>& neu,                                  // 中立開始位置（未使用）
					const Array<std::pair<String, Texture>>& textures,          // タイルテクスチャ配列
					const Array<std::pair<String, Texture>>& texturesBui,       // 建物テクスチャ配列
					const Array<Unit>& arrayEnemy,
	const ResourceManager& resourceManager
)                               // ユニット配列
{
	HashSet<String> group = GroupTexturesByGrid(grid, textures);              // 使用中のタイルテクスチャを抽出
	HashSet<String> groupBui = GroupTexturesByGrid(gridBui, texturesBui);     // 使用中の建物テクスチャを抽出

	TomlBuilder builder;                      // TOML生成器を初期化
	builder.startMap();                       // マップセクション開始

	int32 counter = 0;                        // 要素番号カウンター
	HashTable<String, String> ht;             // ファイルパス→要素キーのマッピング

	// テクスチャ要素の追加（タイル用）
	for (const auto& filePath : group) {
		String fileName = FilePathUtils::extractFileName(filePath);           // パスからファイル名抽出
		builder.addElement(counter, fileName);                                // TOML要素として追加
		ht.emplace(fileName, FilePathUtils::createElementKey(counter));       // マッピングテーブルに登録
		counter++;                            // 次の要素番号に進む
	}

	// テクスチャ要素の追加（建物用）
	for (const auto& filePath : groupBui) {
		String fileName = FilePathUtils::extractFileName(filePath);           // パスからファイル名抽出
		builder.addElement(counter, fileName);                                // TOML要素として追加
		ht.emplace(fileName, FilePathUtils::createElementKey(counter));       // マッピングテーブルに登録
		counter++;                            // 次の要素番号に進む
	}

	builder.startDataSection();               // マップデータセクション開始

	for (size_t i = 0; i < grid.height(); i++)     // 全行を処理
	{
		String rowData;                       // 1行分のデータを蓄積

		for (size_t j = 0; j < grid.width(); j++)   // 全列を処理
		{
			//マップチップ（タイル情報の処理）
			String a = U"*";                  // デフォルト値（何もない状態）
			{
				String fileName = FilePathUtils::extractFileName(textures[grid[j][i]].first);  // タイルファイル名取得
				String ele = ht[fileName];                                                     // 要素キーを逆引き
				a = (ele != U"") ? ele : U"eleNone";                                           // 有効な場合は要素キー、無効ならeleNone
			}

			//建物マップチップ（建物情報の処理）
			String b = U"*";                  // デフォルト値（建物なし）
			{
				int32 buiIndex = gridBui[j][i];                                               // 建物インデックス取得
				if (buiIndex != -1)           // 建物が配置されている場合
				{
					String fileName = FilePathUtils::extractFileName(texturesBui[buiIndex].first); // 建物ファイル名取得
					String ele = ht[fileName];                                                     // 要素キーを逆引き

					if (ele != U"")           // 有効な建物の場合
					{
						String suffix;        // 所属を示すサフィックス
						switch (gridBuiWhichIsThePlayer[j][i]) {                              // 建物の所属に応じて
						case BattleWhichIsThePlayer::Sortie: suffix = U":sor"; break;        // 侵攻側マーク
						case BattleWhichIsThePlayer::Def: suffix = U":def"; break;           // 防衛側マーク
						default: suffix = U":none"; break;                                   // 中立マーク
						}
						b = U"*" + ele + suffix;                                            // 要素キー+所属サフィックス
					}
				}
			}

			//ユニットの情報（配置されているユニットの処理）
			String c = U"*-1";                // デフォルト値（ユニットなし）
			{
				Array<Unit> foundUnits = arrayEnemy.filter([i, j](const Unit& u) {        // 該当座標のユニットを検索
					return (u.x == j && u.y == i);                                        // 座標が一致するものを抽出
				});
				if (foundUnits.size() == 1)   // 1体のユニットが見つかった場合（重複は想定外）
				{
					const Unit& unit = foundUnits[0];                                     // 見つかったユニット
					c = U"*" + unit.ID + U":" + unit.houkou + U":" + unit.BattleWhichIsThePlayer; // ID:向き:陣営の形式
				}
			}

			//【出撃、防衛、中立の位置】もしくは【退却位置】（特殊位置マーカーの処理）
			String d = U"*";                  // デフォルト値（特殊位置なし）
			{
				Point currentPos = { i, j };                                              // 現在処理中の座標
				if (sor.has_value() && sor.value() == currentPos) {                      // 侵攻開始位置の場合
					d = U"*@@";               // 侵攻位置マーク（@@）
				}
				else if (def.has_value() && def.value() == currentPos) {               // 防衛開始位置の場合
					d = U"*@";                // 防衛位置マーク（@）
				}
			}

			String e = U"*";                  // 予備フィールド（将来の拡張用）
			String f = U"*";                  // 予備フィールド（将来の拡張用）

			{
				Point currentPos = { j, i };
				if (auto resource = resourceManager.getResourcePoint(currentPos))
				{
					f = U"*RESOURCE:" + resource->resourceKey;
				}
			}

			rowData += U"{}{}{}{}{}{},"_fmt(a, b, c, d, e, f);                          // 全フィールドを結合してカンマ区切りで追加
		}

		if (rowData.ends_with(U","_sv)) {     // 行末のカンマを処理
			rowData.pop_back();               // 最後のカンマを削除
			rowData += U"$";                  // 行終端マーカー（$）を追加
		}

		builder.addDataRow(rowData);          // 完成した行データをTOMLに追加
	}

	builder.endDataSection();                 // マップデータセクション終了

	// ファイルに書き出す（実際のファイル保存処理）
	TextWriter writer(path);                  // ファイルライター作成
	if (writer) {                             // 正常に開けた場合のみ
		writer.write(builder.build());       // 生成したTOML内容を書き込み
	}
}

// レガシー関数の保持（互換性のため）既存コードとの互換性を保つためのラッパー関数群
Vec2 ToTileBottomCenter(const Point& index, const int32 N) {
	TileCoordinateSystem coordSystem(N);     // 一時的に座標システムを作成
	return coordSystem.toBottomCenter(index); // 新しいメソッドを呼び出し
}

Quad ToTile(const Point& index, const int32 N) {
	TileCoordinateSystem coordSystem(N);     // 一時的に座標システムを作成
	return coordSystem.toTile(index);        // 新しいメソッドを呼び出し
}

Quad ToColumnQuad(const int32 x, const int32 N) {
	TileCoordinateSystem coordSystem(N);     // 一時的に座標システムを作成
	return coordSystem.makeColumnQuads()[x]; // 新しいメソッドを呼び出し
}

Quad ToRowQuad(const int32 y, const int32 N) {
	TileCoordinateSystem coordSystem(N);     // 一時的に座標システムを作成
	return coordSystem.makeRowQuads()[y];    // 新しいメソッドを呼び出し
}

Array<Quad> MakeColumnQuads(const int32 N) {
	TileCoordinateSystem coordSystem(N);     // 一時的に座標システムを作成
	return coordSystem.makeColumnQuads();    // 新しいメソッドを呼び出し
}

Array<Quad> MakeRowQuads(const int32 N) {
	TileCoordinateSystem coordSystem(N);     // 一時的に座標システムを作成
	return coordSystem.makeRowQuads();       // 新しいメソッドを呼び出し
}

Optional<Point> ToIndex(const Vec2& pos, const Array<Quad>& columnQuads, const Array<Quad>& rowQuads) {
	// この関数は現在のインターフェースを維持するためにレガシー関数として保持
	int32 x = -1, y = -1;                    // 初期値は無効値

	for (int32 i = 0; i < columnQuads.size(); ++i) {     // 全列について判定
		if (columnQuads[i].intersects(pos)) {             // マウス座標と列四角形の交差判定
			x = i;
			break;
		}
	}

	for (int32 i = 0; i < rowQuads.size(); ++i) {        // 全行について判定
		if (rowQuads[i].intersects(pos)) {                // マウス座標と行四角形の交差判定
			y = i;
			break;
		}
	}

	if ((x == -1) || (y == -1)) {             // どちらかが見つからない場合
		return none;                          // 無効値を返す
	}

	return Point{ x, y };                     // 有効な座標を返す
}

// グリッド操作の抽象化（マップデータの管理と操作を統一化）
class GridManager {
private:
	Grid<int32> tileGrid;                     // タイル種別を格納するグリッド（各セルにタイル番号）
	Grid<int32> buildingGrid;                 // 建物種別を格納するグリッド（各セルに建物番号、-1は建物なし）
	Grid<BattleWhichIsThePlayer> buildingPlayerGrid; // 建物の所属を格納するグリッド（各建物の陣営）
	int32 size;                               // 現在のグリッドサイズ（N×N）

public:
	// コンストラクタ（初期サイズでグリッドを作成）
	GridManager(int32 initialSize) : size(initialSize) {
		resize(initialSize);                  // 指定サイズで初期化
	}

	// グリッドサイズを変更（マップサイズ変更時に呼び出される）
	void resize(int32 newSize) {
		size = newSize;                       // 新しいサイズを記録
		tileGrid = Grid<int32>(Size{ size, size });                            // タイルグリッドを再作成（デフォルト値0）
		buildingGrid = Grid<int32>(Size{ size, size }, -1);                    // 建物グリッドを再作成（デフォルト値-1で建物なし）
		buildingPlayerGrid = Grid<BattleWhichIsThePlayer>(size, size, BattleWhichIsThePlayer::None); // 所属グリッドを再作成（デフォルト中立）
	}

	// 指定位置にタイルを設置（タイル配置モード時の処理）
	void setTile(const Point& pos, int32 tileType) {
		if (isValidPosition(pos)) {           // 有効な座標範囲内かチェック
			tileGrid[pos] = tileType;         // タイル種別を設定
		}
	}

	// 指定位置に建物を設置（建物配置モード時の処理）
	void setBuilding(const Point& pos, int32 buildingType, BattleWhichIsThePlayer player) {
		if (isValidPosition(pos)) {           // 有効な座標範囲内かチェック
			buildingGrid[pos] = buildingType; // 建物種別を設定
			buildingPlayerGrid[pos] = player; // 建物の所属を設定
		}
	}

	// 全タイルを指定種別で塗りつぶし（塗りつぶし機能）
	void fillTiles(int32 tileType) {
		for (int32 y = 0; y < size; ++y) {    // 全行について
			for (int32 x = 0; x < size; ++x) { // 全列について
				tileGrid[x][y] = tileType;    // 指定タイルで上書き
			}
		}
	}

	// 指定位置のタイル種別を取得（描画やファイル保存時に使用）
	int32 getTile(const Point& pos) const {
		return isValidPosition(pos) ? tileGrid[pos] : -1;      // 無効な位置の場合は-1
	}

	// 指定位置の建物種別を取得（描画やファイル保存時に使用）
	int32 getBuilding(const Point& pos) const {
		return isValidPosition(pos) ? buildingGrid[pos] : -1;  // 無効な位置の場合は-1
	}

	// 指定位置の建物所属を取得（描画やファイル保存時に使用）
	BattleWhichIsThePlayer getBuildingPlayer(const Point& pos) const {
		return isValidPosition(pos) ? buildingPlayerGrid[pos] : BattleWhichIsThePlayer::None; // 無効な位置の場合は中立
	}

	// 現在のグリッドサイズを取得
	int32 getSize() const { return size; }

	// グリッドへの直接アクセス（既存コードとの互換性のため）ファイル保存等で生のグリッドが必要
	Grid<int32>& getTileGrid() { return tileGrid; }
	Grid<int32>& getBuildingGrid() { return buildingGrid; }
	Grid<BattleWhichIsThePlayer>& getBuildingPlayerGrid() { return buildingPlayerGrid; }

	// 読み取り専用グリッドアクセス（constメソッド）
	const Grid<int32>& getTileGrid() const { return tileGrid; }
	const Grid<int32>& getBuildingGrid() const { return buildingGrid; }
	const Grid<BattleWhichIsThePlayer>& getBuildingPlayerGrid() const { return buildingPlayerGrid; }

private:
	// 座標が有効範囲内かをチェック（配列境界を超えないように保護）
	bool isValidPosition(const Point& pos) const {
		return pos.x >= 0 && pos.x < size && pos.y >= 0 && pos.y < size;
	}
};

// UI レイアウトとサイズの定数化（マジックナンバーを排除して保守性向上）
namespace UIConstants {
	constexpr int32 WINDOW_WIDTH = 1920;     // ウィンドウの横幅（フルHD想定）
	constexpr int32 WINDOW_HEIGHT = 1017;    // ウィンドウの縦幅（タスクバー分を除いた実用的なサイズ）
	constexpr int32 SIDEBAR_WIDTH = 320;     // サイドバーの幅（UI要素配置用）
	constexpr int32 TEXTBOX_WIDTH = 120;     // テキストボックスの標準幅
	constexpr int32 TEXTBOX_HEIGHT = 40;     // テキストボックスの標準高さ
	constexpr int32 BUTTON_WIDTH = 160;      // ボタンの標準幅
	constexpr int32 BUTTON_HEIGHT = 40;      // ボタンの標準高さ
	constexpr Vec2 LEFT_PANEL_START{ 20, 240 };           // 左側UIパネルの開始座標
	constexpr Vec2 RIGHT_PANEL_START{ WINDOW_WIDTH - SIDEBAR_WIDTH, 400 }; // 右側UIパネルの開始座標
	constexpr int32 TILE_MENU_COLS = 22;     // タイルメニューの列数（横に並ぶタイル数）
	constexpr int32 TILE_MENU_ROWS = 4;      // タイルメニューの行数（縦に並ぶタイル数）
	constexpr int32 TILE_SIZE = 56;          // タイルメニューの各タイルサイズ
}

// カメラ操作の抽象化（マップの表示範囲制御とマウス操作）
class CameraController {
private:
	Camera2D camera;                          // OpenSiv3Dの2Dカメラオブジェクト
	Optional<Vec2> dragStartPos;              // ドラッグ開始時のマウス座標（ドラッグ中のみ有効）
	Vec2 dragStartCameraCenter;               // ドラッグ開始時のカメラ中心座標

public:
	// コンストラクタ（カメラの初期設定を行う）
	CameraController() : camera{ Vec2{ 0, 0 }, 1.0, (CameraControl::WASDKeys | CameraControl::UpDownKeys | CameraControl::Wheel) } {}

	// カメラドラッグの更新処理（毎フレーム呼び出される）
	bool updateDrag(bool onUI) {
		bool isDragging = (MouseM.pressed() || (KeyShift.pressed() && MouseL.pressed())); // 中ボタンまたはShift+左クリックでドラッグ

		if (isDragging && !onUI) {            // ドラッグ中かつUI上でない場合
			if (!dragStartPos.has_value()) {  // ドラッグ開始の瞬間
				dragStartPos = Cursor::PosF();              // 開始座標を記録
				dragStartCameraCenter = camera.getCenter(); // 開始時のカメラ位置を記録
			}
			else {                          // ドラッグ継続中
				Vec2 currentPos = Cursor::PosF();           // 現在のマウス座標
				Vec2 dragDelta = (dragStartPos.value() - currentPos) / camera.getScale(); // ズームレベルを考慮した移動量計算
				camera.setTargetCenter(dragStartCameraCenter + dragDelta);               // カメラターゲット位置を更新
				camera.jumpTo(dragStartCameraCenter + dragDelta, camera.getScale());     // 即座にカメラ位置を適用
			}
		}
		else {                              // ドラッグ終了
			dragStartPos = none;              // ドラッグ状態をリセット
		}

		camera.update();                      // カメラの状態を更新
		return isDragging;                    // ドラッグ中かどうかを返す（呼び出し元でマップ操作を制御するため）
	}

	// 座標変換オブジェクトを取得（描画時にカメラ座標系を適用するため）
	auto createTransformer() { return camera.createTransformer(); }

	// カメラUI（操作説明等）を描画
	void draw(const ColorF& color) { camera.draw(color); }
};

// レンダリングシステムの抽象化（マップの描画処理を統一化）
class MapRenderer {
private:
	const TileCoordinateSystem& coordSystem;  // 座標変換システムへの参照（描画時の座標計算用）

public:
	// コンストラクタ（座標システムへの参照を保持）
	MapRenderer(const TileCoordinateSystem& coords) : coordSystem(coords) {}

	// タイルと建物の描画（メインの描画処理）
	void renderTiles(const GridManager& gridManager,
					const Array<std::pair<String, Texture>>& textures,       // タイルテクスチャ配列
					const Array<std::pair<String, Texture>>& buildingTextures) { // 建物テクスチャ配列

		int32 N = coordSystem.getMapSize();                                  // 現在のマップサイズ
		Array<std::pair<Vec2, Texture>> buildingRenderList;                  // 建物の描画リスト（後で描画するため）

		// タイルの描画（アイソメトリック表示のため特殊な順序で描画）
		for (int32 i = 0; i < (N * 2 - 1); ++i) {                           // アイソメトリック描画の総ライン数
			const int32 xi = (i < (N - 1)) ? 0 : (i - (N - 1));              // 各ラインの開始X座標
			const int32 yi = (i < (N - 1)) ? i : (N - 1);                    // 各ラインの開始Y座標

			for (int32 k = 0; k < (N - Abs(N - i - 1)); ++k) {               // 各ライン内のタイル数
				const Point index{ (xi + k), (yi - k) };                     // 実際のグリッド座標
				Vec2 pos = coordSystem.toBottomCenter(index);                 // 描画座標に変換

				// タイル描画
				int32 tileIndex = gridManager.getTile(index);                 // タイル種別を取得
				if (tileIndex >= 0 && tileIndex < textures.size()) {          // 有効なタイルインデックスの場合
					textures[tileIndex].second.draw(Arg::bottomCenter = pos); // 底面中央基準で描画
				}
				else if (textures.size() > 1) {                             // 無効な場合はデフォルトタイル（index 1）
					textures[1].second.draw(Arg::bottomCenter = pos);
				}

				// 建物描画リストに追加（建物はタイルより上に描画する必要があるため後回し）
				int32 buildingIndex = gridManager.getBuilding(index);         // 建物種別を取得
				if (buildingIndex > 0 && buildingIndex < buildingTextures.size()) { // 有効な建物がある場合
					buildingRenderList.push_back({ pos.movedBy(0, -15), buildingTextures[buildingIndex].second }); // 少し上にずらして追加
				}
			}
		}

		// 建物描画（タイルの上に重ねるため最後に描画）
		for (const auto& building : buildingRenderList) {
			building.second.draw(Arg::bottomCenter = building.first);         // 建物を描画
		}
	}

	// マウスホバー時のハイライト描画（選択中のタイルを強調表示）
	void renderHighlight(const Optional<Point>& highlightIndex) {
		if (highlightIndex) {                 // ハイライト対象がある場合
			coordSystem.toTile(*highlightIndex).draw(ColorF{ 1.0, 0.2 });    // 半透明の黄色でハイライト
		}
	}

	// グリッド線の描画（マップの区切りを可視化）
	void renderGrid(const Array<Quad>& columnQuads, const Array<Quad>& rowQuads) {
		for (const auto& quad : columnQuads) {                               // 列線を描画
			quad.drawFrame(2);                // 線幅2pxで枠線描画
		}
		for (const auto& quad : rowQuads) {                                  // 行線を描画
			quad.drawFrame(2);                // 線幅2pxで枠線描画
		}
	}

	// 座標インデックスの描画（デバッグ用の座標表示）
	void renderIndices() {
		int32 N = coordSystem.getMapSize();
		for (int32 y = 0; y < N; ++y) {
			for (int32 x = 0; x < N; ++x) {
				const Point index{ x, y };
				const Vec2 pos = coordSystem.toBottomCenter(index).movedBy(0, -TileThickness); // タイル表面の位置
				PutText(U"{}"_fmt(index), pos.movedBy(0, -TileOffset.y - 3));                // 座標を文字で表示
			}
		}
	}

	// 建物の陣営マーカー描画（所属を可視化）
	void renderBattleMarkers(const GridManager& gridManager, size_t selectedIndex) {
		int32 N = coordSystem.getMapSize();
		for (int32 y = 0; y < N; ++y) {
			for (int32 x = 0; x < N; ++x) {
				const Point index{ x, y };
				const Vec2 pos = coordSystem.toBottomCenter(index).movedBy(0, -TileThickness); // タイル表面の位置
				BattleWhichIsThePlayer player = gridManager.getBuildingPlayer(index);          // 建物の所属を取得

				// 選択中の陣営のマーカーのみ表示（UI設定に応じて）
				switch (player) {
				case BattleWhichIsThePlayer::Sortie:
					if (selectedIndex == 0) PutText(U"攻", pos.movedBy(0, -TileOffset.y + 3)); // 侵攻側マーク
					break;
				case BattleWhichIsThePlayer::Def:
					if (selectedIndex == 1) PutText(U"守", pos.movedBy(0, -TileOffset.y + 3)); // 防衛側マーク
					break;
				case BattleWhichIsThePlayer::None:
					if (selectedIndex == 2) PutText(U"中", pos.movedBy(0, -TileOffset.y + 3)); // 中立マーク
					break;
				}
			}
		}
	}

	// ユニットの描画（配置されているユニット情報を表示）
	void renderUnits(const Array<Unit>& units) {
		for (const auto& unit : units) {      // 全ユニットについて
			const Point index{ unit.x, unit.y };                                         // ユニットの座標
			const Vec2 pos = coordSystem.toBottomCenter(index).movedBy(0, -TileThickness); // 描画位置
			PutText(unit.ID, pos.movedBy(0, -TileOffset.y + 6));                         // ユニットID表示
			PutText(unit.BattleWhichIsThePlayer, pos.movedBy(20, -TileOffset.y - 6));    // 所属表示（右側）
			PutText(unit.houkou, pos.movedBy(-20, -TileOffset.y - 6));                   // 向き表示（左側）
		}
	}
};

// TOML読み込み処理の抽象化（保存したマップファイルの読み込み）
class TomlMapLoader {
public:
	// 読み込み結果を格納する構造体
	struct LoadResult {
		bool success = false;                 // 読み込み成功フラグ
		int32 mapSize = 16;                   // マップサイズ（デフォルト16×16）
		Array<String> elements;               // テクスチャ要素配列
		String mapData;                       // マップデータ文字列
		String error;                         // エラーメッセージ
	};

	// TOMLファイルからマップデータを読み込み
	static LoadResult loadFromFile(const Optional<FilePath>& path) {
		LoadResult result;

		if (!path.has_value()) {              // ファイルが選択されていない場合
			result.error = U"No file selected";
			return result;
		}

		const TOMLReader tomlConfig{ path.value() };                         // TOMLファイルを開く
		if (!tomlConfig) {                    // ファイル読み込み失敗
			result.error = U"Failed to load TOML file";
			return result;
		}

		try {
			int32 counter = 0;                // 要素番号カウンター
			for (const auto& table : tomlConfig[U"Map"].tableArrayView()) { // [Map]セクションを処理
				// 要素の読み込み（ele0, ele1, ele2...の順序で）
				while (true) {
					String elementKey = U"ele" + Format(counter);           // 要素キーを生成
					String element = table[elementKey].get<String>();       // 要素値を取得
					if (element.isEmpty()) break;                           // 要素が見つからなければ終了
					result.elements.push_back(element);                     // 要素リストに追加
					counter++;
				}
				result.mapData = table[U"data"].get<String>();              // マップデータ文字列を取得
			}
			result.success = true;            // 読み込み成功
		}
		catch (const std::exception& e) {   // 例外が発生した場合
			result.error = U"Error parsing TOML content";
		}

		return result;
	}

	// 読み込んだデータをグリッドに展開
	static bool parseMapData(const LoadResult& loadResult,
							GridManager& gridManager,                        // 更新対象のグリッド
							const Array<std::pair<String, Texture>>& textures,       // タイルテクスチャ配列
							const Array<std::pair<String, Texture>>& buildingTextures, // 建物テクスチャ配列
							TileCoordinateSystem& coordSystem,               // 更新対象の座標システム
							Array<Quad>& columnQuads,                        // 更新対象の列判定四角形
							Array<Quad>& rowQuads) {                         // 更新対象の行判定四角形
		if (!loadResult.success) return false;                              // 読み込み失敗の場合は何もしない

		Array<String> splitMap = loadResult.mapData.split('$');              // 行区切り文字（$）で分割
		splitMap.remove_if([](const String& str) { return str.contains(U"\t"); }); // タブ文字を含む行を除去（不正データ対策）

		for (const auto&& [rowIndex, rowData] : Indexed(splitMap)) {         // 各行を処理
			Array<String> splitMapRow = rowData.split(',');                  // カンマ区切りでセル分割

			// 最初の行でマップサイズを決定（動的リサイズ）
			if (rowIndex == 0) {
				int32 newSize = static_cast<int32>(splitMapRow.size());       // 列数からサイズを判定
				gridManager.resize(newSize);                                  // グリッドをリサイズ
				coordSystem.updateMapSize(newSize);                           // 座標システムを更新
				columnQuads = coordSystem.makeColumnQuads();                  // 列判定四角形を再生成
				rowQuads = coordSystem.makeRowQuads();                        // 行判定四角形を再生成
			}

			for (const auto&& [colIndex, cellData] : Indexed(splitMapRow)) { // 各セルを処理
				parseCellData(cellData, Point{ colIndex, rowIndex }, loadResult.elements,
							 gridManager, textures, buildingTextures);      // セルデータを解析してグリッドに設定
			}
		}

		return true;
	}

private:
	// 1セル分のデータを解析してグリッドに設定
	static void parseCellData(const String& cellData, const Point& position,
							 const Array<String>& elements,                  // テクスチャ要素配列
							 GridManager& gridManager,                        // 更新対象のグリッド
							 const Array<std::pair<String, Texture>>& textures,       // タイルテクスチャ配列
							 const Array<std::pair<String, Texture>>& buildingTextures) { // 建物テクスチャ配列
		Array<String> parts = cellData.split('*');                          // *区切りでフィールド分割
		if (parts.size() < 2) return;                                       // 最低2フィールド必要

		// タイル解析（第1フィールド）
		if (!parts[0].isEmpty()) {
			char lastChar = parts[0].back();                                 // 要素番号（最後の文字）
			int32 elementIndex = lastChar - '0';                             // 文字を数値に変換
			if (elementIndex >= 0 && elementIndex < elements.size()) {       // 有効な要素番号の場合
				String targetElement = elements[elementIndex];               // 要素名を取得
				for (const auto&& [textureIndex, texturePair] : Indexed(textures)) { // テクスチャ配列を検索
					if (texturePair.first.contains(targetElement)) {         // ファイルパスに要素名が含まれる場合
						gridManager.setTile(position, static_cast<int32>(textureIndex)); // タイルを設定
						break;
					}
				}
			}
		}

		// 建物解析（第2フィールド）
		if (parts.size() > 1 && parts[1].contains(U":")) {                  // 建物データがあり、所属情報も含む場合
			Array<String> buildingParts = parts[1].split(':');               // :区切りで建物と所属を分離
			if (buildingParts.size() >= 2) {
				char lastChar = buildingParts[0].back();                     // 建物要素番号
				int32 elementIndex = lastChar - '0';                         // 文字を数値に変換
				if (elementIndex >= 0 && elementIndex < elements.size()) {   // 有効な要素番号の場合
					String targetElement = elements[elementIndex];           // 要素名を取得
					for (const auto&& [textureIndex, texturePair] : Indexed(buildingTextures)) { // 建物テクスチャ配列を検索
						if (texturePair.first.contains(targetElement)) {     // ファイルパスに要素名が含まれる場合
							BattleWhichIsThePlayer player = BattleWhichIsThePlayer::None; // デフォルトは中立
							if (buildingParts[1] == U"def") player = BattleWhichIsThePlayer::Def;       // 防衛側
							else if (buildingParts[1] == U"sort") player = BattleWhichIsThePlayer::Sortie; // 侵攻側

							gridManager.setBuilding(position, static_cast<int32>(textureIndex), player); // 建物と所属を設定
							break;
						}
					}
				}
			}
		}
	}
};

// Forward declarations for helper functions（ヘルパー関数の前方宣言）
// これらの関数はMain()内で呼び出されるが、定義はMain()の後にあるため前方宣言が必要
void renderTileMenu(const RoundRect& menuRect, int32& selectedType,
				   const Array<std::pair<String, Texture>>& textures,
				   const Array<std::pair<String, Texture>>& buildingTextures,
				   const MapEditorUI& ui);

void handleFileOperations(GridManager& gridManager, TileCoordinateSystem& coordSystem,
						 Array<Quad>& columnQuads, Array<Quad>& rowQuads,
						 const Array<std::pair<String, Texture>>& textures,
						 const Array<std::pair<String, Texture>>& buildingTextures,
						 const Array<Unit>& units, TextEditStateManager& textManager,
						 const Optional<Point>& sor, const Optional<Point>& def, const Optional<Point>& neu,
						 const ResourceManager& resourceManager);

void handleMapSizeChange(GridManager& gridManager, TileCoordinateSystem& coordSystem,
						Array<Quad>& columnQuads, Array<Quad>& rowQuads, TextEditStateManager& textManager);

void handleUnitManagement(MapEditorUI& ui, const Font& font, TextEditStateManager& textManager,
						 Array<Unit>& units, Optional<Point>& sor, Optional<Point>& def, Optional<Point>& neu);
void handleResourceManagement(MapEditorUI& ui, const Font& font, TextEditState& xxx, TextEditState& yyy, TextEditState& key, ResourceManager& resourceManager);
void renderResourcePoints(const ResourceManager& resourceManager, const TileCoordinateSystem& coordSystem);

// メイン関数（プログラムのエントリーポイント）
void Main()
{
	// ウィンドウをリサイズする（作業しやすいサイズに設定）
	Window::Resize(UIConstants::WINDOW_WIDTH, UIConstants::WINDOW_HEIGHT);

	// 背景を水色にする（見やすさとデザイン性を考慮）
	Scene::SetBackground(ColorF{ 0.8, 0.9, 1.0 });

	const Font font{ 25 };                   // UI用フォント（読みやすいサイズに設定）

	// 各タイルのテクスチャ（指定フォルダから自動読み込み）
	Array<std::pair<String, Texture>> textures = TextureLoader::loadFromDirectory(U"Tile/");
	Array<std::pair<String, Texture>> texturesBui = TextureLoader::loadFromDirectory(U"Bui/");

	Array<Unit> arrayEnemy;                  // ユニット配列（名前は歴史的経緯でenemy）

	// システム初期化（デフォルト16×16マップ）
	int32 N = 16;
	GridManager gridManager(N);              // グリッド管理オブジェクト
	TileCoordinateSystem coordSystem(N);     // 座標変換システム
	CameraController cameraController;       // カメラ制御オブジェクト
	MapRenderer renderer(coordSystem);       // 描画システム

	// 各列・行の四角形（マウスクリック判定用）
	Array<Quad> columnQuads = coordSystem.makeColumnQuads();
	Array<Quad> rowQuads = coordSystem.makeRowQuads();

	// タイルメニューで選択されているタイルの種類（初期値は0番）
	int32 tileTypeSelected = 0;

	// タイルメニューの四角形（UIの配置領域を定義）
	constexpr RoundRect TileMenuRoundRect = RectF{ 20, 20, (UIConstants::TILE_SIZE * UIConstants::TILE_MENU_COLS), (50 * UIConstants::TILE_MENU_ROWS) }.stretched(10).rounded(8);

	BattleWhichIsThePlayer nowBattleWhichIsThePlayer = BattleWhichIsThePlayer::Sortie; // 現在選択中の陣営

	TextEditStateManager textManager;        // テキスト入力管理オブジェクト
	textManager.set(U"mapSize", Format(N));  // マップサイズの初期値を設定

	Optional<Point> sor, def, neu;            // 特殊位置（侵攻・防衛・中立開始位置）
	MapEditorUI ui;                          // UI管理オブジェクト
	ResourceManager resourceManager;
	// メインループ（毎フレーム実行される）
	while (System::Update())
	{
		// タイルメニューの四角形の上にマウスカーソルがあるか（UI操作判定のため）
		const bool onTileMenu = TileMenuRoundRect.mouseOver();

		// カメラ更新（ドラッグ操作等を処理）
		bool isDragging = cameraController.updateDrag(onTileMenu);

		{
			// 2D カメラによる座標変換を適用する（描画範囲とズームを反映）
			const auto tr = cameraController.createTransformer();

			// タイルと建物のレンダリング（メインのマップ描画）
			renderer.renderTiles(gridManager, textures, texturesBui);

			// マウスカーソルがタイルメニュー上に無く、ドラッグ中でなければ（マップ操作が可能な状態）
			Optional<Point> hoveredIndex;
			if (!onTileMenu && !isDragging) {
				if (const auto index = coordSystem.getIndexAt(Cursor::PosF(), columnQuads, rowQuads)) { // マウス座標からグリッド座標を取得
					hoveredIndex = index;
					renderer.renderHighlight(index);         // ホバー中のタイルをハイライト

					// マウスの左ボタンが押されていたら（Shiftキーが押されていない場合のみ）
					if (MouseL.pressed() && !KeyShift.pressed()) {
						if (ui.isSettingResource()) {            // リソース配置モードの場合
							textManager.setPoint(U"Resource-currentX", U"Resource-currentY", *index); // リソース用テキストボックスに座標を設定
						}
						else if (ui.isTileMode()) {              // タイルモードの場合
							gridManager.setTile(*index, tileTypeSelected); // タイルを配置
						}
						else {                                  // 建物モードの場合
							gridManager.setBuilding(*index, tileTypeSelected, nowBattleWhichIsThePlayer); // 建物を配置
						}
					}
					else if (MouseR.pressed()) {                 // 右クリックの場合
						textManager.setPoint(U"currentX", U"currentY", *index); // 座標をテキストボックスに設定
					}
				}
			}

			// UI表示（チェックボックスの状態に応じて描画）
			if (ui.shouldShowGrid()) {                        // グリッド表示がONの場合
				renderer.renderGrid(columnQuads, rowQuads);
			}

			if (ui.shouldShowIndex()) {                       // インデックス表示がONの場合
				renderer.renderIndices();
			}

			if (ui.shouldShowGridBuiWhichIsThePlayer()) {     // 建物所属表示がONの場合
				renderer.renderBattleMarkers(gridManager, ui.getSelectedIndex());
			}

			if (ui.shouldShowUnit()) {                        // ユニット表示がONの場合
				renderer.renderUnits(arrayEnemy);
			}
		}

		// 2D カメラの UI を表示する（操作説明等）
		cameraController.draw(Palette::Orange);

		// タイルメニューを表示する（タイル・建物選択UI）
		renderTileMenu(TileMenuRoundRect, tileTypeSelected, textures, texturesBui, ui);

		renderResourcePoints(resourceManager, coordSystem);

		// UI要素の描画（チェックボックス・ボタン等）
		ui.renderBasicCheckboxes();

		if (!ui.isTileMode()) {                               // 建物モードの場合のみ
			nowBattleWhichIsThePlayer = ui.renderBattleTypeRadioButtons(); // 陣営選択UIを表示
		}

		// ファイル操作（保存・読み込みボタンの処理）
		handleFileOperations(gridManager, coordSystem, columnQuads, rowQuads,
							textures, texturesBui, arrayEnemy, textManager, sor, def, neu, resourceManager);

		// サイズ変更処理（マップサイズ変更UIの処理）
		handleMapSizeChange(gridManager, coordSystem, columnQuads, rowQuads, textManager);

		// 塗りつぶし処理（全タイルを同じ種類で埋めるボタン）
		if (SimpleGUI::Button(U"塗りつぶし", Vec2{ 20,440 }, 160)) {
			if (ui.isTileMode()) {                            // タイルモードの場合のみ
				gridManager.fillTiles(tileTypeSelected);     // 選択中のタイルで塗りつぶし
			}
		}

		// ユニット管理（ユニット配置・編集UIの処理）
		handleUnitManagement(ui, font, textManager, arrayEnemy, sor, def, neu);

		// リソース管理UIの処理
		handleResourceManagement(ui, font,
			textManager.get(U"Resource-currentX"),
			textManager.get(U"Resource-currentY"),
			textManager.get(U"Resource-Key"), resourceManager);
	}
}

// ヘルパー関数（メイン処理から分離した補助機能群）
// タイルメニューの描画（タイル・建物選択用のパレット表示）
void renderTileMenu(const RoundRect& menuRect, int32& selectedType,
				   const Array<std::pair<String, Texture>>& textures,
				   const Array<std::pair<String, Texture>>& buildingTextures,
				   const MapEditorUI& ui) {
	// 背景（メニュー全体の背景を描画）
	menuRect.draw();

	// 各タイル（グリッド状にタイル・建物を配置）
	for (int32 y = 0; y < UIConstants::TILE_MENU_ROWS; ++y) {     // 行ループ
		for (int32 x = 0; x < UIConstants::TILE_MENU_COLS; ++x) { // 列ループ
			const Rect rect{ (20 + x * UIConstants::TILE_SIZE), (20 + y * 50), UIConstants::TILE_SIZE, 50 }; // 各タイルの描画領域
			const int32 tileType = (y * UIConstants::TILE_MENU_COLS + x);     // タイル番号を計算

			if (tileType == selectedType) {               // 選択中のタイルの場合
				rect.draw(ColorF{ 0.85 });                // 背景をハイライト
			}

			if (rect.mouseOver()) {                       // マウスが上にある場合
				Cursor::RequestStyle(CursorStyle::Hand);  // カーソルを手の形に変更
				if (MouseL.down()) {                      // クリックされた場合
					selectedType = tileType;              // 選択タイプを更新
				}
			}

			// 現在のモードに応じてテクスチャ配列を選択
			const auto& textureArray = ui.isTileMode() ? textures : buildingTextures;
			if (textureArray.size() > tileType) {        // 有効なテクスチャがある場合
				textureArray[tileType].second.scaled(0.5).drawAt(rect.center()); // 50%縮小して中央に描画
			}
		}
	}
}

// ファイル操作処理（保存・読み込み機能）
void handleFileOperations(GridManager& gridManager, TileCoordinateSystem& coordSystem,
						 Array<Quad>& columnQuads, Array<Quad>& rowQuads,
						 const Array<std::pair<String, Texture>>& textures,
						 const Array<std::pair<String, Texture>>& buildingTextures,
						 const Array<Unit>& units, TextEditStateManager& textManager,
						 const Optional<Point>& sor, const Optional<Point>& def, const Optional<Point>& neu,
						 const ResourceManager& resourceManager) {

	// データ出力ボタン（マップをTOMLファイルに保存）
	if (SimpleGUI::Button(U"Output data", Vec2{ 20,360 }, 160)) {
		WriteTomlFile(U"Output map.toml", gridManager.getTileGrid(), gridManager.getBuildingGrid(),
					 gridManager.getBuildingPlayerGrid(), sor, def, neu, textures, buildingTextures, units, resourceManager);
	}

	// データ読み込みボタン（TOMLファイルからマップを復元）
	if (SimpleGUI::Button(U"Read data", Vec2{ 20,600 }, 160)) {
		Optional<FilePath> path = Dialog::OpenFile({ FileFilter::AllFiles() }); // ファイル選択ダイアログ
		auto loadResult = TomlMapLoader::loadFromFile(path);                    // ファイル読み込み

		if (loadResult.success) {                     // 読み込み成功の場合
			TomlMapLoader::parseMapData(loadResult, gridManager, textures, buildingTextures,
									   coordSystem, columnQuads, rowQuads);            // データをグリッドに展開
			textManager.set(U"mapSize", Format(gridManager.getSize()));                // UIのマップサイズも更新
		}
	}
}

// サイズ変更処理（動的なマップリサイズ機能）
void handleMapSizeChange(GridManager& gridManager, TileCoordinateSystem& coordSystem,
						Array<Quad>& columnQuads, Array<Quad>& rowQuads, TextEditStateManager& textManager) {

	// マップサイズ入力欄（ユーザーが数値を入力）
	SimpleGUI::TextBox(textManager.get(U"mapSize"), Vec2{ 20, 400 }, 160, 4);

	int32 newSize = textManager.getInt(U"mapSize", 16);                       // 入力値を整数として取得
	if (newSize != gridManager.getSize() && newSize > 0) {                   // サイズが変更され、かつ有効な値の場合
		gridManager.resize(newSize);                  // グリッドをリサイズ
		coordSystem.updateMapSize(newSize);           // 座標システムを更新
		columnQuads = coordSystem.makeColumnQuads();  // 判定用四角形を再生成
		rowQuads = coordSystem.makeRowQuads();
	}
}

// ユニット管理処理（ユニット配置・編集機能）
void handleUnitManagement(MapEditorUI& ui, const Font& font, TextEditStateManager& textManager,
						 Array<Unit>& units, Optional<Point>& sor, Optional<Point>& def, Optional<Point>& neu) {

	if (ui.isSettingUnit()) {                         // ユニット配置モードの場合
		// ユニット確定ボタン（設定したユニットを実際に配置）
		if (SimpleGUI::Button(U"確定", Vec2{ UIConstants::RIGHT_PANEL_START.x, 400 }, UIConstants::TEXTBOX_WIDTH)) {
			Unit unit = ui.createUnitFromUI(textManager.getText(U"currentX"), textManager.getText(U"currentY"), textManager.getText(U"unitID")); // UI設定からユニット作成
			units.push_back(unit);                    // ユニット配列に追加
		}
		// ユニット設定UI表示（陣営・向き・座標・ID入力）
		ui.renderUnitInterface(font, textManager.get(U"currentX"), textManager.get(U"currentY"), textManager.get(U"unitID"));
	}
	else {                                          // 通常モードの場合
		// 座標表示とボタン処理（現在選択中の座標を表示・編集）
		font(U"現在の座標").draw(UIConstants::RIGHT_PANEL_START.x - 160, 400, Palette::Black);
		SimpleGUI::TextBox(textManager.get(U"currentX"), Vec2{ UIConstants::RIGHT_PANEL_START.x, 400 }, UIConstants::TEXTBOX_WIDTH, 4);
		SimpleGUI::TextBox(textManager.get(U"currentY"), Vec2{ UIConstants::RIGHT_PANEL_START.x, 440 }, UIConstants::TEXTBOX_WIDTH, 4);

		// 各種位置設定ボタン（特殊位置の設定）
		if (SimpleGUI::Button(U"現在の座標を出撃位置にする", Vec2{ UIConstants::RIGHT_PANEL_START.x - 160, 480 }, 320)) {
			sor = textManager.getPoint(U"currentX", U"currentY");             // 現在座標を取得
			if (sor) textManager.setPoint(U"sortieX", U"sortieY", *sor);      // 表示用テキストボックスにも反映
		}

		if (SimpleGUI::Button(U"現在の座標を防衛位置にする", Vec2{ UIConstants::RIGHT_PANEL_START.x - 160, 600 }, 320)) {
			def = textManager.getPoint(U"currentX", U"currentY");             // 現在座標を取得
			if (def) textManager.setPoint(U"defenseX", U"defenseY", *def);    // 表示用テキストボックスにも反映
		}

		if (SimpleGUI::Button(U"現在の座標を中立部隊の位置にする", Vec2{ UIConstants::RIGHT_PANEL_START.x - 160, 720 }, 320)) {
			neu = textManager.getPoint(U"currentX", U"currentY");             // 現在座標を取得
			if (neu) textManager.setPoint(U"neutralX", U"neutralY", *neu);    // 表示用テキストボックスにも反映
		}

		// 座標表示用テキストボックス（設定済みの特殊位置を表示・編集可能）
		font(U"出撃の座標").draw(UIConstants::RIGHT_PANEL_START.x - 160, 520, Palette::Black);
		SimpleGUI::TextBox(textManager.get(U"sortieX"), Vec2{ UIConstants::RIGHT_PANEL_START.x, 520 }, UIConstants::TEXTBOX_WIDTH, 4);
		SimpleGUI::TextBox(textManager.get(U"sortieY"), Vec2{ UIConstants::RIGHT_PANEL_START.x, 560 }, UIConstants::TEXTBOX_WIDTH, 4);

		font(U"防衛の座標").draw(UIConstants::RIGHT_PANEL_START.x - 160, 640, Palette::Black);
		SimpleGUI::TextBox(textManager.get(U"defenseX"), Vec2{ UIConstants::RIGHT_PANEL_START.x, 640 }, UIConstants::TEXTBOX_WIDTH, 4);
		SimpleGUI::TextBox(textManager.get(U"defenseY"), Vec2{ UIConstants::RIGHT_PANEL_START.x, 680 }, UIConstants::TEXTBOX_WIDTH, 4);

		font(U"中立部隊の座標").draw(UIConstants::RIGHT_PANEL_START.x - 160, 760, Palette::Black);
		SimpleGUI::TextBox(textManager.get(U"neutralX"), Vec2{ UIConstants::RIGHT_PANEL_START.x, 760 }, UIConstants::TEXTBOX_WIDTH, 4);
		SimpleGUI::TextBox(textManager.get(U"neutralY"), Vec2{ UIConstants::RIGHT_PANEL_START.x, 800 }, UIConstants::TEXTBOX_WIDTH, 4);
	}
}

void handleResourceManagement(MapEditorUI& ui, const Font& font,
	TextEditState& xxx, TextEditState& yyy, TextEditState& key, ResourceManager& resourceManager)
{
	if (ui.isSettingResource())
	{
		font(U"Resource-Key").draw(1920 - 120 - 60, 600, Palette::Black);
		SimpleGUI::TextBox(key, Vec2{ 1920 - 120 - 60, 660 }, 120, 64);// Key入力欄
		font(U"X").draw(1920 - 120 - 60, 450, Palette::Black);
		SimpleGUI::TextBox(xxx, Vec2{ 1920 - 120 - 60, 480 }, 120, 4);// X座標入力欄
		font(U"Y").draw(1920 - 120 - 60, 530, Palette::Black);
		SimpleGUI::TextBox(yyy, Vec2{ 1920 - 120 - 60, 560 }, 120, 4);// Y座標入力欄
		if (SimpleGUI::Button(U"確定", Vec2{ 1920 - 120 - 60, 720 }, 120))
		{
			// 入力値の取得
			int32 x = Parse<int32>(xxx.text);
			int32 y = Parse<int32>(yyy.text);
			String resourceKey = key.text;

			if (!resourceKey.isEmpty() && x >= 0 && y >= 0)
			{
				Point pos{ x, y };
				resourceManager.addResourcePoint(pos, resourceKey);

				// 入力欄をクリア
				xxx.text.clear();
				yyy.text.clear();
				key.text.clear();
			}
		}

		// 削除ボタンも追加
		if (SimpleGUI::Button(U"削除", Vec2{ 1920 - 120 - 60, 760 }, 120))
		{
			int32 x = Parse<int32>(xxx.text);
			int32 y = Parse<int32>(yyy.text);

			if (x >= 0 && y >= 0)
			{
				Point pos{ x, y };
				resourceManager.removeResourcePoint(pos);
			}
		}
	}
}

// マップ描画時にリソースポイントを表示
void renderResourcePoints(const ResourceManager& resourceManager, const TileCoordinateSystem& coordSystem)
{
	for (const auto& [pos, resource] : resourceManager.getAllResourcePoints())
	{
		Vec2 drawPos = coordSystem.toBottomCenter(pos);

		// リソースアイコンの描画
		Circle(drawPos, 10).draw(ColorF{ 1.0, 1.0, 0.0, 0.5 });  // 黄色の半透明円

		// キー名の表示
		PutText(resource.resourceKey, drawPos.movedBy(0, -20));
	}
}

// ResourceManagerクラスのメソッド実装を追加
void ResourceManager::addResourcePoint(const Point& pos, const String& key, const String& type,
										int32 amount, const String& displayName, const String& icon) {
	Unit resource;
	resource.isResourcePoint = true;
	resource.resourceKey = key;
	resource.resourceType = type;
	resource.resourceAmount = amount;
	resource.resourceDisplayName = displayName.isEmpty() ? key : displayName;
	resource.resourceIcon = icon.isEmpty() ? U"default.png" : icon;
	resource.x = pos.x;
	resource.y = pos.y;
	
	resourcePoints[pos] = resource;
}

Optional<Unit> ResourceManager::getResourcePoint(const Point& pos) const {
	if (resourcePoints.contains(pos)) {
		return resourcePoints.at(pos);
	}
	return none;
}

void ResourceManager::removeResourcePoint(const Point& pos) {
	resourcePoints.erase(pos);
}
