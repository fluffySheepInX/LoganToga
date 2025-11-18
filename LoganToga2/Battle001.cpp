#include "stdafx.h"
#include "Battle001.h"
#include <ranges>
#include <vector>

namespace {
	// unitID -> (skillTag -> readyAtSeconds)
	HashTable<long long, HashTable<String, double>> gSkillReadyAtSec;
	// 追加: スイング1回あたりの「既に当てたターゲット」を保存（No,RushNo 単位）
	HashTable<uint64, HashSet<long long>> gSwingHitOnce;
	// 建物の最大HP（スポーン時点の HPCastle）を保持して割合表示に使用
	HashTable<long long, double> gBuildingMaxHP;

	// Delay=1 を speed=60 で約 2 秒にするための係数
	constexpr double kAttackDelayBaseTicks = 120.0; // 秒換算すると 120/speed

	inline double getUnitAttackSpeed(const Unit& u) {
		// 速度 0 防止
		return Max(1.0, (u.Speed));
	}

	inline double calcCooldownSec(const Unit& u, const Skill& s) {
		const double delayUnit = Max(0.0, s.Delay); // 未設定や負は 0 扱い
		const double base = kAttackDelayBaseTicks / getUnitAttackSpeed(u);
		return delayUnit * base;
	}

	inline bool isSkillReady(const Unit& u, const Skill& s) {
		const double now = Scene::Time();
		if (const auto itU = gSkillReadyAtSec.find(u.ID); itU != gSkillReadyAtSec.end()) {
			if (const auto itS = itU->second.find(s.nameTag); itS != itU->second.end()) {
				return now >= itS->second;
			}
		}
		return true;
	}

	inline void commitCooldown(const Unit& u, const Skill& s) {
		gSkillReadyAtSec[u.ID][s.nameTag] = Scene::Time() + calcCooldownSec(u, s);
	}

	// unitID -> (skillTag -> 残回数)。設定が無ければ無制限扱い
	HashTable<long long, HashTable<String, int32>> gSkillUsesLeft;

	inline bool hasUsesLeft(const Unit& u, const Skill& s) {
		if (const auto itU = gSkillUsesLeft.find(u.ID); itU != gSkillUsesLeft.end()) {
			if (const auto itS = itU->second.find(s.nameTag); itS != itU->second.end()) {
				return (itS->second > 0);
			}
		}
		return true; // 未設定は無制限
	}

	inline void consumeUse(const Unit& u, const Skill& s) {
		if (auto itU = gSkillUsesLeft.find(u.ID); itU != gSkillUsesLeft.end()) {
			if (auto itS = itU->second.find(s.nameTag); itS != itU->second.end() && itS->second > 0) {
				--(itS->second);
			}
		}
	}

	// ユニット毎・スキル毎に回数を設定するヘルパ
	inline void setSkillUses(Unit& u, const String& skillTag, int32 uses) {
		gSkillUsesLeft[u.ID][skillTag] = Max(0, uses);
	}

	inline void applyMaintainRangeForSkill(Unit& u, const Skill& s)
	{
		// 近接: 接敵を目指す
		const bool isMelee = (s.range <= 80); // 近接の目安。必要ならスキルにフラグを追加
		if (isMelee)
		{
			const int32 keep = Max(0, static_cast<int32>(s.range) - 10);         // 射程-10px 目安
			const int32 band = Clamp(static_cast<int32>(s.range * 0.25), 8, 20); // 8〜20px の狭い帯
			u.MaintainRange = keep;
			u.MaintainRangeBand = band;
			return;
		}

		// 射撃: 射程の少し手前を維持（振れ幅を持たせてスパイク移動を抑制）
		const int32 sweetSpot = static_cast<int32>(Max(0.0, s.range - 40.0)); // マージン40
		const int32 band = Clamp(static_cast<int32>(s.range * 0.35), 60, 200);
		u.MaintainRange = sweetSpot;
		u.MaintainRangeBand = band;
	}

	// Skill.Special >= 0 だけ、使用回数を初期化（-1 などは無制限としてスキップ）
	inline void initSkillUsesFromSpecial(Unit& u)
	{
		for (const auto& s : u.arrSkill)
		{
			if (s.Special >= 0)
			{
				setSkillUses(u, s.nameTag, s.Special);
			}
		}
	}

	inline bool isCombatSkill(const Skill& s)
	{
		// 必要に応じて Support を除外するなど調整
		//return (s.SkillType != SkillType::heal);
		return (s.SkillType == SkillType::missile);
	}

	inline double getRemainingCooldownSec(const Unit& u, const Skill& s) {
		const double now = Scene::Time();
		if (const auto itU = gSkillReadyAtSec.find(u.ID); itU != gSkillReadyAtSec.end()) {
			if (const auto itS = itU->second.find(s.nameTag); itS != itU->second.end()) {
				return Max(0.0, itS->second - now);
			}
		}
		return 0.0;
	}

	// これを超える長いCDならフォールバックを許可（調整用）
	constexpr double kFallbackMeleeAfterCDSec = 4.0;

	// 変更: 「最優先に残弾あり＆短CD中」はその最優先スキルのレンジを維持
	inline void applyMaintainRangeByPreferredAvailableSkill(Unit& u)
	{
		// sortKey 昇順で抽出
		Array<Skill> sorted = u.arrSkill.sorted_by([](const Skill& a, const Skill& b) {
			return a.sortKey < b.sortKey;
		});

		// 最優先の戦闘スキル（missile）を取得
		const Skill* top = nullptr;
		for (const auto& s : sorted) {
			if (isCombatSkill(s)) { top = &s; break; }
		}

		// 追加: 最優先に残弾があり、CD中かつ残CDが閾値以下なら待機＝そのレンジを維持
		if (top && hasUsesLeft(u, *top) && !isSkillReady(u, *top)) {
			if (getRemainingCooldownSec(u, *top) <= kFallbackMeleeAfterCDSec) {
				applyMaintainRangeForSkill(u, *top);
				return;
			}
		}

		// 既存: 第1パス（使用可＆CD明け）
		for (const auto& s : sorted) {
			if (!isCombatSkill(s)) continue;
			if (hasUsesLeft(u, s) && isSkillReady(u, s)) {
				applyMaintainRangeForSkill(u, s);
				return;
			}
		}
		// 既存: 第2パス（使用可）
		for (const auto& s : sorted) {
			if (!isCombatSkill(s)) continue;
			if (hasUsesLeft(u, s)) {
				applyMaintainRangeForSkill(u, s);
				return;
			}
		}
		// 見つからなければ現状維持
	}

}
namespace {
	// hard 仕様: 1ターン=0.01秒。例の「speed=800 で 8ドット/ターン」に一致
	constexpr double kHardTurnSeconds = 0.01;

	struct HitIntervalState {
		Vec2 lastPos;
		double accum = 0.0; // 直近判定からの移動距離（ドット）
	};

	// (No, RushNo) -> 状態
	HashTable<uint64, HitIntervalState> gHitIntervalState;

	inline uint64 makeBulletKey(const ClassBullets& b) noexcept
	{
		return (static_cast<uint64>(static_cast<uint32>(b.No)) << 32)
			| static_cast<uint32>(b.RushNo);
	}

	// 距離閾値 = speed[dot/sec] * 0.01[sec] * hard[turn]
	inline double computeHitIntervalDistance(const Skill& s) noexcept
	{
		if (s.hard <= 0) return 0.0;
		return s.speed * kHardTurnSeconds * s.hard;
	}
}
namespace {
	// --- 30分サバイバル設定 ---
	constexpr double kSurvivalDurationSec = 30.0 * 60.0; // 30分
	static Stopwatch gSurvivalTimer;     // サバイバル用ストップウォッチ
	static bool gSurvivalStarted = false;
	static bool gSurvivalEnded = false;

	inline String formatTimeMMSS(double seconds) {
		const int32 total = Max(0, static_cast<int32>(Ceil(seconds)));
		const int32 mm = (total / 60);
		const int32 ss = (total % 60);
		return U"{:02}:{:02}"_fmt(mm, ss);
	}
}
namespace {
	// シンプルなスポーン定義
	struct SpawnRule {
		String unit;         // ユニット名（NameTag）
		double intervalSec;  // 何秒ごとに出すか
		int32  count;        // 1回で何体出すか（基本1）
	};

	// フェーズ定義（開始〜終了の絶対秒、ルール配列）
	struct PhaseDef {
		String name;
		double startSec;
		double endSec;
		Array<SpawnRule> rules;
	};

	Array<PhaseDef> gPhaseSchedule;
	HashTable<String, Stopwatch> gSpawnClocks; // unit名ごとの独立タイマー
	String gActivePhase;                       // 現在フェーズ名（切替検知用）

	inline Point pickEdgeSpawnTile(const MapTile& mapTile)
	{
		const int edge = Random(0, 3);
		switch (edge)
		{
		case 0:  return Point(0, Random(0, mapTile.N - 1));             // Left
		case 1:  return Point(mapTile.N - 1, Random(0, mapTile.N - 1));  // Right
		case 2:  return Point(Random(0, mapTile.N - 1), 0);              // Top
		case 3:  return Point(Random(0, mapTile.N - 1), mapTile.N - 1);  // Bottom
		}
		return Point(0, 0);
	}

	inline const PhaseDef* findCurrentPhase(double nowSec)
	{
		for (const auto& p : gPhaseSchedule)
		{
			if (nowSec >= p.startSec && nowSec < p.endSec)
				return &p;
		}
		return nullptr;
	}

	// とりあえずのデフォルト（後でここをいじるだけで調整可能）
	inline void initDefaultPhaseSchedule()
	{
		gPhaseSchedule.clear();

		// 0:00〜2:30 静寂期
		gPhaseSchedule.push_back(PhaseDef{
			U"0 静寂期", 0.0, 150.0,
			{
				{ U"Conscript",       8.0, 1 },
			}
		});

		// 2:30〜5:00 選択期
		gPhaseSchedule.push_back(PhaseDef{
			U"1 選択期", 150.0, 300.0,
			{
				{ U"LineInfantryEni",  9.0, 1 },
				{ U"SkirmisherEni",   12.0, 1 },
			}
		});

		// 5:00〜6:00 制圧期（少し密）
		gPhaseSchedule.push_back(PhaseDef{
			U"2 制圧期", 300.0, 360.0,
			{
				{ U"LineInfantryEni",  6.0, 1 },
				{ U"SkirmisherEni",    9.0, 1 },
			}
		});

		// 6:00〜7:30 不穏期（バリエーション増）
		gPhaseSchedule.push_back(PhaseDef{
			U"3 不穏期", 360.0, 450.0,
			{
				{ U"LineInfantryEni",  7.5, 1 },
				{ U"SkirmisherEni",   10.0, 1 },
			}
		});

		// 7:30〜11:00 破壊期（密度アップ）
		gPhaseSchedule.push_back(PhaseDef{
			U"4 破壊期", 450.0, 660.0,
			{
				{ U"SkirmisherEni",    7.0, 1 },
				{ U"LineInfantryEni",  8.5, 1 },
			}
		});

		gActivePhase.clear();
		gSpawnClocks.clear();
	}

	// 実スポーン処理（毎フレーム呼び出し）
	inline void spawnEnemiesByPhaseImpl(Battle001& self, ClassBattle& classBattleManage, const MapTile& mapTile, double nowSec)
	{
		const PhaseDef* phase = findCurrentPhase(nowSec);
		if (!phase) return;

		// フェーズが切り替わったらタイマーリセット
		if (gActivePhase != phase->name)
		{
			gSpawnClocks.clear();
			gActivePhase = phase->name;
		}

		for (const auto& r : phase->rules)
		{
			auto& sw = gSpawnClocks[r.unit];
			if (!sw.isRunning())
				sw.restart();

			if (sw.sF() >= r.intervalSec)
			{
				for (int i = 0; i < r.count; ++i)
				{
					const Point tile = pickEdgeSpawnTile(mapTile);
					self.UnitRegister(classBattleManage, mapTile, r.unit, tile.x, tile.y, 1,
						classBattleManage.listOfAllEnemyUnit, true);
				}
				sw.restart();
			}
		}
	}
}
namespace {
	// 画面座標で塗る、縦グラデーション背景
	inline void drawBackgroundGradient()
	{
		// 上:やや青み / 下:暗め 例
		RectF{ 0, 0, Scene::Width(), Scene::Height() }
		.draw(Arg::top = ColorF{ 0.07, 0.10, 0.14 }, Arg::bottom = ColorF{ 0.02, 0.02, 0.03 });
	}

	// パターン画像のパララックス背景（カメラに対して遅く動く）
	inline void drawParallaxPattern(const Camera2D& cam, const String& assetName = U"bg_pattern.png", double scale = 0.75, double parallax = 0.02)
	{
		if (!TextureAsset::IsRegistered(assetName)) return;

		const Texture tex = TextureAsset(assetName);
		const double stepX = tex.width() * scale;
		const double stepY = tex.height() * scale;

		if (stepX <= 0 || stepY <= 0) return;

		// カメラ中心に応じて緩やかに流れる
		const Vec2 scroll = cam.getCenter() * parallax;
		auto mod = [](double a, double m) { double r = std::fmod(a, m); return (r < 0 ? r + m : r); };
		const double ox = mod(scroll.x, stepX);
		const double oy = mod(scroll.y, stepY);

		// 画面を埋めるようにタイル描画
		const int32 cols = static_cast<int32>(Scene::Width() / stepX) + 3;
		const int32 rows = static_cast<int32>(Scene::Height() / stepY) + 3;

		const TextureRegion scaled = tex.scaled(scale);
		for (int32 iy = -1; iy < rows; ++iy)
		{
			for (int32 ix = -1; ix < cols; ++ix)
			{
				const double x = ix * stepX - ox;
				const double y = iy * stepY - oy;
				scaled.draw(x, y);
			}
		}
	}
}

template<typename DrawFunc>
void forEachVisibleTile(const RectF& cameraView, const MapTile& mapTile, DrawFunc drawFunc)
{
	// カメラビューとタイル範囲の交差判定を事前計算
	const int32 startI = Max(0, static_cast<int32>((cameraView.y - mapTile.TileOffset.y) / mapTile.TileOffset.y) - 1);
	const int32 endI = Min(mapTile.N * 2 - 1, static_cast<int32>((cameraView.bottomY() + mapTile.TileOffset.y) / mapTile.TileOffset.y) + 1);

	for (int32 i = startI; i < endI; ++i)  // ← startI, endI を使用
	{
		int32 xi = (i < (mapTile.N - 1)) ? 0 : (i - (mapTile.N - 1));
		int32 yi = (i < (mapTile.N - 1)) ? i : (mapTile.N - 1);

		for (int32 k = 0; k < (mapTile.N - Abs(mapTile.N - i - 1)); ++k)
		{
			Point index{ xi + k, yi - k };
			Vec2 pos = mapTile.ToTileBottomCenter(index, mapTile.N);

			if (!cameraView.intersects(pos))
				continue;

			drawFunc(index, pos);
		}
	}
}

void Battle001::createRenderTex()
{
	htBuildMenuRenderTexture.clear();

	if (htBuildMenu.empty())
		return;

	Array<std::pair<String, BuildAction>> sortedArray(htBuildMenu.begin(), htBuildMenu.end());
	sortedArray.sort_by([](const auto& a, const auto& b) {
		return a.first < b.first;
	});

	String currentKey = U"";
	int32 counter = 0;

	for (const auto& item : sortedArray)
	{
		const String key = item.first.split('-')[0];  // 1回だけ split

		// 新しいカテゴリの場合、新しいレンダーテクスチャを作成
		if (key != currentKey)
		{
			currentKey = key;

			RenderTexture rt{ 328, 328 };
			{
				const ScopedRenderTarget2D target{ rt.clear(ColorF{ 0.8, 0.8, 0.8, 0.5 }) };
				const ScopedRenderStates2D blend{ MakeBlendState() };
				Rect(328, 328).drawFrame(4, 0, ColorF{ 0.5 });
			}

			htBuildMenuRenderTexture.emplace(key, std::move(rt));
			counter = 0;
		}

		// 建築可能なアイテムのみ描画
		if (item.second.buildCount == 0) continue;

		// レンダーテクスチャに描画
		{
			const ScopedRenderTarget2D target{ htBuildMenuRenderTexture[currentKey] };
			const ScopedRenderStates2D blend{ MakeBlendState() };

			Rect rectBuildMenuHome{
				((counter % 6) * 64) + 4,
				((counter / 6) * 64) + 4,
				64,
				64
			};

			// const_cast を避けるため、非const版を作成することを検討
			const_cast<BuildAction&>(item.second).rectHantei = rectBuildMenuHome;

			const double scale = static_cast<double>(rectBuildMenuHome.w) / rectBuildMenuHome.h;
			TextureAsset(item.second.icon)
				.scaled(scale)
				.draw(rectBuildMenuHome.x, rectBuildMenuHome.y);
		}

		counter++;
	}

	sortedArrayBuildMenu = std::move(sortedArray);
}

static BuildAction& dummyMenu()
{
	static BuildAction instance;
	return instance;
}

MapDetail Battle001::parseMapDetail(StringView tileData, const ClassMap& classMap, CommonConfig& commonConfig)
{
	MapDetail mapDetail;
	String tileDataMutable = String(tileData); // Modifiable copy

	// RESOURCE:XXXパターンをチェック
	if (tileDataMutable.includes(U"RESOURCE:"))
	{
		// RESOURCE:XXXを抽出
		auto resourceStart = tileDataMutable.indexOf(U"RESOURCE:");
		if (resourceStart != String::npos)
		{
			auto resourceEnd = tileDataMutable.indexOf(U',', resourceStart);
			if (resourceEnd == String::npos) resourceEnd = tileDataMutable.length();

			String resourcePattern = tileDataMutable.substr(resourceStart, resourceEnd - resourceStart);
			String resourceName = resourcePattern.substr(9); // "RESOURCE:"を除去

			// commonConfig.htResourceDataから対応するリソース情報を取得
			if (commonConfig.htResourceData.contains(resourceName))
			{
				const ClassResource& resource = commonConfig.htResourceData[resourceName];

				// MapDetailにリソース情報を設定
				mapDetail.isResourcePoint = true;
				mapDetail.resourcePointType = resource.resourceType;
				mapDetail.resourcePointAmount = resource.resourceAmount;
				mapDetail.resourcePointDisplayName = resource.resourceName;
				mapDetail.resourcePointIcon = resource.resourceIcon;
			}

			// RESOURCE:XXXをマップデータから除去
			tileDataMutable = tileDataMutable.replaced(resourcePattern, U"");
		}
	}

	Array<String> main_parts = tileDataMutable.split(U'*');
	//field(床画像
	// 修正: classMap.ele を HashTable から std::unordered_map に変更する必要がある場合、以下のようにアクセスします。
	mapDetail.tip = classMap.ele.at(main_parts[0]);
	//build(城壁や矢倉など
	if (main_parts.size() > 1)
	{
		if (main_parts[1] != U"")
		{
			Array<String> building_strings = main_parts[1].split(U'$');
			for (const String& building_string : building_strings)
			{
				Array<String> building_parts = building_string.split(U':');
				// 修正: classMap.ele を HashTable から std::unordered_map に変更する必要がある場合、以下のようにアクセスします。
				String building_name = classMap.ele.at(building_parts[0]);
				if (building_name != U"")
				{
					std::tuple<String, long, BattleWhichIsThePlayer> building_tuple = { building_name, -1, BattleWhichIsThePlayer::None };
					if (building_parts.size() > 1)
					{
						if (building_parts[1] == U"sor")
						{
							building_tuple = { building_name, -1, BattleWhichIsThePlayer::Sortie };
						}
						else if (building_parts[1] == U"def")
						{
							building_tuple = { building_name, -1, BattleWhichIsThePlayer::Def };
						}
					}
					mapDetail.building.push_back(building_tuple);
				}
			}
		}
	}
	//ユニットの情報
	if (main_parts.size() > 2)
	{
		Array<String> unit_parts = main_parts[2].split(U':');
		if (unit_parts[0] != U"-1")
		{
			mapDetail.unit = unit_parts[0];
			mapDetail.houkou = unit_parts[1];
		}
	}
	//【出撃、防衛、中立の位置】もしくは【退却位置】
	if (main_parts.size() > 3)
	{
		mapDetail.posSpecial = main_parts[3];
		if (mapDetail.posSpecial == U"@@")
		{
			mapDetail.kougekiButaiNoIti = true;
		}
		else if (mapDetail.posSpecial == U"@")
		{
			mapDetail.boueiButaiNoIti = true;
		}
	}
	//陣形
	if (main_parts.size() > 5)
	{
		mapDetail.zinkei = main_parts[5];
	}
	return mapDetail;
}

ClassMapBattle Battle001::GetClassMapBattle(ClassMap classMap, CommonConfig& commonConfig)
{
	ClassMapBattle battleMap;
	battleMap.name = classMap.name;
	for (const String& row_string : classMap.data)
	{
		Array<MapDetail> map_detail_row;
		Array<String> tile_strings = row_string.split(U',');
		for (const auto& tile_data_string : tile_strings)
		{
			map_detail_row.push_back(parseMapDetail(tile_data_string, classMap, commonConfig));
		}
		battleMap.mapData.push_back(std::move(map_detail_row));
	}

	return battleMap;
}

/// @brief Battle001 クラスのコンストラクタ。ゲームデータや設定をもとにバトルマップを初期化し、リソース情報やツールチップの設定を行います。
/// @param saveData ゲームの進行状況などを保持する GameData 型の参照。
/// @param commonConfig 共通設定を保持する CommonConfig 型の参照。
/// @param argSS システム文字列情報を渡す SystemString 型の値。
Battle001::Battle001(GameData& saveData, CommonConfig& commonConfig, SystemString argSS, bool isTest)
	:FsScene(U"Battle")
	, m_saveData{ saveData }
	, m_commonConfig{ commonConfig }
	, ss{ argSS }
	, tempSelectComRight{ dummyMenu() }
	, unitTooltip{ fontInfo.fontSkill }
{
	// Test mode constructor, skip file loading and initialization
}

Battle001::Battle001(GameData& saveData, CommonConfig& commonConfig, SystemString argSS)
	:FsScene(U"Battle")
	, m_saveData{ saveData }
	, m_commonConfig{ commonConfig }
	, ss{ argSS }
	, tempSelectComRight{ dummyMenu() }
	, unitTooltip{ fontInfo.fontSkill }
{
	const TOMLReader map_toml_reader{ PATHBASE + PATH_DEFAULT_GAME + U"/016_BattleMap/map001.toml" };
	if (not map_toml_reader)
		throw Error{ U"Failed to load `map_toml_reader`" };

	ClassMap loaded_map;
	for (const auto& map_table : map_toml_reader[U"Map"].tableArrayView()) {
		const String name = map_table[U"name"].get<String>();

		{
			int32 counter = 0;
			while (true)
			{
				String elementKey = U"ele{}"_fmt(counter);
				const String elementValue = map_table[elementKey].get<String>();
				loaded_map.ele.emplace(elementKey, elementValue);
				counter++;
				if (elementValue == U"")
				{
					break;
				}
			}
		}
		{
			namespace views = std::views;
			const String str = map_table[U"data"].get<String>();
			for (const auto sv : str | views::split(U"$"_sv))
			{
				String processedString
					= ClassStaticCommonMethod::ReplaceNewLine(String(sv.begin(), sv.end()));
				if (processedString != U"")
				{
					loaded_map.data.push_back(processedString);
				}
			}
		}
	}

	classBattleManage.classMapBattle = GetClassMapBattle(loaded_map, commonConfig);

	/// >>>マップの読み込み
	mapTile.N = classBattleManage.classMapBattle.value().mapData.size();
	mapTile.TileOffset = { 50, 25 };
	mapTile.TileThickness = 15;
	/// <<<マップの読み込み

	/// >>>Resourceの設定
	Array<ResourcePointTooltip::TooltipTarget> resourceTargets;
	SetResourceTargets(classBattleManage.classMapBattle.value().mapData, resourceTargets, mapTile);
	resourcePointTooltip.setTargets(resourceTargets);
	resourcePointTooltip.setTooltipEnabled(true);
	/// <<<Resourceの設定

}

