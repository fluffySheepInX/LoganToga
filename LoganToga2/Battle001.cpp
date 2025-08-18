#include "stdafx.h"
#include "Battle001.h"
#include <ranges>
#include <vector>

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

ClassMapBattle Battle001::GetClassMapBattle(ClassMap cm, CommonConfig& commonConfig)
{
	ClassMapBattle cmb;
	cmb.name = cm.name;
	for (String aaa : cm.data)
	{
		Array<MapDetail> aMd;
		Array s = aaa.split(U',');
		for (auto bbb : s)
		{
			MapDetail md;

			// RESOURCE:XXXパターンをチェック
			if (bbb.includes(U"RESOURCE:"))
			{
				// RESOURCE:XXXを抽出
				auto resourceStart = bbb.indexOf(U"RESOURCE:");
				if (resourceStart != String::npos)
				{
					auto resourceEnd = bbb.indexOf(U',', resourceStart);
					if (resourceEnd == String::npos) resourceEnd = bbb.length();

					String resourcePattern = bbb.substr(resourceStart, resourceEnd - resourceStart);
					String resourceName = resourcePattern.substr(9); // "RESOURCE:"を除去

					// commonConfig.htResourceDataから対応するリソース情報を取得
					if (commonConfig.htResourceData.contains(resourceName))
					{
						const ClassResource& resource = commonConfig.htResourceData[resourceName];

						// MapDetailにリソース情報を設定
						md.isResourcePoint = true;
						md.resourcePointType = resource.resourceType;
						md.resourcePointAmount = resource.resourceAmount;
						md.resourcePointDisplayName = resource.resourceName;
						md.resourcePointIcon = resource.resourceIcon;
					}

					// RESOURCE:XXXをマップデータから除去
					bbb = bbb.replaced(resourcePattern, U"");
				}
			}

			Array splitA = bbb.split(U'*');
			//field(床画像
			md.tip = cm.ele[splitA[0]];
			//build(城壁や矢倉など
			if (splitA.size() > 1)
			{
				if (splitA[1] == U"")
				{

				}
				else
				{
					Array splitB = splitA[1].split(U'$');
					//1件だけの場合も考慮
					if (splitB.size() == 1)
					{
						Array splitWi = splitB[0].split(U':');
						String re = cm.ele[splitWi[0]];
						if (re != U"")
						{
							std::tuple<String, long, BattleWhichIsThePlayer> pp = { re,-1, BattleWhichIsThePlayer::None };
							if (splitWi.size() == 1)
							{
								pp = { re,-1, BattleWhichIsThePlayer::None };
							}
							else
							{
								if (splitWi[1] == U"sor")
								{
									pp = { re,-1, BattleWhichIsThePlayer::Sortie };
								}
								else if (splitWi[1] == U"def")
								{
									pp = { re,-1, BattleWhichIsThePlayer::Def };
								}
								else
								{
									pp = { re,-1, BattleWhichIsThePlayer::None };
								}
							}

							md.building.push_back(pp);
						}
					}
					else
					{
						for (String item : splitB)
						{
							Array splitWi = item.split(U':');
							String re = cm.ele[splitWi[0]];
							if (re != U"")
							{
								std::tuple<String, long, BattleWhichIsThePlayer> pp = { re,-1, BattleWhichIsThePlayer::None };
								if (splitWi.size() == 1)
								{
									pp = { re,-1, BattleWhichIsThePlayer::None };
								}
								else
								{
									if (splitWi[1] == U"sor")
									{
										pp = { re,-1, BattleWhichIsThePlayer::Sortie };
									}
									else if (splitWi[1] == U"def")
									{
										pp = { re,-1, BattleWhichIsThePlayer::Def };
									}
									else
									{
										pp = { re,-1, BattleWhichIsThePlayer::None };
									}
								}

								md.building.push_back(pp);
							}
						}
					}
				}
			}
			//ユニットの情報
			if (splitA.size() > 2)
			{
				Array re = splitA[2].split(U':');
				if (re.size() == 0 || re[0] == U"-1")
				{

				}
				else
				{
					md.unit = re[0];
					md.houkou = re[1];
					//
					//md.BattleWhichIsThePlayer = re[2];
				}
			}
			//【出撃、防衛、中立の位置】もしくは【退却位置】
			if (splitA.size() > 3)
			{
				md.posSpecial = splitA[3];
				if (md.posSpecial == U"@@")
				{
					md.kougekiButaiNoIti = true;
				}
				else if (md.posSpecial == U"@")
				{
					md.boueiButaiNoIti = true;
				}
				else
				{

				}
			}
			//陣形
			if (splitA.size() > 5)
			{
				md.zinkei = splitA[5];
			}
			aMd.push_back(std::move(md));
		}
		cmb.mapData.push_back(std::move(aMd));
	}

	return cmb;
}

