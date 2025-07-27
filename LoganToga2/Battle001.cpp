#include "stdafx.h"
#include "Battle001.h"
#include <ranges>
#include <vector>

template<typename DrawFunc>
void forEachVisibleTile(const RectF& cameraView, const MapTile& mapTile, DrawFunc drawFunc)
{
	for (int32 i = 0; i < (mapTile.N * 2 - 1); ++i)
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

/// @brief TODO:後で消す
/// @param cmb 
void Battle001::GetTempResource(ClassMapBattle& cmb)
{
	cmb.mapData[0][10].isResourcePoint = true;
	cmb.mapData[0][10].resourcePointType = resourceKind::Gold;
	cmb.mapData[0][10].resourcePointAmount = 11;
	cmb.mapData[0][10].resourcePointDisplayName = U"金";
	cmb.mapData[0][10].resourcePointIcon = U"point000.png";
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

	classBattleManage.classMapBattle = ClassStaticCommonMethod::GetClassMapBattle(sM);

	/// >>>マップの読み込み
	mapTile.N = classBattleManage.classMapBattle.value().mapData.size();
	mapTile.TileOffset = { 50, 25 };
	mapTile.TileThickness = 15;
	/// <<<マップの読み込み

	/// >>>Resourceの設定
	GetTempResource(classBattleManage.classMapBattle.value());
	Array<ResourcePointTooltip::TooltipTarget> resourceTargets;
	SetResourceTargets(classBattleManage, resourceTargets, mapTile);
	resourcePointTooltip.setTargets(resourceTargets);
	resourcePointTooltip.setTooltipEnabled(true);
	/// <<<Resourceの設定

}
/// @brief 後片付け
Battle001::~Battle001()
{
	aStar.abortAStarEnemy = true;
	aStar.abortAStarMyUnits = true;

	/// >>>完全に終了を待つ---
	if (aStar.taskAStarEnemy.isValid())
		aStar.taskAStarEnemy.wait();
	if (aStar.taskAStarMyUnits.isValid())
		aStar.taskAStarMyUnits.wait();
	/// ---完全に終了を待つ<<<
}
/// @brief 
/// @param classBattleManage 
/// @param resourceTargets 
/// @param mapTile 
void Battle001::SetResourceTargets(
	ClassBattle classBattleManage,
	Array<ResourcePointTooltip::TooltipTarget>& resourceTargets,
	MapTile mapTile)
{
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

			const Circle area =
				Circle(pos.movedBy(0, -mapTile.TileThickness - mapTile.TileOffset.y),
									TextureAsset(tile.resourcePointIcon).region().w / 2);
			String desc = U"資源ポイント\n所有者: {}\n種類:{}\n量:{}"_fmt(
						(tile.whichIsThePlayer == BattleWhichIsThePlayer::Sortie)
				? U"味方"
				: U"敵"
					, U"金", U"5/s");

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
	auto it = std::find_if(m_commonConfig.arrayUnit.begin(), m_commonConfig.arrayUnit.end(),
		[&unitName](const auto& unit) { return unit.NameTag == unitName; });

	if (it == m_commonConfig.arrayUnit.end())
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
			unitsForHsBuildingUnitForAstar.push_back(std::make_unique<Unit>(uu));
			hsBuildingUnitForAstar[uu.initTilePos].push_back(unitsForHsBuildingUnitForAstar.back().get());
		}
	}

	{
		std::scoped_lock lock(unitDataMutex);
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
	std::scoped_lock lock(unitDataMutex);

	//毎タスクで霧gridをfalseにすれば、「生きているユニットの周りだけ明るい」が可能
	// 一度見たタイルは UnseenではなくSeenにしたい
	for (auto&& visibilityElement : visibilityMap)
		visibilityElement = Visibility::Unseen;

	for (auto& units : classBattleManage.listOfAllUnit)
		UpdateVisibility(std::ref(visibilityMap), std::ref(units.ListClassUnit), mapTile.N, mapTile);
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
ClassHorizontalUnit Battle001::getMovableUnits(Array<ClassHorizontalUnit>& source, BattleFormation bf)
{
	ClassHorizontalUnit result;

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
	std::scoped_lock lock(unitDataMutex);
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
	ClassHorizontalUnit chuSor;
	chuSor.FlagBuilding = true;
	{
		for (auto uu : m_commonConfig.arrayUnit)
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

				ClassHorizontalUnit cuu;
				cuu.ListClassUnit.push_back(uu);
				chuSor.ListClassUnit.push_back(uu);

				classBattleManage.hsMyUnitBuilding.insert(&chuSor.ListClassUnit.back());
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
		// 指定した経過時間後に敵ユニットをマップ上にスポーン
		spawnTimedEnemy(classBattleManage, mapTile);

		if (fogUpdateTimer.sF() >= FOG_UPDATE_INTERVAL)
		{
			// 戦場の霧を更新
			refreshFogOfWar(classBattleManage, visibilityMap, mapTile);
			fogUpdateTimer.restart();
		}

		{
			const auto t = camera.createTransformer();
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

			if (MouseR.up())
			{
				Point start = cursPos;
				Point end = Cursor::Pos();

				//部隊を選択状態にする。もしくは既に選択状態なら経路を算出する
				co_await handleRightClickUnitActions(start, end);
			}

		}

		co_await Co::NextFrame();
	}
}

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
	Array<Unit*> buildings;
	for (const auto& group : { classBattleManage.hsMyUnitBuilding,
		classBattleManage.hsEnemyUnitBuilding })
	{
		for (const auto& item : group)
		{
			buildings.push_back(item);
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
				TextureAsset(u->ImageName).size()).drawFrame(BUILDING_FRAME_THICKNESS, Palette::Red);
	}
}
/// @brief カメラビュー内のユニットを描画します。
/// @param cameraView 描画範囲を指定する矩形領域。
/// @param classBattleManage ユニット情報を管理するClassBattleオブジェクト。
void Battle001::drawUnits(const RectF& cameraView, const ClassBattle& classBattleManage) const
{
	std::scoped_lock lock(unitDataMutex);

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
void Battle001::drawBuildDescription() const
{
	if (nowSelectBuildSetumei != U"")
	{
		rectSetumei.draw(Palette::Black);
		fontInfo.fontSkill(nowSelectBuildSetumei).draw(rectSetumei.stretched(-12), Palette::White);
	}
}
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
	const Unit* selectedUnit = nullptr;
	for (const auto& group : classBattleManage.listOfAllUnit)
	{
		for (const auto& unit : group.ListClassUnit)
		{
			if (unit.IsSelect)
			{
				selectedUnit = &unit;
				break;
			}
		}
		if (selectedUnit) break;
	}

	if (!selectedUnit)
		return;

	// ビルドメニューの描画
	const String& targetClassBuild = selectedUnit->classBuild;
	for (const auto& [key, renderTexture] : htBuildMenuRenderTexture)
	{
		if (key == targetClassBuild)
		{
			renderTexture.draw(baseX, baseY);
			break;
		}
	}

	// 建築キューの描画
	if (!selectedUnit->arrYoyakuBuild.empty())
	{
		Rect(baseX - 64 - 6, baseY, 70, 328).drawFrame(4, 0, Palette::Black);

		for (const auto& [i, buildItem] : Indexed(selectedUnit->arrYoyakuBuild))
		{
			if (i == 0)
			{
				// 現在建築中のアイテム
				TextureAsset(buildItem.icon).resized(64).draw(baseX - 64, baseY + 4);

				const double progressRatio = Saturate(selectedUnit->taskTimer.sF() / buildItem.buildTime);
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

	if (longBuildSelectTragetId == -1)
		DrawMiniMap(visibilityMap, camera.getRegion());

}