void Battle001::initializeForTest()
{
	// Initialize necessary members for draw functions to run without crashing.
	visibilityMap = Grid<Visibility>(1, 1, Visibility::Unseen); // Minimal initialization
	mapTile.N = 1;
	// Call the private UI initializer
	initUI();
}

/// @brief 後片付け
Battle001::~Battle001()
{
	aStar.abortAStarEnemy = true;
	aStar.abortAStarMyUnits = true;
	abortFogTask = true;

	/// >>>完全に終了を待つ---
	if (aStar.taskAStarEnemy.isValid())
		aStar.taskAStarEnemy.wait();
	if (aStar.taskAStarMyUnits.isValid())
		aStar.taskAStarMyUnits.wait();
	if (taskFogCalculation.isValid())
		taskFogCalculation.wait();
	/// ---完全に終了を待つ<<<
}
/// @brief 
/// @param classBattleManage 
/// @param resourceTargets 
/// @param mapTile 
void Battle001::SetResourceTargets(
	Array<Array<MapDetail>> mapData,
	Array<ResourcePointTooltip::TooltipTarget>& resourceTargets,
	MapTile mapTile)
{
	const int32 mapSize = static_cast<int32>(mapData.size());

	for (int32 x = 0; x < mapSize; ++x)
	{
		for (int32 y = 0; y < static_cast<int32>(mapData[x].size()); ++y)
		{
			const auto& tile = mapData[x][y];

			if (!tile.isResourcePoint)
				continue;

			const Vec2 pos = mapTile.ToTileBottomCenter(Point(x, y), mapTile.N);

			const Circle area =
				Circle(pos.movedBy(0, -mapTile.TileThickness - mapTile.TileOffset.y),
									TextureAsset(tile.resourcePointIcon).region().w / 2);
			String desc = U"資源ポイント\n所有者: {}\n種類:{}\n量:{}"_fmt(
						(tile.whichIsThePlayer == BattleWhichIsThePlayer::Sortie)
				? U"味方"
				: U"敵"
					, tile.resourcePointDisplayName, U"{0}/s"_fmt(tile.resourcePointAmount));

			resourceTargets << ResourcePointTooltip::TooltipTarget{ area, desc };
		}
	}
}

/// @brief ユニットを指定された位置に登録
/// @param classBattleManage バトル管理用のClassBattleオブジェクト
/// @param mapTile ユニットを配置するマップタイル
/// @param unitName 登録するユニットの名前
/// @param col ユニットを配置する列番号
/// @param row ユニットを配置する行番号
/// @param num 登録するユニットの数
/// @param listU ユニットを格納するClassHorizontalUnitの配列への参照
/// @param enemy ユニットが敵かどうかを示すフラグ
void Battle001::UnitRegister(
	ClassBattle& classBattleManage,
	const MapTile& mapTile,
	const String& unitName,
	int32 col,
	int32 row,
	int32 num,
	Array<ClassHorizontalUnit>& unit_list,
	bool enemy)
{
	// 該当するユニットテンプレートを検索
	const auto it = std::find_if(m_commonConfig.arrayInfoUnit.begin(), m_commonConfig.arrayInfoUnit.end(),
		[&unitName](const auto& unit) { return unit.NameTag == unitName; });

	if (it == m_commonConfig.arrayInfoUnit.end() || num <= 0)
		return;

	Unit base = *it;
	ClassHorizontalUnit new_horizontal_unit;
	new_horizontal_unit.ListClassUnit.reserve(num);
	Array<std::tuple<Point, std::unique_ptr<Unit>, Unit*>> buildingCache;

	for (size_t i = 0; i < num; i++)
	{
		Unit unit_template = base;
		unit_template.ID = classBattleManage.getIDCount();
		unit_template.Hp = unit_template.HpMAX;
		unit_template.initTilePos = Point{ col, row };
		unit_template.colBuilding = col;
		unit_template.rowBuilding = row;
		unit_template.nowPosiLeft = mapTile.ToTile(unit_template.initTilePos, mapTile.N)
			.asPolygon()
			.centroid()
			.movedBy(-(unit_template.yokoUnit / 2), -(unit_template.TakasaUnit / 2));
		unit_template.taskTimer = Stopwatch();
		unit_template.taskTimer.reset();

		Vec2 temp = unit_template.GetNowPosiCenter().movedBy(-(LIQUID_BAR_WIDTH / 2), LIQUID_BAR_HEIGHT_POS);
		unit_template.bLiquidBarBattle = GameUIToolkit::LiquidBarBattle(Rect(temp.x, temp.y, LIQUID_BAR_WIDTH, LIQUID_BAR_HEIGHT));

		if (enemy)
			unit_template.moveState = moveState::MoveAI;

		if (unit_template.IsBuilding)
		{
			auto uptr = std::make_unique<Unit>(unit_template);
			Unit* raw = uptr.get();
			buildingCache.emplace_back(unit_template.initTilePos, std::move(uptr), raw);
			auto sharedPtr = std::make_shared<Unit>(unit_template);
			{
				Vec2 bottom = mapTile.ToTileBottomCenter(Point(sharedPtr->colBuilding, sharedPtr->rowBuilding), mapTile.N)
					.movedBy(0, -mapTile.TileThickness);
				Vec2 barTopLeft = bottom.movedBy(-(LIQUID_BAR_WIDTH / 2.0), 6.0);
				sharedPtr->bLiquidBarBattle = GameUIToolkit::LiquidBarBattle(Rect(barTopLeft.x, barTopLeft.y, LIQUID_BAR_WIDTH, LIQUID_BAR_HEIGHT));

				// 建物の最大HP（比率計算用）を記録（0 回避）
				gBuildingMaxHP[sharedPtr->ID] = Max<double>(1.0, sharedPtr->HPCastle);
			}
			classBattleManage.hsMyUnitBuilding.insert(sharedPtr);
		}

		new_horizontal_unit.ListClassUnit.push_back(std::move(unit_template));

		// Skill.Special を使用回数として初期化
		initSkillUsesFromSpecial(new_horizontal_unit.ListClassUnit.back());
	}

	{
		std::unique_lock lock(aStar.unitListRWMutex);
		unit_list.push_back(std::move(new_horizontal_unit));

		// 建物関連一括反映
		for (auto& [tilePos, uptr, raw] : buildingCache)
		{
			unitsForHsBuildingUnitForAstar.push_back(std::move(uptr));
			hsBuildingUnitForAstar[tilePos].push_back(raw);
		}
	}

	aStar.changeUnitMember.store(true);
}

/// @brief 指定した経過時間後に敵ユニットをマップ上にスポーン
/// @param classBattleManage 全ての敵ユニットのリストなど、バトル管理に関する情報を持つクラス
/// @param mapTile マップのサイズやタイル情報を持つクラス
void Battle001::spawnTimedEnemy(ClassBattle& classBattleManage, MapTile mapTile)
{
	if (stopwatchGameTime.sF() >= ENEMY_SPAWN_INTERVAL)
	{
		const auto edge = static_cast<SpawnEdge>(Random(0, 3));
		int32 x = 0, y = 0;

		switch (edge)
		{
		case SpawnEdge::Left:
			x = 0;
			y = Random(0, mapTile.N - 1);
			break;
		case SpawnEdge::Right:
			x = mapTile.N - 1;
			y = Random(0, mapTile.N - 1);
			break;
		case SpawnEdge::Top:
			x = Random(0, mapTile.N - 1);
			y = 0;
			break;
		case SpawnEdge::Bottom:
			x = Random(0, mapTile.N - 1);
			y = mapTile.N - 1;
			break;
		default:
			x = 0;
			y = 0;
			break;
		}

		if (RandomBool(0.5))
		{
			UnitRegister(classBattleManage, mapTile, U"sniperP99", x, y, 1,
				classBattleManage.listOfAllEnemyUnit, true);
		}
		else
		{
			UnitRegister(classBattleManage, mapTile, U"sniperP99-near", x, y, 1,
				classBattleManage.listOfAllEnemyUnit, true);
		}

		stopwatchGameTime.restart();
	}
}
void Battle001::spawnTimedEnemyEni(ClassBattle& classBattleManage, MapTile mapTile)
{
	if (stopwatchGameTime.sF() >= ENEMY_SPAWN_INTERVAL)
	{
		const auto edge = static_cast<SpawnEdge>(Random(0, 3));
		int32 x = 0, y = 0;

		switch (edge)
		{
		case SpawnEdge::Left:
			x = 0;
			y = Random(0, mapTile.N - 1);
			break;
		case SpawnEdge::Right:
			x = mapTile.N - 1;
			y = Random(0, mapTile.N - 1);
			break;
		case SpawnEdge::Top:
			x = Random(0, mapTile.N - 1);
			y = 0;
			break;
		case SpawnEdge::Bottom:
			x = Random(0, mapTile.N - 1);
			y = mapTile.N - 1;
			break;
		default:
			x = 0;
			y = 0;
			break;
		}

		if (RandomBool(0.5))
		{
			UnitRegister(classBattleManage, mapTile, U"LineInfantryEni", x, y, 1,
				classBattleManage.listOfAllEnemyUnit, true);
		}
		else
		{
			UnitRegister(classBattleManage, mapTile, U"LineInfantryEni", x, y, 1,
				classBattleManage.listOfAllEnemyUnit, true);
		}

		stopwatchGameTime.restart();
	}
}
void Battle001::spawnTimedEnemySkirmisher(ClassBattle& classBattleManage, MapTile mapTile)
{
	if (stopwatchGameTime.sF() >= ENEMY_SPAWN_INTERVAL)
	{
		const auto edge = static_cast<SpawnEdge>(Random(0, 3));
		int32 x = 0, y = 0;

		switch (edge)
		{
		case SpawnEdge::Left:
			x = 0;
			y = Random(0, mapTile.N - 1);
			break;
		case SpawnEdge::Right:
			x = mapTile.N - 1;
			y = Random(0, mapTile.N - 1);
			break;
		case SpawnEdge::Top:
			x = Random(0, mapTile.N - 1);
			y = 0;
			break;
		case SpawnEdge::Bottom:
			x = Random(0, mapTile.N - 1);
			y = mapTile.N - 1;
			break;
		default:
			x = 0;
			y = 0;
			break;
		}

		if (RandomBool(0.5))
		{
			UnitRegister(classBattleManage, mapTile, U"SkirmisherEni", x, y, 1,
				classBattleManage.listOfAllEnemyUnit, true);
		}
		else
		{
			UnitRegister(classBattleManage, mapTile, U"SkirmisherEni", x, y, 1,
				classBattleManage.listOfAllEnemyUnit, true);
		}

		stopwatchGameTime.restart();
	}
}

/// @brief ユニットの視界範囲に基づいて、マップ上の可視性を更新します。
/// @param vis 可視性情報を格納するグリッドへの参照。
/// @param units 可視性を計算するユニットの配列。
/// @param mapSize マップの一辺のサイズ（タイル数）。
/// @param mapTile マップタイル情報への参照。座標変換などに使用されます。
void Battle001::UpdateVisibility(Grid<Visibility>& vis, const Array<Unit>& units, int32 mapSize, MapTile& mapTile) const
{
	for (const auto& unit : units)
	{
		const Vec2 unitPos = unit.GetNowPosiCenter();
		const auto unitIndex = mapTile.ToIndex(unitPos, mapTile.columnQuads, mapTile.rowQuads);

		if (!unitIndex) continue;

		const Point centerTile = unitIndex.value();
		const int32 visionRadius = unit.visionRadius;

		for (int dy = -visionRadius; dy <= visionRadius; ++dy)
		{
			for (int dx = -visionRadius; dx <= visionRadius; ++dx)
			{
				const Point targetTile = centerTile + Point{ dx, dy };

				if (InRange(targetTile.x, 0, mapSize - 1) &&
					InRange(targetTile.y, 0, mapSize - 1) &&
					targetTile.manhattanDistanceFrom(centerTile) <= visionRadius)
				{
					vis[targetTile] = Visibility::Visible;
				}
			}
		}
	}
}
/// @brief 戦闘マップの霧（フォグ・オブ・ウォー）状態を更新します。
/// @param classBattleManage 全ユニットのリストなど、戦闘の管理情報を持つClassBattle型の参照。
/// @param visibilityMap 各マスの可視状態を保持するVisibility型のGrid。
/// @param mapTile マップのタイル情報を保持するMapTile型の参照。
void Battle001::refreshFogOfWar(const ClassBattle& classBattleManage, Grid<Visibility>& visibilityMap, MapTile& mapTile)
{
	// 💡 差分更新に変更
	static HashSet<Point> lastVisibleTiles;
	HashSet<Point> currentVisibleTiles;

	// 新しく見える範囲を計算
	//std::shared_lock lock(aStar.unitListRWMutex);
	for (auto& units : classBattleManage.listOfAllUnit)
	{
		for (const auto& unit : units.ListClassUnit)
		{
			const Vec2 unitPos = unit.GetNowPosiCenter();
			const auto unitIndex = mapTile.ToIndex(unitPos, mapTile.columnQuads, mapTile.rowQuads);
			if (!unitIndex) continue;

			const Point centerTile = unitIndex.value();
			const int32 visionRadius = unit.visionRadius;

			for (int dy = -visionRadius; dy <= visionRadius; ++dy)
			{
				for (int dx = -visionRadius; dx <= visionRadius; ++dx)
				{
					const Point targetTile = centerTile + Point{ dx, dy };
					if (InRange(targetTile.x, 0, mapTile.N - 1) &&
						InRange(targetTile.y, 0, mapTile.N - 1) &&
						targetTile.manhattanDistanceFrom(centerTile) <= visionRadius)
					{
						currentVisibleTiles.insert(targetTile);
					}
				}
			}
		}
	}

	// 味方建物も視界ソースにする
	for (const auto& sp : classBattleManage.hsMyUnitBuilding)
	{
		if (!sp || !sp->IsBattleEnable) continue;

		const Vec2 unitPos = sp->GetNowPosiCenter();
		const auto unitIndex = mapTile.ToIndex(unitPos, mapTile.columnQuads, mapTile.rowQuads);
		if (!unitIndex) continue;

		const Point centerTile = unitIndex.value();
		const int32 visionRadius = sp->visionRadius;

		for (int dy = -visionRadius; dy <= visionRadius; ++dy)
		{
			for (int dx = -visionRadius; dx <= visionRadius; ++dx)
			{
				const Point targetTile = centerTile + Point{ dx, dy };
				if (InRange(targetTile.x, 0, mapTile.N - 1)
					&& InRange(targetTile.y, 0, mapTile.N - 1)
					&& targetTile.manhattanDistanceFrom(centerTile) <= visionRadius)
				{
					currentVisibleTiles.insert(targetTile);
				}
			}
		}
	}

	// 差分のみ更新
	for (const auto& tile : lastVisibleTiles)
	{
		if (!currentVisibleTiles.contains(tile))
			visibilityMap[tile] = Visibility::Unseen;
	}

	for (const auto& tile : currentVisibleTiles)
	{
		visibilityMap[tile] = Visibility::Visible;
	}

	lastVisibleTiles = std::move(currentVisibleTiles);

	////毎タスクで霧gridをfalseにすれば、「生きているユニットの周りだけ明るい」が可能
	//// 一度見たタイルは UnseenではなくSeenにしたい
	//for (auto&& visibilityElement : visibilityMap)
	//	visibilityElement = Visibility::Unseen;

	//for (auto& units : classBattleManage.listOfAllUnit)
	//	UpdateVisibility(std::ref(visibilityMap), std::ref(units.ListClassUnit), mapTile.N, mapTile);
}
/// @brief 指定されたファイルパスとテクスチャ設定から、テクスチャアセットデータを作成します。ロード時にアルファ値を考慮して色成分を補正し、テクスチャを生成します。
/// @param path テクスチャファイルのパス。
/// @param textureDesc テクスチャの設定情報。
/// @return 作成された TextureAssetData のユニークポインタ。
std::unique_ptr<TextureAssetData> Battle001::MakeTextureAssetData1(const FilePath& path, const TextureDesc textureDesc)
{
	// 空のテクスチャアセットデータを作成する
	std::unique_ptr<TextureAssetData> assetData = std::make_unique<TextureAssetData>();

	// ファイルパスを代入する
	assetData->path = path;

	// テクスチャの設定を代入する
	assetData->desc = textureDesc;

	// ロード時の仕事を設定する
	assetData->onLoad = [](TextureAssetData& asset, const String&)
		{
			Image image{ asset.path };
			Color* p = image.data();
			const Color* const pEnd = (p + image.num_pixels());
			while (p != pEnd)
			{
				p->r = static_cast<uint8>((static_cast<uint16>(p->r) * p->a) / 255);
				p->g = static_cast<uint8>((static_cast<uint16>(p->g) * p->a) / 255);
				p->b = static_cast<uint8>((static_cast<uint16>(p->b) * p->a) / 255);
				++p;
			}
			asset.texture = Texture{ image };
			return static_cast<bool>(asset.texture);
		};

	return assetData;
}

void Battle001::updateBuildingHashTable(const Point& tile, const ClassBattle& classBattleManage, Grid<Visibility> visibilityMap, MapTile& mapTile)
{
	// ミニマップの色更新
	mapTile.minimapCols[tile] = ColorF{ 0.5, 0.3, 0.1 }; // 建物の色

	// 視界の更新（建物により視界が変わる場合）
	refreshFogOfWar(classBattleManage, visibilityMap, mapTile);
}
/// @brief カメラ操作の入力を処理します。
void Battle001::handleCameraInput()
{
	// カメラの移動処理（左クリックドラッグ）
	if (MouseL.pressed() == true)
	{
		const auto vPos = (camera.getTargetCenter() - Cursor::Delta());
		camera.jumpTo(vPos, camera.getTargetScale());
	}
}

/// @brief 全ユニットから「移動可能なユニット」だけを抽出して部隊ごとにまとめる
/// @return 
Array<Array<Unit*>> Battle001::GetMovableUnitGroups()
{
	Array<Array<Unit*>> groups;
	std::shared_lock lock(aStar.unitListRWMutex);
	for (auto& target : classBattleManage.listOfAllUnit)
	{
		Array<Unit*> group;

		for (auto& unit : target.ListClassUnit)
		{
			if (unit.moveState == moveState::FlagMoveCalc && unit.IsBattleEnable)
				group.push_back(&unit);
		}

		if (!group.isEmpty())
			groups.push_back(group);
	}

	return groups;
}
/// @brief 指定されたユニット配列を、開始点と終了点を基準とした隊形に割り当てます。
/// @param units 隊形に配置するユニットの配列。
/// @param start 隊形の開始位置を示す2次元座標。
/// @param end 隊形の終了位置を示す2次元座標。
/// @param rowIndex ユニットが配置される行のインデックス。
void Battle001::AssignUnitsInFormation(const Array<Unit*>& units, const Vec2& start, const Vec2& end, int32 rowIndex)
{
	const int32 unitCount = units.size();
	const int32 centerOffset = (unitCount - 1) / 2;

	const double angleForward = (start == end) ? 0.0 : Math::Atan2(end.y - start.y, end.x - start.x);
	const double anglePerpendicular = Math::Pi / 2 - angleForward;

	const double cosPerpendicular = Math::Cos(anglePerpendicular);
	const double sinPerpendicular = Math::Sin(anglePerpendicular);

	for (auto&& [i, unit] : Indexed(units))
	{
		const double unitSpacingCos = Math::Round(DistanceBetweenUnitWidth * cosPerpendicular);
		const int32 spacingOffsetFactor = (i - centerOffset);
		const int32 spacingOffsetX = spacingOffsetFactor * unitSpacingCos;
		const int32 spacingOffsetY = spacingOffsetFactor * Math::Round(DistanceBetweenUnitWidth * sinPerpendicular);

		const double rowOffsetX = rowIndex * DistanceBetweenUnitHeight * Math::Cos(angleForward);
		const double rowOffsetY = rowIndex * DistanceBetweenUnitHeight * Math::Sin(angleForward);

		const double finalX = end.x + spacingOffsetX - rowOffsetX;
		const double finalY = end.y - spacingOffsetY - rowOffsetY;

		const Vec2 unitSize = Vec2(unit->yokoUnit / 2, unit->TakasaUnit / 2);
		unit->orderPosiLeft = Vec2(Floor(finalX), Floor(finalY)) - unitSize;
		unit->orderPosiLeftLast = unit->orderPosiLeft;
		unit->moveState = moveState::MoveAI;
	}
}
/// @brief ユニットの配列から位置ベクトルを取得し、その平均値（重心）を計算します。
/// @param units 位置を計算する対象となるユニットの配列。
/// @param getPos 各ユニットから位置ベクトル（Vec2）を取得する関数。
/// @return ユニットの位置ベクトルの平均値。ユニットが存在しない場合は Vec2::Zero() を返します。
Vec2 Battle001::calcLastMerge(const Array<Unit*>& units, std::function<Vec2(const Unit*)> getPos)
{
	Vec2 sum = Vec2::Zero();
	int count = 0;
	for (const auto* u : units) {
		sum += getPos(u);
		++count;
	}
	return (count > 0) ? (sum / count) : Vec2::Zero();
}
/// @brief 指定されたユニット配列内の各ユニットに、指定した位置を設定するメンバ関数を呼び出します。
/// @param units 位置を設定する対象となるユニットの配列。
/// @param setter 各ユニットの位置を設定するメンバ関数へのポインタ。
/// @param setPos ユニットに設定する位置ベクトル。
void Battle001::setMergePos(const Array<Unit*>& units, void (Unit::* setter)(const Vec2&), const Vec2& setPos)
{
	for (auto* u : units) {
		(u->*setter)(setPos);
	}
}
/// @brief 指定された陣形で移動可能なユニットを抽出します。
/// @param source ユニットの配列。各要素は ClassHorizontalUnit 型です。
/// @param bf 抽出対象となる陣形（BattleFormation 型）。
/// @return 指定された陣形で移動可能かつ戦闘可能なユニットのみを含む ClassHorizontalUnit オブジェクト。
Array<Unit*> Battle001::getMovableUnits(Array<ClassHorizontalUnit>& source, BattleFormation bf)
{
	Array<Unit*> result;
	std::shared_lock lock(aStar.unitListRWMutex);
	for (auto& target : source)
		for (auto& unit : target.ListClassUnit)
		{
			if (unit.Formation == bf
				&& unit.moveState == moveState::FlagMoveCalc
				&& unit.IsBattleEnable == true)
				result.push_back(&unit);
		}

	return result;
}

void Battle001::handleDenseFormation(Point end)
{
	std::shared_lock lock(aStar.unitListRWMutex);
	for (auto& target : classBattleManage.listOfAllUnit)
		for (auto& unit : target.ListClassUnit)
		{
			// 建物・非戦闘ユニットは除外
			if (!unit.IsBattleEnable || unit.IsBuilding)
				continue;

			// 選択（移動対象）フラグが立っているユニットのみ密集移動
			if (unit.moveState != moveState::FlagMoveCalc)
				continue;

			unit.orderPosiLeft =
				end.movedBy(
					Random(-RANDOM_MOVE_RANGE, RANDOM_MOVE_RANGE),
					Random(-RANDOM_MOVE_RANGE, RANDOM_MOVE_RANGE));
			unit.orderPosiLeftLast = unit.orderPosiLeft;
			unit.moveState = moveState::MoveAI;
		}
}
void Battle001::handleHorizontalFormation(Point start, Point end)
{
	std::shared_lock lock(aStar.unitListRWMutex);
	Array<Array<Unit*>> formationGroups;
	formationGroups.push_back(getMovableUnits(classBattleManage.listOfAllUnit, BattleFormation::F));
	formationGroups.push_back(getMovableUnits(classBattleManage.listOfAllUnit, BattleFormation::B));
	formationGroups.push_back(getMovableUnits(classBattleManage.listOfAllUnit, BattleFormation::M));

	for (auto&& [i, group] : Indexed(formationGroups))
	{
		if (group.isEmpty()) continue;

		AssignUnitsInFormation(group, start, end, i);

		if (group.size() > 1)
		{
			auto pos = calcLastMerge(group, [](const Unit* u) { return u->GetOrderPosiCenter(); });
			auto pos2 = calcLastMerge(group, [](const Unit* u) { return u->GetNowPosiCenter(); });
			setMergePos(group, &Unit::setFirstMergePos, pos2);
			setMergePos(group, &Unit::setLastMergePos, pos);
		}
	}
}
void Battle001::handleSquareFormation(Point start, Point end)
{
	Array<Unit*> target;
	auto groups = GetMovableUnitGroups();
	for (auto&& [i, group] : Indexed(groups))
	{
		AssignUnitsInFormation(group, start, end, i);
		target.append(group);
	}
	if (target.size() > 1)
	{
		auto pos = calcLastMerge(target, [](const Unit* u) { return u->GetOrderPosiCenter(); });
		auto pos2 = calcLastMerge(target, [](const Unit* u) { return u->GetNowPosiCenter(); });
		setMergePos(target, &Unit::setFirstMergePos, pos2);
		setMergePos(target, &Unit::setLastMergePos, pos);
	}
}
void Battle001::setUnitsSelectedInRect(const RectF& selectionRect)
{
	std::shared_lock lock(aStar.unitListRWMutex);
	for (auto& target : classBattleManage.listOfAllUnit)
	{
		for (auto& unit : target.ListClassUnit)
		{
			if (unit.IsBuilding) continue;

			const Vec2 gnpc = unit.GetNowPosiCenter();
			const bool inRect = selectionRect.intersects(gnpc);

			if (inRect)
			{
				unit.moveState = moveState::FlagMoveCalc;
				unit.IsSelect = true; // 追加: 視覚的にも選択状態に
				is移動指示 = true;
			}
		}
	}
}

