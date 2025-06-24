#pragma once
#include "Battle.hpp"
#include "ClassBattle.h"
# include "GameUIToolkit.h"
#include <ranges>
#include <vector>
#include <iostream>

/// @brief 指定した座標にあるタイルのインデックスを返します。
/// @param pos 座標
/// @param columnQuads 各列のタイルによって構成される四角形の配列
/// @param rowQuads 各行のタイルによって構成される四角形の配列
/// @return タイルのインデックス。指定した座標にタイルが無い場合は none
Optional<Point> ToIndex(const Vec2& pos, const Array<Quad>& columnQuads, const Array<Quad>& rowQuads)
{
	int32 x = -1, y = -1;

	// タイルの列インデックスを調べる
	for (int32 i = 0; i < columnQuads.size(); ++i)
	{
		if (columnQuads[i].intersects(pos))
		{
			x = i;
			break;
		}
	}

	// タイルの行インデックスを調べる
	for (int32 i = 0; i < rowQuads.size(); ++i)
	{
		if (rowQuads[i].intersects(pos))
		{
			y = i;
			break;
		}
	}

	// インデックスが -1 の場合、タイル上にはない
	if ((x == -1) || (y == -1))
	{
		return none;
	}

	return Point{ x, y };
}
Optional<ClassAStar*> SearchMinScore(const Array<ClassAStar*>& ls) {
	int minScore = std::numeric_limits<int>::max();
	int minCost = std::numeric_limits<int>::max();
	Optional<ClassAStar*> targetClassAStar;

	for (const auto& itemClassAStar : ls) {
		if (itemClassAStar->GetAStarStatus() != AStarStatus::Open)
		{
			continue;
		}
		int score = itemClassAStar->GetCost() + itemClassAStar->GetHCost();

		if (score < minScore || (score == minScore && itemClassAStar->GetCost() < minCost)) {
			minScore = score;
			minCost = itemClassAStar->GetCost();
			targetClassAStar = itemClassAStar;
		}
	}

	return targetClassAStar;
}
int32 BattleMoveAStarMyUnits(Array<ClassHorizontalUnit>& target,
						Array<ClassHorizontalUnit>& enemy,
						Array<Array<MapDetail>> mapData,
						HashTable<int64, Array<Point>>& aiRoot,
						Array<Array<Point>>& debugRoot,
						Array<ClassAStar*>& list,
						Array<Quad>& columnQuads,
						Array<Quad>& rowQuads,
						const int32 N,
						const std::atomic<bool>& abort,
						const std::atomic<bool>& pause
)
{
	for (auto& aaa : target)
	{
		if (abort == true)
			break;

		if (aaa.FlagBuilding == true)
			continue;

		for (auto& listClassUnit : aaa.ListClassUnit)
		{
			if (abort == true)
				break;
			if (listClassUnit.IsBuilding == true && listClassUnit.mapTipObjectType == MapTipObjectType::WALL2)
				continue;
			if (listClassUnit.IsBuilding == true && listClassUnit.mapTipObjectType == MapTipObjectType::GATE)
				continue;
			if (listClassUnit.IsBattleEnable == false)//戦闘不能はスキップ
				continue;
			if (listClassUnit.FlagMoving == true)
				continue;
			if (listClassUnit.FlagMovingEnd == false)
				continue;
			if (listClassUnit.FlagMoveAI == false)
				continue;

			Array<Point> listRoot;

			//指定のマップチップを取得
			s3d::Optional<Size> nowIndexEnemy = ToIndex(listClassUnit.orderPosiLeft, columnQuads, rowQuads);
			if (nowIndexEnemy.has_value() == false)
				continue;

			////現在地を開く
			ClassAStarManager classAStarManager(nowIndexEnemy.value().x, nowIndexEnemy.value().y);

			//現在のマップチップを取得
			s3d::Optional<Size> nowIndex = ToIndex(listClassUnit.GetNowPosiCenter(), columnQuads, rowQuads);
			if (nowIndex.has_value() == false)
				continue;

			Optional<ClassAStar*> startAstar = classAStarManager.OpenOne(nowIndex.value().x, nowIndex.value().y, 0, nullptr, N);
			MicrosecClock mc;
			////移動経路取得
			while (true)
			{
				try
				{
					if (abort == true)
						break;

					if (startAstar.has_value() == false)
					{
						listRoot.clear();
						break;
					}

					//Print << U"AAAAAAAAAAAAAAAAA:" + Format(mc.us());
					classAStarManager.OpenAround(startAstar.value(),
													mapData,
													enemy,
													target,
													N
					);
					//Print << U"BBBBBBBBBBBBBBBB:" + Format(mc.us());
					startAstar.value()->SetAStarStatus(AStarStatus::Closed);

					classAStarManager.RemoveClassAStar(startAstar.value());

					if (classAStarManager.GetListClassAStar().size() != 0)
					{
						startAstar = SearchMinScore(classAStarManager.GetListClassAStar());
					}

					if (startAstar.has_value() == false)
						continue;

					//敵まで到達したか
					if (startAstar.value()->GetRow() == classAStarManager.GetEndX() && startAstar.value()->GetCol() == classAStarManager.GetEndY())
					{
						startAstar.value()->GetRoot(listRoot);
						listRoot.reverse();
						break;
					}
				}
				catch (const std::exception&)
				{
					throw;
				}
			}

			if (listRoot.size() != 0)
			{
				aiRoot[listClassUnit.ID] = listRoot;
				debugRoot.push_back(listRoot);
				listClassUnit.FlagMoveAI = false;
			}
		}
	}
	return -1;
}

static cRightMenu& dummyMenu()
{
	static cRightMenu instance;
	return instance;
}

void GetTempResource(ClassMapBattle& cmb)
{
	cmb.mapData[0][10].isResourcePoint = true;
	cmb.mapData[0][10].resourcePointType = resourceKind::Gold;
	cmb.mapData[0][10].resourcePointAmount = 11;
	cmb.mapData[0][10].resourcePointDisplayName = U"金";
	cmb.mapData[0][10].resourcePointIcon = U"david.png";
}

Battle::Battle(GameData& saveData, CommonConfig& commonConfig)
	:FsScene(U"Battle"), m_saveData{ saveData }, m_commonConfig{ commonConfig }, N{ 64 }
	, visibilityMap{ Grid<Visibility>(Size{ N, N }, Visibility::Unseen) }
	, grid{ Grid<int32>(Size{ N, N }) }
	, tempSelectComRight{ dummyMenu() }
{
	const TOMLReader tomlMap{ PATHBASE + PATH_DEFAULT_GAME + U"/016_BattleMap/map001.toml" };
	if (not tomlMap) // もし読み込みに失敗したら
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

	classBattle.classMapBattle = ClassStaticCommonMethod::GetClassMapBattle(sM);
	GetTempResource(classBattle.classMapBattle.value());

	N = classBattle.classMapBattle.value().mapData.size();
	//Vec2 TileOffset{ 48, 24 };
	//int32 TileThickness = 17;
	Vec2 TileOffset{ 50, 25 };
	int32 TileThickness = 15;
	Array<Quad> columnQuads = MakeColumnQuads(N);
	Array<Quad> rowQuads = MakeRowQuads(N);
}
Battle::~Battle()
{
	abort = true;
	abortMyUnits = true;
}

std::unique_ptr<TextureAssetData> MakeTextureAssetData1(const FilePath& path, const TextureDesc textureDesc)
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

void Battle::renB(RenderTexture& ren, cBuildPrepare& cbp, Array<cRightMenu>& arr)
{
	ren = RenderTexture{ 328, 328 };
	ren.clear(ColorF{ 0.5, 0.0 });
	{
		const ScopedRenderTarget2D target{ ren.clear(ColorF{ 0.8, 0.8, 0.8,0.5 }) };
		const ScopedRenderStates2D blend{ MakeBlendState() };

		arr.clear();
		for (auto&& [i, re] : IndexedRef(cbp.htCountAndSyu))
		{
			if (re.second.count == 0)
			{
				continue;
			}

			cRightMenu crh;
			Rect rectBuildMenuHome;
			rectBuildMenuHome.x = ((re.second.sortId % 6) * 64) + 4;
			rectBuildMenuHome.y = ((re.second.sortId / 6) * 64) + 4;
			rectBuildMenuHome.w = 64;
			rectBuildMenuHome.h = 64;
			crh.sortId = cRightMenuCount;
			cRightMenuCount++;
			crh.key = re.first;
			crh.kindForProcess = re.second.kind;
			crh.rect = rectBuildMenuHome;
			crh.count = re.second.count;
			crh.buiSyu = cbp.buiSyu; // 建築の種類
			crh.time = re.second.time;
			arr.push_back(crh);
		}

		for (auto& icons : arr)
			TextureAsset(icons.key).resized(64).draw(icons.rect.x, icons.rect.y);

		Rect df = Rect(328, 328);
		df.drawFrame(4, 0, ColorF{ 0.5 });
	}
}

