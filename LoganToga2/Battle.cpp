#pragma once
#include "Battle.hpp"
#include "ClassBattle.h"

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

Battle::Battle(GameData& saveData)
	:FsScene(U"Battle"), m_saveData{ saveData }, N{ 64 }, visibilityMap{ Grid<Visibility>(Size{ N, N }, Visibility::Unseen) }, grid{ Grid<int32>(Size{ N, N }) }
{
	Vec2 TileOffset{ 48, 24 };
	int32 TileThickness = 17;
	Array<Quad> columnQuads = MakeColumnQuads(N);
	Array<Quad> rowQuads = MakeRowQuads(N);
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
		TextureAsset::Register(FileSystem::FileName(filePath), filePath);
	for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/043_ChipImageBuild/"))
		TextureAsset::Register(FileSystem::FileName(filePath), filePath);

	arrayBattleZinkei.push_back(false);
	arrayBattleZinkei.push_back(false);
	arrayBattleZinkei.push_back(false);
	arrayBattleCommand.push_back(false);
	arrayBattleCommand.push_back(false);

	battleStatus = BattleStatus::Battle;

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

	renderTextureBuildMenuEmpty = RenderTexture{ 328, 328 };
	renderTextureBuildMenuEmpty.clear(ColorF{ 0.5, 0.0 });
	{
		const ScopedRenderTarget2D target{ renderTextureBuildMenuHome.clear(ColorF{ 0.8, 0.8, 0.8,0.5 }) };
		const ScopedRenderStates2D blend{ MakeBlendState() };
		Rect df = Rect(328, 328);
		df.drawFrame(4, 0, ColorF{ 0.5 });
	}

	renderTextureBuildMenuHome = RenderTexture{ 328, 328 };
	renderTextureBuildMenuHome.clear(ColorF{ 0.5, 0.0 });
	{
		const ScopedRenderTarget2D target{ renderTextureBuildMenuHome.clear(ColorF{ 0.8, 0.8, 0.8,0.5 }) };
		const ScopedRenderStates2D blend{ MakeBlendState() };

		//rectBuildMenuHome.clear();
		htBuildMenuHome.clear();
		Array<String> arrayHome = { U"kouhei.png",U"inhura-kaizen.png",U"kayaku.png" };
		for (size_t i = 0; i < arrayHome.size(); i++)
		{
			Rect rectBuildMenuHome;
			rectBuildMenuHome.x = ((i % 6) * 64) + 4;
			rectBuildMenuHome.y = ((i / 6) * 64) + 4;
			rectBuildMenuHome.w = 64;
			rectBuildMenuHome.h = 64;
			htBuildMenuHome.emplace(arrayHome[i], rectBuildMenuHome);
		}
		for (auto& icons : htBuildMenuHome)
			TextureAsset(icons.first).resized(64).draw(icons.second.x, icons.second.y);

		Rect df = Rect(328, 328);
		df.drawFrame(4, 0, ColorF{ 0.5 });
	}

	{
		Unit uu;
		uu.ID = classBattle.getIDCount();
		uu.IsBuilding = false;
		uu.initTilePos = Point{ 4, 0 };
		uu.orderPosiLeft = Point{ 0, 0 };
		uu.orderPosiLeftLast = Point{ 0, 0 };
		uu.nowPosiLeft = ToTile(uu.initTilePos, N).asPolygon().centroid().movedBy(-(uu.yokoUnit / 2), -(uu.TakasaUnit / 2));
		uu.vecMove = Vec2{ 0, 0 };
		uu.Speed = 1.0;
		uu.FlagMove = false;
		uu.FlagMoving = false;
		uu.Image = U"chip001.png";

		ClassHorizontalUnit cuu;
		cuu.ListClassUnit.push_back(uu);

		classBattle.listOfAllUnit.push_back(cuu);
	}

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

	//始点設定
	viewPos = ToTileBottomCenter(classBattle.listOfAllUnit[0].ListClassUnit[0].initTilePos, N);
	camera.jumpTo(viewPos, camera.getTargetScale());

	//建物関係
	//cb.classMapBattle = ClassStaticCommonMethod::GetClassMapBattle(sM);
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
		unitBui.Image = U"home.png";
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
		unitBui.Image = U"home.png";
		unitBui.rowBuilding = 7;
		unitBui.colBuilding = 7;
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

	co_await mainLoop().pausedWhile([&]
		{
			if (KeySpace.pressed())
			{
				stopwatch.pause();
				return true;
			}
			else
			{
				stopwatch.resume();
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
		for (auto& units : classBattle.listOfAllUnit)
			UpdateVisibility(std::ref(visibilityMap), std::ref(units.ListClassUnit), N);

		// 進行度（0.0 ～ 1.0）
		if (arrBuildMenuHomeYoyaku.size() > 0)
		{
			if (t >= 1.0)
			{
				stopwatch.reset();
				String key = arrBuildMenuHomeYoyaku.front().name;
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

						uu.initTilePos = Point{ 4, 0 };//保険
						for (auto& item : classBattle.listOfAllUnit)
						{
							if (item.FlagBuilding == true &&
								!item.ListClassUnit.empty())
							{
								for (auto& itemUnit : item.ListClassUnit)
								{
									if (itemUnit.mapTipObjectType == MapTipObjectType::HOME)
									{
										uu.initTilePos = Point{ itemUnit.rowBuilding + 1, itemUnit.colBuilding + 1 };
									}
								}
							}
						}
						uu.orderPosiLeft = Point{ 0, 0 };
						uu.orderPosiLeftLast = Point{ 0, 0 };
						uu.nowPosiLeft = ToTile(uu.initTilePos, N).asPolygon().centroid().movedBy(-(uu.yokoUnit / 2), -(uu.TakasaUnit / 2));
						uu.vecMove = Vec2{ 0, 0 };
						uu.Speed = 1.0;
						uu.FlagMove = false;
						uu.FlagMoving = false;
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
			}
			t = Min(stopwatch.sF() / durationSec, 1.0);
		}

		switch (battleStatus)
		{
		case BattleStatus::Battle:
		{
			//カメラ移動 || 部隊を選択状態にする。もしくは既に選択状態なら移動させる
			{
				const auto t = camera.createTransformer();

				//右ダブルクリックで選択解除とする
				//ここでFlagMoveがfalseになる
				//そしてcontinue or break;

				// 建築物選択チェック
				{
					// いったん押された建築物の固有IDを走査する
					long id = -1;
					for (auto& item : classBattle.listOfAllUnit)
					{
						if (item.FlagBuilding == true &&
							!item.ListClassUnit.empty())
						{
							for (auto& itemUnit : item.ListClassUnit)
							{
								//TODO これ、うまくいってない？
								if (ToTile(ToIndex(itemUnit.GetNowPosiCenter(), columnQuads, rowQuads).value(), N).leftClicked())
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
							if (item.FlagBuilding == true &&
								!item.ListClassUnit.empty())
							{
								for (auto& itemUnit : item.ListClassUnit)
								{
									if (id == itemUnit.ID)
									{
										itemUnit.IsSelect = !itemUnit.IsSelect;
										IsBuildMenuHome = itemUnit.IsSelect;
									}
									else
									{
										itemUnit.IsSelect = false;
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
						for (auto& target : classBattle.listOfAllUnit)
							for (auto& unit : target.ListClassUnit)
								if (unit.FlagMove == true && unit.IsBattleEnable == true)
									lisUnit.push_back(&unit);

						if (lisUnit.size() == 1)
						{
							Unit* cu = lisUnit[0];
							// 移動先の座標算出
							Vec2 nor = Vec2(end - start).normalized();
							Vec2 moved = cu->nowPosiLeft + Vec2(nor.x * cu->Speed, nor.y * cu->Speed);
							// 移動先が有効かどうかチェック || 本来は経路探索で移動可能かどうか調べるべき
							auto index = ToIndex(moved, columnQuads, rowQuads);
							if (not index.has_value())
							{
								cu->FlagMove = false;
								co_return;;
							}

							//移動
							Unit& cuu = GetCU(cu->ID);
							cuu.vecMove = Vec2(cu->orderPosiLeft - cu->nowPosiLeft).normalized();
							cuu.orderPosiLeft = end;
							cuu.FlagMove = false;
							cuu.FlagMoving = true;
						}

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

							Array<ClassHorizontalUnit> lisClassHorizontalUnitLoop;
							for (auto& target : classBattle.listOfAllUnit)
							{
								ClassHorizontalUnit chu;
								for (auto& unit : target.ListClassUnit)
								{
									if (unit.FlagMove == true && unit.IsBattleEnable == true)
										chu.ListClassUnit.push_back(unit);
								}
								if (chu.ListClassUnit.size() > 0)
								{
									lisClassHorizontalUnitLoop.push_back(chu);
								}
							}

							for (auto&& [i, loopLisClassHorizontalUnit] : IndexedRef(lisClassHorizontalUnitLoop))
							{
								Array<Unit*> target;
								for (auto& unit : loopLisClassHorizontalUnit.ListClassUnit)
									if (unit.FlagMove == true && unit.IsBattleEnable == true)
										target.push_back(&unit);

								if (target.size() == 0)
									continue;

								//その部隊の人数を取得
								int32 unitCount = target.size();

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

						////経路算出
						//taskMyUnits = Async(BattleMoveAStarMyUnits,
						//				std::ref(getData().classGameStatus.classBattle.sortieUnitGroup),
						//				std::ref(getData().classGameStatus.classBattle.defUnitGroup),
						//				std::ref(mapCreator),
						//				std::ref(getData().classGameStatus.classBattle.classMapBattle.value().mapData),
						//				std::ref(getData().classGameStatus),
						//				std::ref(debugRoot), std::ref(debugAstar),
						//				std::ref(abortMyUnits), std::ref(pauseTaskMyUnits));
					}
					else
					{
						//範囲選択
						for (auto& target : classBattle.listOfAllUnit)
						{
							for (auto& unit : target.ListClassUnit)
							{
								Vec2 gnpc = unit.GetNowPosiCenter();
								if (start.x > end.x)
								{
									//左
									if (start.y > end.y)
									{
										//上
										if (gnpc.x >= end.x && gnpc.x <= start.x
											&& gnpc.y >= end.y && gnpc.y <= start.y)
										{
											GetCU(unit.ID).FlagMove = true;
											IsBattleMove = true;
										}
										else
										{
											GetCU(unit.ID).FlagMove = false;
										}
									}
									else
									{
										//下
										if (gnpc.x >= end.x && gnpc.x <= start.x
											&& gnpc.y >= start.y && gnpc.y <= end.y)
										{
											GetCU(unit.ID).FlagMove = true;
											IsBattleMove = true;
										}
										else
										{
											GetCU(unit.ID).FlagMove = false;
										}
									}
								}
								else
								{
									//右
									if (start.y > end.y)
									{
										//上
										if (gnpc.x >= start.x && gnpc.x <= end.x
											&& gnpc.y >= end.y && gnpc.y <= start.y)
										{
											GetCU(unit.ID).FlagMove = true;
											IsBattleMove = true;
										}
										else
										{
											GetCU(unit.ID).FlagMove = false;
										}
									}
									else
									{
										//下
										if (gnpc.x >= start.x && gnpc.x <= end.x
											&& gnpc.y >= start.y && gnpc.y <= end.y)
										{
											GetCU(unit.ID).FlagMove = true;
											IsBattleMove = true;
										}
										else
										{
											GetCU(unit.ID).FlagMove = false;
										}
									}
								}
							}
						}
					}
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

				for (auto&& [i, re] : Indexed(htBuildMenuHome))
				{
					if (re.second.leftClicked())
					{
						cBuildMenuHomeYoyaku ccc;
						ccc.sortId = cBuildMenuHomeYoyakuIdCount;
						ccc.name = re.first;
						ccc.texture = TextureAsset(re.first);
						arrBuildMenuHomeYoyaku.push_back(ccc);
						cBuildMenuHomeYoyakuIdCount++;
						if (stopwatch.isRunning() == false)
						{
							stopwatch.start();
							t = 0.0;
						}
					}
				}

				arrBuildMenuHomeYoyaku.sort_by([](const cBuildMenuHomeYoyaku& a, const cBuildMenuHomeYoyaku& b)
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

		co_await Co::NextFrame();
	}
}

void Battle::draw() const
{
	FsScene::draw();

	{
		// 2D カメラによる座標変換を適用する
		const auto tr = camera.createTransformer();

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

					// 底辺中央を基準にタイルを描く
					textures[grid[index]].draw(Arg::bottomCenter = pos);

					// 建物描写
					for (auto& abc : bui)
					{
						if (abc.IsBattleEnable == false)
							continue;

						if (abc.rowBuilding == (xi + k) && abc.colBuilding == (yi - k))
						{
							buiTex.push_back(abc);
						}
					}
					//if (visibilityMap[index] == Visibility::Seen)
					//{
					//	texture.draw(Arg::bottomCenter = pos, ColorF{ 0.5 }); // グレー表示
					//}
					//else if (visibilityMap[index] == Visibility::Visible)
					//{
					//	texture.draw(Arg::bottomCenter = pos); // 通常表示
					//}

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

			//bui
			for (auto aaa : buiTex)
			{
				if (aaa.IsSelect)
				{
					TextureAsset(aaa.Image).draw(Arg::bottomCenter = ToTileBottomCenter(Point(aaa.colBuilding, aaa.rowBuilding), N).movedBy(0, -15)).drawFrame(3.0, Palette::Red);
				}
				else
				{
					TextureAsset(aaa.Image).draw(Arg::bottomCenter = ToTileBottomCenter(Point(aaa.colBuilding, aaa.rowBuilding), N).movedBy(0, -15));
				}
			}

			// プレイヤーユニット描画
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

			// 敵ユニット描画
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
	}

	//-30とは下の線のこと
	renderTextureSkill.draw(0, Scene::Size().y - 320 - 30);
	renderTextureSkillUP.draw(0, Scene::Size().y - 320 - 30);

	if (IsBuildMenuHome)
	{
		renderTextureBuildMenuHome.draw(Scene::Size().x - 328, Scene::Size().y - 328 - 30);
		// ゲージの高さ（画像下から上へ）
		const double gaugeHeight = 64 * t;

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

Unit& Battle::GetCU(long ID)
{
	for (auto& temp : classBattle.listOfAllUnit)
		for (auto& temptemp : temp.ListClassUnit)
			if (temptemp.ID == ID)
				return temptemp;
}