void Battle001::issueMoveOrder(Point start, Point end)
{
	if (arrayBattleZinkei[FORMATION_DENSE密集] == true)
	{
		handleDenseFormation(end);
	}
	else if (arrayBattleZinkei[FORMATION_HORIZONTAL横列] == true)
	{
		handleHorizontalFormation(start, end);
	}
	else if (arrayBattleZinkei[FORMATION_SQUARE正方] == true)
	{
		handleSquareFormation(start, end);
	}
	else
	{
		handleSquareFormation(start, end);
	}

	is移動指示 = false;
	// 追加: 移動命令発行を A* に通知（新規ユニット/新規オーダを確実に拾わせる）
	aStar.changeUnitMember.store(true);
	aStar.abortAStarMyUnits = false;
}

Co::Task<void> Battle001::handleRightClickUnitActions(Point start, Point end)
{
	if (is移動指示)
	{
		issueMoveOrder(start, end);
	}
	else
	{
		const RectF selectionRect = RectF::FromPoints(start, end);
		setUnitsSelectedInRect(selectionRect);
	}

	co_return;
}
void Battle001::playResourceEffect()
{
	//CircleEffect(Vec2{ 500, 500 }, 50, ColorF{ 1.0, 1.0, 0.2 });
	//SoundAsset(U"resourceGet").playMulti();
}
void Battle001::updateResourceIncome()
{
	//stopwatchFinanceが一秒経過する度に処理を行う
	if (stopwatchFinance.sF() >= 1.0)
	{
		int32 goldInc = 0;
		int32 trustInc = 0;
		int32 foodInc = 0;

		// 💡 事前にリソースポイントのみをキャッシュ
		static Array<Point> resourcePoints;
		static bool resourcePointsCached = false;

		if (!resourcePointsCached)
		{
			for (int32 x = 0; x < classBattleManage.classMapBattle.value().mapData.size(); ++x)
			{
				for (int32 y = 0; y < classBattleManage.classMapBattle.value().mapData[x].size(); ++y)
				{
					if (classBattleManage.classMapBattle.value().mapData[x][y].isResourcePoint)
					{
						resourcePoints.push_back(Point(x, y));
					}
				}
			}
			resourcePointsCached = true;
		}

		// 💡 リソースポイントのみを走査
		for (const auto& point : resourcePoints)
		{
			const auto& tile = classBattleManage.classMapBattle.value().mapData[point.x][point.y];
			if (tile.whichIsThePlayer == BattleWhichIsThePlayer::Sortie)
			{
				switch (tile.resourcePointType)
				{
				case resourceKind::Gold:
					goldInc += tile.resourcePointAmount;
					break;
				case resourceKind::Trust:
					trustInc += tile.resourcePointAmount;
					break;
				case resourceKind::Food:
					foodInc += tile.resourcePointAmount;
					break;
				}
			}
		}

		stopwatchFinance.restart();
		gold += 10 + goldInc; // 1秒ごとに10ゴールド増加
		trust += 1 + trustInc; // 1秒ごとに1権勢増加
		food += 5 + foodInc; // 1秒ごとに5食料増加

		// 資源取得エフェクトやサウンド再生（省略可）
		playResourceEffect();
	}
}
/// @brief 指定された画像の支配的な色（最も頻度の高い色）を取得します。
/// @param imageName 色を取得する画像の名前。
/// @param data 画像名と色の対応を保持するハッシュテーブル。既に色が記録されていれば再計算せずに返します。
/// @return 画像内で最も頻度の高い色（支配的な色）。
Color Battle001::GetDominantColor(const String imageName, HashTable<String, Color>& data)
{
	// dataに指定された画像名が存在する場合はその色を返す
	if (data.contains(imageName))
		return data[imageName];

	const Image image{ PATHBASE + PATH_DEFAULT_GAME + U"/015_BattleMapCellImage/" + imageName };

	// 色の頻度を記録
	HashTable<Color, int32> colorCount;

	for (const auto& pixel : image)
	{
		// 透明ピクセルを除外したい場合は以下の if を有効に
		if (pixel.a == 0) continue;

		++colorCount[pixel];
	}

	// 最も頻度の高い色を見つける
	Color dominantColor = Palette::Black;
	int32 maxCount = 0;

	for (const auto& [color, count] : colorCount)
	{
		if (count > maxCount)
		{
			dominantColor = color;
			maxCount = count;
		}
	}

	//dataに追加
	data[imageName] = dominantColor;

	return dominantColor;
}
void Battle001::initFormationUI()
{
	rectZinkei.push_back(Rect{ 8,8,60,40 });
	rectZinkei.push_back(Rect{ 76,8,60,40 });
	rectZinkei.push_back(Rect{ 144,8,60,40 });
	renderTextureZinkei = RenderTexture{ 320,60 };
	renderTextureZinkei.clear(ColorF{ 0.5, 0.0 });
	{
		const ScopedRenderTarget2D target{ renderTextureZinkei.clear(ColorF{ 0.8, 0.8, 0.8,0.5 }) };
		const ScopedRenderStates2D blend{ MakeBlendState() };

		Rect background_rect = Rect(320, 60);
		background_rect.drawFrame(4, 0, ColorF{ 0.5 });

		for (auto&& [i, formation_rect] : Indexed(rectZinkei))
		{
			formation_rect.draw(Palette::Aliceblue);
			fontInfo.fontZinkei(ss.Zinkei[i]).draw(formation_rect, Palette::Black);
		}
	}
}

void Battle001::initSkillUI()
{
	renderTextureSkill = RenderTexture{ 320,320 };
	renderTextureSkill.clear(ColorF{ 0.5, 0.0 });
	{
		const ScopedRenderTarget2D target{ renderTextureSkill.clear(ColorF{ 0.8, 0.8, 0.8,0.5 }) };
		const ScopedRenderStates2D blend{ MakeBlendState() };

		// クリック判定テーブルを初期化（再初期化時のゴミ残り防止）
		htSkill.clear();

		//skill抽出
		Array<Skill> all_skills;
		{
			//std::shared_lock lock(aStar.unitListRWMutex);
			for (auto& unit_group : classBattleManage.listOfAllUnit)
			{
				for (auto& unit : unit_group.ListClassUnit)
				{
					for (auto& skill : unit.arrSkill)
						all_skills.push_back(skill);
				}
			}
		}

		// ソート（昇順）
		all_skills.sort_by([](const Skill& a, const Skill& b)
			{
				return a.sortKey < b.sortKey;
			});

		// nameTag でユニーク化（描画とクリック対象を一致させる）
		HashSet<String> seen;
		Array<Skill> unique_skills;
		unique_skills.reserve(all_skills.size());
		for (const auto& s : all_skills)
		{
			if (s.nameTag.isEmpty())
			{
				continue; // 無名はスキップ
			}
			if (!seen.contains(s.nameTag))
			{
				seen.insert(s.nameTag);
				unique_skills.push_back(s);
			}
		}

		for (const auto&& [i, skill] : Indexed(unique_skills))
		{
			Rect rectSkill;
			rectSkill.x = ((i % 10) * 32) + 4;
			rectSkill.y = ((i / 10) * 32) + 4;
			rectSkill.w = 32;
			rectSkill.h = 32;

			// クリック判定用に nameTag -> Rect を登録
			htSkill.emplace(skill.nameTag, rectSkill);

			// 複数アイコン対応（後ろの方が上に来る）
			for (auto& icon : skill.icon.reversed())
			{
				TextureAsset(icon.trimmed()).resized(32).draw(rectSkill.x, rectSkill.y);
			}
		}

		Rect background_rect = Rect(320, 320);
		background_rect.drawFrame(4, 0, ColorF{ 0.5 });
	}
	renderTextureSkillUP = RenderTexture{ 320,320 };
	renderTextureSkillUP.clear(ColorF{ 0.5, 0.0 });
}

void Battle001::initBuildMenu()
{
	renderTextureBuildMenuEmpty = RenderTexture{ 328, 328 };
	renderTextureBuildMenuEmpty.clear(ColorF{ 0.5, 0.0 });
	{
		const ScopedRenderTarget2D target{ renderTextureBuildMenuEmpty.clear(ColorF{ 0.8, 0.8, 0.8,0.5 }) };
		const ScopedRenderStates2D blend{ MakeBlendState() };
		Rect df = Rect(328, 328);
		df.drawFrame(4, 0, ColorF{ 0.5 });
	}

	//建築メニューの初期化
	{
		htBuildMenu.clear();
		for (const auto uigee : m_commonConfig.htBuildMenuBaseData)
		{
			for (const auto fege : uigee.second)
			{
				String key = uigee.first + U"-" + fege.id;
				htBuildMenu.emplace(key, fege);
			}
		}
		createRenderTex();
	}
}

void Battle001::initMinimap()
{
	//ミニマップ用
	for (int32 y = 0; y < visibilityMap.height(); ++y)
	{
		for (int32 x = 0; x < visibilityMap.width(); ++x)
		{
			String ttt = classBattleManage.classMapBattle.value().mapData[x][y].tip + U".png";
			minimapCols.emplace(Point(x, y), GetDominantColor(ttt, colData));
		}
	}
}


/// @brief Battle001のUIを初期化 陣形・スキル・建築メニュー・ミニマップなどの描画用リソースを生成・設定
void Battle001::initUI()
{
	initFormationUI();
	initSkillUI();
	initBuildMenu();
	initMinimap();
}

/// @brief UIエリアによる選択キャンセルをチェックし、必要に応じて選択状態を解除
/// @return 非同期タスク（Co::Task<>）を返します。UIエリアでキャンセル条件が満たされた場合、ユニットの選択状態を解除
Co::Task<> Battle001::checkCancelSelectionByUIArea()
{
	if (Cursor::PosF().y >= Scene::Size().y - underBarHeight)
	{
		longBuildSelectTragetId = -1;
		std::shared_lock lock(aStar.unitListRWMutex);
		for (auto& target : classBattleManage.listOfAllUnit)
		{
			for (auto& unit : target.ListClassUnit)
			{
				unit.IsSelect = false;
			}
		}
		for (const auto& group : { classBattleManage.hsMyUnitBuilding })
		{
			for (const auto& item : group)
			{
				item->IsSelect = false;
			}
		}
		co_await Co::NextFrame();
	}
	co_return;
}
void Battle001::processUnitBuildMenuSelection(Unit& itemUnit)
{
	// メニュー非表示時は無視（隠れたクリック誤爆防止）
	if (!IsBuildMenuHome) return;

	// “ビルド対象”に指定された 1 体のみ処理（多重キュー投入防止）
	if (longBuildSelectTragetId != -1
		&& itemUnit.ID != longBuildSelectTragetId) return;

	if (itemUnit.IsSelect == false) return;

	for (auto& hbm : sortedArrayBuildMenu)
	{
		Array<String> resSp = hbm.first.split('-');
		if (resSp[0] != itemUnit.classBuild) continue;

		if (hbm.second.rectHantei.leftClicked())
		{
			if (hbm.second.isMove == true)
			{
				IsBuildSelectTraget = true;
				itemUnit.tempIsBuildSelectTragetBuildAction = hbm.second;
				tempSelectComRight = hbm.second;
				itemUnit.tempSelectComRight = tempSelectComRight;
				return;
			}

			if (hbm.second.category == U"Carrier")
			{
				handleCarrierStoreCommand(itemUnit);
				continue;
			}

			if (hbm.second.category == U"releaseAll")
			{
				handleCarrierReleaseCommand(itemUnit);
				continue;
			}

			// 設置位置の取得
			if (const auto& index = mapTile.ToIndex(
				itemUnit.GetNowPosiCenter(), mapTile.columnQuads, mapTile.rowQuads))
			{
				hbm.second.rowBuildingTarget = index->y;
				hbm.second.colBuildingTarget = index->x;
				itemUnit.currentTask = UnitTask::None;
			}
			else
			{
				//現在選択ユニットはマップ外にいる……
			}

			IsBuildSelectTraget = false;

			//Battle::updateBuildQueueで作る
			if (itemUnit.taskTimer.isRunning() == false)
			{
				itemUnit.taskTimer.restart();
				itemUnit.progressTime = 0.0;
			}
			itemUnit.arrYoyakuBuild.push_back(hbm.second);
			// 回数制限の更新と再描画
			if (hbm.second.buildCount > 0)
			{
				hbm.second.buildCount--;
				//キーだけ渡して該当のrenderだけ更新するように
				//renB();
			}
		}
		else if (hbm.second.rectHantei.mouseOver())
		{
			nowSelectBuildSetumei = U"~~~Unit Or Build~~~\r\n" + hbm.second.description;
			rectSetumei = { Scene::Size().x - renderTextureBuildMenuEmpty.size().x,
				Scene::Size().y - underBarHeight - renderTextureBuildMenuEmpty.size().y,
				320, 0 };
			rectSetumei.h = fontInfo.fontSkill(nowSelectBuildSetumei).region().h;
			while (!fontInfo.fontSkill(nowSelectBuildSetumei).draw(rectSetumei.stretched(-12), Color(0.0, 0.0)))
			{
				rectSetumei.h += 12;
			}
			rectSetumei.y -= rectSetumei.h;
			break;
		}
		else
		{
			nowSelectBuildSetumei.clear();
		}

	}
	//if (itemUnit.IsSelect == false) return;

	//for (auto& hbm : sortedArrayBuildMenu)
	//{
	//	Array<String> resSp = hbm.first.split('-');
	//	if (resSp[0] != itemUnit.classBuild) continue;

	//	if (hbm.second.rectHantei.leftClicked())
	//	{
	//		if (hbm.second.isMove == true)
	//		{
	//			IsBuildSelectTraget = true;
	//			itemUnit.tempIsBuildSelectTragetBuildAction = hbm.second;
	//			tempSelectComRight = hbm.second;
	//			itemUnit.tempSelectComRight = tempSelectComRight;
	//			return;
	//		}

	//		if (hbm.second.category == U"Carrier")
	//		{
	//			handleCarrierStoreCommand(itemUnit);
	//			continue;
	//		}

	//		if (hbm.second.category == U"releaseAll")
	//		{
	//			handleCarrierReleaseCommand(itemUnit);
	//			continue;
	//		}

	//		// 設置位置の取得
	//		if (const auto& index = mapTile.ToIndex(
	//			itemUnit.GetNowPosiCenter(), mapTile.columnQuads, mapTile.rowQuads))
	//		{
	//			hbm.second.rowBuildingTarget = index->y;
	//			hbm.second.colBuildingTarget = index->x;
	//			itemUnit.currentTask = UnitTask::None;
	//		}
	//		else
	//		{
	//			//現在選択ユニットはマップ外にいる……
	//		}

	//		IsBuildSelectTraget = false;

	//		//Battle::updateBuildQueueで作る
	//		if (itemUnit.taskTimer.isRunning() == false)
	//		{
	//			itemUnit.taskTimer.restart();
	//			itemUnit.progressTime = 0.0;
	//		}
	//		itemUnit.arrYoyakuBuild.push_back(hbm.second);
	//		// 回数制限の更新と再描画
	//		if (hbm.second.buildCount > 0)
	//		{
	//			hbm.second.buildCount--;
	//			//キーだけ渡して該当のrenderだけ更新するように
	//			//renB();
	//		}
	//	}
	//	else if (hbm.second.rectHantei.mouseOver())
	//	{
	//		nowSelectBuildSetumei = U"~~~Unit Or Build~~~\r\n" + hbm.second.description;
	//		rectSetumei = { Scene::Size().x - renderTextureBuildMenuEmpty.size().x,
	//			Scene::Size().y - underBarHeight - renderTextureBuildMenuEmpty.size().y,
	//			320, 0 };
	//		rectSetumei.h = fontInfo.fontSkill(nowSelectBuildSetumei).region().h;
	//		while (!fontInfo.fontSkill(nowSelectBuildSetumei).draw(rectSetumei.stretched(-12), Color(0.0, 0.0)))
	//		{
	//			rectSetumei.h += 12;
	//		}
	//		rectSetumei.y -= rectSetumei.h;
	//		break;
	//	}
	//	else
	//	{
	//		nowSelectBuildSetumei.clear();
	//	}

	//}
}

void Battle001::handleCarrierStoreCommand(Unit& unit)
{
	// キャリアーコンポーネントを取得
	auto* carrierComponent = unit.getComponent<CarrierComponent>();
	if (!carrierComponent) return;

	// 選択ユニットの現在位置のタイル座標を取得
	Optional<Point> carrierTileIndex = mapTile.ToIndex(
		unit.GetNowPosiCenter(), mapTile.columnQuads, mapTile.rowQuads);
	if (!carrierTileIndex.has_value()) return;

	// 周囲3マス範囲のユニットを検索
	Array<Unit*> nearbyUnits;
	const int32 searchRadius = 3;

	// 味方ユニットから検索
	{
		std::shared_lock lock(aStar.unitListRWMutex);
		for (auto& group : classBattleManage.listOfAllUnit)
		{
			for (auto& otherUnit : group.ListClassUnit)
			{
				// 自分自身、建物、戦闘不可ユニット、他のキャリアは除外
				if (otherUnit.ID == unit.ID ||
					otherUnit.IsBuilding ||
					!otherUnit.IsBattleEnable || otherUnit.isCarrierUnit) continue;

				// ユニットの現在位置のタイル座標を取得
				Optional<Point> unitTileIndex = mapTile.ToIndex(
					otherUnit.GetNowPosiCenter(), mapTile.columnQuads, mapTile.rowQuads);
				if (!unitTileIndex.has_value()) continue;

				// 距離をチェック（マンハッタン距離）
				int32 distance = carrierTileIndex->manhattanDistanceFrom(*unitTileIndex);
				if (distance <= searchRadius)
				{
					nearbyUnits.push_back(&otherUnit);
				}
			}
		}
	}

	// 範囲内にユニットがいない場合は処理終了
	if (nearbyUnits.isEmpty())
	{
		Print << U"周囲3マス以内に格納可能なユニットが見つかりません";
		return;
	}

	// ランダムに並び替え
	Shuffle(nearbyUnits);

	// キャリアーの容量まで格納
	int32 storedCount = 0;
	for (Unit* unitToStore : nearbyUnits)
	{
		if (carrierComponent->store(unitToStore))
		{
			storedCount++;
			Print << U"ユニット '{}' を格納しました"_fmt(unitToStore->Name);

			// 容量に達したら終了
			if (carrierComponent->storedUnits.size() >= carrierComponent->capacity)
			{
				break;
			}
		}
		else
		{
			Print << U"キャリアーの容量が満杯です";
			break;
		}
	}

	if (storedCount > 0)
	{
		Print << U"合計 {} 体のユニットを格納しました"_fmt(storedCount);
	}
	else
	{
		Print << U"格納できるユニットがありませんでした";
	}
}

void Battle001::handleCarrierReleaseCommand(Unit& unit)
{
	// キャリアーコンポーネントを取得
	auto* carrierComponent = unit.getComponent<CarrierComponent>();
	if (!carrierComponent) return;

	// 格納されているユニットがあるかチェック
	if (carrierComponent->storedUnits.empty())
	{
		Print << U"格納されているユニットがありません";
		return;
	}

	// 現在のキャリアーユニットの位置を取得
	Vec2 releasePosition = unit.GetNowPosiCenter();

	// 格納されているユニット数を記録（リリース前）
	int32 releasedCount = static_cast<int32>(carrierComponent->storedUnits.size());

	// 全ユニットを解放
	carrierComponent->releaseAll(releasePosition);

	Print << U"合計 {} 体のユニットを解放しました"_fmt(releasedCount);
}

/// @brief 建築予約をするのが本質
void Battle001::handleBuildMenuSelectionA()
{
	// メニューを表示していないときはクリック処理を無効化
	if (!IsBuildMenuHome)
		return;

	const Transformer2D transformer{ Mat3x2::Identity(), Mat3x2::Translate(Scene::Size().x - 328, Scene::Size().y - 328 - 30) };

	// “ビルド対象”に指定された 1 体だけ処理する
	Unit* targetUnit = nullptr;

	// 通常ユニットの中から ID 一致を探す
	{
		for (auto& loau : classBattleManage.listOfAllUnit)
		{
			for (auto& itemUnit : loau.ListClassUnit)
			{
				if (itemUnit.ID == longBuildSelectTragetId)
				{
					targetUnit = &itemUnit;
					break;
				}
			}
			if (targetUnit) break;
		}
	}

	// 見つからなければ建物から探す
	if (!targetUnit)
	{
		for (const auto& group : { classBattleManage.hsMyUnitBuilding })
		{
			for (const auto& item : group)
			{
				if (item->ID == longBuildSelectTragetId)
				{
					targetUnit = item.get();
					break;
				}
			}
			if (targetUnit) break;
		}
	}

	// 対象が見つかった場合のみクリック処理実行
	if (targetUnit)
	{
		processUnitBuildMenuSelection(*targetUnit);
	}

	//// 通常ユニットの処理
	//{
	//	//std::shared_lock lock(aStar.unitListRWMutex);
	//	for (auto& loau : classBattleManage.listOfAllUnit)
	//	{
	//		for (auto& itemUnit : loau.ListClassUnit)
	//		{
	//			processUnitBuildMenuSelection(itemUnit);
	//		}
	//	}
	//}

	//// 建物ユニットの処理
	//Array<std::shared_ptr<Unit>> buildings;
	//for (const auto& group : { classBattleManage.hsMyUnitBuilding })
	//{
	//	for (const auto& item : group)
	//	{
	//		buildings.push_back(item);
	//	}
	//}

	//for (const auto& unitBuildings : buildings)
	//{
	//	processUnitBuildMenuSelection(*unitBuildings);
	//}
}

Optional<long long> Battle001::findClickedBuildingId() const
{
	Array<std::shared_ptr<Unit>> buildings;
	for (const auto& group : { classBattleManage.hsMyUnitBuilding })
	{
		for (const auto& item : group)
		{
			buildings.push_back(item);
		}
	}

	for (const auto& u : buildings)
	{
		Size tempSize = TextureAsset(u->ImageName).size();
		Quad tempQ = mapTile.ToTile(Point(u->colBuilding, u->rowBuilding), mapTile.N);
		// tempQをtempSizeに合わせてスケーリングするための基準値を計算


		// 横幅・縦幅（ToTile 基準）
		double baseWidth = Abs(tempQ.p1.x - tempQ.p3.x);//100
		double baseHeight = Abs(tempQ.p0.y - tempQ.p2.y) + mapTile.TileThickness;//65
		// スケール計算
		double scaleX = tempSize.x / (baseWidth);   // = 1.0
		double scaleY = tempSize.y / (baseHeight);  // = 1.0
		// Quad をスケーリングして移動
		Size posCenter = Size(static_cast<int32>(tempQ.p0.x), static_cast<int32>(tempQ.p1.y));
		auto tempQScaled = tempQ.scaledAt(posCenter, scaleX, scaleY);
		if (tempQScaled.intersects(Cursor::PosF()))
		{
			return u->ID;
		}
	}
	return none;
}

