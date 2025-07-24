#include "stdafx.h"
#include "Battle001.h"
#include <ranges>
#include <vector>

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
				String aaa = U"ele{}"_fmt(counter);
				const String ele = table[aaa].get<String>();
				sM.ele.emplace(aaa, ele);
				counter++;
				if (ele == U"")
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
				String re = ClassStaticCommonMethod::ReplaceNewLine(String(sv.begin(), sv.end()));
				if (re != U"")
				{
					sM.data.push_back(ClassStaticCommonMethod::ReplaceNewLine(re));
				}
			}
		}
	}

	classBattleManage.classMapBattle = ClassStaticCommonMethod::GetClassMapBattle(sM);
	GetTempResource(classBattleManage.classMapBattle.value());

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
	// 事前に容量を確保して配列の再配置を防ぐ
	const size_t expectedSize = listU.size() + 1;
	if (listU.capacity() < expectedSize + 10) // 余裕をもって容量確保
	{
		listU.reserve(expectedSize + 50);
		Print << U"UnitRegister: 配列容量を拡張しました ({} -> {})"_fmt(
			listU.capacity(), expectedSize + 50);
	}

	//新しいコピーを作る
	for (auto uu : m_commonConfig.arrayUnit)
	{
		if (uu.NameTag == unitName)
		{
			uu.ID = classBattleManage.getIDCount();
			uu.initTilePos = Point{ col,row };
			uu.nowPosiLeft =
				mapTile.ToTile(uu.initTilePos, mapTile.N)
				.asPolygon()
				.centroid()
				.movedBy(-(uu.yokoUnit / 2), -(uu.TakasaUnit / 2));
			uu.taskTimer = Stopwatch();
			uu.taskTimer.reset();
			Vec2 temp = uu.GetNowPosiCenter()
				.movedBy(-64 / 2, (32 / 2) + 8);
			uu.bLiquidBarBattle =
				GameUIToolkit::LiquidBarBattle(Rect(temp.x, temp.y, 64, 8));

			if (enemy)
				uu.moveState = moveState::MoveAI;

			ClassHorizontalUnit cuu;

			for (size_t i = 0; i < num; i++)
			{
				uu.ID = classBattleManage.getIDCount();
				auto copy = uu;
				cuu.ListClassUnit.push_back(copy);
				if (uu.IsBuilding)
				{
					unitsForHsBuildingUnitForAstar.push_back(std::make_unique<Unit>(uu));
					// これなら再配置でもアドレスは変わらない(maybe
					hsBuildingUnitForAstar[uu.initTilePos].push_back(unitsForHsBuildingUnitForAstar.back().get());
				}
			}

			listU.push_back(cuu);
		}
	}
}