/// @brief Battle001 クラスのコンストラクタ。ゲームデータや設定をもとにバトルマップを初期化し、リソース情報やツールチップの設定を行います。
/// @param saveData ゲームの進行状況などを保持する GameData 型の参照。
/// @param commonConfig 共通設定を保持する CommonConfig 型の参照。
/// @param argSS システム文字列情報を渡す SystemString 型の値。
Battle001::Battle001(GameData& saveData, CommonConfig& commonConfig, SystemString argSS)
	:FsScene(U"Battle")
	, m_saveData{ saveData }
	, m_commonConfig{ commonConfig }
	, ss{ argSS }
	, tempSelectComRight{ dummyMenu() }
{
	const TOMLReader tomlMap{ PATHBASE + PATH_DEFAULT_GAME + U"/016_BattleMap/map001.toml" };
	if (not tomlMap)
		throw Error{ U"Failed to load `tomlMap`" };

	ClassMap sM;
	for (const auto& table : tomlMap[U"Map"].tableArrayView()) {
		const String name = table[U"name"].get<String>();

		{
			int32 counter = 0;
			while (true)
			{
				String elementKey = U"ele{}"_fmt(counter);
				const String elementValue = table[elementKey].get<String>();
				sM.ele.emplace(elementKey, elementValue);
				counter++;
				if (elementValue == U"")
				{
					break;
				}
			}
		}
		{
			namespace views = std::views;
			const String str = table[U"data"].get<String>();
			for (const auto sv : str | views::split(U"$"_sv))
			{
				String processedString
					= ClassStaticCommonMethod::ReplaceNewLine(String(sv.begin(), sv.end()));
				if (processedString != U"")
				{
					sM.data.push_back(processedString);
				}
			}
		}
	}

	classBattleManage.classMapBattle = GetClassMapBattle(sM, commonConfig);

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
	MapTile mapTile,
	String unitName,
	int32 col,
	int32 row,
	int32 num,
	Array<ClassHorizontalUnit>& listU,
	bool enemy)
{
	// 事前に容量を確保
	listU.reserve(listU.size() + 1);

	// 該当するユニットテンプレートを検索
	auto it = std::find_if(m_commonConfig.arrayInfoUnit.begin(), m_commonConfig.arrayInfoUnit.end(),
		[&unitName](const auto& unit) { return unit.NameTag == unitName; });

	if (it == m_commonConfig.arrayInfoUnit.end())
	{
		Print << U"Warning: Unit '{}' not found in unit templates"_fmt(unitName);
		return;
	}

	if (num <= 0)
	{
		Print << U"Warning: Invalid unit count: {}"_fmt(num);
		return;
	}

	auto uu = *it; // 一度だけコピー
	ClassHorizontalUnit cuu;

	for (size_t i = 0; i < num; i++)
	{
		uu.ID = classBattleManage.getIDCount();
		uu.initTilePos = Point{ col, row };
		uu.colBuilding = col;
		uu.rowBuilding = row;
		uu.nowPosiLeft = mapTile.ToTile(uu.initTilePos, mapTile.N)
			.asPolygon()
			.centroid()
			.movedBy(-(uu.yokoUnit / 2), -(uu.TakasaUnit / 2));
		uu.taskTimer = Stopwatch();
		uu.taskTimer.reset();

		Vec2 temp = uu.GetNowPosiCenter().movedBy(-(LIQUID_BAR_WIDTH / 2), LIQUID_BAR_HEIGHT_POS);
		uu.bLiquidBarBattle = GameUIToolkit::LiquidBarBattle(Rect(temp.x, temp.y, LIQUID_BAR_WIDTH, LIQUID_BAR_HEIGHT));

		if (enemy)
			uu.moveState = moveState::MoveAI;

		cuu.ListClassUnit.push_back(uu);

		if (uu.IsBuilding)
		{
			std::scoped_lock lock(classBattleManage.unitListMutex);
			unitsForHsBuildingUnitForAstar.push_back(std::make_unique<Unit>(uu));
			hsBuildingUnitForAstar[uu.initTilePos].push_back(unitsForHsBuildingUnitForAstar.back().get());
			auto u = std::make_shared<Unit>(uu);
			classBattleManage.hsMyUnitBuilding.insert(u);
		}
	}

	{
		std::scoped_lock lock(classBattleManage.unitListMutex);
		listU.push_back(std::move(cuu));
	}
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

		UnitRegister(classBattleManage, mapTile, U"sniperP99", x, y, 1,
			classBattleManage.listOfAllEnemyUnit, true);

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
	std::scoped_lock lock(classBattleManage.unitListMutex);
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
ClassHorizontalUnit Battle001::getMovableUnits(Array<ClassHorizontalUnit>& source, BattleFormation bf)
{
	ClassHorizontalUnit result;
	std::scoped_lock lock(classBattleManage.unitListMutex);
	for (auto& target : source)
		for (auto& unit : target.ListClassUnit)
		{
			if (unit.Formation == bf && unit.moveState == moveState::FlagMoveCalc && unit.IsBattleEnable == true)
				result.ListClassUnit.push_back(unit);
		}

	return result;
}

void Battle001::handleDenseFormation(Point end)
{
	std::scoped_lock lock(classBattleManage.unitListMutex);
	for (auto& target : classBattleManage.listOfAllUnit)
		for (auto& unit : target.ListClassUnit)
		{
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
	std::scoped_lock lock(classBattleManage.unitListMutex);
	ClassHorizontalUnit liZenei;
	liZenei = getMovableUnits(classBattleManage.listOfAllUnit, BattleFormation::F);
	ClassHorizontalUnit liKouei;
	liKouei = getMovableUnits(classBattleManage.listOfAllUnit, BattleFormation::B);
	ClassHorizontalUnit liKihei;
	liKihei = getMovableUnits(classBattleManage.listOfAllUnit, BattleFormation::M);
	Array<ClassHorizontalUnit> lisClassHorizontalUnitLoop;
	lisClassHorizontalUnitLoop.push_back(liZenei);
	lisClassHorizontalUnitLoop.push_back(liKouei);
	lisClassHorizontalUnitLoop.push_back(liKihei);

	for (auto&& [i, loopLisClassHorizontalUnit] : IndexedRef(lisClassHorizontalUnitLoop))
	{
		Array<Unit*> target;
		for (auto& unit : loopLisClassHorizontalUnit.ListClassUnit)
			if (unit.moveState == moveState::FlagMoveCalc && unit.IsBattleEnable == true)
				target.push_back(&unit);
		if (target.size() == 0) continue;

		AssignUnitsInFormation(target, start, end, i);

		if (target.size() == 1) continue;
		auto pos = calcLastMerge(target, [](const Unit* u) { return u->GetOrderPosiCenter(); });
		auto pos2 = calcLastMerge(target, [](const Unit* u) { return u->GetNowPosiCenter(); });
		setMergePos(target, &Unit::setFirstMergePos, pos2);
		setMergePos(target, &Unit::setLastMergePos, pos);
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
void Battle001::handleUnitSelection(const RectF& selectionRect)
{
	std::scoped_lock lock(classBattleManage.unitListMutex);
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
				is移動指示 = true;
			}
		}
	}
}
Co::Task<void> Battle001::handleRightClickUnitActions(Point start, Point end)
{
	if (is移動指示 == true)
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

		aStar.abortAStarMyUnits = false;
	}
	else
	{
		////範囲選択
		// 範囲選択の矩形を生成（start, endの大小関係を吸収）
		const RectF selectionRect = RectF::FromPoints(start, end);
		handleUnitSelection(selectionRect);
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

		//for (auto ttt : classBattleManage.classMapBattle.value().mapData)
		//{
		//	for (auto jjj : ttt)
		//	{
		//		if (jjj.whichIsThePlayer == BattleWhichIsThePlayer::Sortie)
		//		{
		//			//資金の増加
		//			switch (jjj.resourcePointType)
		//			{
		//			case resourceKind::Gold:
		//				goldInc += jjj.resourcePointAmount;
		//				break;
		//			case resourceKind::Trust:
		//				trustInc += jjj.resourcePointAmount;
		//				break;
		//			case resourceKind::Food:
		//				foodInc += jjj.resourcePointAmount;
		//				break;
		//			default:
		//				break;
		//			}
		//		}
		//	}
		//}

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
/// @brief Battle001のUIを初期化 陣形・スキル・建築メニュー・ミニマップなどの描画用リソースを生成・設定
void Battle001::initUI()
{
	rectZinkei.push_back(Rect{ 8,8,60,40 });
	rectZinkei.push_back(Rect{ 76,8,60,40 });
	rectZinkei.push_back(Rect{ 144,8,60,40 });
	renderTextureZinkei = RenderTexture{ 320,60 };
	renderTextureZinkei.clear(ColorF{ 0.5, 0.0 });
	{
		const ScopedRenderTarget2D target{ renderTextureZinkei.clear(ColorF{ 0.8, 0.8, 0.8,0.5 }) };
		const ScopedRenderStates2D blend{ MakeBlendState() };

		Rect df = Rect(320, 60);
		df.drawFrame(4, 0, ColorF{ 0.5 });

		for (auto&& [i, ttt] : Indexed(rectZinkei))
		{
			ttt.draw(Palette::Aliceblue);
			fontInfo.fontZinkei(ss.Zinkei[i]).draw(ttt, Palette::Black);
		}
	}

	renderTextureSkill = RenderTexture{ 320,320 };
	renderTextureSkill.clear(ColorF{ 0.5, 0.0 });
	{
		const ScopedRenderTarget2D target{ renderTextureSkill.clear(ColorF{ 0.8, 0.8, 0.8,0.5 }) };
		const ScopedRenderStates2D blend{ MakeBlendState() };

		//skill抽出
		Array<Skill> table;
		for (auto& item : classBattleManage.listOfAllUnit)
		{
			for (auto& itemUnit : item.ListClassUnit)
			{
				for (auto& ski : itemUnit.arrSkill)
					table.push_back(ski);
			}
		}

		// ソート
		table.sort_by([](const Skill& a, const Skill& b)
		{
			return a.sortKey < b.sortKey;
		});
		table.erase(std::unique(table.begin(), table.end()), table.end());

		for (const auto&& [i, key] : Indexed(table))
		{
			Rect rectSkill;
			rectSkill.x = ((i % 10) * 32) + 4;
			rectSkill.y = ((i / 10) * 32) + 4;
			rectSkill.w = 32;
			rectSkill.h = 32;
			htSkill.emplace(key.nameTag, rectSkill);

			for (auto& icons : key.icon.reversed())
			{
				TextureAsset(icons.trimmed()).resized(32).draw(rectSkill.x, rectSkill.y);
			}
		}

		Rect df = Rect(320, 320);
		df.drawFrame(4, 0, ColorF{ 0.5 });
	}
	renderTextureSkillUP = RenderTexture{ 320,320 };
	renderTextureSkillUP.clear(ColorF{ 0.5, 0.0 });

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

/// @brief UIエリアによる選択キャンセルをチェックし、必要に応じて選択状態を解除
/// @return 非同期タスク（Co::Task<>）を返します。UIエリアでキャンセル条件が満たされた場合、ユニットの選択状態を解除
Co::Task<> Battle001::checkCancelSelectionByUIArea()
{
	if (Cursor::PosF().y >= Scene::Size().y - underBarHeight)
	{
		longBuildSelectTragetId = -1;
		std::scoped_lock lock(classBattleManage.unitListMutex);
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
				// 周囲3マス範囲のランダムユニット格納処理

					// 現在選択されているユニットを取得
				Unit* selectedCarrierUnit = nullptr;
				{
					std::scoped_lock lock(classBattleManage.unitListMutex);
					for (auto& loau : classBattleManage.listOfAllUnit)
					{
						for (auto& unit : loau.ListClassUnit)
						{
							if (unit.IsSelect)
							{
								selectedCarrierUnit = &unit;
								break;
							}
						}
						if (selectedCarrierUnit) break;
					}
				}

				if (!selectedCarrierUnit) continue;

				// キャリアーコンポーネントを取得
				auto* carrierComponent = selectedCarrierUnit->getComponent<CarrierComponent>();
				if (!carrierComponent) continue;

				// 選択ユニットの現在位置のタイル座標を取得
				Optional<Point> carrierTileIndex = mapTile.ToIndex(
					selectedCarrierUnit->GetNowPosiCenter(), mapTile.columnQuads, mapTile.rowQuads);
				if (!carrierTileIndex.has_value()) continue;

				// 周囲3マス範囲のユニットを検索
				Array<Unit*> nearbyUnits;
				const int32 searchRadius = 3;

				// 味方ユニットから検索
				{
					std::scoped_lock lock(classBattleManage.unitListMutex);
					for (auto& group : classBattleManage.listOfAllUnit)
					{
						for (auto& unit : group.ListClassUnit)
						{
							// 自分自身、建物、戦闘不可ユニットは除外
							if (unit.ID == selectedCarrierUnit->ID ||
								unit.IsBuilding ||
								!unit.IsBattleEnable || unit.isCarrierUnit) continue;

							// ユニットの現在位置のタイル座標を取得
							Optional<Point> unitTileIndex = mapTile.ToIndex(
								unit.GetNowPosiCenter(), mapTile.columnQuads, mapTile.rowQuads);
							if (!unitTileIndex.has_value()) continue;

							// 距離をチェック（マンハッタン距離）
							int32 distance = carrierTileIndex->manhattanDistanceFrom(*unitTileIndex);
							if (distance <= searchRadius)
							{
								nearbyUnits.push_back(&unit);
							}
						}
					}
				}

				// 範囲内にユニットがいない場合は処理終了
				if (nearbyUnits.isEmpty())
				{
					Print << U"周囲3マス以内にユニットが見つかりません";
					continue;
				}

				// ランダムに並び替え
				Shuffle(nearbyUnits);

				// キャリアーの容量まで格納
				int32 storedCount = 0;
				for (Unit* unit : nearbyUnits)
				{
					if (carrierComponent->store(unit))
					{
						storedCount++;
						Print << U"ユニット '{}' を格納しました"_fmt(unit->Name);

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

			if (hbm.second.category == U"releaseAll")
			{
				// 現在選択されているユニットを取得
				Unit* selectedCarrierUnit = nullptr;
				{
					std::scoped_lock lock(classBattleManage.unitListMutex);
					for (auto& loau : classBattleManage.listOfAllUnit)
					{
						for (auto& unit : loau.ListClassUnit)
						{
							if (unit.IsSelect)
							{
								selectedCarrierUnit = &unit;
								break;
							}
						}
						if (selectedCarrierUnit) break;
					}
				}

				if (!selectedCarrierUnit) continue;

				// キャリアーコンポーネントを取得
				auto* carrierComponent = selectedCarrierUnit->getComponent<CarrierComponent>();
				if (!carrierComponent) continue;

				// 格納されているユニットがあるかチェック
				if (carrierComponent->storedUnits.empty())
				{
					Print << U"格納されているユニットがありません";
					continue;
				}

				// 現在のキャリアーユニットの位置を取得
				Vec2 releasePosition = selectedCarrierUnit->GetNowPosiCenter();

				// 格納されているユニット数を記録（リリース前）
				int32 releasedCount = static_cast<int32>(carrierComponent->storedUnits.size());

				// 全ユニットを解放
				carrierComponent->releaseAll(releasePosition);

				Print << U"合計 {} 体のユニットを解放しました"_fmt(releasedCount);
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
}

/// @brief 建築予約をするのが本質
void Battle001::handleBuildMenuSelectionA()
{
	const Transformer2D transformer{ Mat3x2::Identity(), Mat3x2::Translate(Scene::Size().x - 328, Scene::Size().y - 328 - 30) };

	// 通常ユニットの処理
	for (auto& loau : classBattleManage.listOfAllUnit)
	{
		for (auto& itemUnit : loau.ListClassUnit)
		{
			processUnitBuildMenuSelection(itemUnit);
		}
	}

	// 建物ユニットの処理
	Array<std::shared_ptr<Unit>> buildings;
	for (const auto& group : { classBattleManage.hsMyUnitBuilding })
	{
		for (const auto& item : group)
		{
			buildings.push_back(item);
		}
	}

	for (const auto& unitBuildings : buildings)
	{
		processUnitBuildMenuSelection(*unitBuildings);
	}
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
		// ドラッグ距離をチェック
		const double dragDistance = clickStartPos.distanceFrom(Cursor::Pos());
		if (dragDistance < 5) // 例：5ピクセル未満ならクリック扱い
		{
			isUnitSelectionPending = false;

			bool isSeBu = false;
			long selectedBuildingId = -1;

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
					selectedBuildingId = u->ID;
					isSeBu = true;
					break;
				}
			}

			// ユニット選択チェック（建築物が選択されていない場合のみ）
			long selectedUnitId = -1;
			if (!isSeBu)
			{
				std::scoped_lock lock(classBattleManage.unitListMutex);
				for (const auto& target : classBattleManage.listOfAllUnit)
				{
					for (const auto& unit : target.ListClassUnit)
					{
						if ((unit.IsBuilding == false && unit.IsBattleEnable == true)
							|| (unit.IsBuilding == true && unit.IsBattleEnable == true && unit.classBuild != U""))
						{
							Print << Cursor::PosF();
							Print << unit.GetRectNowPosi().center();
							if (unit.GetRectNowPosi().intersects(Cursor::PosF()))
							{
								selectedUnitId = unit.ID;
								break;
							}
						}
					}
					if (selectedUnitId != -1) break;
				}
			}

			// 何も選択されていない場合は全て選択解除
			if (selectedBuildingId == -1 && selectedUnitId == -1)
			{
				std::scoped_lock lock(classBattleManage.unitListMutex);
				for (auto& item : classBattleManage.listOfAllUnit)
				{
					for (auto& itemUnit : item.ListClassUnit)
					{
						itemUnit.IsSelect = false;
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
				return;
			}

			if (!isSeBu)
			{
				// ユニット選択時には、全ての建物の選択を解除
				for (const auto& building : classBattleManage.hsMyUnitBuilding)
				{
					building->IsSelect = false;
				}

				// 選択状態の一括更新
				std::scoped_lock lock(classBattleManage.unitListMutex);
				for (auto& item : classBattleManage.listOfAllUnit)
				{
					for (auto& itemUnit : item.ListClassUnit)
					{
						bool newSelectState = false;

						if (selectedUnitId == itemUnit.ID)
						{
							// クリックされたユニットの選択状態を反転
							newSelectState = !itemUnit.IsSelect;
							IsBuildMenuHome = newSelectState;

							if (newSelectState)
							{
								longBuildSelectTragetId = itemUnit.ID;
							}
							else
							{
								longBuildSelectTragetId = -1;
							}
						}
						else
						{
							// その他のユニットは選択解除
							newSelectState = false;
						}

						// IsSelectの更新
						itemUnit.IsSelect = newSelectState;
					}
				}
			}
			else
			{
				// 建物選択時には、全てのユニットの選択を解除
				std::scoped_lock lock(classBattleManage.unitListMutex);
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
						if (item->ID == selectedBuildingId)
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
		}
		isUnitSelectionPending = false;
	}

	// pressed中でキャンセル条件があれば保留状態をリセット
	// pressed中の処理は削除または条件を変更
	if (MouseL.pressed())
	{
		const double dragDistance = clickStartPos.distanceFrom(Cursor::Pos());
		if (dragDistance >= 5)
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
				std::scoped_lock lock(classBattleManage.unitListMutex);
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

	for (auto& group : classBattleManage.listOfAllUnit)
	{
		if (group.FlagBuilding || group.ListClassUnit.empty())
			continue;

		for (auto& unit : group.ListClassUnit)
			updateBar(unit);
	}

	for (auto& group : classBattleManage.listOfAllEnemyUnit)
	{
		if (group.FlagBuilding || group.ListClassUnit.empty())
			continue;

		for (auto& unit : group.ListClassUnit)
			updateBar(unit);
	}
}
/// @brief 移動処理更新、移動状態や目的地到達を管理　A*経路探索の結果に基づき、位置や移動ベクトルを計算・更新
void Battle001::updateUnitMovements()
{
	//移動処理
	for (auto& item : classBattleManage.listOfAllUnit)
	{
		for (auto& itemUnit : item.ListClassUnit)
		{
			if (itemUnit.IsBuilding == true && itemUnit.mapTipObjectType == MapTipObjectType::WALL2)
				continue;
			if (itemUnit.IsBuilding == true && itemUnit.mapTipObjectType == MapTipObjectType::GATE)
				continue;
			if (itemUnit.IsBattleEnable == false)
				continue;
			if (itemUnit.moveState == moveState::None) continue;
			if (itemUnit.moveState == moveState::MoveAI) continue;
			if (itemUnit.moveState == moveState::FlagMoveCalc) continue;

			{
				std::scoped_lock lock(aStar.aiRootMutex);
				if (!aiRootMy.contains(itemUnit.ID)) continue;
			}
			auto& plan = aiRootMy[itemUnit.ID];

			if (plan.isPathCompleted())
			{
				if (itemUnit.moveState == moveState::MovingEnd)
				{
					itemUnit.moveState = moveState::None; // 移動完了状態から通常状態に戻す
					continue;
				}
				else if (itemUnit.moveState == moveState::Moving)
				{
					//最終移動
					itemUnit.nowPosiLeft += itemUnit.vecMove * ((itemUnit.Move + itemUnit.cts.Speed) / 100.0);

					// 進行方向ベクトル
					Vec2 moveDir = itemUnit.vecMove;
					// 現在位置→目標位置ベクトル
					Vec2 toTarget = itemUnit.GetOrderPosiCenter() - itemUnit.GetNowPosiCenter();

					// 目標を通り過ぎた or 近づいたら到達とみなす
					if (toTarget.dot(moveDir) <= 0 || toTarget.length() < 5.0)
					{
						itemUnit.nowPosiLeft = itemUnit.orderPosiLeft; // 位置をピッタリ補正して止めるのもあり
						itemUnit.FlagReachedDestination = true;
						itemUnit.moveState = moveState::MovingEnd; // 移動完了状態にする
					}
				}

				continue;
			}

			if (itemUnit.moveState == moveState::Moving)
			{
				itemUnit.nowPosiLeft += itemUnit.vecMove * ((itemUnit.Move + itemUnit.cts.Speed) / 100.0);

				if (auto iuyb = plan.getCurrentTarget())
				{
					bool calculateResult = false;
					{
						Optional<Size> nowIndex = mapTile.ToIndex(itemUnit.GetNowPosiCenter(),
							mapTile.columnQuads, mapTile.rowQuads);
						if (nowIndex.has_value())
						{
							if (plan.lastPoint.x != nowIndex->x || plan.lastPoint.y != nowIndex->y)
							{
								plan.lastPoint = nowIndex.value();
								calculateResult = true;
							}
						}
					}
					//if (itemUnit.GetNowPosiCenter().distanceFrom(itemUnit.GetOrderPosiCenter()) < 3.0)
					if (calculateResult)
					{
						plan.stepToNext();
						if (plan.getCurrentTarget())
						{
							// 到達チェック
							const int32 i = plan.getCurrentTarget().value().manhattanLength();
							const int32 xi = (i < (mapTile.N - 1)) ? 0 : (i - (mapTile.N - 1));
							const int32 yi = (i < (mapTile.N - 1)) ? i : (mapTile.N - 1);
							const int32 k2 = (plan.getCurrentTarget().value().manhattanDistanceFrom(Point{ xi, yi }) / 2);
							const double posX = ((i < (mapTile.N - 1)) ? (i * -mapTile.TileOffset.x) : ((i - 2 * mapTile.N + 2) * mapTile.TileOffset.x));
							const double posY = (i * mapTile.TileOffset.y) - mapTile.TileThickness;
							const Vec2 pos = { (posX + mapTile.TileOffset.x * 2 * k2) - (itemUnit.yokoUnit / 2), posY - itemUnit.TakasaUnit - 15 };
							Vec2 nextPos = pos;
							itemUnit.orderPosiLeft = nextPos;
							itemUnit.vecMove = (itemUnit.GetOrderPosiCenter() - itemUnit.GetNowPosiCenter()).normalized();
						}
					}
				}
				if (plan.isPathCompleted())
				{
					itemUnit.orderPosiLeft = itemUnit.orderPosiLeftLast; // 最後の位置に戻す
					Optional<Size> oor = mapTile.ToIndex(itemUnit.GetOrderPosiCenter(), mapTile.columnQuads, mapTile.rowQuads);
					plan.lastPoint = oor.value();
					Vec2 hhh = itemUnit.GetOrderPosiCenter() - itemUnit.GetNowPosiCenter();
					itemUnit.vecMove = hhh.isZero() ? Vec2{ 0, 0 } : hhh.normalized();
				}
				continue;
			}

			// 次のマスに向けて移動準備
			if (auto iuyb = plan.getCurrentTarget())
			{
				plan.lastPoint = iuyb.value();
				plan.stepToNext();

				// そのタイルの底辺中央の座標
				const int32 i = plan.getCurrentTarget().value().manhattanLength();
				const int32 xi = (i < (mapTile.N - 1)) ? 0 : (i - (mapTile.N - 1));
				const int32 yi = (i < (mapTile.N - 1)) ? i : (mapTile.N - 1);
				const int32 k2 = (plan.getCurrentTarget().value().manhattanDistanceFrom(Point{ xi, yi }) / 2);
				const double posX = ((i < (mapTile.N - 1)) ? (i * -mapTile.TileOffset.x) : ((i - 2 * mapTile.N + 2) * mapTile.TileOffset.x));
				const double posY = (i * mapTile.TileOffset.y) - mapTile.TileThickness;
				const Vec2 pos = { (posX + mapTile.TileOffset.x * 2 * k2) - (itemUnit.yokoUnit / 2), posY - itemUnit.TakasaUnit - 15 };

				itemUnit.orderPosiLeft = Vec2(Math::Round(pos.x), Math::Round(pos.y));

				Vec2 hhh = itemUnit.GetOrderPosiCenter() - itemUnit.GetNowPosiCenter();
				itemUnit.vecMove = hhh.isZero() ? Vec2{ 0, 0 } : hhh.normalized();
				itemUnit.moveState = moveState::Moving; // 移動状態にする
			}
		}
	}
	for (auto& item : classBattleManage.listOfAllEnemyUnit)
	{
		for (auto& itemUnit : item.ListClassUnit)
		{
			if (itemUnit.IsBuilding == true && itemUnit.mapTipObjectType == MapTipObjectType::WALL2)
				continue;
			if (itemUnit.IsBuilding == true && itemUnit.mapTipObjectType == MapTipObjectType::GATE)
				continue;
			if (itemUnit.IsBattleEnable == false)
				continue;

			{
				std::scoped_lock lock(aStar.aiRootMutex);
				if (!aiRootEnemy.contains(itemUnit.ID)) continue;
			}

			auto& plan = aiRootEnemy[itemUnit.ID];

			// 1. 移動準備
			if (itemUnit.moveState == moveState::None && plan.getCurrentTarget())
			{
				const Point targetTile = plan.getCurrentTarget().value();

				// そのタイルの底辺中央の座標
				const int32 i = plan.getCurrentTarget().value().manhattanLength();
				const int32 xi = (i < (mapTile.N - 1)) ? 0 : (i - (mapTile.N - 1));
				const int32 yi = (i < (mapTile.N - 1)) ? i : (mapTile.N - 1);
				const int32 k2 = (plan.getCurrentTarget().value().manhattanDistanceFrom(Point{ xi, yi }) / 2);
				const double posX = ((i < (mapTile.N - 1)) ? (i * -mapTile.TileOffset.x) : ((i - 2 * mapTile.N + 2) * mapTile.TileOffset.x));
				const double posY = (i * mapTile.TileOffset.y) - mapTile.TileThickness;
				const Vec2 pos = { (posX + mapTile.TileOffset.x * 2 * k2) - (itemUnit.yokoUnit / 2), posY - itemUnit.TakasaUnit - 15 };
				itemUnit.orderPosiLeft = Vec2(Math::Round(pos.x), Math::Round(pos.y));

				Vec2 hhh = itemUnit.GetOrderPosiCenter() - itemUnit.GetNowPosiCenter();
				itemUnit.vecMove = hhh.isZero() ? Vec2{ 0, 0 } : hhh.normalized();

				itemUnit.moveState = moveState::Moving; // 移動状態にする
			}

			// 2. 移動処理
			if (itemUnit.moveState == moveState::Moving)
			{
				itemUnit.nowPosiLeft += itemUnit.vecMove * ((itemUnit.Move + itemUnit.cts.Speed) / 100.0);

				if (plan.getCurrentTarget())
				{
					if (itemUnit.GetNowPosiCenter().distanceFrom(itemUnit.GetOrderPosiCenter()) <= 3.0)
					{
						plan.stepToNext();
						if (plan.getCurrentTarget())
						{
							const Point targetTile = plan.getCurrentTarget().value();
							const int32 i = plan.getCurrentTarget().value().manhattanLength();
							const int32 xi = (i < (mapTile.N - 1)) ? 0 : (i - (mapTile.N - 1));
							const int32 yi = (i < (mapTile.N - 1)) ? i : (mapTile.N - 1);
							const int32 k2 = (plan.getCurrentTarget().value().manhattanDistanceFrom(Point{ xi, yi }) / 2);
							const double posX = ((i < (mapTile.N - 1)) ? (i * -mapTile.TileOffset.x) : ((i - 2 * mapTile.N + 2) * mapTile.TileOffset.x));
							const double posY = (i * mapTile.TileOffset.y) - mapTile.TileThickness;
							const Vec2 pos = { (posX + mapTile.TileOffset.x * 2 * k2) - (itemUnit.yokoUnit / 2), posY - itemUnit.TakasaUnit - 15 };
							itemUnit.orderPosiLeft = Vec2(Math::Round(pos.x), Math::Round(pos.y));

							Vec2 hhh = itemUnit.GetOrderPosiCenter() - itemUnit.GetNowPosiCenter();
							itemUnit.vecMove = hhh.isZero() ? Vec2{ 0, 0 } : hhh.normalized();

							itemUnit.moveState = moveState::Moving; // 移動状態にする
						}
					}
				}

				if (plan.isPathCompleted())
				{
					itemUnit.FlagReachedDestination = true;
					itemUnit.moveState = moveState::MoveAI;
				}
			}
		}
	}
}

void Battle001::UnitTooltip::updateRenderTexture()
{
	if (content.isEmpty()) return;

	// テキストの測定（原点基準で正確に取得）
	const auto lines = content.split(U'\n');
	int32 maxWidth = 0;
	int32 totalHeight = 0;

	// 原点基準でテキストサイズを測定
	for (const auto& line : lines)
	{
		// region(Vec2::Zero()) で原点基準のサイズを取得
		const auto textRegion = fontInfo.fontSkill(line).region(Vec2::Zero());

		maxWidth = Max(maxWidth, static_cast<int32>(textRegion.w));
		totalHeight += static_cast<int32>(textRegion.h) + 4;
	}

	// パディングとボーダーを考慮
	const int32 padding = 16;
	const int32 borderWidth = 3;
	const int32 width = maxWidth + padding * 2 + borderWidth * 2;
	const int32 height = totalHeight + padding * 2 + borderWidth * 2;

	// テクスチャサイズが同じで、内容が変わっていない場合は作成をスキップ
	if (renderTexture &&
		renderTexture.size() == Size(width, height) &&
		lastRenderedContent == content)
	{
		return;
	}

	// 前回の内容を記録
	lastRenderedContent = content;

	renderTexture = RenderTexture(width, height);

	{
		const ScopedRenderTarget2D target{ renderTexture };
		const ScopedRenderStates2D blend{ MakeBlendState() };

		// 外側のフレーム（光る効果）
		RectF(0, 0, width, height).draw(ColorF{ 0.3, 0.6, 1.0, 0.8 });

		// 内側の背景
		RectF(borderWidth, borderWidth, width - borderWidth * 2, height - borderWidth * 2)
			.draw(ColorF{ 0.05, 0.05, 0.15, 0.95 });

		// グラデーション効果
		for (int32 i = 0; i < borderWidth; ++i)
		{
			const double alpha = 0.3 * (1.0 - static_cast<double>(i) / borderWidth);
			RectF(i, i, width - i * 2, height - i * 2)
				.drawFrame(1, ColorF{ 0.5, 0.8, 1.0, alpha });
		}

		// テキストの描画（座標系を統一）
		int32 yOffset = borderWidth + padding;
		for (const auto& line : lines)
		{
			// 明示的に原点基準で描画
			fontInfo.fontSkill(line).draw(
				Vec2(borderWidth + padding, yOffset),
				Palette::White
			);

			// 次の行の位置を計算（同じregion()メソッドを使用）
			const auto textRegion = fontInfo.fontSkill(line).region(Vec2::Zero());
			yOffset += static_cast<int32>(textRegion.h) + 4;
		}
	}
}

void Battle001::UnitTooltip::draw() const
{
	if (!isVisible || content.isEmpty()) return;

	// フェードイン効果
	const double fadeTime = 0.3;
	const double alpha = Min(fadeTimer.sF() / fadeTime, 1.0);

	// 画面端での位置調整
	Vec2 drawPos = position;
	const auto textureSize = renderTexture.size();

	if (drawPos.x + textureSize.x > Scene::Width())
		drawPos.x = Scene::Width() - textureSize.x - 10;
	if (drawPos.y + textureSize.y > Scene::Height())
		drawPos.y = position.y - textureSize.y - 10;

	// 描画
	renderTexture.draw(drawPos, ColorF{ 1.0, 1.0, 1.0, alpha });
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
		std::scoped_lock lock(classBattleManage.unitListMutex);
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


void Battle001::SkillProcess(Array<ClassHorizontalUnit>& ach, Array<ClassHorizontalUnit>& achTarget, Array<ClassExecuteSkills>& aces)
{
	std::scoped_lock lock(classBattleManage.unitListMutex);
	for (auto& item : ach)
	{
		for (auto& itemUnit : item.ListClassUnit)
		{
			//発動中もしくは死亡ユニットはスキップ
			if (itemUnit.FlagMovingSkill == true || itemUnit.IsBattleEnable == false)
				continue;

			auto temp = itemUnit.arrSkill.filter([&](const Skill& itemSkill) {return nowSelectSkill.contains(itemSkill.nameTag); });

			if (temp.size() > 0)
			{
				for (auto& itemSkill : temp)
				{
					//ターゲットとなるユニットを抽出し、スキル射程範囲を確認
					const auto xA = itemUnit.GetNowPosiCenter();

					if (itemSkill.SkillType == SkillType::heal)
					{
						Array<ClassHorizontalUnit> copy = ach;
						copy.shuffle();
						if (SkillProcess002(copy, xA, itemUnit, itemSkill, aces))
						{
							return;
						}
					}
					else
					{
						if (SkillProcess002(achTarget, xA, itemUnit, itemSkill, aces))
						{
							return;
						}
					}
				}
			}
			else
			{
				//昇順（小さい値から大きい値へ）
				for (Skill& itemSkill : itemUnit.arrSkill.sort_by([](const auto& item1, const auto& item2) { return  item1.sortKey < item2.sortKey; }))
				{
					//ターゲットとなるユニットを抽出し、スキル射程範囲を確認
					const auto xA = itemUnit.GetNowPosiCenter();
					if (itemSkill.SkillType == SkillType::heal)
					{
						Array<ClassHorizontalUnit> copy = ach;
						copy.shuffle();
						if (SkillProcess002(copy, xA, itemUnit, itemSkill, aces))
						{
							return;
						}
					}
					else
					{
						if (SkillProcess002(achTarget, xA, itemUnit, itemSkill, aces))
						{
							return;
						}
					}
				}
			}
		}
	}
}
bool Battle001::SkillProcess002(Array<ClassHorizontalUnit>& aaatarget, Vec2 xA, Unit& itemUnit, Skill& itemSkill, Array<ClassExecuteSkills>& aces)
{
	for (auto& itemTargetHo : aaatarget)
	{
		for (auto& itemTarget : itemTargetHo.ListClassUnit)
		{
			//スキル発動条件確認
			if (itemTarget.IsBattleEnable == false)
			{
				continue;
			}
			if (itemTarget.IsBuilding == true)
			{
				switch (itemTarget.mapTipObjectType)
				{
				case MapTipObjectType::WALL2:
				{
					continue;
				}
				break;
				case MapTipObjectType::GATE:
				{
					if (itemTarget.HPCastle <= 0)
					{
						continue;
					}
				}
				break;
				default:
					break;
				}
			}

			//三平方の定理から射程内か確認
			const Vec2 xB = itemTarget.GetNowPosiCenter();
			const double teihen = xA.x - xB.x;
			const double takasa = xA.y - xB.y;
			const double syahen = (teihen * teihen) + (takasa * takasa);
			const double kyori = std::sqrt(syahen);

			const double xAHankei = (itemUnit.yokoUnit / 2.0) + itemSkill.range;
			const double xBHankei = itemTarget.yokoUnit / 2.0;

			bool check = true;
			if (kyori > (xAHankei + xBHankei))
			{
				check = false;
			}
			// チェック
			if (check == false)
			{
				continue;
			}

			int32 random = classBattleManage.getBattleIDCount();
			int singleAttackNumber = random;

			itemUnit.FlagMovingSkill = true;

			//rush数だけ実行する
			int32 rushBase = 1;
			if (itemSkill.rush > 1) rushBase = itemSkill.rush;

			ClassExecuteSkills ces;
			ces.No = classBattleManage.getDeleteCESIDCount();
			ces.UnitID = itemUnit.ID;
			ces.classSkill = itemSkill;
			ces.classUnit = &itemUnit;
			ces.classUnitHealTarget = &itemTarget;

			for (int iii = 0; iii < rushBase; iii++)
			{
				ClassBullets cbItemUnit;
				cbItemUnit.No = singleAttackNumber;
				cbItemUnit.RushNo = iii;
				cbItemUnit.NowPosition = itemUnit.GetNowPosiCenter();
				cbItemUnit.StartPosition = itemUnit.GetNowPosiCenter();
				if (itemSkill.rushRandomDegree > 1)
				{
					int32 deg = Random(-itemSkill.rushRandomDegree, itemSkill.rushRandomDegree);
					//ラジアン ⇒ 度*π/180 を掛ける
					double rad = deg * (s3d::Math::Pi / 180);
					//[0]を基準にするのでOK
					Vec2 caRe = ConvertVecX(rad,
						itemTarget.GetNowPosiCenter().x,
						itemTarget.GetNowPosiCenter().y,
						cbItemUnit.NowPosition.x,
						cbItemUnit.NowPosition.y);
					cbItemUnit.OrderPosition = caRe;
				}
				else
				{
					cbItemUnit.OrderPosition = itemTarget.GetNowPosiCenter();
				}
				if (itemSkill.speed == 0)
				{
					cbItemUnit.duration = 2.5;
				}
				else
				{
					cbItemUnit.duration = (itemSkill.range + itemSkill.speed - 1) / itemSkill.speed;
				}
				cbItemUnit.lifeTime = 0;

				Vec2 ve = cbItemUnit.OrderPosition - cbItemUnit.NowPosition;
				if (NearlyEqual(ve.x, ve.y))
				{
					cbItemUnit.MoveVec = ve;
				}
				else
				{
					cbItemUnit.MoveVec = ve.normalized();
				}

				//二点間の角度を求める
				cbItemUnit.radian = Math::Atan2((float)(cbItemUnit.OrderPosition.y - cbItemUnit.NowPosition.y),
					(float)(cbItemUnit.OrderPosition.x - cbItemUnit.NowPosition.x));
				cbItemUnit.degree = cbItemUnit.radian * (180 / Math::Pi);
				cbItemUnit.initDegree = cbItemUnit.degree;

				ces.ArrayClassBullet.push_back(cbItemUnit);
			}

			aces.push_back(ces);

			//攻撃を伴う支援技か？
			switch (itemSkill.SkillType)
			{
			case SkillType::missileAdd:
			{
				itemUnit.cts.Speed = itemSkill.plus_speed;
				//speedが0の場合、durationは2.5になる
				for (auto& ttt : aces)
				{
					for (auto& tttt : ttt.ArrayClassBullet)
					{
						tttt.duration = itemSkill.plus_speed_time;
					}
				}
			}
			break;
			default:
				break;
			}

			return true;
		}
	}
	return false;
}

void Battle001::ColliderCheck(RectF rrr, ClassBullets& target, ClassExecuteSkills& loop_Battle_player_skills, Array<int32>& arrayNo, Array<ClassHorizontalUnit>& chu)
{
	Circle tc2 = Circle{ target.NowPosition.x,target.NowPosition.y,loop_Battle_player_skills.classSkill.w / 2.0 };

	bool bombCheck = false;

	for (auto& itemTargetHo : chu)
	{
		for (auto& itemTarget : itemTargetHo.ListClassUnit)
		{
			if (itemTarget.IsBattleEnable == false)continue;

			Vec2 vv;
			if (itemTarget.IsBuilding == true)
			{
				switch (itemTarget.mapTipObjectType)
				{
				case MapTipObjectType::WALL2:
					continue;
					break;
				case MapTipObjectType::GATE:
				{
					Point pt = Point(itemTarget.rowBuilding, itemTarget.colBuilding);
					vv = mapTile.ToTileBottomCenter(pt, mapTile.N);
					vv = { vv.x,vv.y - (25 + 15) };
				}
				break;
				default:
				{
					Point pt = Point(itemTarget.rowBuilding, itemTarget.colBuilding);
					vv = mapTile.ToTileBottomCenter(pt, mapTile.N);
					vv = { vv.x,vv.y - (25 + 15) };
				}
				break;
				}
			}
			else
			{
				vv = itemTarget.GetNowPosiCenter();
			}

			Circle cTar = Circle{ vv,1 };
			if (loop_Battle_player_skills.classSkill.SkillCenter == SkillCenter::end
				&& rrr.rotatedAt(rrr.bottomCenter(), target.radian + Math::ToRadians(90)).intersects(cTar) == true)
			{
				if (ProcessCollid(bombCheck, arrayNo, target, loop_Battle_player_skills, itemTarget))break;
			}
			else if (tc2.intersects(cTar) == true)
			{
				if (ProcessCollid(bombCheck, arrayNo, target, loop_Battle_player_skills, itemTarget))break;
			}
		}
		//一体だけ当たったらそこで終了
		if (bombCheck == true)
		{
			break;
		}
	}
}
void Battle001::ColliderCheckHeal(RectF rrr, ClassBullets& target, ClassExecuteSkills& loop_Battle_player_skills, Array<int32>& arrayNo, Unit* itemTarget)
{
	Circle tc2 = { Circle{ target.NowPosition.x,target.NowPosition.y,loop_Battle_player_skills.classSkill.w / 2.0 } };

	//ここでは不要
	bool bombCheck = false;

	if (itemTarget->IsBattleEnable == false)
	{
		return;
	}

	Vec2 vv;
	if (itemTarget->IsBuilding == true)
	{
		switch (itemTarget->mapTipObjectType)
		{
		case MapTipObjectType::WALL2:
			return;
			break;
		case MapTipObjectType::GATE:
		{
			Point pt = Point(itemTarget->rowBuilding, itemTarget->colBuilding);
			vv = mapTile.ToTileBottomCenter(pt, mapTile.N);
			vv = { vv.x,vv.y - (25 + 15) };
		}
		break;
		default:
		{
			Point pt = Point(itemTarget->rowBuilding, itemTarget->colBuilding);
			vv = mapTile.ToTileBottomCenter(pt, mapTile.N);
			vv = { vv.x,vv.y - (25 + 15) };
		}
		break;
		}
	}
	else
	{
		vv = itemTarget->GetNowPosiCenter();
	}

	Circle cTar = Circle{ vv,1 };
	if (loop_Battle_player_skills.classSkill.SkillCenter == SkillCenter::end
		&& rrr.rotatedAt(rrr.bottomCenter(), target.radian + Math::ToRadians(90)).intersects(cTar) == true)
	{
		ProcessCollid(bombCheck, arrayNo, target, loop_Battle_player_skills, *itemTarget);
	}
	else if (tc2.intersects(cTar) == true)
	{
		ProcessCollid(bombCheck, arrayNo, target, loop_Battle_player_skills, *itemTarget);
	}
}
void Battle001::CalucDamage(Unit& itemTarget, double strTemp, ClassExecuteSkills& ces)
{
	double powerStr = 0;
	double defStr = 0;

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

	if (powerStr < defStr)
	{
		powerStr = 0;
		defStr = 0;
	}

	if (itemTarget.IsBuilding == true)
	{
		itemTarget.HPCastle = itemTarget.HPCastle - (powerStr)+(defStr);
	}
	else
	{
		if (ces.classSkill.SkillType == SkillType::heal)
		{
			if (ces.classSkill.attr == U"mp")
			{
				itemTarget.Mp = itemTarget.Mp + (powerStr)+(defStr);
			}
			else if (ces.classSkill.attr == U"hp")
			{
				itemTarget.Hp = itemTarget.Hp + (powerStr)+(defStr);
			}
		}
		else
		{
			itemTarget.Hp = itemTarget.Hp - (powerStr)+(defStr);
		}
	}
}
bool Battle001::ProcessCollid(bool& bombCheck, Array<int32>& arrayNo, ClassBullets& target, ClassExecuteSkills& loop_Battle_player_skills, Unit& itemTarget)
{
	loop_Battle_player_skills.classUnit->FlagMovingSkill = false;

	CalucDamage(itemTarget, loop_Battle_player_skills.classSkill.str, loop_Battle_player_skills);

	//消滅
	arrayNo.push_back(target.No);
	//一体だけ当たったらそこで終了
	if (loop_Battle_player_skills.classSkill.SkillBomb == SkillBomb::off)
	{
		bombCheck == true;
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
		std::scoped_lock lock(classBattleManage.unitListMutex);
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
				Array<Unit> unitSnapshot;
				{
					std::scoped_lock lock(classBattleManage.unitListMutex);
					for (auto& units : classBattleManage.listOfAllUnit)
					{
						for (const auto& unit : units.ListClassUnit)
						{
							if (unit.IsBattleEnable)
								unitSnapshot.push_back(unit);
						}
					}
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

void Battle001::calculateFogFromUnits(Grid<Visibility>& visMap, const Array<Unit>& units)
{
	static HashSet<Point> lastVisibleTiles;
	HashSet<Point> currentVisibleTiles;

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
Co::Task<void> Battle001::start()
{
	for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/040_ChipImage/"))
		TextureAsset::Register(FileSystem::FileName(filePath), filePath);
	for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/015_BattleMapCellImage/"))
		TextureAsset::Register(FileSystem::FileName(filePath), MakeTextureAssetData1(filePath, TextureDesc::Mipped));
	for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/043_ChipImageBuild/"))
		TextureAsset::Register(FileSystem::FileName(filePath), filePath);
	for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/041_ChipImageSkill/"))
		TextureAsset::Register(FileSystem::FileName(filePath), filePath);

	/// modでカスタマイズ出来るようにあえて配列を使う
	arrayBattleZinkei.push_back(false);
	arrayBattleZinkei.push_back(false);
	arrayBattleZinkei.push_back(false);
	arrayBattleCommand.push_back(false);
	arrayBattleCommand.push_back(false);

	//初期ユニット
	{
		UnitRegister(classBattleManage, mapTile, U"LineInfantryM14",
					10,
					10,
					3,
					classBattleManage.listOfAllUnit, false
		);
	}

	//初期ユニット-建物
	{
		for (auto uu : m_commonConfig.arrayInfoUnit)
		{
			if (uu.NameTag == U"Home")
			{
				uu.ID = classBattleManage.getIDCount();
				uu.IsBuilding = true;
				uu.mapTipObjectType = MapTipObjectType::HOME;
				uu.rowBuilding = mapTile.N / 2;
				uu.colBuilding = mapTile.N / 2;
				uu.initTilePos = Point{ uu.rowBuilding, uu.colBuilding };
				uu.Move = 0.0;
				uu.nowPosiLeft = mapTile.ToTile(uu.initTilePos, mapTile.N)
					.asPolygon()
					.centroid()
					.movedBy(-(uu.yokoUnit / 2), -(uu.TakasaUnit / 2));
				auto u = std::make_shared<Unit>(uu);
				classBattleManage.hsMyUnitBuilding.insert(u);
			}
		}
	}

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

	aStar.taskAStarEnemy = Async([this]() {
		while (!aStar.abortAStarEnemy)
		{
			if (!aStar.pauseAStarTaskEnemy)
			{
				HashTable<Point, Array<Unit*>> hsBuildingUnitForAstarSnapshot;
				{
					std::scoped_lock lock(classBattleManage.unitListMutex);
					hsBuildingUnitForAstarSnapshot = hsBuildingUnitForAstar;
				}
				aStar.BattleMoveAStar(
					classBattleManage.unitListMutex,
					classBattleManage.listOfAllUnit,
					classBattleManage.listOfAllEnemyUnit,
					classBattleManage.classMapBattle.value().mapData,
					aiRootEnemy,
					aStar.abortAStarEnemy,
					aStar.pauseAStarTaskEnemy, aStar.changeUnitMember, hsBuildingUnitForAstarSnapshot, mapTile);
			}
			System::Sleep(1);
		}
	});

	aStar.taskAStarMyUnits = Async([this]() {
		while (!aStar.abortAStarMyUnits)
		{
			if (!aStar.pauseAStarTaskMyUnits)
			{
				HashTable<Point, Array<Unit*>> hsBuildingUnitForAstarSnapshot;
				{
					std::scoped_lock lock(classBattleManage.unitListMutex);
					hsBuildingUnitForAstarSnapshot = hsBuildingUnitForAstar;
				}
				aStar.BattleMoveAStarMyUnitsKai(
					classBattleManage.unitListMutex,
					classBattleManage.listOfAllUnit,
					classBattleManage.listOfAllEnemyUnit,
					classBattleManage.classMapBattle.value().mapData,
					aiRootMy,
					aStar.abortAStarMyUnits,
					aStar.pauseAStarTaskMyUnits, hsBuildingUnitForAstarSnapshot, mapTile);
			}
			System::Sleep(1); // CPU過負荷防止
		}
	});

	startAsyncFogCalculation();

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
Co::Task<void> Battle001::mainLoop()
{
	const auto _tooltip = resourcePointTooltip.playScoped();

	while (true)
	{
		if (shouldExit == true)
			co_return;

		camera.update();
		resourcePointTooltip.setCamera(camera);

		// ユニット情報ツールチップの処理
		handleUnitTooltip();
		// 指定した経過時間後に敵ユニットをマップ上にスポーン
		spawnTimedEnemy(classBattleManage, mapTile);
		// リソース状況の更新
		updateResourceIncome();
		//後でbattle内に移動(ポーズ処理を考慮
		updateBuildQueue();

		//// 戦場の霧を更新
		if (fogDataReady.load())
		{
			std::scoped_lock lock(fogMutex);
			visibilityMap = std::move(nextVisibilityMap);
			fogDataReady = false;
		}

		// 状態によっては選択状態を解除
		co_await checkCancelSelectionByUIArea();

		// ビルドメニュー領域での左クリック
		bool IsBuildSelectTraget = false;
		{
			const Transformer2D transformer{ Mat3x2::Identity(), Mat3x2::Translate(Scene::Size().x - 328, Scene::Size().y - 328 - 30) };
			for (auto& hbm : sortedArrayBuildMenu)
			{
				if (hbm.second.rectHantei.leftClicked())
					IsBuildSelectTraget = true; // ビルドメニューがクリックされたのでhandleUnitAndBuildingSelectionをスキップ
			}
		}

		//カメラ移動 || 部隊を選択状態にする。もしくは既に選択状態なら移動させる
		{
			const auto t = camera.createTransformer();

			if (!IsBuildSelectTraget)
				handleUnitAndBuildingSelection();

			handleCameraInput();

			// 右クリック時のカーソル座標記録処理
			if (MouseR.pressed() == false)
			{
				if (MouseR.up() == false)
					cursPos = Cursor::Pos();
			}
			else if (MouseR.down() && is移動指示)
			{
				cursPos = Cursor::Pos();
			}

			//部隊を選択状態にする。もしくは既に選択状態なら経路を算出する
			if (MouseR.up())
			{
				Point start = cursPos;
				Point end = Cursor::Pos();

				co_await handleRightClickUnitActions(start, end);
			}

			handleBuildTargetSelection();
		}

		//陣形処理
		{
			const Transformer2D transformer{ Mat3x2::Identity(),
				Mat3x2::Translate(0,Scene::Size().y - renderTextureSkill.height() - renderTextureZinkei.height() - underBarHeight) };

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
						const ScopedRenderStates2D blend{ MakeBlendState() };

						Rect df = Rect(320, 60);
						df.drawFrame(4, 0, ColorF{ 0.5 });

						for (auto&& [i, ttt] : Indexed(rectZinkei))
						{
							ttt.draw(Palette::Aliceblue);
							if (arrayBattleZinkei[i] == true)
								ttt.drawFrame(4, 0, Palette::Red);
							fontInfo.fontZinkei(ss.Zinkei[i]).draw(ttt, Palette::Black);
						}
					}

				}
			}
		}

		// ユニットの移動処理
		updateUnitMovements();
		// 体力バーの更新
		updateUnitHealthBars();
		// 技UIの更新
		handleSkillUISelection();
		// ビルドメニューの選択処理(予約をするのが本質)
		handleBuildMenuSelectionA();

		//skill処理
		SkillProcess(classBattleManage.listOfAllUnit, classBattleManage.listOfAllEnemyUnit, m_Battle_player_skills);
		SkillProcess(classBattleManage.listOfAllEnemyUnit, classBattleManage.listOfAllUnit, m_Battle_enemy_skills);

		//skill実行処理
		const double time = Scene::Time();
		{
			Array<ClassExecuteSkills> deleteCES;
			for (ClassExecuteSkills& loop_Battle_player_skills : m_Battle_player_skills)
			{
				//Array<int32> arrayNo;
				//for (auto& target : loop_Battle_player_skills.ArrayClassBullet)
				//{
				//	target.lifeTime += Scene::DeltaTime();
				//	switch (loop_Battle_player_skills.classSkill.SkillType)
				//	{
				//	case SkillType::missileAdd:
				//	{
				//		//時間が過ぎたら補正を終了する
				//		if (target.lifeTime > target.duration)
				//		{
				//			loop_Battle_player_skills.classUnit->cts.Speed = 0.0;
				//			loop_Battle_player_skills.classUnit->cts.plus_speed_time = 0.0;
				//		}
				//	}
				//	break;
				//	default:
				//		break;
				//	}
				//	if (target.lifeTime > target.duration)
				//	{
				//		loop_Battle_player_skills.classUnit->FlagMovingSkill = false;
				//		//消滅
				//		arrayNo.push_back(target.No);
				//		break;
				//	}
				//	else
				//	{
				//		//イージングによって速度を変更させる
				//		if (loop_Battle_player_skills.classSkill.Easing.has_value() == true)
				//		{
				//			// 移動の割合 0.0～1.0
				//			const double t = Min(target.stopwatch.sF(), 1.0);
				//			switch (loop_Battle_player_skills.classSkill.Easing.value())
				//			{
				//			case SkillEasing::easeOutExpo:
				//			{
				//				const double ea = EaseOutExpo(t);
				//				target.NowPosition = Vec2((target.NowPosition.x + (target.MoveVec.x * (loop_Battle_player_skills.classSkill.speed / 100) * (ea* loop_Battle_player_skills.classSkill.EasingRatio))),
				//					(target.NowPosition.y + (target.MoveVec.y * (loop_Battle_player_skills.classSkill.speed / 100) * (ea* loop_Battle_player_skills.classSkill.EasingRatio))));
				//			}
				//			break;
				//			default:
				//				break;
				//			}
				//		}
				//		else
				//		{
				//			switch (loop_Battle_player_skills.classSkill.MoveType)
				//			{
				//			case MoveType::line:
				//			{
				//				if (loop_Battle_player_skills.classSkill.SkillCenter == SkillCenter::end)
				//				{
				//					Vec2 offPos = { -1,-1 };
				//					for (size_t i = 0; i < classBattleManage.listOfAllUnit.size(); i++)
				//					{
				//						for (size_t j = 0; j < classBattleManage.listOfAllUnit[i].ListClassUnit.size(); j++)
				//						{
				//							if (loop_Battle_player_skills.UnitID == classBattleManage.listOfAllUnit[i].ListClassUnit[j].ID)
				//							{
				//								offPos = classBattleManage.listOfAllUnit[i].ListClassUnit[j].GetNowPosiCenter();
				//							}
				//						}
				//					}
				//					target.NowPosition = offPos;
				//				}
				//				else
				//				{
				//					target.NowPosition = Vec2((target.NowPosition.x + (target.MoveVec.x * (loop_Battle_player_skills.classSkill.speed / 100))),
				//						(target.NowPosition.y + (target.MoveVec.y * (loop_Battle_player_skills.classSkill.speed / 100))));
				//				}
				//			}
				//			break;
				//			case MoveType::circle:
				//			{
				//				Vec2 offPos = { -1,-1 };
				//				for (size_t i = 0; i < classBattleManage.listOfAllUnit.size(); i++)
				//				{
				//					for (size_t j = 0; j < classBattleManage.listOfAllUnit[i].ListClassUnit.size(); j++)
				//					{
				//						if (loop_Battle_player_skills.UnitID == classBattleManage.listOfAllUnit[i].ListClassUnit[j].ID)
				//						{
				//							offPos = classBattleManage.listOfAllUnit[i].ListClassUnit[j].GetNowPosiCenter();
				//						}
				//					}
				//				}
				//				const double theta = (target.RushNo * 60_deg + time * (loop_Battle_player_skills.classSkill.speed * Math::Pi / 180.0));
				//				const Vec2 pos = OffsetCircular{ offPos, loop_Battle_player_skills.classSkill.radius, theta };
				//				target.NowPosition = pos;
				//			}
				//			break;
				//			case MoveType::swing:
				//			{
				//				const float degg = target.degree + (loop_Battle_player_skills.classSkill.speed / 100);
				//				if (loop_Battle_player_skills.classSkill.range + target.initDegree > degg)
				//				{
				//					//範囲内
				//					target.degree = degg;
				//					target.radian = ToRadians(target.degree);
				//				}
				//				else
				//				{
				//				}
				//			}
				//			break;
				//			default:
				//				target.NowPosition = Vec2((target.NowPosition.x + (target.MoveVec.x * (loop_Battle_player_skills.classSkill.speed / 100))),
				//					(target.NowPosition.y + (target.MoveVec.y * (loop_Battle_player_skills.classSkill.speed / 100))));
				//				break;
				//			}
				//		}
				//	}
				//	//衝突したらunitのHPを減らし、消滅
				//	RectF rrr = { Arg::bottomCenter(target.NowPosition),(double)loop_Battle_player_skills.classSkill.w,(double)loop_Battle_player_skills.classSkill.h };
				//	bool bombCheck = false;
				//	if (loop_Battle_player_skills.classSkill.SkillType == SkillType::heal)
				//	{
				//		ColliderCheckHeal(rrr, target, loop_Battle_player_skills, arrayNo, loop_Battle_player_skills.classUnitHealTarget);
				//	}
				//	else
				//	{
				//		ColliderCheck(rrr, target, loop_Battle_player_skills, arrayNo, classBattleManage.listOfAllEnemyUnit);
				//	}
				//}
				//loop_Battle_player_skills.ArrayClassBullet.remove_if([&](const ClassBullets& cb)
				//	{
				//		if (arrayNo.includes(cb.No))
				//		{
				//			return true;
				//		}
				//		else
				//		{
				//			return false;
				//		}
				//	});
				//arrayNo.clear();
			}
			m_Battle_player_skills.remove_if([&](const ClassExecuteSkills& a) { return a.ArrayClassBullet.size() == 0; });
		}
		{
			Array<ClassExecuteSkills> deleteCES;
			for (ClassExecuteSkills& loop_Battle_player_skills : m_Battle_enemy_skills)
			{
				//Array<int32> arrayNo;
				//for (auto& target : loop_Battle_player_skills.ArrayClassBullet)
				//{
				//	target.lifeTime += Scene::DeltaTime();
				//	if (target.lifeTime > target.duration)
				//	{
				//		loop_Battle_player_skills.classUnit->FlagMovingSkill = false;
				//		//消滅
				//		arrayNo.push_back(target.No);
				//		break;
				//	}
				//	else
				//	{
				//		//イージングによって速度を変更させる
				//		if (loop_Battle_player_skills.classSkill.Easing.has_value() == true)
				//		{
				//			// 移動の割合 0.0～1.0
				//			const double t = Min(target.stopwatch.sF(), 1.0);
				//			switch (loop_Battle_player_skills.classSkill.Easing.value())
				//			{
				//			case SkillEasing::easeOutExpo:
				//			{
				//				const double ea = EaseOutExpo(t);
				//				target.NowPosition = Vec2((target.NowPosition.x + (target.MoveVec.x * (loop_Battle_player_skills.classSkill.speed / 100) * (ea* loop_Battle_player_skills.classSkill.EasingRatio))),
				//					(target.NowPosition.y + (target.MoveVec.y * (loop_Battle_player_skills.classSkill.speed / 100) * (ea* loop_Battle_player_skills.classSkill.EasingRatio))));
				//			}
				//			break;
				//			default:
				//				break;
				//			}
				//		}
				//		else
				//		{
				//			switch (loop_Battle_player_skills.classSkill.MoveType)
				//			{
				//			case MoveType::line:
				//			{
				//				if (loop_Battle_player_skills.classSkill.SkillCenter == SkillCenter::end)
				//				{
				//					Vec2 offPos = { -1,-1 };
				//					for (size_t i = 0; i < classBattleManage.listOfAllEnemyUnit.size(); i++)
				//					{
				//						for (size_t j = 0; j < classBattleManage.listOfAllEnemyUnit[i].ListClassUnit.size(); j++)
				//						{
				//							if (loop_Battle_player_skills.UnitID == classBattleManage.listOfAllEnemyUnit[i].ListClassUnit[j].ID)
				//							{
				//								offPos = classBattleManage.listOfAllEnemyUnit[i].ListClassUnit[j].GetNowPosiCenter();
				//							}
				//						}
				//					}
				//					target.NowPosition = offPos;
				//				}
				//				else
				//				{
				//					target.NowPosition = Vec2((target.NowPosition.x + (target.MoveVec.x * (loop_Battle_player_skills.classSkill.speed / 100))),
				//						(target.NowPosition.y + (target.MoveVec.y * (loop_Battle_player_skills.classSkill.speed / 100))));
				//				}
				//			}
				//			break;
				//			case MoveType::circle:
				//			{
				//				Vec2 offPos = { -1,-1 };
				//				for (size_t i = 0; i < classBattleManage.listOfAllEnemyUnit.size(); i++)
				//				{
				//					for (size_t j = 0; j < classBattleManage.listOfAllEnemyUnit[i].ListClassUnit.size(); j++)
				//					{
				//						if (loop_Battle_player_skills.UnitID == classBattleManage.listOfAllEnemyUnit[i].ListClassUnit[j].ID)
				//						{
				//							offPos = classBattleManage.listOfAllEnemyUnit[i].ListClassUnit[j].GetNowPosiCenter();
				//						}
				//					}
				//				}
				//				const double theta = (target.RushNo * 60_deg + time * (loop_Battle_player_skills.classSkill.speed * Math::Pi / 180.0));
				//				const Vec2 pos = OffsetCircular{ offPos, loop_Battle_player_skills.classSkill.radius, theta };
				//				target.NowPosition = pos;
				//			}
				//			break;
				//			case MoveType::swing:
				//			{
				//				const float degg = target.degree + (loop_Battle_player_skills.classSkill.speed / 100);
				//				if (loop_Battle_player_skills.classSkill.range + target.initDegree > degg)
				//				{
				//					//範囲内
				//					target.degree = degg;
				//					target.radian = ToRadians(target.degree);
				//				}
				//				else
				//				{
				//				}
				//			}
				//			break;
				//			default:
				//				target.NowPosition = Vec2((target.NowPosition.x + (target.MoveVec.x * (loop_Battle_player_skills.classSkill.speed / 100))),
				//					(target.NowPosition.y + (target.MoveVec.y * (loop_Battle_player_skills.classSkill.speed / 100))));
				//				break;
				//			}
				//		}
				//	}
				//	//衝突したらunitのHPを減らし、消滅
				//	RectF rrr = { Arg::bottomCenter(target.NowPosition),(double)loop_Battle_player_skills.classSkill.w,(double)loop_Battle_player_skills.classSkill.h };
				//	bool bombCheck = false;
				//	if (loop_Battle_player_skills.classSkill.SkillType == SkillType::heal)
				//	{
				//		ColliderCheck(rrr, target, loop_Battle_player_skills, arrayNo, classBattleManage.listOfAllEnemyUnit);
				//	}
				//	else
				//	{
				//		ColliderCheck(rrr, target, loop_Battle_player_skills, arrayNo, classBattleManage.listOfAllUnit);
				//	}
				//}
				//loop_Battle_player_skills.ArrayClassBullet.remove_if([&](const ClassBullets& cb)
				//	{
				//		if (arrayNo.includes(cb.No))
				//		{
				//			return true;
				//		}
				//		else
				//		{
				//			return false;
				//		}
				//	});
				//arrayNo.clear();
			}
			m_Battle_enemy_skills.remove_if([&](const ClassExecuteSkills& a) { return a.ArrayClassBullet.size() == 0; });
		}

		//体力が無くなったunit削除処理
		for (auto& item : classBattleManage.listOfAllUnit)
		{
			for (auto& itemUnit : item.ListClassUnit)
			{
				if (itemUnit.isValidBuilding() == true)
				{
					if (itemUnit.HPCastle <= 0)
						itemUnit.IsBattleEnable = false;
				}
				else
				{
					if (itemUnit.Hp <= 0)
						itemUnit.IsBattleEnable = false;
				}
			}
		}
		for (auto& item : classBattleManage.listOfAllEnemyUnit)
		{
			for (auto& itemUnit : item.ListClassUnit)
			{
				if (itemUnit.isValidBuilding() == true)
				{
					if (itemUnit.HPCastle <= 0)
						itemUnit.IsBattleEnable = false;
				}
				else
				{
					if (itemUnit.Hp <= 0)
						itemUnit.IsBattleEnable = false;
				}
			}
		}

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

	//for (int32 i = 0; i < (mapTile.N * 2 - 1); ++i)
	//{
	//	int32 xi = (i < (mapTile.N - 1)) ? 0 : (i - (mapTile.N - 1));
	//	int32 yi = (i < (mapTile.N - 1)) ? i : (mapTile.N - 1);

	//	for (int32 k = 0; k < (mapTile.N - Abs(mapTile.N - i - 1)); ++k)
	//	{
	//		Point index{ xi + k, yi - k };
	//		Vec2 pos = mapTile.ToTileBottomCenter(index, mapTile.N);
	//		if (!cameraView.intersects(pos))
	//			continue;

	//		const auto& tile = classBattleManage.classMapBattle.value().mapData[index.x][index.y];
	//		TextureAsset(tile.tip + U".png").draw(Arg::bottomCenter = pos);
	//	}
	//}
}
/// @brief カメラビューとマップタイル、可視性マップに基づいてフォグ（霧）を描画します。
/// @param cameraView 描画範囲を指定するカメラの矩形領域。
/// @param mapTile 描画対象となるマップタイルの情報。
/// @param visibilityMap 各タイルの可視状態を示すグリッド。
void Battle001::drawFog(const RectF& cameraView, const MapTile& mapTile, const Grid<Visibility> visibilityMap) const
{
	//for (int32 i = 0; i < (mapTile.N * 2 - 1); ++i)
	//{
	//	int32 xi = (i < (mapTile.N - 1)) ? 0 : (i - (mapTile.N - 1));
	//	int32 yi = (i < (mapTile.N - 1)) ? i : (mapTile.N - 1);

	//	for (int32 k = 0; k < (mapTile.N - Abs(mapTile.N - i - 1)); ++k)
	//	{
	//		Point index{ xi + k, yi - k };
	//		switch (visibilityMap[index])
	//		{
	//		case Visibility::Unseen:
	//			mapTile.ToTile(index, mapTile.N).draw(ColorF{ 0.0, 0.6 });
	//			break;
	//		case Visibility::Visible:
	//			break;
	//		}
	//	}
	//}

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
	}
}
/// @brief カメラビュー内のユニットを描画します。
/// @param cameraView 描画範囲を指定する矩形領域。
/// @param classBattleManage ユニット情報を管理するClassBattleオブジェクト。
void Battle001::drawUnits(const RectF& cameraView, const ClassBattle& classBattleManage) const
{
	auto drawGroup = [&](const Array<ClassHorizontalUnit>& group, const String& ringA, const String& ringB)
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
					if (!u.IsBattleEnable) continue;

					const Vec2 center = u.GetNowPosiCenter();
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

	drawGroup(classBattleManage.listOfAllUnit, U"ringA.png", U"ringB.png");
	drawGroup(classBattleManage.listOfAllEnemyUnit, U"ringA_E.png", U"ringB_E.png");
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

	// 建築キューの描画
	if (!arrYoyakuBuild.empty())
	{
		Rect(baseX - 64 - 6, baseY, 70, 328).drawFrame(4, 0, Palette::Black);

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
void Battle001::draw() const
{
	FsScene::draw();

	{
		// 2D カメラによる座標変換を適用する
		const auto tr = camera.createTransformer();
		// 乗算済みアルファ用のブレンドステートを適用する
		const ScopedRenderStates2D blend{ BlendState::Premultiplied };
		const RectF cameraView = getCameraView(camera, mapTile);

		drawTileMap(cameraView, mapTile, classBattleManage);
		drawFog(cameraView, mapTile, visibilityMap);
		drawBuildings(cameraView, classBattleManage, mapTile);
		drawUnits(cameraView, classBattleManage);
		drawResourcePoints(cameraView, classBattleManage, mapTile);
		resourcePointTooltip.draw();
		drawSelectionRectangleOrArrow();
		drawBuildTargetHighlight(mapTile);
	}

	renderTextureZinkei.draw(
		0,
		Scene::Size().y - renderTextureSkill.height() - renderTextureZinkei.height() - underBarHeight);
	drawSkillUI();
	drawBuildDescription();
	drawBuildMenu();
	drawResourcesUI();

	if (longBuildSelectTragetId == -1)
		DrawMiniMap(visibilityMap, camera.getRegion());

	// ユニット情報ツールチップの描画
	unitTooltip.draw();
}