Optional<long long> Battle001::findClickedUnitId() const
{
	//std::shared_lock lock(aStar.unitListRWMutex);
	for (const auto& target : classBattleManage.listOfAllUnit)
	{
		for (const auto& unit : target.ListClassUnit)
		{
			if ((unit.IsBuilding == false && unit.IsBattleEnable == true)
				|| (unit.IsBuilding == true && unit.IsBattleEnable == true && unit.classBuild != U""))
			{
				if (unit.GetRectNowPosi().intersects(Cursor::PosF()))
				{
					return unit.ID;
				}
			}
		}
	}
	return none;
}


void Battle001::deselectAll()
{
	std::shared_lock lock(aStar.unitListRWMutex);
	for (auto& item : classBattleManage.listOfAllUnit)
	{
		for (auto& itemUnit : item.ListClassUnit)
		{
			itemUnit.IsSelect = false;
			// 追加: 範囲選択の残りを確実に解除
			if (itemUnit.moveState == moveState::FlagMoveCalc)
				itemUnit.moveState = moveState::None;
		}
	}
	for (const auto& group : { classBattleManage.hsMyUnitBuilding })
	{
		for (const auto& item : group)
		{
			item->IsSelect = false;
		}
	}
	IsBuildMenuHome = false;
	longBuildSelectTragetId = -1;
	// 追加: 移動指示状態も解除
	is移動指示 = false;
}

void Battle001::toggleUnitSelection(long long unit_id)
{
	// 全ての建物の選択を解除
	for (const auto& building : classBattleManage.hsMyUnitBuilding)
	{
		building->IsSelect = false;
	}

	// 選択状態の一括更新
	std::shared_lock lock(aStar.unitListRWMutex);
	for (auto& item : classBattleManage.listOfAllUnit)
	{
		for (auto& itemUnit : item.ListClassUnit)
		{
			bool newSelectState = false;

			if (unit_id == itemUnit.ID)
			{
				// クリックされたユニットの選択状態を反転
				newSelectState = !itemUnit.IsSelect;
				IsBuildMenuHome = newSelectState;

				if (newSelectState)
				{
					longBuildSelectTragetId = itemUnit.ID;
					// 追加: 左クリック選択でも移動対象になるようにフラグを立てる
					itemUnit.moveState = moveState::FlagMoveCalc;
					is移動指示 = true;
				}
				else
				{
					longBuildSelectTragetId = -1;
					// 追加: 解除時に FlagMoveCalc を戻す（選択解除の意図を反映）
					if (itemUnit.moveState == moveState::FlagMoveCalc)
						itemUnit.moveState = moveState::None;
				}
			}
			else
			{
				// その他のユニットは選択解除
				// 追加: 他ユニットの FlagMoveCalc も整理（誤爆防止）
				if (itemUnit.moveState == moveState::FlagMoveCalc)
					itemUnit.moveState = moveState::None;
				// その他のユニットは選択解除
				newSelectState = false;
			}

			// IsSelectの更新
			itemUnit.IsSelect = newSelectState;
		}
	}
}

void Battle001::toggleBuildingSelection(long long building_id)
{
	// 全てのユニットの選択を解除
	std::shared_lock lock(aStar.unitListRWMutex);
	for (auto& group : classBattleManage.listOfAllUnit)
	{
		for (auto& unit : group.ListClassUnit)
		{
			unit.IsSelect = false;
		}
	}

	for (const auto& group : { classBattleManage.hsMyUnitBuilding })
	{
		for (const auto& item : group)
		{
			if (item->ID == building_id)
			{
				item->IsSelect = true;
				IsBuildMenuHome = true;
				longBuildSelectTragetId = item->ID;
			}
			else
			{
				item->IsSelect = false;
			}
		}
	}
}

void Battle001::processClickSelection()
{
	const Optional<long long> clickedBuildingId = findClickedBuildingId();
	if (clickedBuildingId)
	{
		toggleBuildingSelection(*clickedBuildingId);
		return;
	}

	const Optional<long long> clickedUnitId = findClickedUnitId();
	if (clickedUnitId)
	{
		toggleUnitSelection(*clickedUnitId);
		return;
	}

	// 何もクリックされなかった場合
	deselectAll();
}

/// @brief ユニットおよび建築物の選択処理を管理する　マウスの左クリック操作に応じて、ユニットや建築物の選択・選択解除を行う
void Battle001::handleUnitAndBuildingSelection()
{
	// 左クリック開始時の処理
	if (MouseL.down())
	{
		isUnitSelectionPending = true;
		clickStartPos = Cursor::Pos(); // クリック開始位置を記録
		return;
	}

	// 左クリック終了時の処理
	if (MouseL.up() && isUnitSelectionPending)
	{
		const double dragDistance = clickStartPos.distanceFrom(Cursor::Pos());
		if (dragDistance < CLICK_THRESHOLD)
		{
			processClickSelection();
		}
		isUnitSelectionPending = false;
	}

	// pressed中でキャンセル条件があれば保留状態をリセット
	if (MouseL.pressed())
	{
		const double dragDistance = clickStartPos.distanceFrom(Cursor::Pos());
		if (dragDistance >= CLICK_THRESHOLD)
		{
			isUnitSelectionPending = false; // ドラッグ開始でキャンセル
		}
	}
}
/// @brief スキルUIの選択処理を行う
void Battle001::handleSkillUISelection()
{
	//skill選択処理
	{
		const Transformer2D transformer{ Mat3x2::Identity(), Mat3x2::Translate(0, Scene::Size().y - 320 - 30) };
		for (auto&& [i, re] : Indexed(htSkill))
		{
			if (re.second.leftClicked())
			{
				bool flgEr = false;
				for (auto it = nowSelectSkill.begin(); it != nowSelectSkill.end(); ++it)
				{
					if (it->contains(re.first))
					{
						nowSelectSkill.erase(it);
						flgEr = true;
						break;
					}
				}

				if (flgEr == false)
				{
					nowSelectSkill.push_back(re.first);
				}
			}
			if (re.second.mouseOver())
			{
				flagDisplaySkillSetumei = true;
				nowSelectSkillSetumei = U"";
				//スキル説明を書く
				std::shared_lock lock(aStar.unitListRWMutex);
				for (auto& item : classBattleManage.listOfAllUnit)
				{
					if (!item.FlagBuilding &&
						!item.ListClassUnit.empty())
						for (auto& itemUnit : item.ListClassUnit)
						{
							for (auto& itemSkill : itemUnit.arrSkill)
							{
								if (itemSkill.nameTag == re.first)
								{
									nowSelectSkillSetumei = itemSkill.name + U"\r\n"
										+ itemSkill.help + U"\r\n"
										+ systemString.SkillAttack + U":" + Format(itemSkill.str);
									;
									break;
								}
							}
							if (nowSelectSkillSetumei != U"")
								break;
						}
					if (nowSelectSkillSetumei != U"")
						break;
				}

				nowSelectSkillSetumei = U"~~~Skill~~~\r\n" + nowSelectSkillSetumei;

				while (not fontInfo.fontSkill(nowSelectSkillSetumei).draw(rectSkillSetumei.stretched(-12), Color(0.0, 0.0)))
				{
					rectSkillSetumei.h = rectSkillSetumei.h + 12;
				}
				rectSkillSetumei.x = re.second.pos.x + 32;
				rectSkillSetumei.y = Scene::Size().y - underBarHeight - rectSkillSetumei.h;
				break;
			}
			else
			{
				flagDisplaySkillSetumei = false;
				nowSelectSkillSetumei = U"";
			}
		}

		{
			const ScopedRenderTarget2D target{ renderTextureSkillUP.clear(ColorF{ 0.5, 0.0, 0.0, 0.0 }) };
			// 描画された最大のアルファ成分を保持するブレンドステート
			const ScopedRenderStates2D blend{ MakeBlendState() };
			for (auto&& [i, re] : Indexed(htSkill))
				for (auto it = nowSelectSkill.begin(); it != nowSelectSkill.end(); ++it)
					if (it->contains(re.first))
					{
						re.second.drawFrame(2, 0, Palette::Red);
						break;
					}
		}
	}
}
/// @brief ユニットの体力バーを現在の体力に基づいて更新
void Battle001::updateUnitHealthBars()
{
	constexpr Vec2 offset{ -32, +22 }; // = -64 / 2, +32 / 2 + 6

	auto updateBar = [&](Unit& unit)
		{
			double hpRatio = static_cast<double>(unit.Hp) / unit.HpMAX;
			unit.bLiquidBarBattle.update(hpRatio);
			unit.bLiquidBarBattle.ChangePoint(unit.GetNowPosiCenter() + offset);
		};

	{
		std::shared_lock lock(aStar.unitListRWMutex);
		for (auto& group : classBattleManage.listOfAllUnit)
		{
			if (group.FlagBuilding || group.ListClassUnit.empty())
				continue;

			for (auto& unit : group.ListClassUnit)
				updateBar(unit);
		}
	}

	for (auto& group : classBattleManage.listOfAllEnemyUnit)
	{
		if (group.FlagBuilding || group.ListClassUnit.empty())
			continue;

		for (auto& unit : group.ListClassUnit)
			updateBar(unit);
	}

	// 建物の HP バー（HPCastle）を更新して建物の真下に配置
	auto updateBuildingBar = [&](const std::shared_ptr<Unit>& sp)
		{
			if (!sp) return;
			Unit& b = *sp;

			// 比率（スポーン時の HPCastle を MaxHP とする）
			const double maxHP = (gBuildingMaxHP.contains(b.ID) ? gBuildingMaxHP[b.ID] : Max<double>(1.0, b.HPCastle));
			const double ratio = (maxHP > 0.0) ? Saturate(b.HPCastle / maxHP) : 0.0;

			// 位置：建物スプライトの底辺下に表示
			Vec2 bottom = mapTile.ToTileBottomCenter(Point(b.colBuilding, b.rowBuilding), mapTile.N)
				.movedBy(0, -mapTile.TileThickness);
			Vec2 barTopLeft = bottom.movedBy(-(LIQUID_BAR_WIDTH / 2.0), 6.0);

			b.bLiquidBarBattle.update(ratio);
			b.bLiquidBarBattle.ChangePoint(barTopLeft);
		};

	for (const auto& sp : classBattleManage.hsMyUnitBuilding)
	{
		updateBuildingBar(sp);
	}
	for (const auto& sp : classBattleManage.hsEnemyUnitBuilding)
	{
		updateBuildingBar(sp);
	}
}

/// @brief 移動処理更新、移動状態や目的地到達を管理　A*経路探索の結果に基づき、位置や移動ベクトルを計算・更新
void Battle001::handleCompletedPlayerPath(Unit& unit, ClassUnitMovePlan& plan)
{
	if (unit.moveState == moveState::MovingEnd)
	{
		unit.moveState = moveState::None; // End of movement
		return;
	}

	if (unit.moveState == moveState::Moving)
	{
		// Final move adjustment
		unit.nowPosiLeft += unit.vecMove * ((unit.Move) / 100.0);

		Vec2 toTarget = unit.GetOrderPosiCenter() - unit.GetNowPosiCenter();
		if (toTarget.dot(unit.vecMove) <= 0 || toTarget.length() < 5.0)
		{
			unit.nowPosiLeft = unit.orderPosiLeft; // Snap to final position
			unit.FlagReachedDestination = true;
			unit.moveState = moveState::MovingEnd;
		}
	}
}

void Battle001::handlePlayerPathMovement(Unit& unit, ClassUnitMovePlan& plan)
{
	unit.nowPosiLeft += unit.vecMove * ((unit.Move) / 100.0);

	bool waypointReached = false;
	if (auto currentTarget = plan.getCurrentTarget())
	{
		Optional<Size> nowIndex = mapTile.ToIndex(unit.GetNowPosiCenter(), mapTile.columnQuads, mapTile.rowQuads);
		if (nowIndex.has_value() && (plan.lastPoint.x != nowIndex->x || plan.lastPoint.y != nowIndex->y))
		{
			plan.lastPoint = nowIndex.value();
			waypointReached = true;
		}
	}

	if (waypointReached)
	{
		plan.stepToNext();
		if (plan.isPathCompleted())
		{
			// Set final destination
			unit.orderPosiLeft = unit.orderPosiLeftLast;
			Vec2 moveVec = unit.GetOrderPosiCenter() - unit.GetNowPosiCenter();
			unit.vecMove = moveVec.isZero() ? Vec2::Zero() : moveVec.normalized();
		}
		else if (plan.getCurrentTarget())
		{
			// Set next waypoint destination
			startPlayerPathMovement(unit, plan); // Re-use start logic for next step
		}
	}
}

void Battle001::startPlayerPathMovement(Unit& unit, ClassUnitMovePlan& plan)
{
	if (auto targetTile = plan.getCurrentTarget())
	{
		plan.lastPoint = targetTile.value();

		// This coordinate calculation is complex and appears duplicated.
		// For now, we keep it as is, but it's a candidate for a future helper function.
		const int32 i = targetTile->manhattanLength();
		const int32 xi = (i < (mapTile.N - 1)) ? 0 : (i - (mapTile.N - 1));
		const int32 yi = (i < (mapTile.N - 1)) ? i : (mapTile.N - 1);
		const int32 k2 = (targetTile->manhattanDistanceFrom(Point{ xi, yi }) / 2);
		const double posX = ((i < (mapTile.N - 1)) ? (i * -mapTile.TileOffset.x) : ((i - 2 * mapTile.N + 2) * mapTile.TileOffset.x));
		const double posY = (i * mapTile.TileOffset.y) - mapTile.TileThickness;
		const Vec2 pos = { (posX + mapTile.TileOffset.x * 2 * k2) - (unit.yokoUnit / 2), posY - unit.TakasaUnit - 15 };

		unit.orderPosiLeft = Vec2(Math::Round(pos.x), Math::Round(pos.y));

		Vec2 moveVec = unit.GetOrderPosiCenter() - unit.GetNowPosiCenter();
		unit.vecMove = moveVec.isZero() ? Vec2::Zero() : moveVec.normalized();
		unit.moveState = moveState::Moving;
	}
}


void Battle001::updatePlayerUnitMovements()
{
	for (auto& item : classBattleManage.listOfAllUnit)
	{
		for (auto& unit : item.ListClassUnit)
		{
			if (unit.IsBuilding || !unit.IsBattleEnable ||
				(unit.moveState != moveState::Moving
					&& unit.moveState != moveState::MovingEnd
					&& unit.moveState != moveState::None
					&& unit.moveState != moveState::MoveAI))
			{
				continue;
			}

			std::scoped_lock lock(aStar.aiRootMutex);
			// 追加: 経路が未生成ならトリガを再送して様子見（フレームまたぎで生成される）
			if (!aiRootMy.contains(unit.ID))
			{
				if (unit.moveState == moveState::MoveAI)
				{
					aStar.changeUnitMember.store(true);
				}
				continue;
			}

			auto& plan = aiRootMy.at(unit.ID);

			//A案
			if (plan.isPathCompleted())
			{
				handleCompletedPlayerPath(unit, plan);
			}
			else
			{
				if (unit.moveState == moveState::Moving)
				{
					handlePlayerPathMovement(unit, plan);
				}
				else // Unit is ready to start a new path
				{
					startPlayerPathMovement(unit, plan);
					plan.stepToNext(); // Move to the first actual point in the path
					startPlayerPathMovement(unit, plan);
				}
			}
		}
	}
}

void Battle001::handleEnemyPathMovement(Unit& unit, ClassUnitMovePlan& plan)
{
	unit.nowPosiLeft += unit.vecMove * ((unit.Move + unit.cts.Speed) / 100.0);

	if (plan.getCurrentTarget())
	{
		if (unit.GetNowPosiCenter().distanceFrom(unit.GetOrderPosiCenter()) <= 3.0)
		{
			plan.stepToNext();
			if (plan.isPathCompleted())
			{
				unit.FlagReachedDestination = true;
				unit.moveState = moveState::MoveAI;
			}
			else
			{
				startEnemyPathMovement(unit, plan); // Set next waypoint
			}
		}
	}
}

void Battle001::startEnemyPathMovement(Unit& unit, ClassUnitMovePlan& plan)
{
	if (auto targetTile = plan.getCurrentTarget())
	{
		const int32 i = targetTile->manhattanLength();
		const int32 xi = (i < (mapTile.N - 1)) ? 0 : (i - (mapTile.N - 1));
		const int32 yi = (i < (mapTile.N - 1)) ? i : (mapTile.N - 1);
		const int32 k2 = (targetTile->manhattanDistanceFrom(Point{ xi, yi }) / 2);
		const double posX = ((i < (mapTile.N - 1)) ? (i * -mapTile.TileOffset.x) : ((i - 2 * mapTile.N + 2) * mapTile.TileOffset.x));
		const double posY = (i * mapTile.TileOffset.y) - mapTile.TileThickness;
		const Vec2 pos = { (posX + mapTile.TileOffset.x * 2 * k2) - (unit.yokoUnit / 2), posY - unit.TakasaUnit - 15 };
		unit.orderPosiLeft = Vec2(Math::Round(pos.x), Math::Round(pos.y));

		Vec2 moveVec = unit.GetOrderPosiCenter() - unit.GetNowPosiCenter();
		unit.vecMove = moveVec.isZero() ? Vec2::Zero() : moveVec.normalized();
		unit.moveState = moveState::Moving;
	}
}


void Battle001::updateEnemyUnitMovements()
{
	for (auto& item : classBattleManage.listOfAllEnemyUnit)
	{
		for (auto& unit : item.ListClassUnit)
		{
			if (unit.IsBuilding || !unit.IsBattleEnable)
			{
				continue;
			}

			std::scoped_lock lock(aStar.aiRootMutex);
			if (!aiRootEnemy.contains(unit.ID)) continue;

			auto& plan = aiRootEnemy.at(unit.ID);

			if (plan.isPathCompleted())
			{
				unit.FlagReachedDestination = true;
				unit.moveState = moveState::MoveAI;
				continue;
			}

			if (unit.moveState == moveState::Moving)
			{
				handleEnemyPathMovement(unit, plan);
			}
			else if (unit.moveState == moveState::None || unit.moveState == moveState::MoveAI)
			{
				startEnemyPathMovement(unit, plan);
			}
		}
	}
}

void Battle001::updateUnitMovements()
{
	updatePlayerUnitMovements();
	updateEnemyUnitMovements();
}

// ユニット情報ツールチップのハンドリング
void Battle001::handleUnitTooltip()
{
	if (!KeyControl.pressed())
	{
		unitTooltip.hide();
		return;
	}

	Vec2 tooltipPos = Cursor::PosF().movedBy(20, -10);
	bool foundUnit = false;
	String currentInfo; // 現在のユニット情報を保存
	{
		const auto t = camera.createTransformer();

		// マウス位置のユニットを検索
		std::shared_lock lock(aStar.unitListRWMutex);
		for (auto& group : { classBattleManage.listOfAllUnit, classBattleManage.listOfAllEnemyUnit })
		{
			for (auto& unitGroup : group)
			{
				if (unitGroup.FlagBuilding || unitGroup.ListClassUnit.empty())
					continue;

				for (const auto& unit : unitGroup.ListClassUnit)
				{
					if (!unit.IsBattleEnable) continue;

					if (unit.GetRectNowPosi().intersects(Cursor::PosF()))
					{
						foundUnit = true;

						// ユニット情報文字列の生成
						String info = U"【{}】\n"_fmt(unit.Name);
						info += U"ID: {}\n"_fmt(unit.ID);
						info += U"HP: {}/{}\n"_fmt(unit.Hp, unit.HpMAX);
						info += U"攻撃力: {}\n"_fmt(unit.Attack);
						info += U"防御力: {}\n"_fmt(unit.Defense);
						info += U"移動力: {}\n"_fmt(unit.Move);
						//info += U"射程: {}\n"_fmt(unit.Reach);

						if (!unit.IsBuilding)
						{
							info += U"陣形: {}\n"_fmt(
								unit.Formation == BattleFormation::F ? U"前衛" :
								unit.Formation == BattleFormation::M ? U"中衛" : U"後衛"
							);

							info += U"状態: {}\n"_fmt(
								unit.moveState == moveState::None ? U"待機" :
								unit.moveState == moveState::Moving ? U"移動中" :
								unit.moveState == moveState::FlagMoveCalc ? U"移動準備" :
								unit.moveState == moveState::MoveAI ? U"AI行動" : U"その他"
							);
						}
						else
						{
							info += U"建築物\n";
							if (!unit.arrYoyakuBuild.isEmpty())
							{
								info += U"建築キュー: {}\n"_fmt(unit.arrYoyakuBuild.size());
							}
						}

						// スキル情報
						if (!unit.arrSkill.isEmpty())
						{
							info += U"\n【スキル】\n";
							for (const auto& skill : unit.arrSkill)
							{
								info += U"・{}\n"_fmt(skill.name);
							}
						}

						// コンポーネント情報（キャリアーなど）
						if (auto* carrierComponent = const_cast<Unit&>(unit).getComponent<CarrierComponent>())
						{
							info += U"\n【キャリアー】\n";
							info += U"積載: {}/{}\n"_fmt(
								carrierComponent->storedUnits.size(),
								carrierComponent->capacity
							);
						}

						currentInfo = info;
						break;
					}
				}
				if (foundUnit) break;
			}
			if (foundUnit) break;
		}
	}

	if (foundUnit)
	{
		// 情報が変わった場合のみshow()を呼び出し
		unitTooltip.show(tooltipPos, currentInfo);
	}
	else
	{
		unitTooltip.hide();
	}
}

static Vec2 ConvertVecX(double rad, double x, double y, double charX, double charY)
{
	//キャラクタを基に回転させないと、バグる

	double x2 = 0;
	double y2 = 0;
	x2 = ((x - charX) * s3d::Math::Cos(rad)) - ((y - charY) * s3d::Math::Sin(rad));
	y2 = ((x - charX) * s3d::Math::Sin(rad)) + ((y - charY) * s3d::Math::Cos(rad));
	return Vec2(x2 + charX, y2 + charY);
}
static bool NearlyEqual(double a, double b)
{
	return abs(a - b) < DBL_EPSILON;
}

Unit* Battle001::selectSwingTarget(Unit& attacker, Array<ClassHorizontalUnit>& targetGroups, const Skill& skill)
{
	Unit* best = nullptr;
	double bestDistSq = DBL_MAX;
	const Vec2 center = attacker.GetNowPosiCenter();

	for (auto& g : targetGroups) {
		for (auto& t : g.ListClassUnit) {
			if (!t.IsBattleEnable) continue;
			if (t.IsBuilding && t.mapTipObjectType == MapTipObjectType::WALL2) continue;
			const double d2 = center.distanceFromSq(t.GetNowPosiCenter());
			// 到達距離判定（reachDist or skill.range を距離として使う）
			const double reach = (skill.MoveType == MoveType::swing)
				? (skill.reachDist > 0 ? skill.reachDist : 80.0)
				: skill.range;
			const double maxDist = (attacker.yokoUnit * 0.5) + reach + (t.yokoUnit * 0.5);
			if (d2 <= maxDist * maxDist) {
				if (d2 < bestDistSq) {
					bestDistSq = d2;
					best = &t;
				}
			}
		}
	}
	return best;
}