/// @brief 指定した経過時間後に敵ユニットをマップ上にスポーン
/// @param classBattleManage 全ての敵ユニットのリストなど、バトル管理に関する情報を持つクラス
/// @param mapTile マップのサイズやタイル情報を持つクラス
void Battle001::spawnTimedEnemy(ClassBattle& classBattleManage, MapTile mapTile)
{
	if (stopwatchGameTime.sF() >= 5)
	{
		int32 kukj = Random(1, 2);
		if (kukj / 2 == 0)
		{
			int32 iyigu = Random(0, mapTile.N - 1);
			UnitRegister(classBattleManage, mapTile, U"sniperP99",
				0,
				Random(0, mapTile.N - 1),
				1, classBattleManage.listOfAllEnemyUnit, true
			);

		}
		else
		{
			int32 iyigu = Random(0, mapTile.N - 1);
			UnitRegister(classBattleManage, mapTile, U"sniperP99",
				Random(0, mapTile.N - 1),
				mapTile.N - 1,
				1, classBattleManage.listOfAllEnemyUnit, true
			);
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
		for (int dy = -unit.visionRadius; dy <= unit.visionRadius; ++dy)
		{
			for (int dx = -unit.visionRadius; dx <= unit.visionRadius; ++dx)
			{
				Vec2 pos = unit.GetNowPosiCenter();
				if (const auto index = mapTile.ToIndex(unit.GetNowPosiCenter(), mapTile.columnQuads, mapTile.rowQuads))
				{
					Point p = index.value() + Point{ dx, dy };
					if (InRange(p.x, 0, mapSize - 1) && InRange(p.y, 0, mapSize - 1))
						if (p.manhattanDistanceFrom(index.value()) <= unit.visionRadius)
							vis[p] = Visibility::Visible;
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
	//毎タスクで霧gridをfalseにすれば、「生きているユニットの周りだけ明るい」が可能
	// 一度見たタイルは UnseenではなくSeenにしたい
	for (auto&& ttt : visibilityMap)
		ttt = Visibility::Unseen;

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
	const int32 count = units.size();
	const int32 centerOffset = (count - 1) / 2;

	double angleForward = (start == end) ? 0.0 : Math::Atan2(end.y - start.y, end.x - start.x);
	double anglePerpendicular = Math::Pi / 2 - angleForward;

	for (auto&& [i, unit] : Indexed(units))
	{
		// 変数名はcopilot君
		double cosPerpendicular = Math::Cos(anglePerpendicular);
		double sinPerpendicular = Math::Sin(anglePerpendicular);

		double distance_between_units_cos = Math::Round(DistanceBetweenUnitWidth * cosPerpendicular);
		int32 unit_spacing_offset_factor = (i - centerOffset);
		int32 unit_spacing_offset_x = unit_spacing_offset_factor * distance_between_units_cos;
		int32 unit_spacing_offset_y = unit_spacing_offset_factor * Math::Round(DistanceBetweenUnitWidth * sinPerpendicular);

		double rowOffsetX = rowIndex * DistanceBetweenUnitHeight * Math::Cos(angleForward);
		double rowOffsetY = rowIndex * DistanceBetweenUnitHeight * Math::Sin(angleForward);

		double x = end.x + unit_spacing_offset_x - rowOffsetX;
		double y = end.y - unit_spacing_offset_y - rowOffsetY;

		unit->orderPosiLeft = Vec2(Floor(end.x), Floor(end.y))
			.movedBy(-(unit->yokoUnit / 2), -(unit->TakasaUnit / 2));
		unit->orderPosiLeftLast = unit->orderPosiLeft = Vec2(Floor(x), Floor(y))
			.movedBy(-(unit->yokoUnit / 2), -(unit->TakasaUnit / 2));
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

Co::Task<void> Battle001::handleRightClickUnitActions(Point start, Point end)
{
	if (is移動指示 == true)
	{
		if (arrayBattleZinkei[0] == true)
		{
			for (auto& target : classBattleManage.listOfAllUnit)
				for (auto& unit : target.ListClassUnit)
				{
					unit.orderPosiLeft = end.movedBy(Random(-10, 10), Random(-10, 10));
					unit.orderPosiLeftLast = unit.orderPosiLeft;
					unit.moveState = moveState::MoveAI;
				}
		}
		else if (arrayBattleZinkei[1] == true)
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
		else
		{
			//正方
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

		is移動指示 = false;

		aStar.abortAStarMyUnits = false;
	}
	else
	{
		////範囲選択

		// 範囲選択の矩形を生成（start, endの大小関係を吸収）
		const RectF selectionRect = RectF::FromPoints(start, end);

		/// lockについて聞く
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

	co_return;
}
/// @brief バトルシーンのメインループを開始し、スペースキーが押されたときに一時停止画面を表示します。
/// @return 非同期タスク（Co::Task<void>）を返します。
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
		if (shouldExit == false)
			co_return;

		camera.update();
		resourcePointTooltip.setCamera(camera);
		// 指定した経過時間後に敵ユニットをマップ上にスポーン
		spawnTimedEnemy(classBattleManage, mapTile);

		if (fogUpdateTimer.sF() >= 0.5)
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

			const auto& tile = classBattleManage.classMapBattle.value().mapData[index.x][index.y];
			TextureAsset(tile.tip + U".png").draw(Arg::bottomCenter = pos);
		}
	}
}
/// @brief カメラビューとマップタイル、可視性マップに基づいてフォグ（霧）を描画します。
/// @param cameraView 描画範囲を指定するカメラの矩形領域。
/// @param mapTile 描画対象となるマップタイルの情報。
/// @param visibilityMap 各タイルの可視状態を示すグリッド。
void Battle001::drawFog(const RectF& cameraView, const MapTile& mapTile, const Grid<Visibility> visibilityMap) const
{
	for (int32 i = 0; i < (mapTile.N * 2 - 1); ++i)
	{
		int32 xi = (i < (mapTile.N - 1)) ? 0 : (i - (mapTile.N - 1));
		int32 yi = (i < (mapTile.N - 1)) ? i : (mapTile.N - 1);

		for (int32 k = 0; k < (mapTile.N - Abs(mapTile.N - i - 1)); ++k)
		{
			Point index{ xi + k, yi - k };
			switch (visibilityMap[index])
			{
			case Visibility::Unseen:
				mapTile.ToTile(index, mapTile.N).draw(ColorF{ 0.0, 0.6 });
				break;
			case Visibility::Visible:
				break;
			}
		}
	}
}
/// @brief カメラビュー内の建物を描画します。
/// @param cameraView 描画範囲を指定するカメラの矩形領域。
/// @param classBattleManage バトルの状態やクラス情報を管理するオブジェクト。
/// @param mapTile 描画対象となるマップタイル。
void Battle001::drawBuildings(const RectF& cameraView, const ClassBattle& classBattleManage, const MapTile mapTile) const
{
	Array<Unit*> buildings;
	for (const auto& group : { classBattleManage.hsMyUnitBuilding, classBattleManage.hsEnemyUnitBuilding })
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
				TextureAsset(u->ImageName).size()).drawFrame(3.0, Palette::Red);
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
						TextureAsset(ringA).drawAt(center.movedBy(0, 8));

					TextureAsset(u.ImageName).draw(Arg::center = center);

					if (u.IsSelect)
						TextureAsset(u.ImageName).draw(Arg::center = center).drawFrame(3.0, Palette::Red);

					if (!u.IsBuilding)
						TextureAsset(ringB).drawAt(center.movedBy(0, 16));

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
						const Vec2 exclamationPos = center.movedBy(0, -u.TakasaUnit / 2 - 18);
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

			Circle(pos.movedBy(0, -mapTile.TileThickness - mapTile.TileOffset.y), 16).drawFrame(4, 0, circleColor);
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
		const double thickness = 3.0;
		double offset = Scene::DeltaTime() * 10;
		const Rect rect{ cursPos, Cursor::Pos() - cursPos };
		rect.top().draw(LineStyle::SquareDot(offset), thickness, Palette::Orange);
		rect.right().draw(LineStyle::SquareDot(offset), thickness, Palette::Orange);
		rect.bottom().draw(LineStyle::SquareDot(offset), thickness, Palette::Orange);
		rect.left().draw(LineStyle::SquareDot(offset), thickness, Palette::Orange);
	}
	else
	{
		Line{ cursPos, Cursor::Pos() }.drawArrow(10, Vec2{ 40, 80 }, Palette::Orange);
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


	}
}