Co::Task<void> Battle::start()
{
	// png フォルダ内のファイルを列挙する
	for (const auto& filePath : FileSystem::DirectoryContents(U"png/"))
	{
		// ファイル名が conifer と tree で始まるファイル（タイルではない）は除外する
		if (const FilePath baseName = FileSystem::BaseName(filePath);
			baseName.starts_with(U"conifer") || baseName.starts_with(U"tree"))
		{
			continue;
		}

		textures << LoadPremultipliedTexture(filePath);
	}
	for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/040_ChipImage/"))
		TextureAsset::Register(FileSystem::FileName(filePath), filePath);
	for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/015_BattleMapCellImage/"))
		TextureAsset::Register(FileSystem::FileName(filePath), MakeTextureAssetData1(filePath, TextureDesc::Mipped));
	for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/043_ChipImageBuild/"))
		TextureAsset::Register(FileSystem::FileName(filePath), filePath);
	for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/041_ChipImageSkill/"))
		TextureAsset::Register(FileSystem::FileName(filePath), filePath);


	arrayBattleZinkei.push_back(false);
	arrayBattleZinkei.push_back(false);
	arrayBattleZinkei.push_back(false);
	arrayBattleCommand.push_back(false);
	arrayBattleCommand.push_back(false);

	battleStatus = BattleStatus::Battle;

	renderTextureBuildMenuEmpty = RenderTexture{ 328, 328 };
	renderTextureBuildMenuEmpty.clear(ColorF{ 0.5, 0.0 });
	{
		const ScopedRenderTarget2D target{ renderTextureBuildMenuHome.clear(ColorF{ 0.8, 0.8, 0.8,0.5 }) };
		const ScopedRenderStates2D blend{ MakeBlendState() };
		Rect df = Rect(328, 328);
		df.drawFrame(4, 0, ColorF{ 0.5 });
	}

	//建築メニューの初期化
	{
		cbp.htCountAndSyu.clear();
		{
			cBuildPrepareKindAndCount cbpHome;
			cbpHome.sortId = 0;
			cbpHome.count = -1;
			cbpHome.kind = U"Unit";
			cbp.htCountAndSyu.emplace(U"kouhei.png", cbpHome);
		}
		{
			cBuildPrepareKindAndCount cbpHome;
			cbpHome.sortId = 1;
			cbpHome.count = 5;
			cbpHome.kind = U"bahu";
			cbp.htCountAndSyu.emplace(U"inhura-kaizen.png", cbpHome);
		}
		{
			cBuildPrepareKindAndCount cbpHome;
			cbpHome.sortId = 2;
			cbpHome.count = 1;
			cbpHome.kind = U"skill";
			cbp.htCountAndSyu.emplace(U"kayaku.png", cbpHome);
		}
		cbp.buiSyu = 0; // 建築の種類
		renB(renderTextureBuildMenuHome, cbp, arrayComRight_BuildMenu_Home);
	}
	{
		cbpThunderwalker.htCountAndSyu.clear();
		{
			cBuildPrepareKindAndCount cbpHome;
			cbpHome.sortId = 0;
			cbpHome.count = -1;
			cbpHome.kind = U"Unit";
			cbpThunderwalker.htCountAndSyu.emplace(U"david.png", cbpHome);//ゴリアテの反対
		}

		cbpThunderwalker.buiSyu = 1; // 建築の種類
		renB(renderTextureBuildMenuThunderwalker, cbpThunderwalker, arrayComRight_BuildMenu_Thunderwalker);
	}
	{
		cbpKouhei.htCountAndSyu.clear();
		{
			cBuildPrepareKindAndCount cbpHome;
			cbpHome.sortId = 0;
			cbpHome.count = -1;
			cbpHome.kind = U"Unit";
			cbpHome.time = 2.0;
			cbpKouhei.htCountAndSyu.emplace(U"zirai.png", cbpHome);
		}
		{
			cBuildPrepareKindAndCount cbpHome;
			cbpHome.sortId = 1;
			cbpHome.count = -1;
			cbpHome.kind = U"UnitPro";
			cbpHome.time = 1.0;
			cbpKouhei.htCountAndSyu.emplace(U"keisou-hohei.png", cbpHome);
		}

		cbpKouhei.buiSyu = 5; // 建築の種類
		renB(renderTextureBuildMenuKouhei, cbpKouhei, arrayComRight_BuildMenu_Kouhei);
	}
	{
		cbpKeisouHoheiT.htCountAndSyu.clear();
		{
			cBuildPrepareKindAndCount cbpHome;
			cbpHome.sortId = 0;
			cbpHome.count = -1;
			cbpHome.kind = U"Unit";
			cbpKeisouHoheiT.htCountAndSyu.emplace(U"keisou-chipGene006.png", cbpHome);
		}
		cbpKeisouHoheiT.buiSyu = 6; // 建築の種類
		renB(renderTextureBuildMenuKeisouHoheiT, cbpKeisouHoheiT, arrayComRight_BuildMenu_KeisouHoheiT);
	}

	//ユニットの初期化
	{
		for (auto uu : m_commonConfig.arrayUnit)
		{
			if (uu.Name == U"M14 Infantry Rifle")
			{
				uu.ID = classBattle.getIDCount();
				uu.buiSyu = 1;
				uu.initTilePos = Point{ 4, 0 };
				uu.nowPosiLeft = ToTile(uu.initTilePos, N).asPolygon().centroid().movedBy(-(uu.yokoUnit / 2), -(uu.TakasaUnit / 2));
				ClassHorizontalUnit cuu;
				cuu.ListClassUnit.push_back(uu);
				classBattle.listOfAllUnit.push_back(cuu);
			}
		}
	}
	{
		Unit uu;
		uu.ID = classBattle.getIDCount();
		uu.IsBuilding = false;

		uu.initTilePos = Point{ 2, 2 };//保険
		uu.orderPosiLeft = Point{ 0, 0 };
		uu.orderPosiLeftLast = Point{ 0, 0 };
		uu.nowPosiLeft = ToTile(uu.initTilePos, N).asPolygon().centroid().movedBy(-(uu.yokoUnit / 2), -(uu.TakasaUnit / 2));
		uu.vecMove = Vec2{ 0, 0 };
		uu.Speed = 1.0;
		uu.Move = 500.0;
		uu.FlagMove = false;
		uu.FlagMoving = false;
		uu.IsBattleEnable = true;
		uu.buiSyu = 5;
		uu.Image = U"chip006.png";
		uu.Hp = 100;
		uu.HpMAX = 100;
		ClassHorizontalUnit cuu;
		cuu.ListClassUnit.push_back(uu);

		classBattle.listOfAllUnit.push_back(cuu);
	}

	//敵ユニットの初期化
	{
		Unit uu;
		uu.ID = classBattle.getIDCount();
		uu.IsBuilding = false;
		uu.initTilePos = Point{ 0, 4 };
		uu.orderPosiLeft = Point{ 0, 0 };
		uu.orderPosiLeftLast = Point{ 0, 0 };
		uu.nowPosiLeft = ToTile(uu.initTilePos, N).asPolygon().centroid().movedBy(-(uu.yokoUnit / 2), -(uu.TakasaUnit / 2));
		//uu.nowPosiLeft = ToTile(uu.initTilePos, N).movedBy(uu.yokoUnit / 2, uu.TakasaUnit / 2).asPolygon().centroid();
		uu.vecMove = Vec2{ 0, 0 };
		uu.Speed = 1.0;
		uu.FlagMove = false;
		uu.FlagMoving = false;
		uu.Image = U"chipGene007.png";
		//uu.FlagBattleEnable = true;

		ClassHorizontalUnit cuu;
		cuu.ListClassUnit.push_back(uu);

		classBattle.listOfAllEnemyUnit.push_back(cuu);
	}

	renderTextureSkill = RenderTexture{ 320,320 };
	renderTextureSkill.clear(ColorF{ 0.5, 0.0 });
	{
		const ScopedRenderTarget2D target{ renderTextureSkill.clear(ColorF{ 0.8, 0.8, 0.8,0.5 }) };
		const ScopedRenderStates2D blend{ MakeBlendState() };

		//skill抽出
		Array<Skill> table;
		for (auto& item : classBattle.listOfAllUnit)
		{
			if (item.ListClassUnit.empty())
				continue;
			for (auto& itemUnit : item.ListClassUnit)
			{
				for (auto& ski : itemUnit.Skill)
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

	//始点設定
	viewPos = ToTileBottomCenter(classBattle.listOfAllUnit[0].ListClassUnit[0].initTilePos, N);
	camera.jumpTo(viewPos, camera.getTargetScale());

	//ユニット体力バーの設定
	for (auto& item : classBattle.listOfAllUnit)
	{
		if (!item.FlagBuilding &&
			!item.ListClassUnit.empty())
		{
			for (auto& itemUnit : item.ListClassUnit)
			{
				Vec2 temp = itemUnit.GetNowPosiCenter().movedBy(-64 / 2, (32 / 2) + 6);
				itemUnit.bLiquidBarBattle = GameUIToolkit::LiquidBarBattle(Rect(temp.x, temp.y, 64, 16));
			}
		}
	}
	for (auto& item : classBattle.listOfAllEnemyUnit)
	{
		if (!item.FlagBuilding &&
			!item.ListClassUnit.empty())
		{
			for (auto& itemUnit : item.ListClassUnit)
			{
				Vec2 temp = itemUnit.GetNowPosiCenter().movedBy(-64 / 2, (32 / 2) + 6);
				itemUnit.bLiquidBarBattle = GameUIToolkit::LiquidBarBattle(Rect(temp.x, temp.y, 64, 16));
			}
		}
	}

	//建物関係
	ClassHorizontalUnit chuSor;
	ClassHorizontalUnit chuDef;
	ClassHorizontalUnit chuNa;
	chuSor.FlagBuilding = true;
	chuDef.FlagBuilding = true;
	chuNa.FlagBuilding = true;

	{
		Unit unitBui;
		unitBui.IsBuilding = true;
		unitBui.ID = classBattle.getIDCount();
		unitBui.mapTipObjectType = MapTipObjectType::HOME;
		unitBui.NoWall2 = 0;
		unitBui.HPCastle = 1000;
		unitBui.CastleDefense = 1000;
		unitBui.CastleMagdef = 1000;
		unitBui.Image = U"home1.png";
		unitBui.rowBuilding = 1;
		unitBui.colBuilding = 1;
		chuSor.ListClassUnit.push_back(unitBui);
	}
	{
		Unit unitBui;
		unitBui.IsBuilding = true;
		unitBui.ID = classBattle.getIDCount();
		unitBui.mapTipObjectType = MapTipObjectType::HOME;
		unitBui.NoWall2 = 0;
		unitBui.HPCastle = 1000;
		unitBui.CastleDefense = 1000;
		unitBui.CastleMagdef = 1000;
		unitBui.Image = U"home2.png";
		unitBui.rowBuilding = N - 2;
		unitBui.colBuilding = N - 2;
		chuDef.ListClassUnit.push_back(unitBui);
	}

	//for (size_t indexRow = 0; indexRow < cb.classMapBattle.value().mapData.size(); ++indexRow)
	//{
	//	for (size_t indexCol = 0; indexCol < cb.classMapBattle.value().mapData[indexRow].size(); ++indexCol)
	//	{
	//		for (auto& bui : cb.classMapBattle.value().mapData[indexRow][indexCol].building)
	//		{
	//			String key = std::get<0>(bui);
	//			BattleWhichIsThePlayer bw = std::get<2>(bui);

	//			// arrayClassObjectMapTip から適切な ClassObjectMapTip オブジェクトを見つける
	//			for (const auto& mapTip : getData().classGameStatus.arrayClassObjectMapTip)
	//			{
	//				if (mapTip.nameTag == key)
	//				{
	//					// ClassUnit の設定を行う
	//					Unit unitBui;
	//					unitBui.IsBuilding = true;
	//					unitBui.ID = getData().classGameStatus.getIDCount();
	//					std::get<1>(bui) = unitBui.ID;
	//					unitBui.mapTipObjectType = mapTip.type;
	//					unitBui.NoWall2 = mapTip.noWall2;
	//					unitBui.HPCastle = mapTip.castle;
	//					unitBui.CastleDefense = mapTip.castleDefense;
	//					unitBui.CastleMagdef = mapTip.castleMagdef;
	//					unitBui.Image = mapTip.nameTag;
	//					unitBui.rowBuilding = indexRow;
	//					unitBui.colBuilding = indexCol;

	//					if (bw == BattleWhichIsThePlayer::Sortie)
	//					{
	//						chuSor.ListClassUnit.push_back(unitBui);
	//					}
	//					else if (bw == BattleWhichIsThePlayer::Def)
	//					{
	//						chuDef.ListClassUnit.push_back(unitBui);
	//					}
	//					else
	//					{
	//						chuNa.ListClassUnit.push_back(unitBui);
	//					}
	//					break; // 適切なオブジェクトが見つかったのでループを抜ける
	//				}
	//			}
	//		}
	//	}
	//}
	classBattle.listOfAllUnit.push_back(chuSor);
	classBattle.listOfAllEnemyUnit.push_back(chuDef);
	//cb.neutralUnitGroup.push_back(chuNa);

	//// 建物初期位置
	//for (auto& item : classBattle.listOfAllUnit)
	//{
	//	if (item.FlagBuilding == true &&
	//		!item.ListClassUnit.empty())
	//	{
	//		for (auto& itemUnit : item.ListClassUnit)
	//		{
	//			Point pt = Point(itemUnit.rowBuilding, itemUnit.colBuilding);
	//			Vec2 vv = ToTileBottomCenter(pt, N);
	//			vv = { vv.x,vv.y - (25 + 15) };
	//			itemUnit.nowPosiLeft = vv;
	//		}
	//	}
	//}
	//for (auto& item : classBattle.listOfAllEnemyUnit)
	//{
	//	if (item.FlagBuilding == true &&
	//		!item.ListClassUnit.empty())
	//	{
	//		for (auto& itemUnit : item.ListClassUnit)
	//		{
	//			Point pt = Point(itemUnit.rowBuilding, itemUnit.colBuilding);
	//			Vec2 vv = ToTileBottomCenter(pt, N);
	//			vv = { vv.x,vv.y - (25 + 15) };
	//			itemUnit.nowPosiLeft = vv;
	//		}
	//	}
	//}

	stopwatchFinance.restart();

	co_await mainLoop().pausedWhile([&]
		{
			if (KeySpace.pressed())
			{
				stopwatch.pause();
				stopwatch001.pause();
				stopwatch002.pause();
				stopwatch003.pause();
				Rect rectPauseBack{ 0, 0, Scene::Width(), Scene::Height() };
				rectPauseBack.draw(ColorF{ 0.0, 0.0, 0.0, 0.5 });
				const String pauseText = U"Pause";

				Rect rectPause{ int32(Scene::Width() / 2 - systemFont(pauseText).region().w / 2),
								0,
								int32(systemFont(pauseText).region().w),
								int32(systemFont(pauseText).region().h) };
				rectPause.draw(Palette::Black);
				systemFont(pauseText).drawAt(rectPause.x + rectPause.w / 2, rectPause.y + rectPause.h / 2, Palette::White);

				return true;
			}
			else
			{
				stopwatch.resume();
				stopwatch001.resume();
				stopwatch002.resume();
				stopwatch003.resume();
			}
		});; // メインループ実行
}

Co::Task<void> Battle::mainLoop()
{
	while (true)
	{
		if (shouldExit == false)
			co_return;

		camera.update();

		//stopwatchFinanceが一秒経過する度に処理を行う
		if (stopwatchFinance.sF() >= 1.0)
		{
			int32 goldInc = 0;
			int32 trustInc = 0;
			int32 foodInc = 0;

			for (auto ttt : classBattle.classMapBattle.value().mapData)
			{
				for (auto jjj : ttt)
				{
					if (jjj.whichIsThePlayer == BattleWhichIsThePlayer::Sortie)
					{
						//資金の増加
						switch (jjj.resourcePointType)
						{
						case resourceKind::Gold:
							goldInc += jjj.resourcePointAmount;
							break;
						case resourceKind::Trust:
							trustInc += jjj.resourcePointAmount;
							break;
						case resourceKind::Food:
							foodInc += jjj.resourcePointAmount;
							break;
						default:
							break;
						}
					}
				}
			}

			stopwatchFinance.restart();
			gold += 10 + goldInc; // 1秒ごとに10ゴールド増加
			trust += 1 + trustInc; // 1秒ごとに1権勢増加
			food += 5 + foodInc; // 1秒ごとに5食料増加
		}

		//毎タスクで霧gridをfalseにすれば、「生きているユニットの周りだけ明るい」が可能
		// 一度見たタイルは UnseenではなくSeenにしたい
		for (auto&& ttt : visibilityMap)
			ttt = Visibility::Unseen;

		for (auto& units : classBattle.listOfAllUnit)
			UpdateVisibility(std::ref(visibilityMap), std::ref(units.ListClassUnit), N);

		//後でbattle内に移動(ポーズ処理を考慮
		// 進行度（0.0 ～ 1.0）
		if (arrBuildMenuHomeYoyaku.size() > 0)
		{
			if (t >= 1.0)
			{
				stopwatch.reset();
				String key = arrBuildMenuHomeYoyaku.front().key;
				String kindForProcess = arrBuildMenuHomeYoyaku.front().kindForProcess;
				int32 cou = arrBuildMenuHomeYoyaku.front().count;
				arrBuildMenuHomeYoyaku.pop_front();
				if (arrBuildMenuHomeYoyaku.size() > 0)
				{
					t = 0.0;
					stopwatch.start();
				}
				else
				{
					t = -1.0;
				}

				// 実行
				if (key == U"kouhei.png")
				{
					{
						Unit uu;
						uu.ID = classBattle.getIDCount();
						uu.IsBuilding = false;

						for (auto& item : classBattle.listOfAllUnit)
						{
							if (item.FlagBuilding == true &&
								!item.ListClassUnit.empty())
							{
								for (auto& itemUnit : item.ListClassUnit)
								{
									if (itemUnit.mapTipObjectType == MapTipObjectType::HOME)
									{
										uu.initTilePos = Point{ itemUnit.colBuilding + 1, itemUnit.rowBuilding + 1 };
									}
								}
							}
						}
						uu.orderPosiLeft = Point{ 0, 0 };
						uu.orderPosiLeftLast = Point{ 0, 0 };
						uu.nowPosiLeft = ToTile(uu.initTilePos, N).asPolygon().centroid().movedBy(-(uu.yokoUnit / 2), -(uu.TakasaUnit / 2));
						uu.vecMove = Vec2{ 0, 0 };
						uu.Speed = 1.0;
						uu.Move = 500.0;
						uu.FlagMove = false;
						uu.FlagMoving = false;
						uu.IsBattleEnable = true;
						uu.buiSyu = 5;
						uu.Image = U"chip006.png";

						ClassHorizontalUnit cuu;
						cuu.ListClassUnit.push_back(uu);

						classBattle.listOfAllUnit.push_back(cuu);
					}
				}
				else if (key == U"inhura-kaizen.png")
				{

				}
				else if (key == U"kayaku.png")
				{
				}
				else if (key == U"")
				{

				}
				else if (key == U"")
				{

				}
				else if (key == U"")
				{

				}
			}
			t = Min(stopwatch.sF() / durationSec, 1.0);
		}
		if (arrBuildMenuThunderwalkerYoyaku.size() > 0)
		{
			if (arrT[1] >= 1.0)
			{
				stopwatch001.reset();
				String key = arrBuildMenuThunderwalkerYoyaku.front().key;
				String syu = arrBuildMenuThunderwalkerYoyaku.front().kindForProcess;
				int32 cou = arrBuildMenuThunderwalkerYoyaku.front().count;
				arrBuildMenuThunderwalkerYoyaku.pop_front();
				if (arrBuildMenuThunderwalkerYoyaku.size() > 0)
				{
					arrT[1] = 0.0;
					stopwatch001.start();
				}
				else
				{
					arrT[1] = -1.0;
				}

				// 実行
			}
			arrT[1] = Min(stopwatch001.sF() / durationSec, 1.0);
		}
		if (arrBuildMenuKouheiYoyaku.size() > 0)
		{
			double tempTime = arrBuildMenuKouheiYoyaku.front().time;
			if (arrT[2] >= 1.0)
			{
				stopwatch002.reset();
				String key = arrBuildMenuKouheiYoyaku.front().key;
				String syu = arrBuildMenuKouheiYoyaku.front().kindForProcess;
				int32 cou = arrBuildMenuKouheiYoyaku.front().count;
				int32 rowBuildingTarget = arrBuildMenuKouheiYoyaku.front().rowBuilding;
				int32 colBuildingTarget = arrBuildMenuKouheiYoyaku.front().colBuilding;

				//場所と建築コマンドを紐づける
				for (auto&& [i, re] : IndexedRef(arrayComRight_BuildMenu_Kouhei))
				{
					if (re.sortId == arrBuildMenuKouheiYoyaku.front().sortId)
					{
						arrayComRight_BuildMenu_KeisouHoheiT[0].rowBuilding = rowBuildingTarget;
						arrayComRight_BuildMenu_KeisouHoheiT[0].colBuilding = colBuildingTarget;
					}
				}

				arrBuildMenuKouheiYoyaku.pop_front();
				if (arrBuildMenuKouheiYoyaku.size() > 0)
				{
					arrT[2] = 0.0;
					stopwatch002.start();
				}
				else
				{
					arrT[2] = -1.0;
				}

				// 実行

				if (key == U"keisou-hohei.png")
				{
					{
						Unit uu;
						uu.ID = classBattle.getIDCount();
						uu.IsBuilding = true;
						uu.initTilePos = Point{ colBuildingTarget, rowBuildingTarget };
						uu.nowPosiLeft = ToTile(uu.initTilePos, N).asPolygon().centroid().movedBy(-(uu.yokoUnit / 2), -(uu.TakasaUnit / 2));
						uu.rowBuilding = rowBuildingTarget;
						uu.colBuilding = colBuildingTarget;
						uu.NoWall2 = 0;
						uu.HPCastle = 1000;
						uu.CastleDefense = 1000;
						uu.CastleMagdef = 1000;
						uu.buiSyu = 6;
						uu.Image = U"bu-keiso.png";
						Vec2 temp = uu.GetNowPosiCenter().movedBy(-64 / 2, (32 / 2) + 6);
						uu.bLiquidBarBattle = GameUIToolkit::LiquidBarBattle(Rect(temp.x, temp.y, 64, 16));

						ClassHorizontalUnit cuu;
						cuu.FlagBuilding = true;
						cuu.ListClassUnit.push_back(uu);

						classBattle.listOfAllUnit.push_back(cuu);
					}
				}

			}
			arrT[2] = Min(stopwatch002.sF() / tempTime, 1.0);
		}
		if (arrBuildMenuKeisouYoyaku.size() > 0)
		{
			double tempTime = arrBuildMenuKeisouYoyaku.front().time;
			if (arrT[3] >= 1.0)
			{
				stopwatch003.reset();
				String key = arrBuildMenuKeisouYoyaku.front().key;
				String syu = arrBuildMenuKeisouYoyaku.front().kindForProcess;
				int32 cou = arrBuildMenuKeisouYoyaku.front().count;
				int32 rowBuildingTarget = arrBuildMenuKeisouYoyaku.front().rowBuilding;
				int32 colBuildingTarget = arrBuildMenuKeisouYoyaku.front().colBuilding;
				arrBuildMenuKeisouYoyaku.pop_front();
				if (arrBuildMenuKeisouYoyaku.size() > 0)
				{
					arrT[3] = 0.0;
					stopwatch003.start();
				}
				else
				{
					arrT[3] = -1.0;
				}

				// 実行

				if (key == U"keisou-chipGene006.png")
				{
					{
						Unit uu;
						uu.IsBuilding = false;

						uu.initTilePos =
							Point{ arrayComRight_BuildMenu_KeisouHoheiT[0].colBuilding,
									arrayComRight_BuildMenu_KeisouHoheiT[0].rowBuilding };
						uu.orderPosiLeft = Point{ 0, 0 };
						uu.orderPosiLeftLast = Point{ 0, 0 };
						uu.nowPosiLeft = ToTile(uu.initTilePos, N).asPolygon().centroid().movedBy(-(uu.yokoUnit / 2), -(uu.TakasaUnit / 2));
						uu.vecMove = Vec2{ 0, 0 };
						uu.Speed = 1.0;
						uu.Move = 500.0;
						uu.FlagMove = false;
						uu.FlagMoving = false;
						uu.IsBattleEnable = true;
						uu.Hp = 100;
						uu.HpMAX = 100;
						uu.Image = U"chipGene006.png";
						Vec2 temp = uu.GetNowPosiCenter().movedBy(-64 / 2, (32 / 2) + 6);
						uu.bLiquidBarBattle = GameUIToolkit::LiquidBarBattle(Rect(temp.x, temp.y, 64, 16));

						ClassHorizontalUnit cuu;
						for (size_t i = 0; i < 3; i++)
						{
							uu.ID = classBattle.getIDCount();
							cuu.ListClassUnit.push_back(uu);
						}

						classBattle.listOfAllUnit.push_back(cuu);
					}
				}

			}
			arrT[3] = Min(stopwatch003.sF() / tempTime, 1.0);
		}

		if (isMovedYoyaku == true)
		{
			if (aiRoot[longIsMovedYoyakuId].size() > 0)
			{
				//移動中
			}
			else
			{
				isMovedYoyaku = false;//移動が終わった為
				longIsMovedYoyakuId = -1; // IDをリセット

				Array<cRightMenu> temp;
				if (buiSyu == 0)
				{
					temp = arrayComRight_BuildMenu_Home;
				}
				else if (buiSyu == 1)
				{
					temp = arrayComRight_BuildMenu_Thunderwalker;
				}
				else if (buiSyu == 5)
				{
					temp = arrayComRight_BuildMenu_Kouhei;
				}
				else if (buiSyu == 6)
				{
					temp = arrayComRight_BuildMenu_KeisouHoheiT;
				}

				for (auto&& [i, re] : IndexedRef(temp))
				{
					if (re.sortId == cRightMenuTargetCount)
					{
						cRightMenu ccc;
						ccc.sortId = cRightMenuTargetCount;
						ccc.sortYoyakuId = longBuildMenuHomeYoyakuIdCount;
						ccc.key = re.key;
						ccc.kindForProcess = re.kindForProcess;
						ccc.texture = TextureAsset(re.key);
						ccc.time = re.time;
						ccc.rowBuilding = rowBuildingTarget;
						ccc.colBuilding = colBuildingTarget;
						rowBuildingTarget = -1; // リセット
						colBuildingTarget = -1; // リセット
						longBuildMenuHomeYoyakuIdCount++;
						if (re.buiSyu == 0)
						{
							if (stopwatch.isRunning() == false)
							{
								stopwatch.start();
								t = 0.0;
							}
							arrBuildMenuHomeYoyaku.push_back(ccc);
						}
						else if (re.buiSyu == 1)
						{
							if (stopwatch001.isRunning() == false)
							{
								stopwatch001.start();
								arrT[1] = 0.0;
							}
							arrBuildMenuThunderwalkerYoyaku.push_back(ccc);
						}
						else if (re.buiSyu == 5)
						{
							if (stopwatch002.isRunning() == false)
							{
								stopwatch002.start();
								arrT[2] = 0.0;
							}
							arrBuildMenuKouheiYoyaku.push_back(ccc);
						}
						else if (re.buiSyu == 6)
						{
							if (stopwatch003.isRunning() == false)
							{
								stopwatch003.start();
								arrT[3] = 0.0;
							}
							arrBuildMenuKeisouYoyaku.push_back(ccc);
						}

						//回数制限のある建物を選択した場合
						if (re.count > 0)
						{
							re.count--;
							cbp.htCountAndSyu[re.key].count = re.count;
							if (re.buiSyu == 0)
							{
								renB(renderTextureBuildMenuHome, cbp, arrayComRight_BuildMenu_Home);
							}
							else if (re.buiSyu == 1)
							{
								renB(renderTextureBuildMenuKouhei, cbpKouhei, arrayComRight_BuildMenu_Kouhei);
							}
						}
					}
				}
			}
		}
		else if (isGetResource == true)
		{
			if (aiRoot[longIsGetResourceId].size() > 0)
			{
				//移動中
			}
			else
			{
				isGetResource = false;//移動が終わった為

				resourceWait resWait;
				resWait.id = longIsGetResourceId;
				longIsGetResourceId = -1; // IDをリセット
				resWait.waitTime = GetCU(resWait.id).waitTimeResource;
				resWait.stopwatch.restart();
				resWait.rowResourceTarget = rowResourceTarget;
				resWait.colResourceTarget = colResourceTarget;
				rowResourceTarget = -1; // リセット
				colResourceTarget = -1; // リセット
				arrResourceWait.push_back(resWait);
			}
		}

		/// 資源ポイント獲得処理
		Array<int32> deleteId;
		for (const auto& ttt : arrResourceWait)
		{
			if (ttt.stopwatch.sF() >= ttt.waitTime)
			{
				classBattle.classMapBattle.value().mapData[ttt.colResourceTarget][ttt.rowResourceTarget]
					.whichIsThePlayer = BattleWhichIsThePlayer::Sortie;
				classBattle.classMapBattle.value().mapData[ttt.colResourceTarget][ttt.rowResourceTarget]
					.resourcePointAmount += 7;
				// 待機リストから削除
				deleteId.push_back(ttt.id);
			}
		}
		for (auto id : deleteId)
		{
			arrResourceWait.remove_if([&](const resourceWait& rw) { return rw.id == id; });
		}

		switch (battleStatus)
		{
		case BattleStatus::Battle:
		{
			//カメラ移動 || 部隊を選択状態にする。もしくは既に選択状態なら移動させる
			{
				const auto t = camera.createTransformer();

				// 建築物選択チェック
				bool isSeBu = false;
				{
					// いったん押された建築物の固有IDを走査する
					// 対象以外のIsSelectを全てfalseにしたいが為
					long id = -1;
					for (auto& item : classBattle.listOfAllUnit)
					{
						if (item.FlagBuilding == true &&
							!item.ListClassUnit.empty())
						{
							for (auto& itemUnit : item.ListClassUnit)
							{
								Size tempSize = TextureAsset(itemUnit.Image).size();
								Quad tempQ = ToTile(Point(itemUnit.colBuilding, itemUnit.rowBuilding), N);
								const Vec2 leftCenter = (tempQ.p0 + tempQ.p3) / 2.0;
								const Vec2 rightCenter = (tempQ.p1 + tempQ.p2) / 2.0;
								const double horizontalWidth = Abs(rightCenter.x - leftCenter.x);
								const double vHe = Abs(rightCenter.y - leftCenter.y);
								double scale = tempSize.x / (horizontalWidth * 2);

								if (tempQ.scaled(scale).movedBy(0, -(vHe * scale) + TileThickness).leftClicked())
								{
									id = itemUnit.ID;
								}
							}
						}
					}

					if (id != -1)
					{
						for (auto& item : classBattle.listOfAllUnit)
						{
							for (auto& itemUnit : item.ListClassUnit)
							{
								if (id == itemUnit.ID)
								{
									itemUnit.IsSelect = !itemUnit.IsSelect;
									IsBuildMenuHome = itemUnit.IsSelect;
									buiSyu = itemUnit.buiSyu; // 建築の種類
									isSeBu = true;
								}
								else
								{
									itemUnit.IsSelect = false;
								}
							}
						}
					}
				}

				// ユニット選択チェック
				{
					if (isSeBu == false)
					{
						//全ユニット走査、posからrectを生成し、クリックされたかチェック
						for (auto& target : classBattle.listOfAllUnit)
						{
							for (auto& unit : target.ListClassUnit)
							{
								if ((unit.IsBuilding == false && unit.IsBattleEnable == true)
									|| (unit.IsBuilding == true && unit.IsBattleEnable == true && unit.buiSyu != -1))
								{
									RectF rect = RectF(unit.nowPosiLeft, unit.yokoUnit, unit.TakasaUnit);
									if (rect.leftClicked())
									{
										//クリックされたユニットのIDを取得
										//そのユニットのIsSelectを反転させる
										//IsBuildMenuHomeを反転させる
										//他のユニットのIsSelectはfalseにする
										for (auto& item : classBattle.listOfAllUnit)
										{
											for (auto& itemUnit : item.ListClassUnit)
											{
												if (unit.ID == itemUnit.ID)
												{

												}
												else
												{
													itemUnit.IsSelect = false;
												}
											}
										}
										unit.IsSelect = !unit.IsSelect;
										IsBuildMenuHome = unit.IsSelect;
										buiSyu = unit.buiSyu; // 建築の種類
										longBuildSelectTragetId = unit.ID; // 選択されたユニットのIDを保存
										IsResourceSelectTraget = true;
									}
								}
							}
						}
					}
				}

				//カメラ移動
				if (MouseL.pressed() == true)
				{
					const auto vPos = (camera.getTargetCenter() - Cursor::Delta());
					camera.jumpTo(vPos, camera.getTargetScale());
				}
				if (MouseR.pressed() == false && IsBattleMove == false)
				{
					if (MouseR.up() == false)
					{
						cursPos = Cursor::Pos();
					}
				}
				else if (MouseR.pressed() == false && IsBattleMove == true)
				{
					if (MouseR.up() == false)
					{
						cursPos = Cursor::Pos();
					}
				}
				else if (MouseR.down() == true && IsBattleMove == true)
				{
					cursPos = Cursor::Pos();
				}

				//部隊を選択状態にする。もしくは既に選択状態なら経路を算出する
				if (MouseR.up() == true)
				{
					Point start = cursPos;
					Point end = Cursor::Pos();

					if (IsBattleMove == true)
					{
						Array<Unit*> lisUnit;
						for (auto& target : classBattle.listOfAllUnit) {
							for (auto& unit : target.ListClassUnit)
							{
								if (unit.FlagMove == true && unit.IsBattleEnable == true)
									lisUnit.push_back(&unit);
							}
						}

						//if (lisUnit.size() == 1)
						//{
						//	Unit* cu = lisUnit[0];
						//	// 移動先の座標算出
						//	Vec2 nor = Vec2(end - start).normalized();
						//	Vec2 moved = cu->nowPosiLeft + Vec2(nor.x * cu->Speed, nor.y * cu->Speed);
						//	// 移動先が有効かどうかチェック || 本来は経路探索で移動可能かどうか調べるべき
						//	auto index = ToIndex(moved, columnQuads, rowQuads);
						//	if (not index.has_value())
						//	{
						//		cu->FlagMove = false;
						//		//co_return;
						//		co_await Co::NextFrame();
						//	}

						//	//移動
						//	Unit& cuu = GetCU(cu->ID);
						//	cuu.vecMove = Vec2(cu->orderPosiLeft - cu->nowPosiLeft).normalized();
						//	cuu.orderPosiLeft = end;
						//	cuu.FlagMove = false;
						//	//cuu.FlagMoving = true;
						//	cuu.FlagMoveAI = true;
						//}

						if (arrayBattleZinkei[0] == true)
						{
							//密集

							for (auto& target : classBattle.listOfAllUnit)
								for (auto& unit : target.ListClassUnit)
								{
									Unit& cuu = GetCU(unit.ID);
									cuu.orderPosiLeft = end.movedBy(Random(-10, 10), Random(-10, 10));
									cuu.orderPosiLeftLast = cuu.orderPosiLeft;
									cuu.FlagMove = false;
									cuu.FlagMoveAI = true;
								}
						}
						else if (arrayBattleZinkei[1] == true)
						{
							//横列

							ClassHorizontalUnit liZenei;
							for (auto& target : classBattle.listOfAllUnit)
								for (auto& unit : target.ListClassUnit)
								{
									if (unit.Formation == BattleFormation::F && unit.FlagMove == true && unit.IsBattleEnable == true)
										liZenei.ListClassUnit.push_back(unit);
								}

							ClassHorizontalUnit liKouei;
							for (auto& target : classBattle.listOfAllUnit)
								for (auto& unit : target.ListClassUnit)
								{
									if (unit.Formation == BattleFormation::B && unit.FlagMove == true && unit.IsBattleEnable == true)
										liKouei.ListClassUnit.push_back(unit);
								}

							ClassHorizontalUnit liKihei;
							for (auto& target : classBattle.listOfAllUnit)
								for (auto& unit : target.ListClassUnit)
								{
									if (unit.Formation == BattleFormation::M && unit.FlagMove == true && unit.IsBattleEnable == true)
										liKihei.ListClassUnit.push_back(unit);
								}

							Array<ClassHorizontalUnit> lisClassHorizontalUnitLoop;
							lisClassHorizontalUnitLoop.push_back(liZenei);
							lisClassHorizontalUnitLoop.push_back(liKouei);
							lisClassHorizontalUnitLoop.push_back(liKihei);

							for (auto&& [i, loopLisClassHorizontalUnit] : IndexedRef(lisClassHorizontalUnitLoop))
							{
								Array<Unit*> target;
								for (auto& unit : loopLisClassHorizontalUnit.ListClassUnit)
									if (unit.FlagMove == true && unit.IsBattleEnable == true)
										target.push_back(&unit);

								if (target.size() == 0)
									continue;

								//その部隊の人数を取得
								size_t unitCount = target.size();

								//商の数
								int32 result = (unitCount - 1) / 2;

								// 角度_X軸との角度を計算_θ'=直線とx軸のなす角度
								double angle2 = Math::Atan2(end.y - start.y,
													   end.x - start.x);
								//θ
								double angle = Math::Pi / 2 - angle2;

								//移動フラグが立っているユニットだけ、繰り返す
								if (unitCount % 2 == 1)//偶奇判定
								{
									for (auto&& [ii, unit] : Indexed(target))
									{
										//px+(b-切り捨て商)＊dcosθ+a＊d'cosθ’
										double xPos = end.x
											+ (
												(ii - (result))
												* (DistanceBetweenUnit * Math::Cos(angle))
												)
											-
											(i * (DistanceBetweenUnitTate * Math::Cos(angle2)));
										//py+(b-切り捨て商)＊dsinθ-a＊d'sinθ’
										double yPos = end.y
											- (
											(ii - (result))
											* (DistanceBetweenUnit * Math::Sin(angle))

											)
											-
											(i * (DistanceBetweenUnitTate * Math::Sin(angle2)));

										Unit& cuu = GetCU(unit->ID);
										cuu.orderPosiLeft = Vec2(xPos, yPos);
										cuu.orderPosiLeftLast = Vec2(xPos, yPos);
										cuu.FlagMove = false;
										cuu.FlagMoveAI = true;

										//unit->orderPosiLeft = Vec2(xPos, yPos);
										//unit->orderPosiLeftLast = Vec2(xPos, yPos);
										//unit->FlagMove = false;
										//unit->FlagMoveAI = true;

										auto index = ToIndex(unit->orderPosiLeft, columnQuads, rowQuads);
									}
								}
								else
								{
									for (auto&& [ii, unit] : Indexed(target))
									{
										//px+(b-切り捨て商)＊dcosθ+a＊d'cosθ’
										double xPos = end.x
											+ (
												(ii - (result))
												* (DistanceBetweenUnit * Math::Cos(angle))
												)
											-
											(i * (DistanceBetweenUnitTate * Math::Cos(angle2)));
										//py+(b-切り捨て商)＊dsinθ-a＊d'sinθ’
										double yPos = end.y
											- (
											(ii - (result))
											* (DistanceBetweenUnit * Math::Sin(angle))

											)
											-
											(i * (DistanceBetweenUnitTate * Math::Sin(angle2)));

										Unit& cuu = GetCU(unit->ID);
										cuu.orderPosiLeft = Vec2(xPos, yPos);
										cuu.orderPosiLeftLast = Vec2(xPos, yPos);
										cuu.FlagMove = false;
										cuu.FlagMoveAI = true;

										//unit->orderPosiLeft = Vec2(xPos, yPos);
										//unit->orderPosiLeftLast = Vec2(xPos, yPos);
										//unit->FlagMove = false;
										//unit->FlagMoveAI = true;

										auto index = ToIndex(unit->orderPosiLeft, columnQuads, rowQuads);
									}
								}
							}
						}
						else
						{
							//正方
							auto groups = GetMovableUnitGroups();
							for (auto&& [i, group] : Indexed(groups))
							{
								AssignUnitsInFormation(group, start, end, i);
							}
						}

						IsBattleMove = false;

						// 実行途中のタスクがあれば完了まで待つ。
						if (taskMyUnits.isValid())
						{
							// 中断指示を出す
							abortMyUnits = true;

							// 完全に処理が完了する前に制御を返してくれる
							taskMyUnits.wait();
						}

						abortMyUnits = false;

						//経路算出
						taskMyUnits = Async(BattleMoveAStarMyUnits,
										std::ref(classBattle.listOfAllUnit),
										std::ref(classBattle.listOfAllEnemyUnit),
										std::ref(classBattle.classMapBattle.value().mapData),
										std::ref(aiRoot),
										std::ref(debugRoot),
										std::ref(debugAstar),
										std::ref(columnQuads),
										std::ref(rowQuads),
										N,
										std::ref(abortMyUnits),
										std::ref(pauseTaskMyUnits));
					}
					else
					{
						//範囲選択
						// 範囲選択の矩形を生成（start, endの大小関係を吸収）
						const RectF selectionRect = RectF::FromPoints(start, end);

						for (auto& target : classBattle.listOfAllUnit)
						{
							for (auto& unit : target.ListClassUnit)
							{
								if (unit.IsBuilding)
									continue;

								const Vec2 gnpc = unit.GetNowPosiCenter();

								const bool inRect = selectionRect.intersects(gnpc);

								GetCU(unit.ID).FlagMove = inRect;

								if (inRect)
								{
									IsBattleMove = true;
								}
							}
						}
					}
				}

				//建物の建築場所を選択する処理
				//ここに存在するのは、MouseL.pressedで建築メニュー押下時に続けて処理されてしまう為
				if (IsBuildSelectTraget == true)
				{
					if (const auto index = ToIndex(Cursor::PosF(), columnQuads, rowQuads))
					{
						if (ToTile(*index, N).leftClicked())
						{
							{
								IsBuildSelectTraget = false;
								IsBuildMenuHome = false;
								Unit& cu = GetCU(longBuildSelectTragetId);
								longBuildSelectTragetId = -1;

								longIsMovedYoyakuId = cu.ID; // 移動中のユニットIDを保存

								//これだと、将来困るかも
								//移動中に別の生産場所を建築予定すると、上書きされる
								rowBuildingTarget = index.value().y;
								colBuildingTarget = index.value().x;
								//tempSelectComRight.rowBuilding = rowBuildingTarget;
								//tempSelectComRight.colBuilding = colBuildingTarget;
								//tempSelectComRight = dummyMenu();

								// 移動先の座標算出
								Vec2 nor = Cursor::PosF();
								// 移動先が有効かどうかチェックは実質上で済んでいる
								cu.orderPosiLeft = nor;
								cu.orderPosiLeftLast = nor;
								cu.vecMove = Vec2(cu.orderPosiLeft - cu.nowPosiLeft).normalized();
								cu.FlagMove = false;
								//cuu.FlagMoving = true;
								cu.FlagMoveAI = true;
								cu.IsSelect = false;

								IsBattleMove = false;

								// 実行途中のタスクがあれば完了まで待つ。
								if (taskMyUnits.isValid())
								{
									// 中断指示を出す
									abortMyUnits = true;

									// 完全に処理が完了する前に制御を返してくれる
									taskMyUnits.wait();
								}

								abortMyUnits = false;

								//予約移動状態であることを示す
								isMovedYoyaku = true;


								taskMyUnits = Async(BattleMoveAStarMyUnits,
						std::ref(classBattle.listOfAllUnit),
						std::ref(classBattle.listOfAllEnemyUnit),
						std::ref(classBattle.classMapBattle.value().mapData),
						std::ref(aiRoot),
						std::ref(debugRoot),
						std::ref(debugAstar),
						std::ref(columnQuads),
						std::ref(rowQuads),
						N,
						std::ref(abortMyUnits),
						std::ref(pauseTaskMyUnits));

							}
						}
					}
				}
				else if (IsResourceSelectTraget == true)
				{
					//資源ポイントの選択
					if (const auto index = ToIndex(Cursor::PosF(), columnQuads, rowQuads))
					{
						if (ToTile(*index, N).leftClicked() && longBuildSelectTragetId != -1)//ダブルクリックが良いかも　画面ドラッグを考慮し
						{
							if (classBattle.classMapBattle.value().mapData[index->x][index->y].isResourcePoint)
							{
								IsBuildSelectTraget = false;
								IsBuildMenuHome = false;
								Unit& cu = GetCU(longBuildSelectTragetId);
								longBuildSelectTragetId = -1;
								longIsGetResourceId = cu.ID; // 資源ポイントを選択したユニットIDを保存

								rowResourceTarget = index.value().y;
								colResourceTarget = index.value().x;

								// 移動先の座標算出
								Vec2 nor = Cursor::PosF();
								// 移動先が有効かどうかチェックは実質上で済んでいる
								cu.vecMove = Vec2(cu.orderPosiLeft - cu.nowPosiLeft).normalized();
								cu.orderPosiLeft = nor;
								cu.orderPosiLeftLast = nor;
								cu.vecMove = Vec2(cu.orderPosiLeft - cu.nowPosiLeft).normalized();
								cu.FlagMove = false;
								//cuu.FlagMoving = true;
								cu.FlagMoveAI = true;
								cu.IsSelect = false;

								IsBattleMove = false;

								// 実行途中のタスクがあれば完了まで待つ。
								if (taskMyUnits.isValid())
								{
									// 中断指示を出す
									abortMyUnits = true;

									// 完全に処理が完了する前に制御を返してくれる
									taskMyUnits.wait();
								}

								abortMyUnits = false;
								isGetResource = true; // 資源ポイントを選択したことを示す

								taskMyUnits = Async(BattleMoveAStarMyUnits,
													std::ref(classBattle.listOfAllUnit),
													std::ref(classBattle.listOfAllEnemyUnit),
													std::ref(classBattle.classMapBattle.value().mapData),
													std::ref(aiRoot),
													std::ref(debugRoot),
													std::ref(debugAstar),
													std::ref(columnQuads),
													std::ref(rowQuads),
													N,
													std::ref(abortMyUnits),
													std::ref(pauseTaskMyUnits)
								);
							}
						}
					}
				}
			}

			//移動処理
			{
				for (auto& item : classBattle.listOfAllUnit)
				{
					for (auto& itemUnit : item.ListClassUnit)
					{
						if (itemUnit.IsBuilding == true && itemUnit.mapTipObjectType == MapTipObjectType::WALL2)
						{
							continue;
						}
						if (itemUnit.IsBattleEnable == false)
							continue;
						if (itemUnit.FlagMoving == true)
						{
							// 移動実行
							itemUnit.nowPosiLeft = itemUnit.nowPosiLeft + (itemUnit.vecMove * ((itemUnit.Move + itemUnit.cts.Speed) / 100));

							// 目標に到達したか（例えば1.0ピクセル未満まで近づいたら止める）
							if (itemUnit.GetNowPosiCenter().distanceFrom(itemUnit.GetOrderPosiCenter()) < 3.0)
							{
								itemUnit.FlagMoving = false;
								itemUnit.nowPosiLeft = itemUnit.orderPosiLeft; // 位置をピッタリ補正して止めるのもあり
								itemUnit.FlagReachedDestination = true;
								itemUnit.FlagMovingEnd = true;
							}

							continue;
						}

						if (aiRoot[itemUnit.ID].size() == 1)
						{
							aiRoot[itemUnit.ID].pop_front();
							itemUnit.orderPosiLeft = itemUnit.orderPosiLeftLast;
							Vec2 hhh = itemUnit.GetOrderPosiCenter() - itemUnit.GetNowPosiCenter();
							itemUnit.vecMove = hhh.normalized();
							itemUnit.FlagMoving = true;
							itemUnit.FlagMovingEnd = false;
							continue;
						}

						// タイルのインデックス
						Point index;
						try
						{
							if (aiRoot[itemUnit.ID].size() >= 2)
							{
								aiRoot[itemUnit.ID].pop_front();
								index = aiRoot[itemUnit.ID].front();
							}
							else
							{
								continue;
							}
						}
						catch (const std::exception&)
						{
							continue;
						}

						// そのタイルの底辺中央の座標
						const int32 i = index.manhattanLength();
						const int32 xi = (i < (N - 1)) ? 0 : (i - (N - 1));
						const int32 yi = (i < (N - 1)) ? i : (N - 1);
						const int32 k2 = (index.manhattanDistanceFrom(Point{ xi, yi }) / 2);
						const double posX = ((i < (N - 1)) ? (i * -TileOffset.x) : ((i - 2 * N + 2) * TileOffset.x));
						const double posY = (i * TileOffset.y) - TileThickness;
						const Vec2 pos = { (posX + TileOffset.x * 2 * k2) - (itemUnit.yokoUnit / 2), posY - itemUnit.TakasaUnit - 15 };

						itemUnit.orderPosiLeft = Vec2(Math::Round(pos.x), Math::Round(pos.y));

						//if (aiRoot[itemUnit.ID].size() == 1)
						//{
						//	itemUnit.orderPosiLeft = itemUnit.orderPosiLeftLast;
						//}
						//else
						//{
						//}

						Vec2 hhh = itemUnit.GetOrderPosiCenter() - itemUnit.GetNowPosiCenter();
						if (hhh.x == 0 && hhh.y == 0)
						{
							itemUnit.vecMove = { 0,0 };
						}
						else
						{
							itemUnit.vecMove = hhh.normalized();
						}
						itemUnit.FlagMoving = true;
						itemUnit.FlagMovingEnd = false;
					}
				}
				for (auto& item : classBattle.listOfAllEnemyUnit)
				{
					for (auto& itemUnit : item.ListClassUnit)
					{
						if (itemUnit.IsBuilding == true && itemUnit.mapTipObjectType == MapTipObjectType::WALL2)
							continue;
						if (itemUnit.IsBuilding == true && itemUnit.mapTipObjectType == MapTipObjectType::GATE)
							continue;
						if (itemUnit.IsBattleEnable == false)
							continue;

						//実際に動く処理

						if (itemUnit.FlagMoving == true)
						{
							itemUnit.nowPosiLeft = itemUnit.nowPosiLeft + (itemUnit.vecMove * ((itemUnit.Move + itemUnit.cts.Speed) / 100));

							double current_distanceX = itemUnit.orderPosiLeft.x - itemUnit.nowPosiLeft.x;
							double current_distanceY = itemUnit.orderPosiLeft.y - itemUnit.nowPosiLeft.y;
							double next_distanceX = current_distanceX - (itemUnit.vecMove.x * ((itemUnit.Move + itemUnit.cts.Speed) / 100));
							double next_distanceY = current_distanceY - (itemUnit.vecMove.y * ((itemUnit.Move + itemUnit.cts.Speed) / 100));
							if (next_distanceX * next_distanceX + next_distanceY * next_distanceY >= current_distanceX * current_distanceX + current_distanceY * current_distanceY)
								itemUnit.FlagMoving = false;
							continue;
						}

						if (aiRoot[itemUnit.ID].isEmpty() == true
							|| aiRoot[itemUnit.ID].size() == 0)
						{
							//itemUnit.FlagMovingEnd = true;
							//itemUnit.FlagMoving = false;
							continue;
						}

						// タイルのインデックス
						Point index;
						try
						{
							aiRoot[itemUnit.ID].pop_front();
							auto rthrthrt = aiRoot[itemUnit.ID];
							index = aiRoot[itemUnit.ID][0];
						}
						catch (const std::exception&)
						{
							throw;
							continue;
						}

						if (aiRoot[itemUnit.ID].size() == 1)
						{
							itemUnit.orderPosiLeft = itemUnit.orderPosiLeftLast;
						}
						else
						{
							// そのタイルの底辺中央の座標
							const int32 i = index.manhattanLength();
							const int32 xi = (i < (N - 1)) ? 0 : (i - (N - 1));
							const int32 yi = (i < (N - 1)) ? i : (N - 1);
							const int32 k2 = (index.manhattanDistanceFrom(Point{ xi, yi }) / 2);
							const double posX = ((i < (N - 1)) ? (i * -TileOffset.x) : ((i - 2 * N + 2) * TileOffset.x));
							const double posY = (i * TileOffset.y) - TileThickness;
							const Vec2 pos = { (posX + TileOffset.x * 2 * k2) - (itemUnit.yokoUnit / 2), posY - itemUnit.TakasaUnit - 15 };

							itemUnit.orderPosiLeft = pos;
						}

						Vec2 hhh = itemUnit.GetOrderPosiCenter() - itemUnit.GetNowPosiCenter();
						if (hhh.x == 0 && hhh.y == 0)
						{
							itemUnit.vecMove = { 0,0 };
						}
						else
						{
							itemUnit.vecMove = hhh.normalized();
						}
						itemUnit.FlagMoving = true;
						itemUnit.FlagMovingEnd = false;
					}
				}
			}

			//ユニット体力バーの設定
			for (auto& item : classBattle.listOfAllUnit)
			{
				if (!item.FlagBuilding &&
					!item.ListClassUnit.empty())
					for (auto& itemUnit : item.ListClassUnit)
					{
						double hpp = (static_cast<double>(itemUnit.Hp) / itemUnit.HpMAX);
						itemUnit.bLiquidBarBattle.update(hpp);
						itemUnit.bLiquidBarBattle.ChangePoint(itemUnit.GetNowPosiCenter().movedBy(-64 / 2, (32 / 2) + 6));
					}
			}
			for (auto& item : classBattle.listOfAllEnemyUnit)
			{
				if (!item.FlagBuilding &&
					!item.ListClassUnit.empty())
					for (auto& itemUnit : item.ListClassUnit)
					{
						double hpp = (static_cast<double>(itemUnit.Hp) / itemUnit.HpMAX);
						itemUnit.bLiquidBarBattle.update(hpp);
						itemUnit.bLiquidBarBattle.ChangePoint(itemUnit.GetNowPosiCenter().movedBy(-64 / 2, (32 / 2) + 6));
					}
			}

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
						for (auto& item : classBattle.listOfAllUnit)
						{
							if (!item.FlagBuilding &&
								!item.ListClassUnit.empty())
								for (auto& itemUnit : item.ListClassUnit)
								{
									for (auto& itemSkill : itemUnit.Skill)
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

						while (not font(nowSelectSkillSetumei).draw(rectSkillSetumei.stretched(-12), ColorF{ 0.0 }))
						{
							rectSkillSetumei.h = rectSkillSetumei.h + 12;
						}
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

			//建物メニュー選択
			{
				const Transformer2D transformer{ Mat3x2::Identity(), Mat3x2::Translate(Scene::Size().x - 328, Scene::Size().y - 328 - 30) };

				Array<cRightMenu> temp;
				if (buiSyu == 0)
				{
					temp = arrayComRight_BuildMenu_Home;
				}
				else if (buiSyu == 1)
				{
					temp = arrayComRight_BuildMenu_Thunderwalker;
				}
				else if (buiSyu == 5)
				{
					temp = arrayComRight_BuildMenu_Kouhei;
				}
				else if (buiSyu == 6)
				{
					temp = arrayComRight_BuildMenu_KeisouHoheiT;
				}

				for (auto&& [i, re] : IndexedRef(temp))
				{
					if (re.rect.leftClicked())
					{
						if (re.kindForProcess == U"UnitPro")
						{
							IsBuildSelectTraget = true;
							IsResourceSelectTraget = false;
							cRightMenuTargetCount = re.sortId;
							tempSelectComRight = re;
							break;
						}
						else
						{
							IsBuildSelectTraget = false;
						}

						cRightMenu ccc;
						//ccc.sortId = re.sortId;
						ccc.sortYoyakuId = longBuildMenuHomeYoyakuIdCount;
						ccc.key = re.key;
						ccc.kindForProcess = re.kindForProcess;
						ccc.texture = TextureAsset(re.key);
						ccc.time = re.time;
						//ccc.rowBuilding = re.rowBuilding;
						//ccc.colBuilding = re.colBuilding;
						longBuildMenuHomeYoyakuIdCount++;

						if (re.buiSyu == 0)
						{
							if (stopwatch.isRunning() == false)
							{
								stopwatch.start();
								t = 0.0;
							}
							arrBuildMenuHomeYoyaku.push_back(ccc);
						}
						else if (re.buiSyu == 1)
						{
							if (stopwatch001.isRunning() == false)
							{
								stopwatch001.start();
								arrT[1] = 0.0;
							}
							arrBuildMenuThunderwalkerYoyaku.push_back(ccc);
						}
						else if (re.buiSyu == 5)
						{
							if (stopwatch002.isRunning() == false)
							{
								stopwatch002.start();
								arrT[2] = 0.0;
							}
							arrBuildMenuKouheiYoyaku.push_back(ccc);
						}
						else if (re.buiSyu == 6)
						{
							if (stopwatch003.isRunning() == false)
							{
								stopwatch003.start();
								arrT[3] = 0.0;
							}
							arrBuildMenuKeisouYoyaku.push_back(ccc);
						}

						//回数制限のある建物を選択した場合
						if (re.count > 0)
						{
							re.count--;
							cbp.htCountAndSyu[re.key].count = re.count;
							if (re.buiSyu == 0)
							{
								renB(renderTextureBuildMenuHome, cbp, arrayComRight_BuildMenu_Home);
							}
							else if (re.buiSyu == 1)
							{
								renB(renderTextureBuildMenuKouhei, cbpKouhei, arrayComRight_BuildMenu_Kouhei);
							}
						}
						break;
					}
				}

				arrBuildMenuHomeYoyaku.sort_by([](const cRightMenu& a, const cRightMenu& b)
					{
						return a.sortId < b.sortId;
					});
				arrBuildMenuThunderwalkerYoyaku.sort_by([](const cRightMenu& a, const cRightMenu& b)
					{
						return a.sortId < b.sortId;
					});
				arrBuildMenuKouheiYoyaku.sort_by([](const cRightMenu& a, const cRightMenu& b)
					{
						return a.sortId < b.sortId;
					});
				arrBuildMenuKeisouYoyaku.sort_by([](const cRightMenu& a, const cRightMenu& b)
					{
						return a.sortId < b.sortId;
					});
			}
		}
		break;
		case BattleStatus::Message:
			break;
		case BattleStatus::Event:
			break;
		default:
			break;
		}

		// 非同期タスクが完了したら
		if (task.isReady())
		{
			//// 結果を取得する
			//Print << task.get();
		}
		// 非同期タスクが完了したら
		if (taskMyUnits.isReady())
		{
			//// 結果を取得する
			//Print << task.get();
		}

		co_await Co::NextFrame();
	}
}

void Battle::draw() const
{
	FsScene::draw();

	{
		// 2D カメラによる座標変換を適用する
		const auto tr = camera.createTransformer();

		// カメラの視界範囲（ワールド座標での視界矩形）
		int32 testPadding = -TileOffset.x;
		const RectF cameraView = RectF{
			camera.getCenter() - (Scene::Size() / 2.0) / camera.getScale(),
			Scene::Size() / camera.getScale()
		}.stretched(-testPadding);

		{
			// 乗算済みアルファ用のブレンドステートを適用する
			const ScopedRenderStates2D blend{ BlendState::Premultiplied };

			Array<Unit> bui;
			for (auto& item : classBattle.listOfAllUnit)
			{
				if (item.FlagBuilding == true &&
					!item.ListClassUnit.empty())
				{
					for (auto& itemUnit : item.ListClassUnit)
					{
						bui.push_back(itemUnit);
					}
				}
			}
			for (auto& item : classBattle.listOfAllEnemyUnit)
			{
				if (item.FlagBuilding == true &&
					!item.ListClassUnit.empty())
				{
					for (auto& itemUnit : item.ListClassUnit)
					{
						bui.push_back(itemUnit);
					}
				}
			}
			Array<Unit> buiTex;

			Array<map_detail_position> amd;

			// 上から順にタイルを描く
			for (int32 i = 0; i < (N * 2 - 1); ++i)
			{
				// x の開始インデックス
				const int32 xi = (i < (N - 1)) ? 0 : (i - (N - 1));

				// y の開始インデックス
				const int32 yi = (i < (N - 1)) ? i : (N - 1);

				// 左から順にタイルを描く
				for (int32 k = 0; k < (N - Abs(N - i - 1)); ++k)
				{
					// タイルのインデックス
					const Point index{ (xi + k), (yi - k) };

					// そのタイルの底辺中央の座標
					const Vec2 pos = ToTileBottomCenter(index, N);

					if (cameraView.intersects(Vec2(pos)))
					{
						String tip = classBattle.classMapBattle.value().mapData[index.x][index.y].tip;
						TextureAsset(tip + U".png").draw(Arg::bottomCenter = pos);

						if (classBattle.classMapBattle.value().mapData[index.x][index.y].isResourcePoint)
						{
							map_detail_position tempQ;
							tempQ.classMapBattle = classBattle.classMapBattle.value().mapData[index.x][index.y];
							tempQ.pos = pos;
							amd.push_back(tempQ);
						}
					}

					// 建物描写
					for (auto& abc : bui)
					{
						if (abc.IsBattleEnable == false)
							continue;

						if (abc.rowBuilding == (xi + k) && abc.colBuilding == (yi - k))
							buiTex.push_back(abc);
					}

					// Fog 描画
					switch (visibilityMap[index])
					{
					case Visibility::Unseen:
						ToTile(index, N).draw(ColorF{ 0.0, 0.6 }); // 半透明
						//ToTile(index, N).draw(ColorF{ 0.0 }); // 完全に隠す
						break;
						//case Visibility::Seen://使わない
						//	break;
					case Visibility::Visible:
						break;
					}
				}
			}

			// 建物描画
			for (auto itemUnit : buiTex)
			{
				const Vec2 pos = ToTileBottomCenter(Point(itemUnit.colBuilding, itemUnit.rowBuilding), N);
				if (cameraView.intersects(pos))
				{
					itemUnit.IsSelect ?
						TextureAsset(itemUnit.Image).draw(Arg::bottomCenter = pos.movedBy(0, -TileThickness)).drawFrame(3.0, Palette::Red) :
						TextureAsset(itemUnit.Image).draw(Arg::bottomCenter = pos.movedBy(0, -TileThickness));

					//tempQ.scaled(scale).movedBy(0, -(vHe * scale)+15).draw(Palette::Red);
					//ToTile(Point(aaa.colBuilding, aaa.rowBuilding), N).movedBy(0, 0).draw(ColorF{ 0.0, 0.6 });
				}
			}

			/// 資源ポイントのアイコン描画
			for (auto ttt : amd)
			{
				TextureAsset(ttt.classMapBattle.resourcePointIcon).draw(Arg::bottomCenter = ttt.pos.movedBy(0, -TileThickness));
				switch (ttt.classMapBattle.whichIsThePlayer)
				{
				case BattleWhichIsThePlayer::Sortie:
					Circle(ttt.pos, 16).draw(ColorF{ 0.0, 0.6 });
					break;
				default:
					break;
				}
			}

			//体力ゲージ
			for (auto& item : classBattle.listOfAllUnit)
			{
				if (!item.FlagBuilding &&
					!item.ListClassUnit.empty())
				{
					for (auto& itemUnit : item.ListClassUnit)
					{
						if (itemUnit.IsBattleEnable == false)
							continue;
						itemUnit.bLiquidBarBattle.draw(ColorF{ 0.9, 0.1, 0.1 }, ColorF{ 0.7, 0.05, 0.05 }, ColorF{ 0.9, 0.5, 0.1 });
					}
				}
			}
			for (auto& item : classBattle.listOfAllEnemyUnit)
			{
				if (!item.FlagBuilding &&
					!item.ListClassUnit.empty())
				{
					for (auto& itemUnit : item.ListClassUnit)
					{
						if (itemUnit.IsBattleEnable == false)
							continue;
						itemUnit.bLiquidBarBattle.draw(ColorF{ 0.9, 0.1, 0.1 }, ColorF{ 0.7, 0.05, 0.05 }, ColorF{ 0.9, 0.5, 0.1 });
					}
				}
			}

			// プレイヤーユニット描画
			for (auto& item : classBattle.listOfAllUnit)
			{
				if (!item.FlagBuilding &&
					!item.ListClassUnit.empty())
				{
					for (auto& itemUnit : item.ListClassUnit)
					{
						if (itemUnit.IsBattleEnable == false)
							continue;

						TextureAsset(U"ringA.png").drawAt(itemUnit.GetNowPosiCenter().movedBy(0, 8));
					}
				}
			}
			for (auto& item : classBattle.listOfAllUnit)
			{
				if (item.FlagBuilding == false &&
					!item.ListClassUnit.empty())
				{
					for (auto& itemUnit : item.ListClassUnit)
					{
						if (itemUnit.IsBattleEnable == false)
							continue;
						if (itemUnit.IsSelect)
						{
							TextureAsset(itemUnit.Image).draw(Arg::center = itemUnit.GetNowPosiCenter()).drawFrame(3.0, Palette::Red);
						}
						else
						{
							TextureAsset(itemUnit.Image).draw(Arg::center = itemUnit.GetNowPosiCenter());
						}
					}
				}
			}
			for (auto& item : classBattle.listOfAllUnit)
			{
				if (!item.FlagBuilding &&
					!item.ListClassUnit.empty())
				{
					for (auto& itemUnit : item.ListClassUnit)
					{
						if (itemUnit.IsBattleEnable == false)
							continue;

						TextureAsset(U"ringB.png").drawAt(itemUnit.GetNowPosiCenter().movedBy(0, 16));
					}
				}
			}

			// 敵ユニット描画
			for (auto& item : classBattle.listOfAllEnemyUnit)
			{
				if (!item.FlagBuilding &&
					!item.ListClassUnit.empty())
				{
					for (auto& itemUnit : item.ListClassUnit)
					{
						if (itemUnit.IsBattleEnable == false)
							continue;

						TextureAsset(U"ringA_E.png").drawAt(itemUnit.GetNowPosiCenter().movedBy(0, 8));
					}
				}
			}
			for (auto& item : classBattle.listOfAllEnemyUnit)
			{
				if (item.FlagBuilding == false &&
					!item.ListClassUnit.empty())
				{
					for (auto& itemUnit : item.ListClassUnit)
					{
						if (itemUnit.IsBattleEnable == false)
							continue;
						if (itemUnit.IsSelect)
						{
							TextureAsset(itemUnit.Image).draw(Arg::center = itemUnit.GetNowPosiCenter()).drawFrame(3.0, Palette::Red);
						}
						else
						{
							TextureAsset(itemUnit.Image).draw(Arg::center = itemUnit.GetNowPosiCenter());
						}
					}
				}
			}
			for (auto& item : classBattle.listOfAllEnemyUnit)
			{
				if (!item.FlagBuilding &&
					!item.ListClassUnit.empty())
				{
					for (auto& itemUnit : item.ListClassUnit)
					{
						if (itemUnit.IsBattleEnable == false)
							continue;

						TextureAsset(U"ringB_E.png").drawAt(itemUnit.GetNowPosiCenter().movedBy(0, 16));
					}
				}
			}
		}

		//範囲指定もしくは移動先矢印
		if (MouseR.pressed())
		{
			if (IsBattleMove == false)
			{
				const double thickness = 3.0;
				double offset = 0.0;

				offset += (Scene::DeltaTime() * 10);

				const Rect rect{ cursPos, Cursor::Pos() - cursPos };
				rect.top().draw(LineStyle::SquareDot(offset), thickness, Palette::Orange);
				rect.right().draw(LineStyle::SquareDot(offset), thickness, Palette::Orange);
				rect.bottom().draw(LineStyle::SquareDot(offset), thickness, Palette::Orange);
				rect.left().draw(LineStyle::SquareDot(offset), thickness, Palette::Orange);
			}
			else
			{
				Line{ cursPos, Cursor::Pos() }
				.drawArrow(10, Vec2{ 40, 80 }, Palette::Orange);
			}
		}

		if (IsBuildSelectTraget == true)
		{
			if (const auto index = ToIndex(Cursor::PosF(), columnQuads, rowQuads))
			{
				// マウスカーソルがあるタイルを強調表示する
				ToTile(*index, N).draw(ColorF{ 1.0, 0.2 });
			}
		}
	}

	//-30とは下の線のこと
	renderTextureSkill.draw(0, Scene::Size().y - 320 - 30);
	renderTextureSkillUP.draw(0, Scene::Size().y - 320 - 30);
	//現在の資源を左上に表示する
	const String goldText = U"Gold:{0}"_fmt(gold);
	const String trustText = U"Trust:{0}"_fmt(trust);
	const String foodText = U"Food:{0}"_fmt(food);
	{
		Rect rectPause{ 0,
					0,
					int32(systemFont(goldText).region().w),
					int32(systemFont(goldText).region().h) };
		rectPause.draw(Palette::Black);
		systemFont(goldText).drawAt(rectPause.center(), Palette::White);
	}
	{
		Rect rectPause{ 0,
					int32(systemFont(goldText).region().h),
					int32(systemFont(trustText).region().w),
					int32(systemFont(trustText).region().h) };
		rectPause.draw(Palette::Black);
		systemFont(trustText).drawAt(rectPause.center(), Palette::White);
	}
	{
		Rect rectPause{ 0,
					int32(systemFont(goldText).region().h + int32(systemFont(trustText).region().h)),
					int32(systemFont(foodText).region().w),
					int32(systemFont(foodText).region().h) };
		rectPause.draw(Palette::Black);
		systemFont(foodText).drawAt(rectPause.center(), Palette::White);
	}



	if (IsBuildMenuHome)
	{
		switch (buiSyu)
		{
		case 0:
		{
			// ゲージの高さ（画像下から上へ）
			const double gaugeHeight = 64 * t;
			renderTextureBuildMenuHome.draw(Scene::Size().x - 328, Scene::Size().y - 328 - 30);
			for (auto&& [i, re] : Indexed(arrBuildMenuHomeYoyaku))
			{
				if (i == 0)
				{
					// 明るくする矩形領域（ゲージ部分）
					RectF gaugeRect{ Scene::Size().x - 328 - 64, Scene::Size().y - 328 - 30 + 4, 64, gaugeHeight };
					re.texture.resized(64).draw(Scene::Size().x - 328 - 64, Scene::Size().y - 328 - 30 + 4);
					gaugeRect
						.draw(ColorF{ 0.0, 0.5 }); // 上が透明、下が白
				}
				else
				{
					re.texture.resized(32).draw(Scene::Size().x - 328 - 32, Scene::Size().y - 328 - 30 + 32 + (i * 32) + 4);
				}
			}
			break;
		}
		case 1:
		{
			// ゲージの高さ（画像下から上へ）
			const double gaugeHeight = 64 * arrT[1];
			renderTextureBuildMenuThunderwalker.draw(Scene::Size().x - 328, Scene::Size().y - 328 - 30);
			for (auto&& [i, re] : Indexed(arrBuildMenuThunderwalkerYoyaku))
			{
				if (i == 0)
				{
					// 明るくする矩形領域（ゲージ部分）
					RectF gaugeRect{ Scene::Size().x - 328 - 64, Scene::Size().y - 328 - 30 + 4, 64, gaugeHeight };
					re.texture.resized(64).draw(Scene::Size().x - 328 - 64, Scene::Size().y - 328 - 30 + 4);
					gaugeRect
						.draw(ColorF{ 0.0, 0.5 }); // 上が透明、下が白
				}
				else
				{
					re.texture.resized(32).draw(Scene::Size().x - 328 - 32, Scene::Size().y - 328 - 30 + 32 + (i * 32) + 4);
				}
			}
			break;
		}
		case 5:
		{
			// ゲージの高さ（画像下から上へ）
			const double gaugeHeight = 64 * arrT[2];
			renderTextureBuildMenuKouhei.draw(Scene::Size().x - 328, Scene::Size().y - 328 - 30);
			//TODO 到着時に以下を行う
			for (auto&& [i, re] : Indexed(arrBuildMenuKouheiYoyaku))
			{
				if (i == 0)
				{
					re.texture.resized(64).draw(Scene::Size().x - 328 - 64, Scene::Size().y - 328 - 30 + 4);

					if (re.isMoved)
					{

					}
					else
					{
						// 明るくする矩形領域（ゲージ部分）
						RectF gaugeRect{ Scene::Size().x - 328 - 64, Scene::Size().y - 328 - 30 + 4, 64, gaugeHeight };
						gaugeRect
							.draw(ColorF{ 0.0, 0.5 }); // 上が透明、下が白
					}
				}
				else
				{
					re.texture.resized(32).draw(Scene::Size().x - 328 - 32, Scene::Size().y - 328 - 30 + 32 + (i * 32) + 4);
				}
			}
			break;
		}
		case 6:
		{
			// ゲージの高さ（画像下から上へ）
			const double gaugeHeight = 64 * arrT[3];
			renderTextureBuildMenuKeisouHoheiT.draw(Scene::Size().x - 328, Scene::Size().y - 328 - 30);
			//TODO 到着時に以下を行う
			for (auto&& [i, re] : Indexed(arrBuildMenuKeisouYoyaku))
			{
				if (i == 0)
				{
					re.texture.resized(64).draw(Scene::Size().x - 328 - 64, Scene::Size().y - 328 - 30 + 4);

					if (re.isMoved)
					{

					}
					else
					{
						// 明るくする矩形領域（ゲージ部分）
						RectF gaugeRect{ Scene::Size().x - 328 - 64, Scene::Size().y - 328 - 30 + 4, 64, gaugeHeight };
						gaugeRect
							.draw(ColorF{ 0.0, 0.5 }); // 上が透明、下が白
					}
				}
				else
				{
					re.texture.resized(32).draw(Scene::Size().x - 328 - 32, Scene::Size().y - 328 - 30 + 32 + (i * 32) + 4);
				}
			}
			break;
		}
		default:
			break;
		}
	}
	else
	{
		renderTextureBuildMenuEmpty.draw(Scene::Size().x - 328, Scene::Size().y - 328 - 30);
	}
}

void Battle::UpdateVisibility(Grid<Visibility>& vis, const Array<Unit>& units, int32 mapSize) const
{
	for (const auto& unit : units)
	{
		for (int dy = -unit.visionRadius; dy <= unit.visionRadius; ++dy)
		{
			for (int dx = -unit.visionRadius; dx <= unit.visionRadius; ++dx)
			{
				Vec2 pos = unit.GetNowPosiCenter();
				if (const auto index = ToIndex(unit.GetNowPosiCenter(), columnQuads, rowQuads))
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
/// @brief タイルのインデックスから、タイルの底辺中央の座標を計算します。
/// @param index タイルのインデックス
/// @param N マップの一辺のタイル数
/// @return タイルの底辺中央の座標
Vec2 Battle::ToTileBottomCenter(const Point& index, const int32 N) const
{
	const int32 i = index.manhattanLength();
	const int32 xi = (i < (N - 1)) ? 0 : (i - (N - 1));
	const int32 yi = (i < (N - 1)) ? i : (N - 1);
	const int32 k = (index.manhattanDistanceFrom(Point{ xi, yi }) / 2);
	const double posX = ((i < (N - 1)) ? (i * -TileOffset.x) : ((i - 2 * N + 2) * TileOffset.x));
	const double posY = (i * TileOffset.y);
	return{ (posX + TileOffset.x * 2 * k), posY };
}
/// @brief タイルのインデックスから、タイルの四角形を計算します。
/// @param index タイルのインデックス
/// @param N マップの一辺のタイル数
/// @return タイルの四角形
Quad Battle::ToTile(const Point& index, const int32 N) const
{
	const Vec2 bottomCenter = ToTileBottomCenter(index, N);

	return Quad{
		bottomCenter.movedBy(0, -TileThickness).movedBy(0, -TileOffset.y * 2),
		bottomCenter.movedBy(0, -TileThickness).movedBy(TileOffset.x, -TileOffset.y),
		bottomCenter.movedBy(0, -TileThickness),
		bottomCenter.movedBy(0, -TileThickness).movedBy(-TileOffset.x, -TileOffset.y)
	};
}
/// @brief 指定した列のタイルによって構成される四角形を計算します。
/// @param x 列インデックス
/// @param N マップの一辺のタイル数
/// @return 指定した列のタイルによって構成される四角形
Quad Battle::ToColumnQuad(const int32 x, const int32 N) const
{
	return{
		ToTileBottomCenter(Point{ x, 0 }, N).movedBy(0, -TileThickness).movedBy(0, -TileOffset.y * 2),
		ToTileBottomCenter(Point{ x, 0 }, N).movedBy(0, -TileThickness).movedBy(TileOffset.x, -TileOffset.y),
		ToTileBottomCenter(Point{ x, (N - 1) }, N).movedBy(0, -TileThickness).movedBy(0, 0),
		ToTileBottomCenter(Point{ x, (N - 1) }, N).movedBy(0, -TileThickness).movedBy(-TileOffset.x, -TileOffset.y)
	};
}

/// @brief 指定した行のタイルによって構成される四角形を計算します。
/// @param y 行インデックス
/// @param N マップの一辺のタイル数
/// @return 指定した行のタイルによって構成される四角形
Quad Battle::ToRowQuad(const int32 y, const int32 N) const
{
	return{
		ToTileBottomCenter(Point{ 0, y }, N).movedBy(0, -TileThickness).movedBy(-TileOffset.x, -TileOffset.y),
		ToTileBottomCenter(Point{ 0, y }, N).movedBy(0, -TileThickness).movedBy(0, -TileOffset.y * 2),
		ToTileBottomCenter(Point{ (N - 1), y }, N).movedBy(0, -TileThickness).movedBy(TileOffset.x, -TileOffset.y),
		ToTileBottomCenter(Point{ (N - 1), y }, N).movedBy(0, -TileThickness).movedBy(0, 0)
	};
}

/// @brief 各列のタイルによって構成される四角形の配列を作成します。
/// @param N マップの一辺のタイル数
/// @return 各列のタイルによって構成される四角形の配列
Array<Quad> Battle::MakeColumnQuads(const int32 N) const
{
	Array<Quad> quads;

	for (int32 x = 0; x < N; ++x)
	{
		quads << ToColumnQuad(x, N);
	}

	return quads;
}

/// @brief 各行のタイルによって構成される四角形の配列を作成します。
/// @param N マップの一辺のタイル数
/// @return 各行のタイルによって構成される四角形の配列
Array<Quad> Battle::MakeRowQuads(const int32 N) const
{
	Array<Quad> quads;

	for (int32 y = 0; y < N; ++y)
	{
		quads << ToRowQuad(y, N);
	}

	return quads;
}

/// @brief 画像を読み込み、アルファ乗算済みのテクスチャを作成します。
/// @param path 画像ファイルのパス
/// @return アルファ乗算済みのテクスチャ
/// @remark 境界付近の品質を向上させるため、アルファ乗算済みのテクスチャを作成します。
/// @remark 描画時は `BlendState::Premultiplied` を指定してください。
[[nodiscard]]
Texture Battle::LoadPremultipliedTexture(FilePathView path)
{
	Image image{ path };
	Color* p = image.data();
	const Color* const pEnd = (p + image.num_pixels());
	while (p != pEnd)
	{
		p->r = static_cast<uint8>((static_cast<uint16>(p->r) * p->a) / 255);
		p->g = static_cast<uint8>((static_cast<uint16>(p->g) * p->a) / 255);
		p->b = static_cast<uint8>((static_cast<uint16>(p->b) * p->a) / 255);
		++p;
	}
	return Texture{ image };
}

/// @brief 
/// @param ID 
/// @return 
Unit& Battle::GetCU(long ID)
{
	for (auto& temp : classBattle.listOfAllUnit)
		for (auto& temptemp : temp.ListClassUnit)
			if (temptemp.ID == ID)
				return temptemp;
}

/// @brief 全ユニットから「移動可能なユニット」だけを抽出して部隊ごとにまとめる
/// @return 
Array<Array<Unit*>> Battle::GetMovableUnitGroups()
{
	Array<Array<Unit*>> groups;

	for (auto& target : classBattle.listOfAllUnit)
	{
		Array<Unit*> group;

		for (auto& unit : target.ListClassUnit)
		{
			if (unit.FlagMove && unit.IsBattleEnable)
				group.push_back(&unit);
		}

		if (!group.isEmpty())
			groups.push_back(group);
	}

	return groups;
}

/// @brief 指定されたユニットの部隊に対して、指定された開始位置と終了位置に沿ってユニットを配置します。
/// @param units 
/// @param start 
/// @param end 
/// @param rowIndex 
void Battle::AssignUnitsInFormation(const Array<Unit*>& units, const Vec2& start, const Vec2& end, int32 rowIndex)
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

		double distance_between_units_cos = Math::Round(DistanceBetweenUnit * cosPerpendicular);
		int32 unit_spacing_offset_factor = (i - centerOffset);
		int32 unit_spacing_offset_x = unit_spacing_offset_factor * distance_between_units_cos;
		int32 unit_spacing_offset_y = unit_spacing_offset_factor * Math::Round(DistanceBetweenUnit * sinPerpendicular);

		double rowOffsetX = rowIndex * DistanceBetweenUnitTate * Math::Cos(angleForward);
		double rowOffsetY = rowIndex * DistanceBetweenUnitTate * Math::Sin(angleForward);

		double x = end.x + unit_spacing_offset_x - rowOffsetX;
		double y = end.y - unit_spacing_offset_y - rowOffsetY;

		Unit& cuu = GetCU(unit->ID);
		cuu.orderPosiLeft = Vec2(Floor(x), Floor(y));
		cuu.orderPosiLeftLast = cuu.orderPosiLeft;
		cuu.FlagMove = false;
		cuu.FlagMoveAI = true;
	}
}