void Battle001::findAndExecuteSkillForUnit(Unit& unit, Array<ClassHorizontalUnit>& target_groups, Array<ClassExecuteSkills>& executed_skills)
{
	// 発動中もしくは死亡ユニットはスキップ
	if (unit.FlagMovingSkill == true || unit.IsBattleEnable == false)
		return;

	// 追加: 現在使用可能な最優先スキルに基づき維持距離を再設定（射撃→近接の自動切替）
	applyMaintainRangeByPreferredAvailableSkill(unit);

	// どのスキルを試行するか決定する
	Array<Skill> skills_to_try;
	auto manually_selected_skills = unit.arrSkill.filter([&](const Skill& itemSkill)
		{
			return nowSelectSkill.contains(itemSkill.nameTag);
		}
	);

	const bool isManual = (manually_selected_skills.size() > 0);
	skills_to_try = isManual
		? manually_selected_skills
		: unit.arrSkill.sorted_by([](const Skill& a, const Skill& b) { return a.sortKey < b.sortKey; });

	// 追加: 自動時のみ「最優先が残弾ありCD中なら待機」を判定
	if (!isManual)
	{
		// sortKey 最小の戦闘スキル＝最優先
		Array<Skill> sorted = unit.arrSkill.sorted_by([](const Skill& a, const Skill& b) {
			return a.sortKey < b.sortKey;
		});
		auto itTop = std::find_if(sorted.begin(), sorted.end(), [](const Skill& s) { return isCombatSkill(s); });
		if (itTop != sorted.end())
		{
			const Skill& top = *itTop;
			// 残弾あり＆CD中
			if (hasUsesLeft(unit, top) && !isSkillReady(unit, top))
			{
				// 短いCDなら待機（＝発動試行せず復帰を待つ）
				const double remain = getRemainingCooldownSec(unit, top);
				if (remain <= kFallbackMeleeAfterCDSec)
				{
					// MaintainRange は applyMaintainRangeByPreferredAvailableSkill() が
					// 「CD中は hasUsesLeft の第2パス」で最優先スキルの値を適用済み
					return;
				}
				// 長いCDなら以下でフォールバック（近接など）を試す
			}
		}
	}

	// =========== 追加: swing 専用即時処理 ===========
	auto executeSwing = [&](Skill& skill) {
		// クールダウン / 使用回数チェック
		if (!hasUsesLeft(unit, skill) || !isSkillReady(unit, skill))
			return false;

		const Vec2 center = unit.GetNowPosiCenter();
		const double radius = Max<double>(skill.range, skill.w); // 半径基準
		// 対象グループ（敵側前提）
		for (auto& tg : target_groups) {
			for (auto& target : tg.ListClassUnit) {
				if (!target.IsBattleEnable) continue;
				if (target.IsBuilding && target.mapTipObjectType == MapTipObjectType::WALL2) continue;
				const double distSq = center.distanceFromSq(target.GetNowPosiCenter());
				if (distSq <= radius * radius) {
					// 1 体ずつ直接ダメージ適用（CalucDamage 利用）
					ClassExecuteSkills fake;
					fake.classSkill = skill;
					fake.classUnit = &unit;
					CalucDamage(target, skill.str, fake);
				}
			}
		}
		commitCooldown(unit, skill);
		consumeUse(unit, skill);
		applyMaintainRangeByPreferredAvailableSkill(unit);
		unit.FlagMovingSkill = false;
		return true;
		};
	// ================================================

	// スキルを試行
	for (size_t i = 0; i < skills_to_try.size(); ++i)
	{
		auto& skill = skills_to_try[i];

		//// swing は飛翔体を生成せず、ここで個別 AoE を適用
		//if (skill.MoveType == MoveType::swing) {
		//	if (executeSwing(skill)) return;
		//	continue;
		//}

		if (!hasUsesLeft(unit, skill)) {
			continue;
		}

		// クールダウン中の扱い
		if (!isSkillReady(unit, skill)) {
			//// 自動運用かつ「最優先スキル」なら、以降のスキルを試さず待機
			//if (!isManual && i == 0) {
			//	return;
			//}
			continue;
		}

		const auto attacker_pos = unit.GetNowPosiCenter();

		// ターゲットグループを「参照」で決定（コピーしない）
		Array<ClassHorizontalUnit>* potential_targets =
			(skill.SkillType == SkillType::heal)
			? &classBattleManage.listOfAllUnit   // 味方全体（コピーなし）
			: &target_groups;                     // 敵グループ（参照）

		// シャッフルによる全体コピー・再配置はコストが大きいため撤廃
		// 必要なら軽量にランダムサンプリングする実装へ置き換える

		if (tryActivateSkillOnTargetGroup(*potential_targets, attacker_pos, unit, skill, executed_skills))
		{
			// スキルが発動したら、このユニットは一旦終了
			return;
		}
	}
}

void Battle001::SkillProcess(Array<ClassHorizontalUnit>& attacker_groups, Array<ClassHorizontalUnit>& target_groups, Array<ClassExecuteSkills>& executed_skills)
{
	std::shared_lock lock(aStar.unitListRWMutex);
	for (auto& unit_group : attacker_groups)
	{
		for (auto& unit : unit_group.ListClassUnit)
		{
			findAndExecuteSkillForUnit(unit, target_groups, executed_skills);
		}
	}
}

bool Battle001::isTargetInRange(const Unit& attacker, const Unit& target, const Skill& skill) const
{
	const Vec2 attacker_pos = attacker.GetNowPosiCenter();
	const Vec2 target_pos = target.GetNowPosiCenter();
	const double distance_sq = attacker_pos.distanceFromSq(target_pos);

	const double range_with_radii = (attacker.yokoUnit / 2.0) + skill.range + (target.yokoUnit / 2.0);
	return distance_sq <= (range_with_radii * range_with_radii);
}

ClassExecuteSkills Battle001::createSkillExecution(Unit& attacker, const Unit& target, const Skill& skill)
{
	attacker.FlagMovingSkill = true;

	const int32 rush_count = (skill.rush > 1) ? skill.rush : 1;
	const int32 single_attack_number = classBattleManage.getBattleIDCount();

	ClassExecuteSkills executed_skill;
	executed_skill.No = classBattleManage.getDeleteCESIDCount();
	executed_skill.UnitID = attacker.ID;
	executed_skill.classSkill = skill;
	executed_skill.classUnit = &attacker;
	executed_skill.classUnitHealTarget = const_cast<Unit*>(&target); // Original code did this, needs review

	if (skill.MoveType == MoveType::swing) {
		ClassBullets b;
		b.No = classBattleManage.getBattleIDCount();
		b.RushNo = 0;
		b.StartPosition = attacker.GetNowPosiCenter();
		b.NowPosition = b.StartPosition;
		const Vec2 v = (target.GetNowPosiCenter() - b.StartPosition);
		double baseDeg = ToDegrees(Math::Atan2(v.y, v.x));
		// startDegreeType==6 で左右反転
		double offset = skill.startDegree;
		if (skill.startDegreeType == 6) {
			bool right = (v.x >= 0);
			offset = right ? Abs(offset) : -Abs(offset);
		}
		double startDeg = baseDeg + offset;
		b.initDegree = static_cast<float>(startDeg);
		b.degree = b.initDegree;
		b.radian = Math::ToRadians(startDeg);

		// 角速度 (deg/sec)
		double degPerSec = Max(1.0, skill.speed * 0.6); // 60fps換算調整
		double arc = (skill.arcDeg > 0 ? skill.arcDeg : skill.range); // arcDeg 未使用なら range
		b.lifeTime = 0.0;
		b.duration = Max(0.05, arc / degPerSec);
		executed_skill.ArrayClassBullet << b;
		return executed_skill;
	}

	//ここから従来の line/throw 等
	for (int i = 0; i < rush_count; ++i)
	{
		ClassBullets bullet;
		bullet.No = single_attack_number;
		bullet.RushNo = i;
		bullet.StartPosition = attacker.GetNowPosiCenter();
		bullet.NowPosition = bullet.StartPosition;

		if (skill.rushRandomDegree > 1)
		{
			const double rad = Random(-skill.rushRandomDegree, skill.rushRandomDegree) * Math::Pi / 180.0;
			bullet.OrderPosition = ConvertVecX(rad, target.GetNowPosiCenter().x, target.GetNowPosiCenter().y, bullet.NowPosition.x, bullet.NowPosition.y);
		}
		else
		{
			bullet.OrderPosition = target.GetNowPosiCenter();
		}

		// 変更: throw は「距離 / 速度」で飛翔時間を決める
		if (skill.MoveType == MoveType::thr)
		{
			const double dist = (bullet.OrderPosition - bullet.StartPosition).length();
			bullet.duration = (skill.speed > 0) ? (dist / skill.speed) : 2.5;
		}
		else
		{
			// 既存の計算を維持
			bullet.duration = (skill.speed == 0) ? 2.5 : ((skill.range + skill.speed - 1) / skill.speed);
		}
		bullet.lifeTime = 0;

		const Vec2 move_vec = bullet.OrderPosition - bullet.NowPosition;
		bullet.MoveVec = move_vec.isZero() ? Vec2::Zero() : move_vec.normalized();
		bullet.radian = Math::Atan2(move_vec.y, move_vec.x);
		bullet.degree = ToDegrees(bullet.radian);
		bullet.initDegree = bullet.degree;

		executed_skill.ArrayClassBullet.push_back(bullet);
	}

	//攻撃を伴う支援技か？
	if (skill.SkillType == SkillType::missileAdd)
	{
		attacker.cts.Speed = skill.plus_speed;
		for (auto& bullet : executed_skill.ArrayClassBullet)
		{
			bullet.duration = skill.plus_speed_time;
		}
	}

	return executed_skill;
}

bool Battle001::tryActivateSkillOnTargetGroup(Array<ClassHorizontalUnit>& target_groups, const Vec2& attacker_pos, Unit& attacker, Skill& skill, Array<ClassExecuteSkills>& executed_skills)
{
	if (skill.MoveType == MoveType::swing) {
		Unit* target = selectSwingTarget(attacker, target_groups, skill);
		if (!target) return false;
		ClassExecuteSkills ex = createSkillExecution(attacker, *target, skill);
		executed_skills.push_back(ex);
		commitCooldown(attacker, skill);
		consumeUse(attacker, skill);
		applyMaintainRangeByPreferredAvailableSkill(attacker);
		return true;
	}

	const bool isHeal = (skill.SkillType == SkillType::heal);

	for (auto& target_group : target_groups)
	{
		for (auto& target_unit : target_group.ListClassUnit)
		{
			//スキル発動条件確認
			if (not target_unit.IsBattleEnable) continue;
			if (target_unit.IsBuilding)
			{
				if (target_unit.mapTipObjectType == MapTipObjectType::WALL2) continue;
				if (target_unit.mapTipObjectType == MapTipObjectType::GATE && target_unit.HPCastle <= 0) continue;
			}

			// 回復スキルは満タン対象をスキップ（弾生成・判定を抑制）
			if (isHeal)
			{
				if (target_unit.IsBuilding)
				{
					// 建物の上限が未管理なら下限のみでもよいが、少なくとも 0 以下は除外
					if (target_unit.HPCastle <= 0) continue;
				}
				else
				{
					if (target_unit.Hp >= target_unit.HpMAX) continue;
				}
			}

			if (isTargetInRange(attacker, target_unit, skill))
			{
				ClassExecuteSkills new_skill_execution = createSkillExecution(attacker, target_unit, skill);
				executed_skills.push_back(new_skill_execution);
				commitCooldown(attacker, skill);
				consumeUse(attacker, skill);
				// 変更点: 「発動スキル」ではなく「現時点で最優先の使用可能スキル」に合わせて維持距離を更新
				applyMaintainRangeByPreferredAvailableSkill(attacker);

				return true; // スキル発動成功
			}
		}
	}
	return false; // 適切なターゲットが見つからなかった
}

/// @brief 弾（スキル弾道）の当たり判定とヒット処理を行う
/// @param bullet        現在処理中の弾。位置や移動ベクトルなどを保持（in/out）
/// @param executedSkill 弾を生成したスキル実行情報。スキル設定（サイズ、速度、貫通可否など）を参照（in）
/// @param targetUnits   当たり判定対象のユニット群（敵または味方のグループ一覧）（in）
/// @param isBomb        ヒット後に「非貫通（1体ヒットで終了）」なら true を返すフラグ（out）
///
/// 処理の流れ
/// 1) 弾の当たり判定円を作成
///    - 中心: bullet.NowPosition
///    - 半径: executedSkill.classSkill.w / 2.0（スキルで定義された見た目幅をヒット半径として利用）
///
/// 2) 対象ユニット群（targetUnits）を走査し、各ユニットに対して当たり判定
///    - 非戦闘状態（!IsBattleEnable）のユニットはスキップ
///    - 建物のうち壁（MapTipObjectType::WALL2）はスキップ（その他の建物は対象）
///    - 衝突判定用ターゲット中心:
///      ・通常ユニット: Unit::GetNowPosiCenter()
///      ・建物: mapTile.ToTileBottomCenter(タイル座標, mapTile.N) からタイル厚み分だけ上方向補正（-(25 + 15)）
///    - ターゲットの当たり判定円:
///      ・中心: 上記 targetPos
///      ・半径: targetUnit.yokoUnit / 2.0（見た目横幅の半分をヒット半径として使用）
///
/// 3) 弾円とターゲット円が交差したらヒット処理
///    - applySkillEffectAndRegisterHit(...) を呼び出し、ダメージ/回復などスキル効果を適用
///      ・術者の FlagMovingSkill を解除
///      ・ダメージ/回復計算を実施（CalucDamage）
///      ・「非貫通（SkillBomb::off）」のスキルであれば bombCheck を true に設定
///    - 本メソッド内の bulletsToRemove は互換用のダミー（上位で実弾削除を行うため、ここでは使用しない）
///
/// 4) 非貫通スキルなら以降のチェックを打ち切り
///    - isBomb が true になった時点で
///      ・同一グループ内のループを抜ける
///      ・他グループの判定も打ち切る（最初の被弾で終了）
///
/// 注意点
/// - 実際の弾の削除は上位の updateAndCheckCollisions() 側で、isBomb を見て行われる設計
/// - 建物の当たり判定は yokoUnit/2.0 を共用。必要なら建物専用の半径や形状に差し替え可能
/// - 壁(WALL2)はスキップする一方、他の建物は当たり判定対象（拠点などを撃てる）
///
/// 設計意図
/// - 見た目に近い円当たり判定で軽量に衝突判定を行う
/// - 最初の被弾で止まる（非貫通）/貫通する（複数ヒット）を isBomb で上位へ伝搬
/// - ヒット処理の副作用（ダメージ、状態更新）は applySkillEffectAndRegisterHit に委譲
void Battle001::handleBulletCollision(ClassBullets& bullet, ClassExecuteSkills& executedSkill, Array<ClassHorizontalUnit>& targetUnits, bool& isBomb)
{
	Circle bulletHitbox{ bullet.NowPosition, executedSkill.classSkill.w / 2.0 };

	if (executedSkill.classSkill.MoveType == MoveType::swing)
	{
		// 画像の「末端（先端）」が術者中心から伸びる想定
		const double reach = Max(0.0, static_cast<double>(executedSkill.classSkill.h)) * 0.5; // だいたい画像高さの半分
		const Vec2 tip = bullet.NowPosition + Vec2(Math::Cos(bullet.radian), Math::Sin(bullet.radian)) * reach;
		const double radius = Max(4.0, static_cast<double>(executedSkill.classSkill.w) * 0.5); // 幅をヒット半径の目安に
		bulletHitbox = Circle{ tip, radius };
	}
	else
	{
		bulletHitbox = Circle{ bullet.NowPosition, executedSkill.classSkill.w / 2.0 };
	}

	const uint64 bkey = makeBulletKey(bullet);
	auto& swingHitSet = gSwingHitOnce[bkey]; // 空なら自動生成

	for (auto& targetGroup : targetUnits)
	{
		for (auto& targetUnit : targetGroup.ListClassUnit)
		{
			if (!targetUnit.IsBattleEnable) continue;
			if (targetUnit.IsBuilding && targetUnit.mapTipObjectType == MapTipObjectType::WALL2) continue;

			Vec2 targetPos = targetUnit.GetNowPosiCenter();
			if (targetUnit.IsBuilding)
			{
				Point pt = Point(targetUnit.rowBuilding, targetUnit.colBuilding);
				targetPos = mapTile.ToTileBottomCenter(pt, mapTile.N).movedBy(0, -(25 + 15));
			}

			Circle targetHitbox{ targetPos, targetUnit.yokoUnit / 2.0 }; // Use unit size for hitbox

			if (bulletHitbox.intersects(targetHitbox))
			{
				// 追加: swing は同一ターゲットに一度だけ
				if (executedSkill.classSkill.MoveType == MoveType::swing) {
					if (swingHitSet.contains(targetUnit.ID)) {
						continue;
					}
				}

				Array<int32> bulletsToRemove; // Legacy parameter, might be removable later
				const bool hit = applySkillEffectAndRegisterHit(isBomb, bulletsToRemove, bullet, executedSkill, targetUnit);

				if (executedSkill.classSkill.MoveType == MoveType::swing) {
					// 記録して次へ（弾は消さない）
					swingHitSet.insert(targetUnit.ID);
					isBomb = false; // スイングは最後まで振る
					continue;
				}

				if (hit) {
					// 非スイングは従来通り
					return;
				}
			}
		}

		if (isBomb)
		{
			break;
		}
	}
}

void Battle001::updateAndCheckCollisions(ClassExecuteSkills& executedSkill, Array<ClassHorizontalUnit>& targetUnits, Array<ClassHorizontalUnit>& friendlyUnits)
{
	Array<int32> bulletsToRemove;
	Array<uint64> keysToErase;
	const bool isHeal = executedSkill.classSkill.SkillType == SkillType::heal;

	// Unit 検索の安全ヘルパ
	auto findUnitById = [&](long long uid) -> Unit*
		{
			std::shared_lock lock(aStar.unitListRWMutex);
			for (auto& grp : classBattleManage.listOfAllUnit)
				for (auto& u : grp.ListClassUnit)
					if (u.ID == uid) return &u;
			for (auto& grp : classBattleManage.listOfAllEnemyUnit)
				for (auto& u : grp.ListClassUnit)
					if (u.ID == uid) return &u;
			for (const auto& sp : classBattleManage.hsMyUnitBuilding)
				if (sp->ID == uid) return sp.get();
			for (const auto& sp : classBattleManage.hsEnemyUnitBuilding)
				if (sp->ID == uid) return sp.get();
			return nullptr;
		};

	for (auto& bullet : executedSkill.ArrayClassBullet)
	{
		bullet.lifeTime += Scene::DeltaTime();
		if (bullet.lifeTime > bullet.duration)
		{
			bulletsToRemove.push_back(bullet.No);
			continue;
		}

		// --- swing: 角度で進行し、術者に追従 ---
		if (executedSkill.classSkill.MoveType == MoveType::swing)
		{
			Unit* caster = findUnitById(executedSkill.UnitID);
			if (!caster || !caster->IsBattleEnable)
			{
				bulletsToRemove.push_back(bullet.No);
				continue;
			}

			// homing
			if (executedSkill.classSkill.homing) {
				bullet.NowPosition = caster->GetNowPosiCenter();
			}
			else if (bullet.lifeTime == 0.0) {
				bullet.NowPosition = caster->GetNowPosiCenter();
			}

			// 回転方向: 右向きなら時計回り(-)、左向きなら反時計(+)
			double dirSign = +1.0;
			if (executedSkill.classSkill.startDegreeType == 6) {
				bool facingRight = false;
				if (!caster->vecMove.isZero()) {
					facingRight = (caster->vecMove.x >= 0);
				}
				else {
					Vec2 toOrder = (caster->GetOrderPosiCenter() - caster->GetNowPosiCenter());
					facingRight = toOrder.isZero() ? true : (toOrder.x >= 0);
				}
				dirSign = (facingRight ? -1.0 : +1.0);
			}

			// 開始角から arc を dirSign 方向へ等速回転
			const double t = Saturate(bullet.lifeTime / Max(0.0001, bullet.duration));
			const double swingArc = (executedSkill.classSkill.arcDeg > 0.0
				? executedSkill.classSkill.arcDeg
				: static_cast<double>(executedSkill.classSkill.range));
			const double curDeg = static_cast<double>(bullet.initDegree) + dirSign * swingArc * t;
			bullet.degree = static_cast<float>(curDeg);
			bullet.radian = Math::ToRadians(curDeg);

			// 毎フレーム判定
			Array<ClassHorizontalUnit>& currentTargetGroup = isHeal ? friendlyUnits : targetUnits;
			bool isBomb = false;
			handleBulletCollision(bullet, executedSkill, currentTargetGroup, isBomb);
			continue;
		}
		else
		{
			// 従来の移動
			if (executedSkill.classSkill.MoveType == MoveType::thr)
			{
				const double t = Saturate(bullet.lifeTime / Max(0.0001, bullet.duration));
				const Vec2 flat = bullet.StartPosition + (bullet.OrderPosition - bullet.StartPosition) * t;

				const double dist = (bullet.OrderPosition - bullet.StartPosition).length();
				const double heightRatio = (executedSkill.classSkill.height == 0)
					? 0.5
					: (static_cast<double>(executedSkill.classSkill.height) / 100.0);
				const double arcHeight = dist * heightRatio;

				const double s = (2.0 * t - 1.0);
				const double yOffset = arcHeight * (1.0 - (s * s));
				bullet.NowPosition = flat.movedBy(0, -yOffset);
			}
			else
			{
				bullet.NowPosition += bullet.MoveVec * executedSkill.classSkill.speed * Scene::DeltaTime();
			}
		}

		// 判定間隔（貫通時）: swing は距離ベースと相性悪いので毎フレーム許可
		bool allowHitCheck = (executedSkill.classSkill.MoveType == MoveType::swing);

		if (!allowHitCheck)
		{
			const bool piercing = (executedSkill.classSkill.hard >= 1);
			const uint64 key = makeBulletKey(bullet);

			if (piercing)
			{
				const double threshold = computeHitIntervalDistance(executedSkill.classSkill);
				if (threshold <= 0.0) allowHitCheck = true;
				else
				{
					if (auto it = gHitIntervalState.find(key); it == gHitIntervalState.end())
					{
						gHitIntervalState[key] = HitIntervalState{ bullet.NowPosition, 0.0 };
						allowHitCheck = false;
					}
					else
					{
						auto& st = it->second;
						st.accum += (bullet.NowPosition - st.lastPos).length();
						st.lastPos = bullet.NowPosition;
						if (st.accum >= threshold) { allowHitCheck = true; st.accum = 0.0; }
						else allowHitCheck = false;
					}
				}
			}
			else allowHitCheck = true;
		}

		if (executedSkill.classSkill.MoveType == MoveType::thr
			&& bullet.lifeTime < bullet.duration)
		{
			allowHitCheck = false;
		}

		if (allowHitCheck)
		{
			Array<ClassHorizontalUnit>& currentTargetGroup = isHeal ? friendlyUnits : targetUnits;
			bool isBomb = false;
			handleBulletCollision(bullet, executedSkill, currentTargetGroup, isBomb);
			if (isBomb) bulletsToRemove.push_back(bullet.No);
		}
	}

	if (!bulletsToRemove.isEmpty())
	{
		Array<uint64> toErase;
		for (const auto& kv : gHitIntervalState)
		{
			const uint64 key = kv.first;
			const int32 noUpper = static_cast<int32>(key >> 32);
			if (bulletsToRemove.contains(noUpper)) toErase.push_back(key);
		}
		// 追加: swing のヒット済み集合も掃除
		Array<uint64> toEraseSwing;
		for (const auto& kv : gSwingHitOnce)
		{
			const uint64 key = kv.first;
			const int32 noUpper = static_cast<int32>(key >> 32);
			if (bulletsToRemove.contains(noUpper)) toEraseSwing.push_back(key);
		}
		for (const auto key : toEraseSwing) gSwingHitOnce.erase(key);
		for (const auto key : toErase) gHitIntervalState.erase(key);
	}

	executedSkill.ArrayClassBullet.remove_if([&](const ClassBullets& b) {
		return bulletsToRemove.contains(b.No);
	});
}

//void Battle001::updateAndCheckCollisions(ClassExecuteSkills& executedSkill, Array<ClassHorizontalUnit>& targetUnits, Array<ClassHorizontalUnit>& friendlyUnits)
//{
//	Array<int32> bulletsToRemove;
//	Array<uint64> keysToErase; // 状態掃除用
//	const bool isHeal = executedSkill.classSkill.SkillType == SkillType::heal;
//
//	for (auto& bullet : executedSkill.ArrayClassBullet)
//	{
//		// 寿命と移動更新
//		bullet.lifeTime += Scene::DeltaTime();
//		if (bullet.lifeTime > bullet.duration)
//		{
//			bulletsToRemove.push_back(bullet.No);
//			continue;
//		}
//
//		// 位置更新
//		const Vec2 prevPos = bullet.NowPosition;
//
//		if (executedSkill.classSkill.MoveType == MoveType::thr)
//		{
//			// t: 0→1
//			const double t = Saturate(bullet.lifeTime / Max(0.0001, bullet.duration));
//			// 直線補間
//			const Vec2 flat = bullet.StartPosition + (bullet.OrderPosition - bullet.StartPosition) * t;
//
//			// 山の高さ = 距離 * (heightRatio)
//			const double dist = (bullet.OrderPosition - bullet.StartPosition).length();
//			const double heightRatio = (executedSkill.classSkill.height == 0)
//				? 0.5
//				: (static_cast<double>(executedSkill.classSkill.height) / 100.0);
//			const double arcHeight = dist * heightRatio;
//
//			// 放物線オフセット: peak at t=0.5
//			const double s = (2.0 * t - 1.0);
//			const double yOffset = arcHeight * (1.0 - (s * s)); // 0→peak→0
//
//			// アイソメ画面では「上」は -Y 方向に持ち上げる
//			bullet.NowPosition = flat.movedBy(0, -yOffset);
//		}
//		else
//		{
//			// 既存の直線移動
//			bullet.NowPosition += bullet.MoveVec * executedSkill.classSkill.speed * Scene::DeltaTime();
//		}
//
//		// 判定間隔の計算（hard>=1 のときだけ距離間隔で判定）
//		const bool piercing = (executedSkill.classSkill.hard >= 1);
//		bool allowHitCheck = true;
//		const uint64 key = makeBulletKey(bullet);
//
//		if (piercing)
//		{
//			const double threshold = computeHitIntervalDistance(executedSkill.classSkill);
//			if (threshold <= 0.0)
//			{
//				allowHitCheck = true;
//			}
//			else
//			{
//				if (auto it = gHitIntervalState.find(key); it == gHitIntervalState.end())
//				{
//					// 生成直後は lastPos 初期化のみ（最初の閾値到達までは判定しない）
//					gHitIntervalState[key] = HitIntervalState{ bullet.NowPosition, 0.0 };
//					allowHitCheck = false;
//				}
//				else
//				{
//					auto& st = it->second;
//					st.accum += (bullet.NowPosition - st.lastPos).length();
//					st.lastPos = bullet.NowPosition;
//
//					if (st.accum >= threshold)
//					{
//						allowHitCheck = true;
//						st.accum = 0.0; // 次の閾値へ
//					}
//					else
//					{
//						allowHitCheck = false;
//					}
//				}
//			}
//		}
//
//		//もし「着弾のみで判定」にしたい場合は、以下のように throw の途中判定を抑止できます:
//		if (executedSkill.classSkill.MoveType == MoveType::thr
//			&& bullet.lifeTime < bullet.duration) {
//			allowHitCheck = false;
//		}
//
//		// 当たり判定
//		if (allowHitCheck)
//		{
//			Array<ClassHorizontalUnit>& currentTargetGroup = isHeal ? friendlyUnits : targetUnits;
//
//			bool isBomb = false; // 非貫通時のみ true になり、弾を消す契機になる
//			handleBulletCollision(bullet, executedSkill, currentTargetGroup, isBomb);
//
//			// 非貫通でヒット済みなら削除
//			if (isBomb)
//			{
//				bulletsToRemove.push_back(bullet.No);
//			}
//		}
//	}
//
//	// 削除予定の弾に紐づく間隔状態を掃除
//	if (!bulletsToRemove.isEmpty())
//	{
//		Array<uint64> toErase;
//		for (const auto& kv : gHitIntervalState)
//		{
//			const uint64 key = kv.first;
//			const int32 noUpper = static_cast<int32>(key >> 32);
//			if (bulletsToRemove.contains(noUpper))
//			{
//				toErase.push_back(key);
//			}
//		}
//		for (const auto key : toErase)
//		{
//			gHitIntervalState.erase(key);
//		}
//	}
//
//	// 弾を削除
//	executedSkill.ArrayClassBullet.remove_if([&](const ClassBullets& b) {
//		return bulletsToRemove.contains(b.No);
//	});
//}

void Battle001::processSkillEffects()
{
	// Player skills vs Enemies
	for (auto& executedSkill : m_Battle_player_skills)
	{
		updateAndCheckCollisions(executedSkill, classBattleManage.listOfAllEnemyUnit, classBattleManage.listOfAllUnit);
	}

	// Enemy skills vs Players
	for (auto& executedSkill : m_Battle_enemy_skills)
	{
		updateAndCheckCollisions(executedSkill, classBattleManage.listOfAllUnit, classBattleManage.listOfAllEnemyUnit);
	}

	// Neutral skills vs ...? (Logic unclear, assuming vs all for now)
	for (auto& executedSkill : m_Battle_neutral_skills)
	{
		updateAndCheckCollisions(executedSkill, classBattleManage.listOfAllUnit, classBattleManage.listOfAllEnemyUnit);
		updateAndCheckCollisions(executedSkill, classBattleManage.listOfAllEnemyUnit, classBattleManage.listOfAllUnit);
	}

	// 変更点: 弾が空になったスキルは術者の FlagMovingSkill を確実に解除してから破棄
	auto clearEnded = [&](Array<ClassExecuteSkills>& list)
		{
			auto findUnitById = [&](long long uid) -> Unit*
				{
					std::shared_lock lock(aStar.unitListRWMutex);
					for (auto& grp : classBattleManage.listOfAllUnit)
						for (auto& u : grp.ListClassUnit)
							if (u.ID == uid) return &u;
					for (auto& grp : classBattleManage.listOfAllEnemyUnit)
						for (auto& u : grp.ListClassUnit)
							if (u.ID == uid) return &u;
					for (const auto& sp : classBattleManage.hsMyUnitBuilding)
						if (sp->ID == uid) return sp.get();
					for (const auto& sp : classBattleManage.hsEnemyUnitBuilding)
						if (sp->ID == uid) return sp.get();
					return nullptr;
				};

			list.remove_if([&](const ClassExecuteSkills& s)
			{
				if (!s.ArrayClassBullet.isEmpty())
					return false;

				if (Unit* caster = findUnitById(s.UnitID))
					caster->FlagMovingSkill = false; // ヒット不発でも解除

				return true;
			});
		};

	clearEnded(m_Battle_player_skills);
	clearEnded(m_Battle_enemy_skills);
	clearEnded(m_Battle_neutral_skills);
}


void Battle001::CalucDamage(Unit& itemTarget, double strTemp, ClassExecuteSkills& ces)
{
	double powerStr = 0;
	double defStr = 0;

	// 基礎パワー（術者側の攻撃/魔法）だけを計算
	switch (ces.classSkill.SkillStrKind)
	{
	case SkillStrKind::attack:
	{
		powerStr = (ces.classUnit->Attack
			* (strTemp / 100)
			)
			* Random(0.8, 1.2);
		defStr = ces.classUnit->Defense * Random(0.8, 1.2);
	}
	break;
	case SkillStrKind::attack_magic:
	{
		powerStr = (
			(ces.classUnit->Attack + ces.classUnit->Magic)
			* (strTemp / 100)
			)
			* Random(0.8, 1.2);
		defStr = ces.classUnit->Defense * Random(0.8, 1.2);
	}
	break;
	default:
		break;
	}

	const bool isHeal = (ces.classSkill.SkillType == SkillType::heal);

	if (!isHeal)
	{
		// 被ダメージ時のみ、防御は「対象の Defense」を使う
		defStr = (itemTarget.Defense) * Random(0.8, 1.2);

		// マイナスダメージにならないよう 0 切り上げ
		if (powerStr < defStr)
		{
			powerStr = 0.0;
			defStr = 0.0;
		}
	}
	else
	{
		// 回復では防御を考慮しない（負の回復を避けるため 0 未満は 0）
		if (powerStr < 0.0)
			powerStr = 0.0;
		defStr = 0.0;
	}

	// ログ（ダメ時のみ）
	if (!isHeal)
	{
		const double damage = (powerStr - defStr);
		if (damage > 0.0)
		{
			//Print << U"[DAMAGE_LOG] Target: " << itemTarget.Name << U" (ID:" << itemTarget.ID << U", HP:" << itemTarget.Hp << U")"
			//	<< U" takes " << damage << U" damage from Attacker: " << ces.classUnit->Name << U" (ID:" << ces.classUnit->ID << U")"
			//	<< U" with Skill: " << ces.classSkill.nameTag;
		}
	}

	// 適用
	if (itemTarget.IsBuilding)
	{
		if (isHeal)
		{
			// 建物回復量の上限は不明なため、下限のみクランプ
			itemTarget.HPCastle = itemTarget.HPCastle + powerStr;
			if (itemTarget.HPCastle < 0.0) itemTarget.HPCastle = 0.0;
		}
		else
		{
			itemTarget.HPCastle = itemTarget.HPCastle - (powerStr - defStr);
			if (itemTarget.HPCastle < 0.0) itemTarget.HPCastle = 0.0;
		}
	}
	else
	{
		if (isHeal)
		{
			if (ces.classSkill.attr == U"mp")
			{
				itemTarget.Mp = itemTarget.Mp + powerStr; // Mp の上限があればここでクランプ
			}
			else // 既定は HP 回復
			{
				itemTarget.Hp = itemTarget.Hp + powerStr;
				if (itemTarget.Hp > itemTarget.HpMAX) itemTarget.Hp = itemTarget.HpMAX;
			}
		}
		else
		{
			itemTarget.Hp = itemTarget.Hp - (powerStr - defStr);
			if (itemTarget.Hp < 0.0) itemTarget.Hp = 0.0;
		}
	}
}
bool Battle001::applySkillEffectAndRegisterHit(bool& bombCheck, Array<int32>& arrayNo, ClassBullets& target, ClassExecuteSkills& loop_Battle_player_skills, Unit& itemTarget)
{
	// 術者を安全に引き直す（ID から検索）
	auto findUnitById = [&](long long uid) -> Unit*
		{
			std::shared_lock lock(aStar.unitListRWMutex);
			for (auto& grp : classBattleManage.listOfAllUnit)
				for (auto& u : grp.ListClassUnit)
					if (u.ID == uid) return &u;

			for (auto& grp : classBattleManage.listOfAllEnemyUnit)
				for (auto& u : grp.ListClassUnit)
					if (u.ID == uid) return &u;

			for (const auto& sp : classBattleManage.hsMyUnitBuilding)
				if (sp->ID == uid) return sp.get();
			for (const auto& sp : classBattleManage.hsEnemyUnitBuilding)
				if (sp->ID == uid) return sp.get();

			return nullptr;
		};

	if (Unit* attacker = findUnitById(loop_Battle_player_skills.UnitID))
	{
		attacker->FlagMovingSkill = false;
		// 以降の計算でも安全に使えるよう、ポインタを再バインド
		loop_Battle_player_skills.classUnit = attacker;
	}
	// 見つからない場合はフラグ解除をスキップ（ダメージ計算は術者参照が必要なので下で再バインドできないなら無理に触らない）

	CalucDamage(itemTarget, loop_Battle_player_skills.classSkill.str, loop_Battle_player_skills);

	//消滅
	//一体だけ当たったらそこで終了
	if (loop_Battle_player_skills.classSkill.hard <= 0)
	{
		arrayNo.push_back(target.No);
		bombCheck = true;
		return true;
	}
	return false;
}

void Battle001::handleBuildTargetSelection()
{
	if (!IsBuildSelectTraget)
		return;

	if (const auto index = mapTile.ToIndex(Cursor::PosF(), mapTile.columnQuads, mapTile.rowQuads))
	{
		if (mapTile.ToTile(*index, mapTile.N).leftClicked() && longBuildSelectTragetId != -1) {
			processBuildOnTilesWithMovement({ *index });
		}
		// 右クリックドラッグによる範囲選択
		if (MouseR.up() && longBuildSelectTragetId != -1) {
			Point startTile, endTile;

			if (auto startIndex = mapTile.ToIndex(cursPos, mapTile.columnQuads, mapTile.rowQuads)) {
				startTile = *startIndex;
				endTile = *index;

				Array<Point> selectedTiles = getRangeSelectedTiles(startTile, endTile, mapTile);
				// 新しい移動→建築処理を呼び出し
				processBuildOnTilesWithMovement(selectedTiles);
			}
		}
	}
}

void Battle001::processBuildOnTilesWithMovement(const Array<Point>& tiles)
{
	if (longBuildSelectTragetId == -1)
	{
		Print << U"建築ユニットが選択されていません";
		return;
	}

	Unit* selectedUnitPtr = nullptr;
	{
		std::shared_lock lock(aStar.unitListRWMutex);
		for (auto& group : classBattleManage.listOfAllUnit) {
			for (auto& unit : group.ListClassUnit) {
				if (unit.ID == longBuildSelectTragetId) {
					selectedUnitPtr = &unit;
					break;
				}
			}
			if (selectedUnitPtr) break;
		}
	}

	if (!selectedUnitPtr) {
		Print << U"選択された建築ユニットが見つかりません";
		return;
	}
	Unit& selectedUnit = *selectedUnitPtr;

	// 建築可能なタイルのみを抽出
	Array<Point> validTiles;
	for (const auto& tile : tiles)
	{
		if (canBuildOnTile(tile, classBattleManage, mapTile))
		{
			validTiles.push_back(tile);
		}
	}

	if (validTiles.isEmpty())
	{
		Print << U"建築可能なタイルがありません";
		return;
	}

	// 現在位置からの距離でソート（最適な移動順序）
	Optional<Point> currentTilePos = mapTile.ToIndex(selectedUnit.GetNowPosiCenter(), mapTile.columnQuads, mapTile.rowQuads);
	if (currentTilePos)
	{
		validTiles.sort_by([&](const Point& a, const Point& b) {
			return currentTilePos->manhattanDistanceFrom(a) < currentTilePos->manhattanDistanceFrom(b);
		});
	}

	// 各タイルに対して移動→建築のタスクを作成
	for (const auto& tile : validTiles)
	{
		BuildAction buildAction = selectedUnit.tempIsBuildSelectTragetBuildAction;
		buildAction.rowBuildingTarget = tile.y;
		buildAction.colBuildingTarget = tile.x;

		// 移動が必要な建築として特別なタスクタイプを設定
		selectedUnit.requiresMovement = true;

		selectedUnit.arrYoyakuBuild.push_back(buildAction);
		Print << U"タイル({}, {})への移動→建築を予約しました"_fmt(tile.x, tile.y);
	}

	// 最初の建築タスクを開始
	if (!selectedUnit.arrYoyakuBuild.isEmpty())
	{
		selectedUnit.FlagReachedDestination = false;
		selectedUnit.currentTask = UnitTask::MovingToBuild;

		// 最初のタイルへの移動を開始
		Point firstTile = Point(selectedUnit.arrYoyakuBuild.front().colBuildingTarget,
			selectedUnit.arrYoyakuBuild.front().rowBuildingTarget);

		Vec2 targetPos = mapTile.ToTileBottomCenter(firstTile, mapTile.N);
		selectedUnit.orderPosiLeft = targetPos.movedBy(-(selectedUnit.yokoUnit / 2), -(selectedUnit.TakasaUnit / 2));
		selectedUnit.orderPosiLeftLast = targetPos;
		selectedUnit.vecMove = (selectedUnit.orderPosiLeft - selectedUnit.nowPosiLeft).normalized();
		selectedUnit.moveState = moveState::MoveAI;
	}

	// 建築選択状態を解除
	IsBuildSelectTraget = false;
	IsBuildMenuHome = false;
	selectedUnit.IsSelect = false;
	longBuildSelectTragetId = -1;
}

void Battle001::processUnitBuildQueue(Unit& itemUnit, Array<ProductionOrder>& productionList)
{
	if (itemUnit.arrYoyakuBuild.isEmpty()) return;

	auto& buildAction = itemUnit.arrYoyakuBuild.front();

	// isMoveフラグを持つ建築アクションの場合、目的地到着を待つ
	if (buildAction.isMove)
	{
		// まだ目的地に到着していなければ、キューの処理をスキップ
		if (!itemUnit.FlagReachedDestination)
		{
			return;
		}

		// 到着済みで、タイマーが動いていなければ開始する
		if (itemUnit.FlagReachedDestination && !itemUnit.taskTimer.isRunning())
		{
			itemUnit.taskTimer.restart();
			itemUnit.progressTime = 0.0;
		}
	}
	else
	{
		// 通常の建築アクションで、タイマーが動いていなければ開始する（安全対策）
		if (!itemUnit.taskTimer.isRunning())
		{
			itemUnit.taskTimer.restart();
			itemUnit.progressTime = 0.0;
		}
	}


	const double tempTime = buildAction.buildTime;
	auto tempBA = buildAction.result;
	const int32 tempRowBuildingTarget = itemUnit.arrYoyakuBuild.front().rowBuildingTarget;
	const int32 tempColBuildingTarget = itemUnit.arrYoyakuBuild.front().colBuildingTarget;
	const int32 createCount = itemUnit.arrYoyakuBuild.front().createCount;

	if (itemUnit.progressTime >= 1.0)
	{
		itemUnit.taskTimer.reset();
		itemUnit.arrYoyakuBuild.pop_front();
		if (!itemUnit.arrYoyakuBuild.isEmpty())
		{
			itemUnit.progressTime = 0.0;
			itemUnit.taskTimer.restart();
		}
		else
		{
			itemUnit.progressTime = -1.0;
		}

		if (tempBA.type == U"unit")
		{
			ProductionOrder order;
			order.spawn = tempBA.spawn;
			order.tempColBuildingTarget = tempColBuildingTarget;
			order.tempRowBuildingTarget = tempRowBuildingTarget;
			order.count = createCount;
			productionList.push_back(order);
		}
	}

	// プログレス更新
	if (itemUnit.taskTimer.isRunning())
	{
		itemUnit.progressTime = Min(itemUnit.taskTimer.sF() / tempTime, 1.0);
	}
}

void Battle001::updateBuildQueue()
{
	Array<ProductionOrder> productionList;

	// 通常ユニットのキューを処理
	{
		for (auto& loau : classBattleManage.listOfAllUnit)
		{
			for (auto& itemUnit : loau.ListClassUnit)
			{
				processUnitBuildQueue(itemUnit, productionList);
			}
		}
	}

	// 建築ユニットのキューを処理
	for (auto& buildingUnit : classBattleManage.hsMyUnitBuilding)
	{
		processUnitBuildQueue(*buildingUnit, productionList);
	}

	// 生産リストに基づいてユニットを登録
	for (auto& order : productionList)
	{
		UnitRegister(classBattleManage, mapTile, order.spawn, order.tempColBuildingTarget, order.tempRowBuildingTarget, order.count, classBattleManage.listOfAllUnit, false);
	}
	if (productionList.size() > 0)
	{
		initSkillUI();
	}
}

void Battle001::startAsyncFogCalculation()
{
	taskFogCalculation = Async([this]() {
		while (!abortFogTask)
		{
			if (fogUpdateTimer.sF() >= FOG_UPDATE_INTERVAL)
			{
				Grid<Visibility> tempMap = Grid<Visibility>(mapTile.N, mapTile.N, Visibility::Unseen);

				//ユニットデータのスナップショットを作成
				//TODO:全てのアクセスを mutex で保護する
				Array<const Unit*> unitSnapshot;
				{
					std::shared_lock lock(aStar.unitListRWMutex);
					for (auto& units : classBattleManage.listOfAllUnit)
					{
						for (const auto& unit : units.ListClassUnit)
						{
							if (unit.IsBattleEnable)
								unitSnapshot.push_back(&unit);
						}
					}

					// 建物を追加
					for (const auto& sp : classBattleManage.hsMyUnitBuilding)
						if (sp && sp->IsBattleEnable)
							unitSnapshot.push_back(sp.get());
				}

				// スナップショットを使って安全に計算
				calculateFogFromUnits(tempMap, unitSnapshot);

				// 結果をメインスレッドで使用可能にする
				{
					std::scoped_lock lock(fogMutex);
					nextVisibilityMap = std::move(tempMap);
					fogDataReady = true;
				}

				fogUpdateTimer.restart();
			}

			System::Sleep(1); // CPU負荷軽減
		}
	});
}

void Battle001::calculateFogFromUnits(Grid<Visibility>& visMap, const Array<const Unit*>& units)
{
	static HashSet<Point> lastVisibleTiles;
	HashSet<Point> currentVisibleTiles;

	for (const auto& unit : units)
	{
		const Vec2 unitPos = unit->GetNowPosiCenter();
		const auto unitIndex = mapTile.ToIndex(unitPos, mapTile.columnQuads, mapTile.rowQuads);
		if (!unitIndex) continue;

		const Point centerTile = unitIndex.value();
		const int32 visionRadius = unit->visionRadius;

		for (int dy = -visionRadius; dy <= visionRadius; ++dy)
		{
			for (int dx = -visionRadius; dx <= visionRadius; ++dx)
			{
				const Point targetTile = centerTile + Point{ dx, dy };
				if (InRange(targetTile.x, 0, mapTile.N - 1) &&
					InRange(targetTile.y, 0, mapTile.N - 1) &&
					targetTile.manhattanDistanceFrom(centerTile) <= visionRadius)
				{
					currentVisibleTiles.insert(targetTile);
				}
			}
		}
	}

	// 差分更新
	for (const auto& tile : lastVisibleTiles)
	{
		if (!currentVisibleTiles.contains(tile))
			visMap[tile] = Visibility::Unseen;
	}

	for (const auto& tile : currentVisibleTiles)
	{
		visMap[tile] = Visibility::Visible;
	}

	lastVisibleTiles = std::move(currentVisibleTiles);
}

/// @brief バトルシーンのメインループを開始し、スペースキーが押されたときに一時停止画面を表示
/// @return 非同期タスク
void Battle001::registerTextureAssets()
{
	for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/040_ChipImage/"))
		TextureAsset::Register(FileSystem::FileName(filePath), filePath);
	for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/015_BattleMapCellImage/"))
		TextureAsset::Register(FileSystem::FileName(filePath), MakeTextureAssetData1(filePath, TextureDesc::Mipped));
	for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/043_ChipImageBuild/"))
		TextureAsset::Register(FileSystem::FileName(filePath), filePath);
	for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/041_ChipImageSkill/"))
		TextureAsset::Register(FileSystem::FileName(filePath), filePath);
}

void Battle001::setupInitialUnits()
{
	//初期ユニット
	{
		//UnitRegister(classBattleManage, mapTile, U"LineInfantryM14",
		//	10,
		//	10,
		//	3,
		//	classBattleManage.listOfAllUnit, false
		//);
	}

	//初期ユニット-建物
	{
		for (auto unit_template : m_commonConfig.arrayInfoUnit)
		{
			if (unit_template.NameTag == U"Home")
			{
				unit_template.ID = classBattleManage.getIDCount();
				unit_template.IsBuilding = true;
				unit_template.mapTipObjectType = MapTipObjectType::HOME;
				unit_template.rowBuilding = mapTile.N / 2;
				unit_template.colBuilding = mapTile.N / 2;
				unit_template.initTilePos = Point{ unit_template.rowBuilding, unit_template.colBuilding };
				unit_template.Move = 0.0;
				unit_template.nowPosiLeft = mapTile.ToTile(unit_template.initTilePos, mapTile.N)
					.asPolygon()
					.centroid()
					.movedBy(-(unit_template.yokoUnit / 2), -(unit_template.TakasaUnit / 2));

				// 建物 HP バー初期化（真下に配置）
				{
					Vec2 bottom = mapTile.ToTileBottomCenter(Point(unit_template.colBuilding, unit_template.rowBuilding), mapTile.N)
						.movedBy(0, -mapTile.TileThickness);
					Vec2 barTopLeft = bottom.movedBy(-(LIQUID_BAR_WIDTH / 2.0), 6.0);
					unit_template.bLiquidBarBattle = GameUIToolkit::LiquidBarBattle(Rect(barTopLeft.x, barTopLeft.y, LIQUID_BAR_WIDTH, LIQUID_BAR_HEIGHT));
				}

				auto building_unit_ptr = std::make_shared<Unit>(unit_template);

				// 最大HPを保持（0 回避）
				gBuildingMaxHP[building_unit_ptr->ID] = Max<double>(1.0, building_unit_ptr->HPCastle);

				classBattleManage.hsMyUnitBuilding.insert(building_unit_ptr);
			}
		}
	}

}

void Battle001::startAsyncTasks()
{
	aStar.taskAStarEnemy = Async([this]() {
		try {
			while (!aStar.abortAStarEnemy)
			{
				if (!aStar.pauseAStarTaskEnemy)
				{
					HashTable<Point, Array<Unit*>> hsBuildingUnitForAstarSnapshot;
					{
						std::shared_lock lock(aStar.unitListRWMutex);
						hsBuildingUnitForAstarSnapshot = hsBuildingUnitForAstar;
					}
					aStar.BattleMoveAStar(
						classBattleManage.listOfAllUnit,
						classBattleManage.listOfAllEnemyUnit,
						classBattleManage.classMapBattle.value().mapData,
						aiRootEnemy,
						aStar.abortAStarEnemy,
						aStar.pauseAStarTaskEnemy, aStar.changeUnitMember, hsBuildingUnitForAstarSnapshot, mapTile);
				}
				System::Sleep(1);
			}
			System::Sleep(1); // CPU過負荷防止
		}
		catch (const std::exception& e) {
			std::cerr << "Thread exception: " << e.what() << std::endl;
		}
		catch (...) {
			std::cerr << "Thread unknown exception" << std::endl;
		}

		});

	aStar.taskAStarMyUnits = Async([this]() {
		while (!aStar.abortAStarMyUnits)
		{
			try {
				if (!aStar.pauseAStarTaskMyUnits)
				{
					HashTable<Point, Array<Unit*>> hsBuildingUnitForAstarSnapshot;
					{
						std::shared_lock lock(aStar.unitListRWMutex);
						hsBuildingUnitForAstarSnapshot = hsBuildingUnitForAstar;
					}
					aStar.BattleMoveAStarMyUnitsKai(
						classBattleManage.listOfAllUnit,
						classBattleManage.listOfAllEnemyUnit,
						classBattleManage.classMapBattle.value().mapData,
						aiRootMy,
						aStar.abortAStarMyUnits,
						aStar.pauseAStarTaskMyUnits, hsBuildingUnitForAstarSnapshot, mapTile);
				}
				System::Sleep(1); // CPU過負荷防止
			}
			catch (const std::exception& e) {
				std::cerr << "Thread exception: " << e.what() << std::endl;
			}
			catch (...) {
				std::cerr << "Thread unknown exception" << std::endl;
			}
		}
		});

	startAsyncFogCalculation();
}


Co::Task<void> Battle001::start()
{
	// シーン開始時にクールタイムを初期化
	gSkillReadyAtSec.clear();
	gHitIntervalState.clear();
	gSkillUsesLeft.clear();

	registerTextureAssets();

	/// modでカスタマイズ出来るようにあえて配列を使う
	arrayBattleZinkei.push_back(false);
	arrayBattleZinkei.push_back(false);
	arrayBattleZinkei.push_back(false);
	arrayBattleCommand.push_back(false);
	arrayBattleCommand.push_back(false);

	setupInitialUnits();

	visibilityMap = Grid<Visibility>(mapTile.N, mapTile.N, Visibility::Unseen);

	initUI();

	//始点設定
	camera.jumpTo(
		mapTile.ToTileBottomCenter(
			Point(10, 10),
			mapTile.N),
		camera.getTargetScale());
	resourcePointTooltip.setCamera(camera);

	stopwatchFinance.restart();
	stopwatchGameTime.restart();

	// --- サバイバルタイマー開始 ---
	gSurvivalTimer.restart();
	gSurvivalStarted = true;
	gSurvivalEnded = false;

	// 追加: フェーズスケジュール初期化
	initDefaultPhaseSchedule();

	startAsyncTasks();

	co_await mainLoop().pausedWhile([&]
	{
		if (KeySpace.pressed())
		{
			Rect rectPauseBack{ 0, 0, Scene::Width(), Scene::Height() };
			rectPauseBack.draw(ColorF{ 0.0, 0.0, 0.0, 0.5 });
			const String pauseText = U"Pause";

			Rect rectPause{ int32(Scene::Width() / 2 - fontInfo.fontSystem(pauseText).region().w / 2),
							0,
							int32(fontInfo.fontSystem(pauseText).region().w),
							int32(fontInfo.fontSystem(pauseText).region().h) };
			rectPause.draw(Palette::Black);
			fontInfo.fontSystem(pauseText)
				.drawAt(rectPause.x + rectPause.w / 2, rectPause.y + rectPause.h / 2, Palette::White);

			return true;
		}
		else
		{
		}
	});;
}
void Battle001::updateGameSystems()
{
	// 一時停止（スペース）時はサバイバル時間を止める
	if (gSurvivalStarted && !gSurvivalEnded)
	{
		if (KeySpace.pressed()) {
			gSurvivalTimer.pause();
		}
		else {
			// 走っていなければ再開（pause→resume）
			if (!gSurvivalTimer.isRunning()) {
				gSurvivalTimer.resume();
			}
		}

		// 残り時間チェック → 0でゲームを終了
		const double remain = kSurvivalDurationSec - gSurvivalTimer.sF();
		if (remain <= 0.0) {
			gSurvivalEnded = true;
			shouldExit = true; // シーンのメインループも抜ける
			System::Exit();    // アプリ終了
			return;
		}
	}

	camera.update();
	resourcePointTooltip.setCamera(camera);

	handleUnitTooltip();
	// 追加: フェーズ定義に基づくスポーン
	spawnEnemiesByPhaseImpl(*this, classBattleManage, mapTile, gSurvivalTimer.sF());

	updateResourceIncome();
	updateBuildQueue();

	if (fogDataReady.load())
	{
		std::scoped_lock lock(fogMutex);
		visibilityMap = std::move(nextVisibilityMap);
		fogDataReady = false;
	}
}

bool Battle001::wasBuildMenuClicked()
{
	const Transformer2D transformer{ Mat3x2::Identity(), Mat3x2::Translate(Scene::Size().x - 328, Scene::Size().y - 328 - 30) };
	for (auto& hbm : sortedArrayBuildMenu)
	{
		if (hbm.second.rectHantei.leftClicked())
		{
			return true;
		}
	}
	return false;
}

Co::Task<void> Battle001::handleRightClickInput()
{
	if (!MouseR.pressed())
	{
		if (!MouseR.up())
		{
			cursPos = Cursor::Pos();
		}
	}
	else if (MouseR.down() && is移動指示)
	{
		cursPos = Cursor::Pos();
	}

	if (MouseR.up())
	{
		Point start = cursPos;
		Point end = Cursor::Pos();
		co_await handleRightClickUnitActions(start, end);
	}
}

void Battle001::handleFormationSelection()
{
	// 陣形処理
	const Transformer2D transformer{ Mat3x2::Identity(), Mat3x2::Translate(0, Scene::Size().y - renderTextureSkill.height() - renderTextureZinkei.height() - underBarHeight) };
	for (auto&& [j, ttt] : Indexed(rectZinkei))
	{
		if (ttt.leftClicked())
		{
			arrayBattleZinkei.clear();
			for (size_t k = 0; k < rectZinkei.size(); k++)
			{
				arrayBattleZinkei.push_back(false);
			}
			arrayBattleZinkei[j] = true;

			renderTextureZinkei.clear(ColorF{ 0.5, 0.0 });
			{
				const ScopedRenderTarget2D target{ renderTextureZinkei.clear(ColorF{ 0.8, 0.8, 0.8,0.5 }) };

				// 描画された最大のアルファ成分を保持するブレンドステート
				const ScopedRenderStates2D blend{ MakeBlendState() };

				Rect df = Rect(320, 60);
				df.drawFrame(4, 0, ColorF{ 0.5 });

				for (auto&& [i, ttt] : Indexed(rectZinkei))
				{
					if (i == 0)
					{
						if (i == j)
						{
							ttt.draw(Palette::Darkred);
						}
						else
						{
							ttt.draw(Palette::Aliceblue);
						}
						fontInfo.fontZinkei(U"密集").draw(ttt, Palette::Black);
						continue;
					}
					else if (i == 1)
					{
						if (i == j)
						{
							ttt.draw(Palette::Darkred);
						}
						else
						{
							ttt.draw(Palette::Aliceblue);
						}
						fontInfo.fontZinkei(U"横列").draw(ttt, Palette::Black);
						continue;
					}
					else if (i == 2)
					{
						if (i == j)
						{
							ttt.draw(Palette::Darkred);
						}
						else
						{
							ttt.draw(Palette::Aliceblue);
						}
						fontInfo.fontZinkei(U"正方").draw(ttt, Palette::Black);
						continue;
					}
				}
			}
		}
	}
}

Co::Task<void> Battle001::handlePlayerInput()
{
	co_await checkCancelSelectionByUIArea();

	const bool buildMenuClicked = wasBuildMenuClicked();

	{
		const auto t = camera.createTransformer();

		if (!buildMenuClicked)
		{
			handleUnitAndBuildingSelection();
		}

		handleCameraInput();

		co_await handleRightClickInput();

		handleBuildTargetSelection();
	}

	handleFormationSelection();

	handleSkillUISelection();
	handleBuildMenuSelectionA();
}

void Battle001::updateAllUnits()
{
	updateUnitMovements();
	updateUnitHealthBars();
}

void Battle001::processCombat()
{
	// 1. Activate new skills for units that are ready to attack
	SkillProcess(classBattleManage.listOfAllUnit, classBattleManage.listOfAllEnemyUnit, m_Battle_player_skills);
	SkillProcess(classBattleManage.listOfAllEnemyUnit, classBattleManage.listOfAllUnit, m_Battle_enemy_skills);

	// 2. Process all active skill effects (move bullets, check collision)
	processSkillEffects();
}

void Battle001::checkUnitDeaths()
{
	for (auto& item : classBattleManage.listOfAllUnit)
	{
		for (auto& itemUnit : item.ListClassUnit)
		{
			if (itemUnit.isValidBuilding())
			{
				if (itemUnit.HPCastle <= 0) {
					itemUnit.IsBattleEnable = false;
					// 追加: クールタイムエントリを掃除
					gSkillReadyAtSec.erase(itemUnit.ID);
					gSkillUsesLeft.erase(itemUnit.ID);
					if (itemUnit.IsSelect || longBuildSelectTragetId == itemUnit.ID)
					{
						IsBuildSelectTraget = false;
						IsBuildMenuHome = false;
						itemUnit.IsSelect = false;
						longBuildSelectTragetId = -1;
					}
				}
			}
			else
			{
				if (itemUnit.Hp <= 0) {
					itemUnit.IsBattleEnable = false;
					// 追加: クールタイムエントリを掃除
					gSkillReadyAtSec.erase(itemUnit.ID);
					gSkillUsesLeft.erase(itemUnit.ID);
					if (itemUnit.IsSelect || longBuildSelectTragetId == itemUnit.ID)
					{
						IsBuildSelectTraget = false;
						IsBuildMenuHome = false;
						itemUnit.IsSelect = false;
						longBuildSelectTragetId = -1;
					}
				}
			}
		}
	}
	for (auto& item : classBattleManage.listOfAllEnemyUnit)
	{
		for (auto& itemUnit : item.ListClassUnit)
		{
			if (itemUnit.isValidBuilding())
			{
				if (itemUnit.HPCastle <= 0) {
					itemUnit.IsBattleEnable = false;
					// 追加: クールタイムエントリを掃除
					gSkillReadyAtSec.erase(itemUnit.ID);
					gSkillUsesLeft.erase(itemUnit.ID);
				}
			}
			else
			{
				if (itemUnit.Hp <= 0) {
					itemUnit.IsBattleEnable = false;
					// 追加: クールタイムエントリを掃除
					gSkillReadyAtSec.erase(itemUnit.ID);
					gSkillUsesLeft.erase(itemUnit.ID);
				}
			}
		}
	}
}

Co::Task<void> Battle001::mainLoop()
{
	const auto _tooltip = resourcePointTooltip.playScoped();

	while (true)
	{
		{
			bool found_producing = false;
			for (auto& loau : classBattleManage.listOfAllUnit)
			{
				for (auto& itemUnit : loau.ListClassUnit)
				{
					if (!itemUnit.arrYoyakuBuild.isEmpty())
					{
						//Print << U"[HP_WATCH] Frame Start. Producing Unit ID: " << itemUnit.ID << U", HP: " << itemUnit.Hp;
						found_producing = true;
						break;
					}
				}
				if (found_producing) break;
			}
		}
		if (shouldExit)
			co_return;

		updateGameSystems();
		co_await handlePlayerInput();
		updateAllUnits();
		processCombat();
		checkUnitDeaths();

		co_await Co::NextFrame();
	}
}


/// <<< UI

/// @brief カメラの現在のビュー領域（矩形）を計算します。
/// @param camera ビュー領域を計算するためのCamera2Dオブジェクト。
/// @param mapTile タイルのオフセット情報を含むMapTileオブジェクト。
/// @return カメラの中心位置、スケール、およびタイルのオフセットに基づいて計算されたRectF型のビュー領域。
RectF Battle001::getCameraView(const Camera2D& camera, const MapTile& mapTile) const
{
	int32 testPadding = -mapTile.TileOffset.x;
	return RectF{
		camera.getCenter() - (Scene::Size() / 2.0) / camera.getScale(),
		Scene::Size() / camera.getScale()
	}.stretched(-testPadding);
}
/// @brief タイルマップをカメラビュー内に描画します。
/// @param cameraView 描画範囲を指定するカメラの矩形領域。
/// @param mapTile タイルマップの情報を持つオブジェクト。
/// @param classBattleManage バトルマップのデータを管理するクラス。
void Battle001::drawTileMap(const RectF& cameraView, const MapTile& mapTile, const ClassBattle& classBattleManage) const
{
	forEachVisibleTile(cameraView, mapTile, [&](const Point& index, const Vec2& pos) {
		const auto& tile = classBattleManage.classMapBattle.value().mapData[index.x][index.y];
		TextureAsset(tile.tip + U".png").draw(Arg::bottomCenter = pos);
	});
}
/// @brief カメラビューとマップタイル、可視性マップに基づいてフォグ（霧）を描画します。
/// @param cameraView 描画範囲を指定するカメラの矩形領域。
/// @param mapTile 描画対象となるマップタイルの情報。
/// @param visibilityMap 各タイルの可視状態を示すグリッド。
void Battle001::drawFog(const RectF& cameraView, const MapTile& mapTile, const Grid<Visibility> visibilityMap) const
{
	forEachVisibleTile(cameraView, mapTile, [&](const Point& index, const Vec2& pos) {
		if (visibilityMap[index] == Visibility::Unseen)
		{
			mapTile.ToTile(index, mapTile.N).draw(ColorF{ 0.0, 0.6 });
		}
	});
}
/// @brief カメラビュー内の建物を描画します。
/// @param cameraView 描画範囲を指定するカメラの矩形領域。
/// @param classBattleManage バトルの状態やクラス情報を管理するオブジェクト。
/// @param mapTile 描画対象となるマップタイル。
void Battle001::drawBuildings(const RectF& cameraView, const ClassBattle& classBattleManage, const MapTile mapTile) const
{
	Array<std::shared_ptr<Unit>> buildings;
	{
		for (const auto& group : { classBattleManage.hsMyUnitBuilding,
			classBattleManage.hsEnemyUnitBuilding })
		{
			for (const auto& item : group)
			{
				buildings.push_back(item);
			}
		}
	}

	for (const auto& u : buildings)
	{
		Vec2 pos = mapTile.ToTileBottomCenter(Point(u->colBuilding, u->rowBuilding), mapTile.N);
		if (!cameraView.intersects(pos))
			continue;

		TextureAsset(u->ImageName).draw(Arg::bottomCenter = pos.movedBy(0, -mapTile.TileThickness));
		if (u->IsSelect)
			RectF(Arg::bottomCenter = pos.movedBy(0, -mapTile.TileThickness),
				TextureAsset(u->ImageName).size()
			)
			.drawFrame(BUILDING_FRAME_THICKNESS, Palette::Red);

		// 建物 HP バー（ユニットと同じ見た目で、色だけ建物用）
		u->bLiquidBarBattle.draw(ColorF{ 0.5, 0.1, 1.0 }, ColorF{ 0.7, 0.05, 0.05 }, ColorF{ 0.9, 0.5, 0.1 });
	}
}
/// @brief カメラビュー内のユニットを描画します。
/// @param cameraView 描画範囲を指定する矩形領域。
/// @param classBattleManage ユニット情報を管理するClassBattleオブジェクト。
void Battle001::drawUnits(const RectF& cameraView, const ClassBattle& classBattleManage) const
{
	auto isTileVisible = [&](const Vec2& worldPos) -> bool
		{
			if (const auto idx = mapTile.ToIndex(worldPos, mapTile.columnQuads, mapTile.rowQuads))
			{
				return (visibilityMap[*idx] == Visibility::Visible);
			}
			return false; // マップ外などは非表示扱い
		};

	auto drawGroup = [&](const Array<ClassHorizontalUnit>& group, const String& ringA, const String& ringB, bool cullByFog)
		{
			/// 範囲選択が本質の処理では、forループからHashTableへの置き換えは意味がない
			/// 範囲選択は「連続した領域内のすべての要素」を対象とするため、
			/// 全要素の走査が避けられず、HashTableの「O(1)での直接アクセス」という利点を活かせない
			for (const auto& item : group)
			{
				if (item.FlagBuilding || item.ListClassUnit.empty())
					continue;

				for (const auto& u : item.ListClassUnit)
				{
					if (u.IsBuilding) continue;
					if (!u.IsBattleEnable) continue;

					const Vec2 center = u.GetNowPosiCenter();

					// 敵ユニットのみ霧でカリング
					if (cullByFog)
					{
						if (!isTileVisible(center))
							continue;
					}

					// ユニットの見た目サイズに応じて範囲を広げてチェック
					const double size = u.TakasaUnit; // もしくは画像サイズ
					const RectF unitRect(center.movedBy(-size / 2, -size / 2), size, size);

					// 画面外なら描画しない
					// ユニットが部分的にでも画面にかかっていれば描画すべき
					if (!cameraView.intersects(unitRect))
						continue;

					if (!u.IsBuilding)
						TextureAsset(ringA).drawAt(center.movedBy(0, RING_OFFSET_Y1));

					TextureAsset(u.ImageName).draw(Arg::center = center);

					if (u.IsSelect)
						TextureAsset(u.ImageName)
						.draw(Arg::center = center)
						.drawFrame(BUILDING_FRAME_THICKNESS, Palette::Red);

					if (!u.IsBuilding)
						TextureAsset(ringB).drawAt(center.movedBy(0, RING_OFFSET_Y2));

					if (!u.IsBuilding)
					{
						u.bLiquidBarBattle.draw(ColorF{ 0.9, 0.1, 0.1 }, ColorF{ 0.7, 0.05, 0.05 }, ColorF{ 0.9, 0.5, 0.1 });
					}
					else
					{
						u.bLiquidBarBattle.draw(ColorF{ 0.5, 0.1, 1.0 }, ColorF{ 0.7, 0.05, 0.05 }, ColorF{ 0.9, 0.5, 0.1 });
					}

					if (u.moveState == moveState::FlagMoveCalc || u.moveState == moveState::MoveAI)
					{
						const Vec2 exclamationPos = center.movedBy(0, -u.TakasaUnit / 2 - EXCLAMATION_OFFSET_Y);
						Color color = (u.moveState == moveState::FlagMoveCalc) ? Palette::Orange : Palette::Red;
						fontInfo.font(U"！").drawAt(exclamationPos, color);
					}
				}
			}
		};

	drawGroup(classBattleManage.listOfAllUnit, U"ringA.png", U"ringB.png", false);
	drawGroup(classBattleManage.listOfAllEnemyUnit, U"ringA_E.png", U"ringB_E.png", true);
}
/// @brief リソースポイントをカメラビュー内に描画します。
/// @param cameraView 描画範囲を指定するカメラの矩形領域。
/// @param classBattleManage バトルの状態やマップデータを管理するクラス。
/// @param mapTile タイル座標や描画位置の計算に使用するマップタイル情報。
void Battle001::drawResourcePoints(const RectF& cameraView, const ClassBattle& classBattleManage, const MapTile mapTile) const
{
	if (!classBattleManage.classMapBattle)
		return;

	const auto& mapData = classBattleManage.classMapBattle.value().mapData;

	const int32 mapSize = static_cast<int32>(mapData.size());

	for (int32 x = 0; x < mapSize; ++x)
	{
		for (int32 y = 0; y < static_cast<int32>(mapData[x].size()); ++y)
		{
			const auto& tile = mapData[x][y];

			if (!tile.isResourcePoint)
				continue;

			const Vec2 pos = mapTile.ToTileBottomCenter(Point(x, y), mapTile.N);

			if (!cameraView.intersects(pos))
				continue;

			// アイコンの描画
			TextureAsset(tile.resourcePointIcon).draw(Arg::bottomCenter = pos.movedBy(0, -mapTile.TileThickness));

			// 所有者に応じて円枠の色を変える
			const ColorF circleColor = (tile.whichIsThePlayer == BattleWhichIsThePlayer::Sortie)
				? ColorF{ Palette::Aqua }
			: ColorF(Palette::Red);

			Circle(pos.movedBy(0, -mapTile.TileThickness - mapTile.TileOffset.y)
					, RESOURCE_CIRCLE_RADIUS)
				.drawFrame(CIRCLE_FRAME_THICKNESS, 0, circleColor);
		}
	}
}
/// @brief 選択範囲の矩形または矢印を描画します。
void Battle001::drawSelectionRectangleOrArrow() const
{
	if (!MouseR.pressed())
		return;

	if (!is移動指示)
	{
		const double offset = Scene::DeltaTime() * 10;
		const Rect rect{ cursPos, Cursor::Pos() - cursPos };
		rect.top().draw(LineStyle::SquareDot(offset), SELECTION_THICKNESS, Palette::Orange);
		rect.right().draw(LineStyle::SquareDot(offset), SELECTION_THICKNESS, Palette::Orange);
		rect.bottom().draw(LineStyle::SquareDot(offset), SELECTION_THICKNESS, Palette::Orange);
		rect.left().draw(LineStyle::SquareDot(offset), SELECTION_THICKNESS, Palette::Orange);
	}
	else
	{
		Line{ cursPos, Cursor::Pos() }.drawArrow(ARROW_THICKNESS, ARROW_HEAD_SIZE, Palette::Orange);
	}
}
/// @brief 指定タイルに建築可能かチェック
/// @param tile タイル座標
/// @return 建築可能ならtrue
bool Battle001::canBuildOnTile(const Point& tile, const ClassBattle& classBattleManage, const MapTile& mapTile) const
{
	// マップ範囲外チェック
	if (tile.x < 0 || tile.x >= mapTile.N || tile.y < 0 || tile.y >= mapTile.N)
	{
		return false;
	}

	// 建物が既に存在するかチェック
	for (const auto& group : classBattleManage.hsMyUnitBuilding)
	{
		if (group->initTilePos == tile && group->IsBattleEnable)
		{
			return false; // 既に建物が存在
		}
	}

	// 敵の建物もチェック
	for (const auto& group : classBattleManage.hsEnemyUnitBuilding)
	{
		if (group->initTilePos == tile && group->IsBattleEnable)
		{
			return false; // 既に建物が存在
		}
	}

	// ユニットが存在するかチェック
	for (const auto& group : classBattleManage.listOfAllUnit)
	{
		for (const auto& unit : group.ListClassUnit)
		{
			if (!unit.IsBuilding && unit.IsBattleEnable)
			{
				Optional<Point> unitTilePos = mapTile.ToIndex(unit.GetNowPosiCenter(), mapTile.columnQuads, mapTile.rowQuads);
				if (unitTilePos && *unitTilePos == tile)
				{
					return false; // ユニットが存在
				}
			}
		}
	}

	// 地形チェック（水タイルなど建築不可地形があれば追加）
	// TODO: mapDataから地形情報を取得して建築可否を判定

	return true; // 建築可能
}
/// @brief 範囲選択されたタイル配列を取得（線状選択版）
/// @param start 開始タイル座標
/// @param end 終了タイル座標
/// @return 選択範囲内のタイル座標配列（線状）
Array<Point> Battle001::getRangeSelectedTiles(const Point& start, const Point& end, const MapTile mapTile) const
{
	Array<Point> selectedTiles;

	// 開始点と終了点の差分を計算
	int32 deltaX = end.x - start.x;
	int32 deltaY = end.y - start.y;

	// どちらの方向により大きく動いたかを判定
	bool isHorizontalDominant = Abs(deltaX) >= Abs(deltaY);

	if (isHorizontalDominant)
	{
		// 横方向が主軸：水平線を描画
		int32 minX = Min(start.x, end.x);
		int32 maxX = Max(start.x, end.x);
		int32 fixedY = start.y; // Y座標は開始点で固定

		for (int32 x = minX; x <= maxX; ++x)
		{
			// マップ範囲内かチェック
			if (x >= 0 && x < mapTile.N && fixedY >= 0 && fixedY < mapTile.N)
			{
				selectedTiles.push_back(Point(x, fixedY));
			}
		}

		Print << U"水平線選択: Y={}, X={}〜{} ({} タイル)"_fmt(
			fixedY, minX, maxX, selectedTiles.size());
	}
	else
	{
		// 縦方向が主軸：垂直線を描画
		int32 minY = Min(start.y, end.y);
		int32 maxY = Max(start.y, end.y);
		int32 fixedX = start.x; // X座標は開始点で固定

		for (int32 y = minY; y <= maxY; ++y)
		{
			// マップ範囲内かチェック
			if (fixedX >= 0 && fixedX < mapTile.N && y >= 0 && y < mapTile.N)
			{
				selectedTiles.push_back(Point(fixedX, y));
			}
		}

		Print << U"垂直線選択: X={}, Y={}〜{} ({} タイル)"_fmt(
			fixedX, minY, maxY, selectedTiles.size());
	}

	return selectedTiles;
}

void Battle001::drawBuildTargetHighlight(const MapTile& mapTile) const
{
	if (IsBuildSelectTraget)
	{
		if (const auto index = mapTile.ToIndex(Cursor::PosF(), mapTile.columnQuads, mapTile.rowQuads))
		{
			// 右クリック範囲選択中の処理
			if (MouseR.pressed() && longBuildSelectTragetId != -1)
			{
				// 開始点の取得
				if (auto startIndex = mapTile.ToIndex(cursPos, mapTile.columnQuads, mapTile.rowQuads))
				{
					Point startTile = *startIndex;
					Point endTile = *index;

					// **線状選択のプレビュー**
					Array<Point> previewTiles = getRangeSelectedTiles(startTile, endTile, mapTile);

					// 選択方向の表示
					int32 deltaX = endTile.x - startTile.x;
					int32 deltaY = endTile.y - startTile.y;
					bool isHorizontal = Abs(deltaX) >= Abs(deltaY);

					String directionText = isHorizontal ? U"水平線" : U"垂直線";
					ColorF lineColor = isHorizontal ?
						ColorF{ 0.0, 0.8, 1.0, 0.6 } :  // 水平：シアン
						ColorF{ 1.0, 0.8, 0.0, 0.6 };   // 垂直：オレンジ

					// 線状選択プレビューの描画
					for (const auto& tile : previewTiles)
					{
						// 建築可能かどうかで色を調整
						ColorF tileColor = canBuildOnTile(tile, classBattleManage, mapTile) ?
							lineColor :  // 元の方向色
							ColorF{ 1.0, 0.2, 0.2, 0.4 };   // 赤半透明：建築不可

						mapTile.ToTile(tile, mapTile.N).draw(tileColor);
					}

					// 方向線の描画（開始点→終了点）
					{
						Vec2 startPos = mapTile.ToTileBottomCenter(startTile, mapTile.N);
						Vec2 endPos = mapTile.ToTileBottomCenter(endTile, mapTile.N);

						// 方向を示すアロー
						Line{ startPos, endPos }.drawArrow(
							DIRECTION_ARROW_THICKNESS,
							DIRECTION_ARROW_HEAD_SIZE,
							ColorF{ 1.0, 1.0, 0.0, 0.9 }
						);
					}

					// 選択情報の表示
					{
						int32 validCount = 0;
						int32 totalCount = previewTiles.size();

						for (const auto& tile : previewTiles)
						{
							if (canBuildOnTile(tile, classBattleManage, mapTile)) validCount++;
						}

						// カーソル近くに情報表示
						const String infoText = U"{}: {}/{} タイル"_fmt(
							directionText, validCount, totalCount);
						const Vec2 textPos = Cursor::PosF().movedBy(INFO_TEXT_OFFSET_X, INFO_TEXT_OFFSET_Y);

						// 背景を描画
						const auto textRegion = fontInfo.font(infoText).region();
						const RectF bgRect = RectF(textPos, textRegion.size).stretched(INFO_TEXT_PADDING);
						bgRect.draw(ColorF{ 0.0, 0.0, 0.0, 0.8 });

						// テキストを描画
						fontInfo.fontSkill(infoText).draw(textPos, Palette::White);
					}
				}
			}
			else
			{
				// 通常の単一タイルハイライト
				mapTile.ToTile(*index, mapTile.N).draw(ColorF{ 1.0, 1.0, 0.2, 0.3 });
			}
		}
	}
}
/// @brief スキル選択UIを描画
void Battle001::drawSkillUI() const
{
	const int32 baseY = Scene::Size().y - renderTextureSkill.height() - underBarHeight;

	renderTextureSkill.draw(0, baseY);
	renderTextureSkillUP.draw(0, baseY);

	if (!nowSelectSkillSetumei.isEmpty())
	{
		rectSkillSetumei.draw(Palette::Black);
		fontInfo.fontSkill(nowSelectSkillSetumei).draw(rectSkillSetumei.stretched(-12), Palette::White);
	}
}
/// @brief 選択中のビルドの説明を描画
void Battle001::drawBuildDescription() const
{
	if (nowSelectBuildSetumei != U"")
	{
		rectSetumei.draw(Palette::Black);
		fontInfo.fontSkill(nowSelectBuildSetumei).draw(rectSetumei.stretched(-12), Palette::White);
	}
}
/// @brief ビルドメニューと建築キューを描画
void Battle001::drawBuildMenu() const
{
	if (!IsBuildMenuHome)
	{
		//ミニマップの邪魔
		//renderTextureBuildMenuEmpty.draw(Scene::Size().x - 328, Scene::Size().y - 328 - underBarHeight);
		return;
	}

	const int32 baseX = Scene::Size().x - 328;
	const int32 baseY = Scene::Size().y - 328 - underBarHeight;

	// 選択されたユニットを1回の検索で取得
	String targetClassBuild = U"";
	Array<BuildAction> arrYoyakuBuild;
	Stopwatch taskTimer;
	{
		for (auto& group : classBattleManage.listOfAllUnit)
		{
			for (auto& unit : group.ListClassUnit)
			{
				if (unit.IsSelect)
				{
					targetClassBuild = unit.classBuild;
					arrYoyakuBuild = unit.arrYoyakuBuild;
					taskTimer = unit.taskTimer;
					break;
				}
			}
			if (targetClassBuild != U"") break;
		}
	}

	//　建物ユニットチェック
	for (const auto& unit : classBattleManage.hsMyUnitBuilding)
	{
		if (unit->IsSelect)
		{
			targetClassBuild = unit->classBuild;
			arrYoyakuBuild = unit->arrYoyakuBuild;
			taskTimer = unit->taskTimer;
			break;
		}
	}

	if (targetClassBuild == U"")
		return;

	// ビルドメニューの描画
	for (const auto& [key, renderTexture] : htBuildMenuRenderTexture)
	{
		if (key == targetClassBuild)
		{
			renderTexture.draw(baseX, baseY);
			break;
		}
	}

	Rect(baseX - 64 - 6, baseY, 70, 328).drawFrame(4, 0, Palette::Black);

	// 建築キューの描画
	if (!arrYoyakuBuild.empty())
	{
		for (const auto& [i, buildItem] : Indexed(arrYoyakuBuild))
		{
			if (i == 0)
			{
				// 現在建築中のアイテム
				TextureAsset(buildItem.icon).resized(64).draw(baseX - 64, baseY + 4);

				const double progressRatio = Saturate(taskTimer.sF() / buildItem.buildTime);
				const double gaugeHeight = 64 * Max(progressRatio, 0.1);

				RectF{ baseX - 64, baseY + 4, 64, gaugeHeight }.draw(ColorF{ 0.0, 0.5 });
			}
			else
			{
				// キューに入っているアイテム
				TextureAsset(buildItem.icon)
					.resized(32)
					.draw(baseX - 32, baseY + 32 + (i * 32) + 4);
			}
		}
	}
}
/// @brief リソース（Gold、Trust、Food）のUIを描画します。リソース値が変更された場合のみ表示テキストを更新
void Battle001::drawResourcesUI() const
{
	// リソース値の変更時のみ文字列を更新（キャッシュ機構を検討）
	static int32 cachedGold = -1;
	static int32 cachedTrust = -1;
	static int32 cachedFood = -1;
	static Array<String> cachedTexts;

	if (gold != cachedGold || trust != cachedTrust || food != cachedFood)
	{
		cachedTexts = {
			U"Gold:{0}"_fmt(gold),
			U"Trust:{0}"_fmt(trust),
			U"Food:{0}"_fmt(food)
		};
		cachedGold = gold;
		cachedTrust = trust;
		cachedFood = food;
	}

	int32 baseX = 0;
	int32 baseY = 0;

	if (longBuildSelectTragetId != -1)
	{
		baseX = Scene::Size().x - 328 - int32(fontInfo.fontSystem(cachedTexts[0]).region().w) - 64 - 6;
		baseY = Scene::Size().y - 328 - 30;
	}

	for (size_t i = 0; i < cachedTexts.size(); ++i)
	{
		const String& text = cachedTexts[i];
		const auto region = fontInfo.fontSystem(text).region();

		const Rect rect{
			baseX,
			baseY + static_cast<int32>(i * region.h),
			static_cast<int32>(region.w),
			static_cast<int32>(region.h)
		};

		rect.draw(Palette::Black);
		fontInfo.fontSystem(text).drawAt(rect.center(), Palette::White);
	}
}
void Battle001::DrawMiniMap(const Grid<Visibility>& map, const RectF& cameraRect) const
{
	//何故このサイズだとちょうどいいのかよくわかっていない
	//pngファイルは100*65
	//ミニマップ描写方法と通常マップ描写方法は異なるので、無理に合わせなくて良い？
	const Vec2 tileSize = Vec2(50, 25 + 25);

	// --- ミニマップに表示する位置・サイズ
	const Vec2 miniMapTopLeft = miniMapPosition;
	const SizeF miniMapDrawArea = miniMapSize;

	// --- マップの4隅をアイソメ変換 → 範囲を取得
	const Vec2 isoA = mapTile.ToIso(0, 0) * tileSize;
	const Vec2 isoB = mapTile.ToIso(map.width(), 0) * tileSize;
	const Vec2 isoC = mapTile.ToIso(0, map.height()) * tileSize;
	const Vec2 isoD = mapTile.ToIso(map.width(), map.height()) * tileSize;

	const double minX = Min({ isoA.x, isoB.x, isoC.x, isoD.x });
	const double minY = Min({ isoA.y, isoB.y, isoC.y, isoD.y });
	const double maxX = Max({ isoA.x, isoB.x, isoC.x, isoD.x });
	const double maxY = Max({ isoA.y, isoB.y, isoC.y, isoD.y });

	const SizeF isoMapSize = Vec2(maxX - minX, maxY - minY);

	// --- ミニマップに収めるためのスケーリング
	const double scale = Min(miniMapDrawArea.x / isoMapSize.x, miniMapDrawArea.y / isoMapSize.y);

	// --- スケーリング後のマップサイズ
	const SizeF scaledMapSize = isoMapSize * scale;

	// --- 中央に表示するためのオフセット（ミニマップ内で中央揃え）
	const Vec2 offset = miniMapTopLeft + (miniMapDrawArea - scaledMapSize) / 2.0 - Vec2(minX, minY) * scale;

	// --- 背景（薄い色で下地）を描画
	RectF(miniMapTopLeft, miniMapDrawArea).draw(ColorF(0.1, 0.1, 0.1, 0.5));

	// --- タイルの描画
	for (int32 y = 0; y < map.height(); ++y)
	{
		for (int32 x = 0; x < map.width(); ++x)
		{
			const Vec2 iso = mapTile.ToIso(x, y) * tileSize;
			const Vec2 miniPos = iso * scale + offset;
			const SizeF tileSizeScaled = tileSize * scale;

			// デフォルト色
			ColorF tileColor = Palette::White;
			if (auto it = minimapCols.find(Point(x, y)); it != minimapCols.end())
			{
				tileColor = it->second;
			}

			// 描画
			RectF(miniPos, tileSizeScaled).draw(tileColor);
		}
	}

	// --- カメラ範囲を表示（白い枠）
	{
		//const Vec2 camTopLeftIso = ToIso(cameraRect.x, cameraRect.y) * tileSize;
		//const Vec2 camBottomRightIso = ToIso(cameraRect.x + cameraRect.w, cameraRect.y + cameraRect.h) * tileSize;

		//const Vec2 topLeft = camTopLeftIso * scale + offset;
		//const Vec2 bottomRight = camBottomRightIso * scale + offset;
		//const RectF cameraBox = RectF(topLeft, bottomRight - topLeft);

		//cameraBox.drawFrame(1.5, ColorF(1.0));

		const Vec2 camTopLeft = cameraRect.pos * Vec2(1.0, 1.0);
		const Vec2 camBottomRight = camTopLeft + cameraRect.size;
		RectF(camTopLeft * scale + offset, cameraRect.size * scale).drawFrame(1.5, ColorF(1.0));

	}
}
void Battle001::drawFormationUI() const
{
	renderTextureZinkei.draw(
		0,
		Scene::Size().y - renderTextureSkill.height() - renderTextureZinkei.height() - underBarHeight);
}

void Battle001::drawMinimap() const
{
	if (longBuildSelectTragetId == -1)
		DrawMiniMap(visibilityMap, camera.getRegion());
}

void Battle001::drawHUD() const
{
	drawFormationUI();
	drawSkillUI();
	drawBuildDescription();
	drawBuildMenu();
	drawResourcesUI();
	drawMinimap();

	// ユニット情報ツールチップの描画
	unitTooltip.draw();

	// --- 残り時間（右上） ---
	{
		const double elapsed = (gSurvivalStarted ? gSurvivalTimer.sF() : 0.0);
		const double remain = Max(0.0, kSurvivalDurationSec - elapsed);
		const String label = U"残り時間 " + formatTimeMMSS(remain);

		const auto region = fontInfo.fontSystem(label).region();
		const int32 x = Scene::Size().x - region.w - 16;
		const int32 y = 8;

		RectF{ x - 8, y - 6, region.w + 16, region.h + 12 }.draw(ColorF{ 0.0, 0.0, 0.0, 0.6 });
		fontInfo.fontSystem(label).draw(x, y, Palette::White);
	}
}

// 影描画専用ヘルパを追加（drawUnits / drawBuildings と同じスコープ付近に置く）
void Battle001::drawShadows(const RectF& cameraView, const ClassBattle& classBattleManage) const
{
	constexpr double kSquashY = 1.25;           // 縦潰し率
	constexpr double kRotateDeg = 35.0;         // 影の寝かせ角（光源が斜め上から来る想定）
	constexpr Vec2   kOffset{ 20, -10 };         // 足から伸びる方向オフセット（右下方向）
	constexpr ColorF kShadowColor{ 0.0, 0.0, 0.0, 0.82 };

	auto getFacingRad = [](const Unit& u, const Vec2& center) -> double {
		if (!u.vecMove.isZero()) return Math::Atan2(u.vecMove.y, u.vecMove.x);
		Vec2 toOrder = (u.GetOrderPosiCenter() - center);
		if (!toOrder.isZero()) return Math::Atan2(toOrder.y, toOrder.x);
		return 0.0;
		};

	auto isTileVisible = [&](const Vec2& worldPos) -> bool
		{
			if (const auto idx = mapTile.ToIndex(worldPos, mapTile.columnQuads, mapTile.rowQuads))
			{
				return (visibilityMap[*idx] == Visibility::Visible);
			}
			return false; // マップ外などは非表示扱い
		};

	auto drawUnitShadowGroup = [&](const Array<ClassHorizontalUnit>& groups, bool cullByFog) {
		for (const auto& gh : groups) {
			for (const auto& u : gh.ListClassUnit) {
				if (u.IsBuilding || !u.IsBattleEnable) continue;

				const Vec2 center = u.GetNowPosiCenter();
				// 敵ユニットのみ霧でカリング
				if (cullByFog)
				{
					if (!isTileVisible(center))
						continue;
				}

				const double logicalHalfH = u.TakasaUnit * 0.5;

				// ユニット矩形（可視判定）
				RectF unitRect(center.movedBy(-u.yokoUnit / 2.0, -logicalHalfH),
							   u.yokoUnit, u.TakasaUnit);
				if (!cameraView.intersects(unitRect)) continue;

				// 足位置（本体中心基準で下端）
				// 画像の高さと TakasaUnit が異なるなら Texture の高さで再算出する:
				const Texture& tex = TextureAsset(u.ImageName);
				const Vec2 footAnchor = center.movedBy(0, tex.size().y * 0.5); // 本体描画が center 基準なので底辺 = center + (高さ/2)

				// 影行列（足位置を pivot にして潰し→回転→オフセット）
				const ScopedColorMul2D mul{ kShadowColor };
				const Transformer2D xf{
					Mat3x2::Scale(1.0, kSquashY, footAnchor)
					* Mat3x2::Rotate(Math::ToRadians(kRotateDeg), footAnchor)
				};

				// 影本体（底辺を footAnchor に揃える）
				tex.draw(Arg::bottomCenter = footAnchor);
			}
		}
		};

	auto drawBuildingShadowSet = [&](const HashSet<std::shared_ptr<Unit>>& buildings) {
		for (const auto& sp : buildings) {
			const Unit& b = *sp;
			if (!b.IsBattleEnable) continue;

			Vec2 basePos = mapTile.ToTileBottomCenter(Point(b.colBuilding, b.rowBuilding), mapTile.N)
				.movedBy(0, -mapTile.TileThickness);

			if (!cameraView.intersects(basePos)) continue;

			const Texture& tex = TextureAsset(b.ImageName);
			const Vec2 footAnchor = basePos; // 建物は already bottomCenter で描画している前提

			const ScopedColorMul2D mul{ kShadowColor };
			const Transformer2D xf{
				Mat3x2::Scale(1.0, kSquashY, footAnchor)
				* Mat3x2::Translate(kOffset)
			};

			tex.draw(Arg::bottomCenter = footAnchor);
		}
		};

	drawUnitShadowGroup(classBattleManage.listOfAllUnit, false);
	drawUnitShadowGroup(classBattleManage.listOfAllEnemyUnit, true);
	drawBuildingShadowSet(classBattleManage.hsMyUnitBuilding);
	drawBuildingShadowSet(classBattleManage.hsEnemyUnitBuilding);
}
void Battle001::draw() const
{
	FsScene::draw();

	//背景レイヤ（画面座標）
	drawBackgroundGradient();
	//パターン背景（bg_pattern.png）
	drawParallaxPattern(camera);
	//（簡易ビネット）
	RectF{ 0,0, Scene::Width(), Scene::Height() }.draw(ColorF{ 0,0,0,0.5 });

	{
		const auto tr = camera.createTransformer();
		const ScopedRenderStates2D blend{ BlendState::Premultiplied };
		const RectF cameraView = getCameraView(camera, mapTile);

		// 背景や地形
		drawTileMap(cameraView, mapTile, classBattleManage);
		drawFog(cameraView, mapTile, visibilityMap);
		drawResourcePoints(cameraView, classBattleManage, mapTile);
		resourcePointTooltip.draw();
		drawSelectionRectangleOrArrow();
		drawBuildTargetHighlight(mapTile);

		// 弾など既存描画
		for (auto& skill : m_Battle_player_skills)
		{
			for (auto& acb : skill.ArrayClassBullet)
			{
				if (skill.classSkill.image == U"")
				{
					Circle{ acb.NowPosition.x,acb.NowPosition.y,30 }.draw();
					continue;
				}

				if (skill.classSkill.SkillForceRay == SkillForceRay::on)
				{
					Line{ acb.StartPosition, acb.NowPosition }.draw(skill.classSkill.rayStrokeThickness, ColorF{ skill.classSkill.ray[1], skill.classSkill.ray[2], skill.classSkill.ray[3], skill.classSkill.ray[0] });
				}

				if (skill.classSkill.SkillD360 == SkillD360::on)
				{
					if (skill.classSkill.SkillCenter == SkillCenter::end)
					{
						const Texture texture = TextureAsset(skill.classSkill.image + U".png");
						texture
							.resized(skill.classSkill.w, skill.classSkill.h)
							.rotatedAt(texture.resized(skill.classSkill.w, skill.classSkill.h).region().bottomCenter(), acb.radian + Math::ToRadians(90))
							.drawAt(acb.NowPosition);
					}
					else
					{
						TextureAsset(skill.classSkill.image + U".png")
							.resized(skill.classSkill.w, skill.classSkill.h)
							.rotated(acb.lifeTime * 10)
							.drawAt(acb.NowPosition);
					}
					continue;
				}

				if (acb.degree == 0 || acb.degree == 90 || acb.degree == 180 || acb.degree == 270)
				{
					TextureAsset(skill.classSkill.image + U"N.png")
						.resized(skill.classSkill.w, skill.classSkill.h)
						.rotated(acb.radian + Math::ToRadians(90))
						.drawAt(acb.NowPosition);
					continue;
				}

				TextureAsset(skill.classSkill.image + U"NW.png")
					.resized(skill.classSkill.w, skill.classSkill.h)
					.rotated(acb.radian + Math::ToRadians(135))
					.drawAt(acb.NowPosition);
			}
		}
		for (auto& skill : m_Battle_enemy_skills)
		{
			for (auto& acb : skill.ArrayClassBullet)
			{
				if (skill.classSkill.image == U"")
				{
					Circle{ acb.NowPosition.x,acb.NowPosition.y,30 }.draw();
					continue;
				}

				if (skill.classSkill.SkillForceRay == SkillForceRay::on)
				{
					Line{ acb.StartPosition, acb.NowPosition }.draw(skill.classSkill.rayStrokeThickness, ColorF{ skill.classSkill.ray[1], skill.classSkill.ray[2], skill.classSkill.ray[3], skill.classSkill.ray[0] });
				}

				if (skill.classSkill.SkillD360 == SkillD360::on)
				{
					if (skill.classSkill.SkillCenter == SkillCenter::end)
					{
						const Texture texture = TextureAsset(skill.classSkill.image + U".png");
						texture
							.resized(skill.classSkill.w, skill.classSkill.h)
							.rotatedAt(texture.resized(skill.classSkill.w, skill.classSkill.h).region().bottomCenter(), acb.radian + Math::ToRadians(90))
							.drawAt(acb.NowPosition);
					}
					else
					{
						TextureAsset(skill.classSkill.image + U".png")
							.resized(skill.classSkill.w, skill.classSkill.h)
							.rotated(acb.lifeTime * 10)
							.drawAt(acb.NowPosition);
					}
					continue;
				}

				if (acb.degree == 0 || acb.degree == 90 || acb.degree == 180 || acb.degree == 270)
				{
					TextureAsset(skill.classSkill.image + U"N.png")
						.resized(skill.classSkill.w, skill.classSkill.h)
						.rotated(acb.radian + Math::ToRadians(90))
						.drawAt(acb.NowPosition);
					continue;
				}

				TextureAsset(skill.classSkill.image + U"NW.png")
					.resized(skill.classSkill.w, skill.classSkill.h)
					.rotated(acb.radian + Math::ToRadians(135))
					.drawAt(acb.NowPosition);
			}
		}

		// 影 → 本体の順
		drawShadows(cameraView, classBattleManage);
		drawBuildings(cameraView, classBattleManage, mapTile);
		drawUnits(cameraView, classBattleManage);
	}
	drawHUD();
}
