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

std::mutex aiRootMutex;

/// @brief アスターアルゴリズムで移動経路取得
/// @param target 
/// @param enemy 
/// @param mapData 
/// @param aiRoot 
/// @param debugRoot 
/// @param list 
/// @param columnQuads 
/// @param rowQuads 
/// @param N 
/// @param abort 
/// @param pause 
/// @return 
int32 BattleMoveAStar(Array<ClassHorizontalUnit>& target,
						Array<ClassHorizontalUnit>& enemy,
						Array<Array<MapDetail>> mapData,
						HashTable<int64, UnitMovePlan>& aiRoot,
						Array<Array<Point>>& debugRoot,
						Array<ClassAStar*>& list,
						Array<Quad>& columnQuads,
						Array<Quad>& rowQuads,
						const int32 N,
						const std::atomic<bool>& abort,
						const std::atomic<bool>& pause,
						std::atomic<bool>& changeUnitMember
)
{
	while (true)
	{
		if (changeUnitMember == true) continue;
		if (abort == true) break;
		if (pause == true) continue;

		while (true)
		{
			const auto isValidTarget = [](const Unit& unit) -> bool
				{
					if (unit.IsBuilding && (unit.mapTipObjectType == MapTipObjectType::WALL2 || unit.mapTipObjectType == MapTipObjectType::GATE))
						return false;

					if (!unit.IsBattleEnable)
						return false;

					return true;
				};

			if (abort == true) break;
			if (pause == true) continue;
			if (changeUnitMember == true) break;
			const auto targetSnapshot = target;
			for (auto& aaa : targetSnapshot)
			{
				if (abort == true) break;
				if (aaa.FlagBuilding == true) continue;

				for (auto& bbb : aaa.ListClassUnit)
				{
					if (abort == true) break;
					if (!isValidTarget(bbb)) continue;
					if (aiRoot.contains(bbb.ID) && !aiRoot[bbb.ID].isPathCompleted()) continue;

					Array<Point> listRoot;

					//まず現在のマップチップを取得
					s3d::Optional<Size> nowIndex = ToIndex(bbb.GetNowPosiCenter(), columnQuads, rowQuads);
					if (nowIndex.has_value() == false)
						continue;

					//標的は次のうちのどれか
					//1.ランダムに決定
					//2.最寄りの敵
					//3.一番弱い敵
					//4.一番体力の無い敵
					//...など

					//最寄りの敵の座標を取得
					HashTable<double, Unit> dicDis;
					Vec2 posA = bbb.GetNowPosiCenter();
					try
					{
						for (auto& ccc : enemy) {
							for (auto& ddd : ccc.ListClassUnit) {
								if (!isValidTarget(ddd)) continue;

								Vec2 posB = ddd.GetNowPosiCenter();
								double dist = posA.distanceFrom(posB);
								while (dicDis.contains(dist)) {
									dist += 0.0001; // 衝突回避
								}
								if (abort == true) break;
								dicDis.emplace(dist, ddd);
							}
						}
					}
					catch (const std::exception&)
					{
						throw;
					}

					if (dicDis.size() == 0)
						continue;

					auto minElement = dicDis.begin();
					for (auto it = dicDis.begin(); it != dicDis.end(); ++it)
					{
						if (it->first < minElement->first)
						{
							minElement = it;
						}
					}
					Print << minElement->second.ID;
					bool flagGetEscapeRange = false;
					Vec2 retreatTargetPos;
					//escape_rangeの範囲なら、撤退。その為、反対側の座標を調整したものを扱う
					if (bbb.Escape_range >= 1)
					{
						Circle cCheck = Circle(bbb.GetNowPosiCenter(), bbb.Escape_range);
						Circle cCheck2 = Circle(minElement->second.GetNowPosiCenter(), 1);
						if (cCheck.intersects(cCheck2) == true)
						{
							//撤退
							double newDistance = 50.0;
							double angle = atan2(minElement->second.GetNowPosiCenter().y - bbb.GetNowPosiCenter().y, minElement->second.GetNowPosiCenter().x - bbb.GetNowPosiCenter().x);
							double xC, yC;
							// 反対方向に進むために角度を180度反転
							angle += Math::Pi;
							xC = bbb.GetNowPosiCenter().x + newDistance * cos(angle);
							yC = bbb.GetNowPosiCenter().y + newDistance * sin(angle);
							//minElement->second.nowPosiLeft = Vec2(xC, yC);

							//TODO 画面端だとタイル外となるので、調整
							retreatTargetPos = Vec2(xC, yC);
							flagGetEscapeRange = true;
						}
					}

					if (bbb.FlagMoving == true && flagGetEscapeRange == false)
						continue;
					if (bbb.FlagMovingEnd == false && flagGetEscapeRange == false)
						continue;

					//最寄りの敵のマップチップを取得
					s3d::Optional<Size> nowIndexEnemy;
					if (flagGetEscapeRange)
					{
						nowIndexEnemy = ToIndex(retreatTargetPos, columnQuads, rowQuads);
					}
					else
					{
						nowIndexEnemy = ToIndex(minElement->second.GetNowPosiCenter(), columnQuads, rowQuads);
					}

					if (nowIndexEnemy.has_value() == false) continue;
					if (nowIndexEnemy.value() == nowIndex.value()) continue;

					////現在地を開く
					ClassAStarManager classAStarManager(nowIndexEnemy.value().x, nowIndexEnemy.value().y);
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
							{
								continue;
							}

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

					// 経路が取得できた場合、aiRootにセット
					if (listRoot.size() != 0)
					{
						UnitMovePlan plan;
						if (flagGetEscapeRange)
						{
							// もし撤退中なら特別なターゲットIDを設定
							// 撤退中は、経路の最初の位置を最後に見た敵の位置として記録する
							plan.setRetreating(true);
							plan.setTarget(-1); // -1: 撤退中の特別なターゲットID
							Unit iugiu = minElement->second;
							plan.setLastKnownEnemyPos(minElement->second.GetNowPosiCenter());

							{
								std::scoped_lock lock(aiRootMutex);
								for (auto& iydihlfdvhjkl : target)
								{
									for (auto& jouihdsjk : iydihlfdvhjkl.ListClassUnit)
									{
										//一致するユニットの情報を変更
										if (jouihdsjk.ID == bbb.ID)
										{
											// 強制的に再移動準備させる
											jouihdsjk.FlagMoving = false;
											jouihdsjk.FlagMovingEnd = true;
										}
									}
								}
							}
						}
						else
						{
							plan.setTarget(minElement->second.ID);
						}
						plan.setPath(listRoot);
						{
							std::scoped_lock lock(aiRootMutex);
							aiRoot[bbb.ID] = plan;
							// 経路セット時に1個除去しておく
							if (aiRoot[bbb.ID].getPath().size() > 1)
								aiRoot[bbb.ID].stepToNext();
						}
					}
				}
			}
		}
	}

	return -1;
}
int32 BattleMoveAStarMyUnits(Array<ClassHorizontalUnit>& target,
						Array<ClassHorizontalUnit>& enemy,
						Array<Array<MapDetail>> mapData,
						HashTable<int64, UnitMovePlan>& aiRoot,
						Array<Array<Point>>& debugRoot,
						Array<ClassAStar*>& list,
						Array<Quad>& columnQuads,
						Array<Quad>& rowQuads,
						const int32 N,
						const std::atomic<bool>& abort,
						const std::atomic<bool>& pause
)
{
	int32 eneCou = 0;
	for (auto ttt : enemy)
	{
		for (auto ijhiu : ttt.ListClassUnit)
		{
			eneCou++;
		}
	}
	if (eneCou == 0)
	{
		return -1;
	}

	const auto targetSnapshot = target;
	for (auto& aaa : targetSnapshot)
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
				UnitMovePlan plan;
				plan.setPath(listRoot);
				{
					std::scoped_lock lock(aiRootMutex);
					aiRoot[listClassUnit.ID] = plan;
				}
				// 経路セット時に1個除去しておく
				if (aiRoot[listClassUnit.ID].getPath().size() > 1)
					aiRoot[listClassUnit.ID].stepToNext();
				debugRoot.push_back(listRoot);
				{
					std::scoped_lock lock(aiRootMutex);
					for (auto& iydihlfdvhjkl : target)
					{
						for (auto& jouihdsjk : iydihlfdvhjkl.ListClassUnit)
						{
							//一致するユニットの情報を変更
							if (jouihdsjk.ID == listClassUnit.ID)
							{
								jouihdsjk.FlagMoveAI = false;
							}
						}
					}
				}
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
	cmb.mapData[0][10].resourcePointIcon = U"point000.png";
}

Vec2 ToIso(double x, double y)
{
	return Vec2((x - y), (x + y) / 2.0);
}
Color GetDominantColor(const String imageName, HashTable<String, Color>& data)
{
	// dataに指定された画像名が存在する場合はその色を返す
	if (data.contains(imageName))
	{
		return data[imageName];
	}

	const Image image{ PATHBASE + PATH_DEFAULT_GAME + U"/015_BattleMapCellImage/" + imageName };

	// 色の頻度を記録するハッシュマップ
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
void Battle::DrawMiniMap(const Grid<int32>& map, const RectF& cameraRect) const
{
	//何故このサイズだとちょうどいいのかよくわかっていない
	//pngファイルは100*65
	//ミニマップ描写方法と通常マップ描写方法は異なるので、無理に合わせなくて良い？
	const Vec2 tileSize = Vec2(50, 25 + 25);

	// --- ミニマップに表示する位置・サイズ
	const Vec2 miniMapTopLeft = miniMapPosition;
	const SizeF miniMapDrawArea = miniMapSize;

	// --- マップの4隅をアイソメ変換 → 範囲を取得
	const Vec2 isoA = ToIso(0, 0) * tileSize;
	const Vec2 isoB = ToIso(map.width(), 0) * tileSize;
	const Vec2 isoC = ToIso(0, map.height()) * tileSize;
	const Vec2 isoD = ToIso(map.width(), map.height()) * tileSize;

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
			const Vec2 iso = ToIso(x, y) * tileSize;
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
Array<ResourcePointTooltip::TooltipTarget> Battle::SetResourceTargets(Array<ResourcePointTooltip::TooltipTarget> resourceTargets)
{
	{
		const auto& mapData = classBattle.classMapBattle.value().mapData;

		const int32 mapSize = static_cast<int32>(mapData.size());

		for (int32 x = 0; x < mapSize; ++x)
		{
			for (int32 y = 0; y < static_cast<int32>(mapData[x].size()); ++y)
			{
				const auto& tile = mapData[x][y];

				if (!tile.isResourcePoint)
					continue;

				const Vec2 pos = ToTileBottomCenter(Point(x, y), N);

				const Circle area = Circle(pos.movedBy(0, -TileThickness - TileOffset.y), TextureAsset(tile.resourcePointIcon).region().w / 2);
				String desc = U"資源ポイント\n所有者: {}\n種類:{}\n量:{}"_fmt(
							(tile.whichIsThePlayer == BattleWhichIsThePlayer::Sortie) ? U"味方" : U"敵", U"金", U"5/s");

				resourceTargets << ResourcePointTooltip::TooltipTarget{ area, desc };
				resourcePointTooltip.setTargets(resourceTargets);
			}
		}
	}

	return resourceTargets;
}

Battle::Battle(GameData& saveData, CommonConfig& commonConfig, SystemString argSS)
	:FsScene(U"Battle"), m_saveData{ saveData }, m_commonConfig{ commonConfig }, N{ 64 }
	, ss{ argSS }
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
	Array<ResourcePointTooltip::TooltipTarget> resourceTargets;
	resourceTargets = SetResourceTargets(resourceTargets);
	resourcePointTooltip.setTooltipEnabled(true);

	N = classBattle.classMapBattle.value().mapData.size();
	grid = Grid<int32>(Size{ N, N });
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
			crh.setumei = re.second.setumei;
			arr.push_back(crh);
		}

		for (auto& icons : arr)
			TextureAsset(icons.key).resized(64).draw(icons.rect.x, icons.rect.y);

		Rect df = Rect(328, 328);
		df.drawFrame(4, 0, ColorF{ 0.5 });
	}
}

void Battle::UnitRegister(String unitName, int32 col, int32 row, int32 num, Array<ClassHorizontalUnit>& listU)
{
	for (auto uu : m_commonConfig.arrayUnit)
	{
		if (uu.Name == unitName)
		{
			uu.ID = classBattle.getIDCount();
			uu.buiSyu = 1;
			uu.initTilePos =
				Point{ col,
						row };
			uu.nowPosiLeft = ToTile(uu.initTilePos, N).asPolygon().centroid().movedBy(-(uu.yokoUnit / 2), -(uu.TakasaUnit / 2));
			Vec2 temp = uu.GetNowPosiCenter().movedBy(-64 / 2, (32 / 2) + 8);
			uu.bLiquidBarBattle = GameUIToolkit::LiquidBarBattle(Rect(temp.x, temp.y, 64, 8));
			ClassHorizontalUnit cuu;

			for (size_t i = 0; i < num; i++)
			{
				uu.ID = classBattle.getIDCount();
				cuu.ListClassUnit.push_back(uu);
			}

			listU.push_back(cuu);
		}
	}
}

Co::Task<void> Battle::start()
{
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
			fontZinkei(ss.Zinkei[i]).draw(ttt, Palette::Black);
		}
	}

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
			cbpHome.setumei = U"軽装歩兵詰所を建築します。";
			cbpKouhei.htCountAndSyu.emplace(U"keisou-hohei.png", cbpHome);
		}

		cbpKouhei.buiSyu = 2; // 建築の種類
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
		{
			cBuildPrepareKindAndCount cbpHome;
			cbpHome.sortId = 1;
			cbpHome.count = -1;
			cbpHome.kind = U"Unit";
			cbpKeisouHoheiT.htCountAndSyu.emplace(U"keisou-chipGene008.png", cbpHome);
		}
		cbpKeisouHoheiT.buiSyu = 3; // 建築の種類
		renB(renderTextureBuildMenuKeisouHoheiT, cbpKeisouHoheiT, arrayComRight_BuildMenu_KeisouHoheiT);
	}

	//初期ユニット
	{
		for (auto uu : m_commonConfig.arrayUnit)
		{
			if (uu.Name == U"M14 Infantry Rifle")
			{
				uu.ID = classBattle.getIDCount();
				uu.buiSyu = 2;
				uu.Image = U"chip006.png";
				uu.initTilePos = Point{ 10, 10 };
				uu.nowPosiLeft = ToTile(uu.initTilePos, N).asPolygon().centroid().movedBy(-(uu.yokoUnit / 2), -(uu.TakasaUnit / 2));
				ClassHorizontalUnit cuu;
				cuu.ListClassUnit.push_back(uu);
				classBattle.listOfAllUnit.push_back(cuu);
			}
		}
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
	resourcePointTooltip.setCamera(camera);

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
		for (auto uu : m_commonConfig.arrayUnit)
		{
			if (uu.Name == U"M14 Infantry Rifle")
			{
				uu.ID = classBattle.getIDCount();
				uu.IsBuilding = true;
				uu.mapTipObjectType = MapTipObjectType::HOME;
				uu.NoWall2 = 0;
				uu.HPCastle = 1000;
				uu.CastleDefense = 1000;
				uu.CastleMagdef = 1000;
				uu.Image = U"home1.png";
				uu.rowBuilding = N / 2;
				uu.colBuilding = N / 2;
				uu.initTilePos = Point{ uu.rowBuilding, uu.colBuilding };
				uu.Move = 0.0;
				uu.buiSyu = 0;
				uu.nowPosiLeft = ToTile(uu.initTilePos, N).asPolygon().centroid().movedBy(-(uu.yokoUnit / 2), -(uu.TakasaUnit / 2));
				ClassHorizontalUnit cuu;
				cuu.ListClassUnit.push_back(uu);
				chuSor.ListClassUnit.push_back(uu);
			}
		}
	}

	classBattle.listOfAllUnit.push_back(chuSor);
	//classBattle.listOfAllEnemyUnit.push_back(chuDef);

	//ミニマップ用
	for (int32 y = 0; y < grid.height(); ++y)
	{
		for (int32 x = 0; x < grid.width(); ++x)
		{
			String ttt = classBattle.classMapBattle.value().mapData[x][y].tip + U".png";
			minimapCols.emplace(Point(x, y), GetDominantColor(ttt, colData));
		}
	}

	stopwatchFinance.restart();
	stopwatchGameTime.restart();

	task = Async(BattleMoveAStar,
		std::ref(classBattle.listOfAllEnemyUnit),
		std::ref(classBattle.listOfAllUnit),
		std::ref(classBattle.classMapBattle.value().mapData),
		std::ref(aiRootEnemy),
		std::ref(debugRoot), std::ref(debugAstar),
		std::ref(columnQuads),
		std::ref(rowQuads),
		N,
		std::ref(abort), std::ref(pauseTask), std::ref(changeUnitMember));

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

void Battle::playResourceEffect()
{
	//CircleEffect(Vec2{ 500, 500 }, 50, ColorF{ 1.0, 1.0, 0.2 });
	//SoundAsset(U"resourceGet").playMulti();
}
void Battle::updateResourceIncome()
{
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

		// 資源取得エフェクトやサウンド再生（省略可）
		playResourceEffect();
	}
}
void Battle::refreshFogOfWar()
{

	//毎タスクで霧gridをfalseにすれば、「生きているユニットの周りだけ明るい」が可能
	// 一度見たタイルは UnseenではなくSeenにしたい
	for (auto&& ttt : visibilityMap)
		ttt = Visibility::Unseen;

	for (auto& units : classBattle.listOfAllUnit)
		UpdateVisibility(std::ref(visibilityMap), std::ref(units.ListClassUnit), N);
}
void Battle::spawnTimedEnemy()
{
	if (stopwatchGameTime.sF() >= 5)
	{
		int32 kukj = Random(1, 2);
		if (kukj / 2 == 0)
		{
			int32 iyigu = Random(0, N - 1);
			changeUnitMember = true;
			UnitRegister(U"P99 Sniper Rifle",
			0,
			Random(0, N - 1),
			1, classBattle.listOfAllEnemyUnit);
			changeUnitMember = false;
			stopwatchGameTime.restart();
		}
		else
		{
			int32 iyigu = Random(0, N - 1);
			changeUnitMember = true;
			UnitRegister(U"P99 Sniper Rifle",
			Random(0, N - 1),
			N - 1,
			1, classBattle.listOfAllEnemyUnit);
			changeUnitMember = false;
			stopwatchGameTime.restart();
		}
	}
}
void Battle::updateResourceCapture()
{
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
}
void Battle::updateUnitHealthBars()
{
	constexpr Vec2 offset{ -32, +22 }; // = -64 / 2, +32 / 2 + 6

	auto updateBar = [&](Unit& unit)
		{
			double hpRatio = static_cast<double>(unit.Hp) / unit.HpMAX;
			unit.bLiquidBarBattle.update(hpRatio);
			unit.bLiquidBarBattle.ChangePoint(unit.GetNowPosiCenter() + offset);
		};

	for (auto& group : classBattle.listOfAllUnit)
	{
		if (group.FlagBuilding || group.ListClassUnit.empty())
			continue;

		for (auto& unit : group.ListClassUnit)
			updateBar(unit);
	}

	for (auto& group : classBattle.listOfAllEnemyUnit)
	{
		if (group.FlagBuilding || group.ListClassUnit.empty())
			continue;

		for (auto& unit : group.ListClassUnit)
			updateBar(unit);
	}
}
void Battle::handleBuildMenuSelectionA()
{
	const Transformer2D transformer{ Mat3x2::Identity(), Mat3x2::Translate(Scene::Size().x - 328, Scene::Size().y - 328 - 30) };

	// 対象の建物カテゴリ（buiSyu）に応じたメニューを選択
	Array<cRightMenu> temp;
	switch (buiSyu)
	{
	case 0:
		temp = arrayComRight_BuildMenu_Home; break;
	case 1:
		temp = arrayComRight_BuildMenu_Thunderwalker; break;
	case 2:
		temp = arrayComRight_BuildMenu_Kouhei; break;
	case 3:
		temp = arrayComRight_BuildMenu_KeisouHoheiT; break;
	default:
		return; // 無効なカテゴリ
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

			IsBuildSelectTraget = false;

			// 予約作成
			cRightMenu ccc;
			ccc.sortId = re.sortId;
			ccc.sortYoyakuId = longBuildMenuHomeYoyakuIdCount++;
			ccc.key = re.key;
			ccc.kindForProcess = re.kindForProcess;
			ccc.texture = TextureAsset(re.key);
			ccc.time = re.time;

			// 設置位置の取得
			Unit& cu = GetCU(longBuildSelectTragetId);
			if (const auto index = ToIndex(cu.GetNowPosiCenter(), columnQuads, rowQuads))
			{
				ccc.rowBuilding = index->y;
				ccc.colBuilding = index->x;
			}

			// カテゴリに追加
			int index = re.buiSyu;
			if (InRange(index, 0, (int32)buildMenus.size()))
			{
				auto& menu = buildMenus[index];
				if (!menu.timer.isRunning())
				{
					menu.timer.start();
					menu.progressTime = 0.0;
				}
				menu.reservations.push_back(ccc);
			}

			// 回数制限の更新と再描画
			if (re.count > 0)
			{
				re.count--;
				cbp.htCountAndSyu[re.key].count = re.count;
				switch (re.buiSyu)
				{
				case 0: renB(renderTextureBuildMenuHome, cbp, arrayComRight_BuildMenu_Home); break;
				case 2: renB(renderTextureBuildMenuKouhei, cbpKouhei, arrayComRight_BuildMenu_Kouhei); break;
					// 他カテゴリ対応は必要に応じて
				}
			}
			break;
		}
		else if (re.rect.mouseOver())
		{
			nowSelectBuildSetumei = U"~~~Unit Or Build~~~\r\n" + re.setumei;
			rectSetumei = { Scene::Size().x - renderTextureBuildMenuEmpty.size().x,
				Scene::Size().y - underBarHeight - renderTextureBuildMenuEmpty.size().y,
				320, 0 };
			rectSetumei.h = fontSkill(nowSelectBuildSetumei).region().h;
			while (!fontSkill(nowSelectBuildSetumei).draw(rectSetumei.stretched(-12), Color(0.0, 0.0)))
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

	// sortYoyakuId による予約ソート
	for (auto& menu : buildMenus)
	{
		menu.reservations.sort_by([](const cRightMenu& a, const cRightMenu& b)
		{
			return a.sortId < b.sortId;
		});
	}
}
void Battle::handleBuildMenuSelectionB()
{
	if (aiRootMy[longIsMovedYoyakuId].getPath().size() > 0)
	{
		//移動中
	}
	else
	{
		isMovedYoyaku = false;//移動が終わった為
		longIsMovedYoyakuId = -1; // IDをリセット

		// 対象の建物カテゴリ（buiSyu）に応じたメニューを選択
		Array<cRightMenu> temp;
		switch (buiSyu)
		{
		case 0:
			temp = arrayComRight_BuildMenu_Home; break;
		case 1:
			temp = arrayComRight_BuildMenu_Thunderwalker; break;
		case 2:
			temp = arrayComRight_BuildMenu_Kouhei; break;
		case 3:
			temp = arrayComRight_BuildMenu_KeisouHoheiT; break;
		default:
			break;
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

				// カテゴリに追加
				int index = re.buiSyu;
				if (InRange(index, 0, (int32)buildMenus.size()))
				{
					auto& menu = buildMenus[index];
					if (!menu.timer.isRunning())
					{
						menu.timer.start();
						menu.progressTime = 0.0;
					}
					menu.reservations.push_back(ccc);
				}

				// 回数制限の更新と再描画
				if (re.count > 0)
				{
					re.count--;
					cbp.htCountAndSyu[re.key].count = re.count;
					switch (re.buiSyu)
					{
					case 0: renB(renderTextureBuildMenuHome, cbp, arrayComRight_BuildMenu_Home); break;
					case 2: renB(renderTextureBuildMenuKouhei, cbpKouhei, arrayComRight_BuildMenu_Kouhei); break;
						// 他カテゴリ対応は必要に応じて
					}
				}
			}
		}
	}

}
void Battle::updateBuildQueue()
{
	for (int index = 0; index < buildMenus.size(); ++index)
	{
		auto& menu = buildMenus[index];
		if (menu.reservations.isEmpty())
			continue;

		const double tempTime = menu.reservations.front().time;

		if (menu.progressTime >= 1.0)
		{
			menu.timer.reset();

			const cRightMenu& task = menu.reservations.front();
			const String key = task.key;
			const String kind = task.kindForProcess;
			const int32 count = task.count;
			const int32 row = task.rowBuilding;
			const int32 col = task.colBuilding;

			menu.reservations.pop_front();
			if (!menu.reservations.isEmpty())
			{
				menu.progressTime = 0.0;
				menu.timer.start();
			}
			else
			{
				menu.progressTime = -1.0;
			}

			// 実行内容
			if (index == 0) // Home
			{
				if (key == U"kouhei.png")
				{
					Unit uu;
					uu.ID = classBattle.getIDCount();
					uu.IsBuilding = false;
					for (auto& item : classBattle.listOfAllUnit)
					{
						if (item.FlagBuilding && !item.ListClassUnit.isEmpty())
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
					uu.orderPosiLeft = uu.orderPosiLeftLast = Point{ 0, 0 };
					uu.nowPosiLeft = ToTile(uu.initTilePos, N).asPolygon().centroid().movedBy(-uu.yokoUnit / 2, -uu.TakasaUnit / 2);
					uu.vecMove = Vec2{ 0, 0 };
					uu.Speed = 1.0;
					uu.Move = 500.0;
					uu.FlagMove = uu.FlagMoving = false;
					uu.IsBattleEnable = true;
					uu.buiSyu = 2;
					uu.Image = U"chip006.png";

					ClassHorizontalUnit cuu;
					cuu.ListClassUnit.push_back(uu);
					classBattle.listOfAllUnit.push_back(cuu);
				}
				// その他の Home の key に応じた処理は必要に応じて追加
			}
			else if (index == 1) // Thunderwalker
			{
				if (key == U"zirai.png")
				{
					UnitRegister(U"LandmineAA", col, row, 1, classBattle.listOfAllUnit);
				}
			}
			else if (index == 2) // Kouhei
			{
				for (auto&& [i, re] : IndexedRef(arrayComRight_BuildMenu_Kouhei))
				{
					if (re.sortId == task.sortId)
					{
						arrayComRight_BuildMenu_KeisouHoheiT[0].rowBuilding = row;
						arrayComRight_BuildMenu_KeisouHoheiT[0].colBuilding = col;
					}
				}
				if (key == U"keisou-hohei.png")
				{
					Unit uu;
					uu.ID = classBattle.getIDCount();
					uu.IsBuilding = true;
					uu.initTilePos = Point{ col, row };
					uu.nowPosiLeft = ToTile(uu.initTilePos, N).asPolygon().centroid().movedBy(-uu.yokoUnit / 2, -uu.TakasaUnit / 2);
					uu.rowBuilding = row;
					uu.colBuilding = col;
					uu.NoWall2 = 0;
					uu.HPCastle = uu.CastleDefense = uu.CastleMagdef = 1000;
					uu.buiSyu = 3;
					uu.Image = U"bu-keiso.png";
					Vec2 temp = uu.GetNowPosiCenter().movedBy(-32, 22);
					uu.bLiquidBarBattle = GameUIToolkit::LiquidBarBattle(Rect(temp.x, temp.y, 64, 16));

					ClassHorizontalUnit cuu;
					cuu.FlagBuilding = true;
					cuu.ListClassUnit.push_back(uu);
					classBattle.listOfAllUnit.push_back(cuu);
				}
				else if (key == U"zirai.png")
				{
					UnitRegister(U"LandmineAA", col, row, 1, classBattle.listOfAllUnit);
				}
			}
			else if (index == 3) // KeisouHoheiT
			{
				if (key == U"keisou-chipGene006.png")
				{
					UnitRegister(U"M14 Infantry Rifle", arrayComRight_BuildMenu_KeisouHoheiT[0].colBuilding,
						arrayComRight_BuildMenu_KeisouHoheiT[0].rowBuilding,
						3, classBattle.listOfAllUnit);
				}
				else if (key == U"keisou-chipGene008.png")
				{
					UnitRegister(U"P99 Sniper Rifle", arrayComRight_BuildMenu_KeisouHoheiT[0].colBuilding,
						arrayComRight_BuildMenu_KeisouHoheiT[0].rowBuilding,
						1, classBattle.listOfAllUnit);
				}
			}
		}

		// プログレス更新
		menu.progressTime = Min(menu.timer.sF() / tempTime, 1.0);
	}
}
Co::Task<> Battle::checkCancelSelectionByUIArea()
{
	if (Cursor::PosF().y >= Scene::Size().y - underBarHeight)
	{
		buiSyu = -1;
		longBuildSelectTragetId = -1;
		IsResourceSelectTraget = false;

		for (auto& target : classBattle.listOfAllUnit)
		{
			for (auto& unit : target.ListClassUnit)
			{
				unit.IsSelect = false;
			}
		}

		co_await Co::NextFrame();
	}
	co_return;
}
void Battle::updateUnitMovements()
{
	//移動処理
	{
		for (auto& item : classBattle.listOfAllUnit)
		{
			for (auto& itemUnit : item.ListClassUnit)
			{
				if (itemUnit.IsBuilding == true && itemUnit.mapTipObjectType == MapTipObjectType::WALL2)
					continue;
				if (itemUnit.IsBuilding == true && itemUnit.mapTipObjectType == MapTipObjectType::GATE)
					continue;
				if (itemUnit.IsBattleEnable == false)
					continue;

				if (!aiRootMy.contains(itemUnit.ID))
					continue;

				auto& plan = aiRootMy[itemUnit.ID];

				if (plan.isPathCompleted())
				{
					if (itemUnit.FlagMoving == false && itemUnit.FlagMovingEnd == true)
						continue;

					//最終移動
					itemUnit.nowPosiLeft += itemUnit.vecMove * ((itemUnit.Move + itemUnit.cts.Speed) / 100.0);
					if (itemUnit.GetNowPosiCenter().distanceFrom(itemUnit.GetOrderPosiCenter()) < 3.0)
					{
						itemUnit.FlagMoving = false;
						itemUnit.nowPosiLeft = itemUnit.orderPosiLeft; // 位置をピッタリ補正して止めるのもあり
						itemUnit.FlagReachedDestination = true;
						itemUnit.FlagMovingEnd = true;
					}

					continue;
				}

				if (itemUnit.FlagMoving)
				{
					itemUnit.nowPosiLeft += itemUnit.vecMove * ((itemUnit.Move + itemUnit.cts.Speed) / 100.0);

					if (plan.getCurrentTarget())
					{
						if (itemUnit.GetNowPosiCenter().distanceFrom(itemUnit.GetOrderPosiCenter()) <= 3.0)
						{
							plan.stepToNext();
							if (plan.getCurrentTarget())
							{
								// 到達チェック
								const int32 i = plan.getCurrentTarget().value().manhattanLength();
								const int32 xi = (i < (N - 1)) ? 0 : (i - (N - 1));
								const int32 yi = (i < (N - 1)) ? i : (N - 1);
								const int32 k2 = (plan.getCurrentTarget().value().manhattanDistanceFrom(Point{ xi, yi }) / 2);
								const double posX = ((i < (N - 1)) ? (i * -TileOffset.x) : ((i - 2 * N + 2) * TileOffset.x));
								const double posY = (i * TileOffset.y) - TileThickness;
								const Vec2 pos = { (posX + TileOffset.x * 2 * k2) - (itemUnit.yokoUnit / 2), posY - itemUnit.TakasaUnit - 15 };
								Vec2 nextPos = pos;
								itemUnit.orderPosiLeft = nextPos;
								itemUnit.vecMove = (itemUnit.GetOrderPosiCenter() - itemUnit.GetNowPosiCenter()).normalized();
							}
						}
					}
					if (plan.isPathCompleted())
					{
						itemUnit.orderPosiLeft = itemUnit.orderPosiLeftLast; // 最後の位置に戻す
						Vec2 hhh = itemUnit.GetOrderPosiCenter() - itemUnit.GetNowPosiCenter();
						itemUnit.vecMove = hhh.isZero() ? Vec2{ 0, 0 } : hhh.normalized();
					}
					continue;
				}

				// 次のマスに向けて移動準備
				if (plan.getCurrentTarget().has_value())
				{
					// そのタイルの底辺中央の座標
					const int32 i = plan.getCurrentTarget().value().manhattanLength();
					const int32 xi = (i < (N - 1)) ? 0 : (i - (N - 1));
					const int32 yi = (i < (N - 1)) ? i : (N - 1);
					const int32 k2 = (plan.getCurrentTarget().value().manhattanDistanceFrom(Point{ xi, yi }) / 2);
					const double posX = ((i < (N - 1)) ? (i * -TileOffset.x) : ((i - 2 * N + 2) * TileOffset.x));
					const double posY = (i * TileOffset.y) - TileThickness;
					const Vec2 pos = { (posX + TileOffset.x * 2 * k2) - (itemUnit.yokoUnit / 2), posY - itemUnit.TakasaUnit - 15 };

					itemUnit.orderPosiLeft = Vec2(Math::Round(pos.x), Math::Round(pos.y));

					Vec2 hhh = itemUnit.GetOrderPosiCenter() - itemUnit.GetNowPosiCenter();
					itemUnit.vecMove = hhh.isZero() ? Vec2{ 0, 0 } : hhh.normalized();

					itemUnit.FlagMoving = true;
					itemUnit.FlagMovingEnd = false;
				}
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

				if (!aiRootEnemy.contains(itemUnit.ID))
					continue;

				auto& plan = aiRootEnemy[itemUnit.ID];

				// 1. 移動準備（FlagMoving == false の場合のみ）
				if (!itemUnit.FlagMoving && plan.getCurrentTarget())
				{
					const Point targetTile = plan.getCurrentTarget().value();

					// そのタイルの底辺中央の座標
					const int32 i = plan.getCurrentTarget().value().manhattanLength();
					const int32 xi = (i < (N - 1)) ? 0 : (i - (N - 1));
					const int32 yi = (i < (N - 1)) ? i : (N - 1);
					const int32 k2 = (plan.getCurrentTarget().value().manhattanDistanceFrom(Point{ xi, yi }) / 2);
					const double posX = ((i < (N - 1)) ? (i * -TileOffset.x) : ((i - 2 * N + 2) * TileOffset.x));
					const double posY = (i * TileOffset.y) - TileThickness;
					const Vec2 pos = { (posX + TileOffset.x * 2 * k2) - (itemUnit.yokoUnit / 2), posY - itemUnit.TakasaUnit - 15 };
					itemUnit.orderPosiLeft = Vec2(Math::Round(pos.x), Math::Round(pos.y));

					Vec2 hhh = itemUnit.GetOrderPosiCenter() - itemUnit.GetNowPosiCenter();
					itemUnit.vecMove = hhh.isZero() ? Vec2{ 0, 0 } : hhh.normalized();

					itemUnit.FlagMoving = true;
					itemUnit.FlagMovingEnd = false;
				}

				// 2. 移動処理（FlagMoving == true の場合）
				if (itemUnit.FlagMoving)
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

								// タイルの底辺中央座標を計算（略）
															// そのタイルの底辺中央の座標
								const int32 i = plan.getCurrentTarget().value().manhattanLength();
								const int32 xi = (i < (N - 1)) ? 0 : (i - (N - 1));
								const int32 yi = (i < (N - 1)) ? i : (N - 1);
								const int32 k2 = (plan.getCurrentTarget().value().manhattanDistanceFrom(Point{ xi, yi }) / 2);
								const double posX = ((i < (N - 1)) ? (i * -TileOffset.x) : ((i - 2 * N + 2) * TileOffset.x));
								const double posY = (i * TileOffset.y) - TileThickness;
								const Vec2 pos = { (posX + TileOffset.x * 2 * k2) - (itemUnit.yokoUnit / 2), posY - itemUnit.TakasaUnit - 15 };
								itemUnit.orderPosiLeft = Vec2(Math::Round(pos.x), Math::Round(pos.y));

								Vec2 hhh = itemUnit.GetOrderPosiCenter() - itemUnit.GetNowPosiCenter();
								itemUnit.vecMove = hhh.isZero() ? Vec2{ 0, 0 } : hhh.normalized();

								itemUnit.FlagMoving = true;
								itemUnit.FlagMovingEnd = false;
							}
						}
					}

					if (plan.isPathCompleted())
					{
						itemUnit.FlagMoving = false;
						itemUnit.FlagReachedDestination = true;
						itemUnit.FlagMovingEnd = true;
					}
				}
			}
		}
	}

	//auto updateMovement = [&](Array<ClassHorizontalUnit>& units, HashTable<int64, UnitMovePlan>& aiRoot)
	//	{
	//		for (auto& item : units)
	//		{
	//			for (auto& itemUnit : item.ListClassUnit)
	//			{
	//				if ((itemUnit.IsBuilding && itemUnit.mapTipObjectType == MapTipObjectType::WALL2) ||
	//					(itemUnit.IsBuilding && itemUnit.mapTipObjectType == MapTipObjectType::GATE) ||
	//					!itemUnit.IsBattleEnable ||
	//					!aiRoot.contains(itemUnit.ID))
	//				{
	//					continue;
	//				}
	//				auto& plan = aiRoot[itemUnit.ID];
	//				if (plan.isPathCompleted())
	//				{
	//					// 最終位置補正
	//					if (!itemUnit.FlagMoving && itemUnit.FlagMovingEnd)
	//						continue;
	//					itemUnit.nowPosiLeft += itemUnit.vecMove * ((itemUnit.Move + itemUnit.cts.Speed) / 100.0);
	//					if (itemUnit.GetNowPosiCenter().distanceFrom(itemUnit.GetOrderPosiCenter()) < 3.0)
	//					{
	//						itemUnit.FlagMoving = false;
	//						itemUnit.nowPosiLeft = itemUnit.orderPosiLeft;
	//						itemUnit.FlagReachedDestination = true;
	//						itemUnit.FlagMovingEnd = true;
	//					}
	//					continue;
	//				}
	//				if (itemUnit.FlagMoving)
	//				{
	//					// 移動処理
	//					itemUnit.nowPosiLeft += itemUnit.vecMove * ((itemUnit.Move + itemUnit.cts.Speed) / 100.0);
	//					if (plan.getCurrentTarget())
	//					{
	//						if (itemUnit.GetNowPosiCenter().distanceFrom(itemUnit.GetOrderPosiCenter()) <= 3.0)
	//						{
	//							plan.stepToNext();
	//							if (plan.getCurrentTarget())
	//							{
	//								// 到達チェック
	//								const int32 i = plan.getCurrentTarget().value().manhattanLength();
	//								const int32 xi = (i < (N - 1)) ? 0 : (i - (N - 1));
	//								const int32 yi = (i < (N - 1)) ? i : (N - 1);
	//								const int32 k2 = (plan.getCurrentTarget().value().manhattanDistanceFrom(Point{ xi, yi }) / 2);
	//								const double posX = ((i < (N - 1)) ? (i * -TileOffset.x) : ((i - 2 * N + 2) * TileOffset.x));
	//								const double posY = (i * TileOffset.y) - TileThickness;
	//								const Vec2 pos = { (posX + TileOffset.x * 2 * k2) - (itemUnit.yokoUnit / 2), posY - itemUnit.TakasaUnit - 15 };
	//								Vec2 nextPos = pos;
	//								itemUnit.orderPosiLeft = nextPos;
	//								itemUnit.vecMove = (itemUnit.GetOrderPosiCenter() - itemUnit.GetNowPosiCenter()).normalized();
	//							}
	//						}
	//					}
	//					if (plan.isPathCompleted())
	//					{
	//						itemUnit.orderPosiLeft = itemUnit.orderPosiLeftLast; // 最後の位置に戻す
	//						Vec2 hhh = itemUnit.GetOrderPosiCenter() - itemUnit.GetNowPosiCenter();
	//						itemUnit.vecMove = hhh.isZero() ? Vec2{ 0, 0 } : hhh.normalized();
	//					}
	//					//if (plan.getCurrentTarget())
	//					//{
	//					//	if (itemUnit.GetNowPosiCenter().distanceFrom(itemUnit.GetOrderPosiCenter()) <= 3.0)
	//					//	{
	//					//		plan.stepToNext();
	//					//		if (plan.getCurrentTarget())
	//					//		{
	//					//			const Point target = plan.getCurrentTarget().value();
	//					//			const int32 i = target.manhattanLength();
	//					//			const int32 xi = (i < (N - 1)) ? 0 : (i - (N - 1));
	//					//			const int32 yi = (i < (N - 1)) ? i : (N - 1);
	//					//			const int32 k2 = (target.manhattanDistanceFrom(Point{ xi, yi }) / 2);
	//					//			const double posX = ((i < (N - 1)) ? (i * -TileOffset.x) : ((i - 2 * N + 2) * TileOffset.x));
	//					//			const double posY = (i * TileOffset.y) - TileThickness;
	//					//			const Vec2 pos = { (posX + TileOffset.x * 2 * k2) - (itemUnit.yokoUnit / 2), posY - itemUnit.TakasaUnit - 15 };
	//					//			itemUnit.orderPosiLeft = pos;
	//					//			itemUnit.vecMove = (itemUnit.GetOrderPosiCenter() - itemUnit.GetNowPosiCenter()).normalized();
	//					//		}
	//					//	}
	//					//}
	//					//if (plan.isPathCompleted())
	//					//{
	//					//	itemUnit.orderPosiLeft = itemUnit.orderPosiLeftLast;
	//					//	Vec2 hhh = itemUnit.GetOrderPosiCenter() - itemUnit.GetNowPosiCenter();
	//					//	itemUnit.vecMove = hhh.isZero() ? Vec2{ 0, 0 } : hhh.normalized();
	//					//}
	//					continue;
	//				}
	//				if (plan.getCurrentTarget())
	//				{
	//					// 移動準備
	//					const Point target = plan.getCurrentTarget().value();
	//					const int32 i = target.manhattanLength();
	//					const int32 xi = (i < (N - 1)) ? 0 : (i - (N - 1));
	//					const int32 yi = (i < (N - 1)) ? i : (N - 1);
	//					const int32 k2 = (target.manhattanDistanceFrom(Point{ xi, yi }) / 2);
	//					const double posX = ((i < (N - 1)) ? (i * -TileOffset.x) : ((i - 2 * N + 2) * TileOffset.x));
	//					const double posY = (i * TileOffset.y) - TileThickness;
	//					const Vec2 pos = { (posX + TileOffset.x * 2 * k2) - (itemUnit.yokoUnit / 2), posY - itemUnit.TakasaUnit - 15 };
	//					itemUnit.orderPosiLeft = Vec2(Math::Round(pos.x), Math::Round(pos.y));
	//					Vec2 hhh = itemUnit.GetOrderPosiCenter() - itemUnit.GetNowPosiCenter();
	//					itemUnit.vecMove = hhh.isZero() ? Vec2{ 0, 0 } : hhh.normalized();
	//					itemUnit.FlagMoving = true;
	//					itemUnit.FlagMovingEnd = false;
	//				}
	//			}
	//		}
	//	};
	//updateMovement(classBattle.listOfAllUnit, aiRootMy);
	//updateMovement(classBattle.listOfAllEnemyUnit, aiRootEnemy);
}
void Battle::handleSkillUISelection()
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

				while (not fontSkill(nowSelectSkillSetumei).draw(rectSkillSetumei.stretched(-12), Color(0.0, 0.0)))
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
void Battle::handleCameraInput()
{
	// カメラの移動処理（左クリックドラッグ）
	if (MouseL.pressed() == true)
	{
		const auto vPos = (camera.getTargetCenter() - Cursor::Delta());
		camera.jumpTo(vPos, camera.getTargetScale());
	}
	// 右クリック時のカーソル座標記録処理
	if (MouseR.pressed() == false)
	{
		if (MouseR.up() == false)
			cursPos = Cursor::Pos();
	}
	else if (MouseR.down() && IsBattleMove)
	{
		cursPos = Cursor::Pos();
	}
}
void Battle::handleUnitAndBuildingSelection()
{
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

							if (unit.IsSelect == true)
							{
								buiSyu = unit.buiSyu; // 建築の種類
								longBuildSelectTragetId = unit.ID; // 選択されたユニットのIDを保存
								IsResourceSelectTraget = true;
							}
							else
							{
								buiSyu = -1; // 建築の種類をリセット
								longBuildSelectTragetId = -1; // 選択されたユニットのIDをリセット
								IsResourceSelectTraget = false;
							}
						}
					}
				}
			}
		}
	}
}
void Battle::handleBuildTargetSelection()
{
	if (!IsBuildSelectTraget)
		return;

	if (const auto index = ToIndex(Cursor::PosF(), columnQuads, rowQuads))
	{
		if (ToTile(*index, N).leftClicked() && longBuildSelectTragetId != -1)
		{
			IsBuildSelectTraget = false;
			IsBuildMenuHome = false;

			Unit& cu = GetCU(longBuildSelectTragetId);
			longBuildSelectTragetId = -1;
			longIsMovedYoyakuId = cu.ID;

			rowBuildingTarget = index->y;
			colBuildingTarget = index->x;

			Vec2 nor = Cursor::PosF();
			cu.orderPosiLeft = nor.movedBy(-(cu.yokoUnit / 2), -(cu.TakasaUnit / 2));
			cu.orderPosiLeftLast = nor;
			cu.vecMove = (cu.orderPosiLeft - cu.nowPosiLeft).normalized();
			cu.FlagMove = false;
			cu.FlagMoveAI = true;
			cu.IsSelect = false;

			IsBattleMove = false;

			if (taskMyUnits.isValid())
			{
				abortMyUnits = true;
				taskMyUnits.wait();
			}
			abortMyUnits = false;
			isMovedYoyaku = true;

			taskMyUnits = Async(BattleMoveAStarMyUnits,
				std::ref(classBattle.listOfAllUnit),
				std::ref(classBattle.listOfAllEnemyUnit),
				std::ref(classBattle.classMapBattle.value().mapData),
				std::ref(aiRootMy),
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
Co::Task<void> Battle::co_handleResourcePointSelection()
{
	//資源ポイントの選択
	if (const auto index = ToIndex(cursPos, columnQuads, rowQuads))
	{
		// 敵がいない時は動かない仕様になっている
		if (ToTile(*index, N).leftClicked() && longBuildSelectTragetId != -1)//ダブルクリックが良いかも　画面ドラッグを考慮し
		{
			//[index->x]は試して駄目だったので[index->y]に
			//そもそも0番目で良いだろう　どこも横は同じサイズである
			int32 xxx = classBattle.classMapBattle.value().mapData.size();
			int32 yyy = classBattle.classMapBattle.value().mapData[0].size();
			int32 indexX = index->x;
			int32 indexY = index->y;
			if (index->x < 0 || index->y < 0 || index->x >= xxx || index->y >= yyy)
			{
				IsResourceSelectTraget = false;
				co_await Co::NextFrame(); // 範囲外アクセスを防ぐ
			}

			//TODO 閉じるボタン押下時にここでエラー発生　原因・修正はともかく把握しておくこと
			//ユニット選択後に閉じるボタン押下→問題無し
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
				Vec2 nor = ToTileBottomCenter(*index, N);
				// 移動先が有効かどうかチェックは実質上で済んでいる
				cu.vecMove = Vec2(cu.orderPosiLeft - cu.nowPosiLeft).normalized();
				cu.orderPosiLeft = nor.movedBy(-(cu.yokoUnit / 2), -(cu.TakasaUnit / 2));
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
									std::ref(aiRootMy),
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
		if (ToTile(*index, N).mouseOver())
		{
			int32 xxx = classBattle.classMapBattle.value().mapData.size();
			int32 yyy = classBattle.classMapBattle.value().mapData[0].size();
			int32 indexX = index->x;
			int32 indexY = index->y;
			if (index->x < 0 || index->y < 0 || index->x >= xxx || index->y >= yyy)
			{
				IsResourceSelectTraget = false;
				co_return;
			}
			if (classBattle.classMapBattle.value().mapData[index->x][index->y].isResourcePoint)
				Cursor::RequestStyle(CursorStyle::Hand);
		}
	}
}
Co::Task<void> Battle::processBattlePhase()
{
	co_await checkCancelSelectionByUIArea();

	{
		const auto t = camera.createTransformer();

		//// プレイヤー入力処理全般
		//handleUnitAndBuildingSelection();
		//handleCameraInput();
		//handleRightClickMovementOrSelection();
		//handleBuildTargetSelection();
		//handleResourcePointSelection();
	}

	updateUnitMovements();
	updateUnitHealthBars();
	handleSkillUISelection();
	handleBuildMenuSelectionA();

	co_return;
}
Co::Task<void> Battle::handleRightClickUnitActions(Point start, Point end)
{
	if (IsBattleMove == true)
	{
		if (arrayBattleZinkei[0] == true)
		{
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
			ClassHorizontalUnit liZenei;
			liZenei = getMovableUnits(classBattle.listOfAllUnit, BattleFormation::F);
			ClassHorizontalUnit liKouei;
			liKouei = getMovableUnits(classBattle.listOfAllUnit, BattleFormation::B);
			ClassHorizontalUnit liKihei;
			liKihei = getMovableUnits(classBattle.listOfAllUnit, BattleFormation::M);
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
				if (target.size() == 0) continue;

				AssignUnitsInFormation(target, start, end, i);
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
						std::ref(aiRootMy),
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
					IsBattleMove = true;
			}
		}
	}

	co_return;
}

Co::Task<void> Battle::mainLoop()
{
	const auto _tooltip = resourcePointTooltip.playScoped();

	while (true)
	{
		if (shouldExit == false)
			co_return;

		camera.update();
		resourcePointTooltip.setCamera(camera);

		spawnTimedEnemy();
		updateResourceIncome();
		refreshFogOfWar();

		////後でbattle内に移動(ポーズ処理を考慮
		updateBuildQueue();

		if (isMovedYoyaku == true)
		{
			handleBuildMenuSelectionB();
		}
		else if (isGetResource == true)
		{
			if (aiRootMy[longIsGetResourceId].getPath().size() > 0)
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

		updateResourceCapture();

		switch (battleStatus)
		{
		case BattleStatus::Battle:
		{
			co_await checkCancelSelectionByUIArea();

			//カメラ移動 || 部隊を選択状態にする。もしくは既に選択状態なら移動させる
			{
				const auto t = camera.createTransformer();

				handleUnitAndBuildingSelection();
				handleCameraInput();

				if (MouseR.up())
				{
					Point start = cursPos;
					Point end = Cursor::Pos();

					//部隊を選択状態にする。もしくは既に選択状態なら経路を算出する
					co_await handleRightClickUnitActions(start, end);
				}

				//建物の建築場所を選択する処理
				//ここに存在するのは、MouseL.pressedで建築メニュー押下時に続けて処理されてしまう為
				handleBuildTargetSelection();
				co_await co_handleResourcePointSelection();
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
								fontZinkei(ss.Zinkei[i]).draw(ttt, Palette::Black);
							}
						}

					}
				}
			}


			updateUnitMovements();
			updateUnitHealthBars();
			handleSkillUISelection();
			handleBuildMenuSelectionA();
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
		if (task.isReady()) {}
		if (taskMyUnits.isReady()) {}

		co_await Co::NextFrame();
	}
}

RectF Battle::getCameraView() const
{
	int32 testPadding = -TileOffset.x;
	return RectF{
		camera.getCenter() - (Scene::Size() / 2.0) / camera.getScale(),
		Scene::Size() / camera.getScale()
	}.stretched(-testPadding);
}
void Battle::drawTileMap(const RectF& cameraView) const
{
	for (int32 i = 0; i < (N * 2 - 1); ++i)
	{
		int32 xi = (i < (N - 1)) ? 0 : (i - (N - 1));
		int32 yi = (i < (N - 1)) ? i : (N - 1);

		for (int32 k = 0; k < (N - Abs(N - i - 1)); ++k)
		{
			Point index{ xi + k, yi - k };
			Vec2 pos = ToTileBottomCenter(index, N);
			if (!cameraView.intersects(pos))
				continue;

			const auto& tile = classBattle.classMapBattle.value().mapData[index.x][index.y];
			TextureAsset(tile.tip + U".png").draw(Arg::bottomCenter = pos);
		}
	}
}
void Battle::drawFog(const RectF& cameraView) const
{
	for (int32 i = 0; i < (N * 2 - 1); ++i)
	{
		int32 xi = (i < (N - 1)) ? 0 : (i - (N - 1));
		int32 yi = (i < (N - 1)) ? i : (N - 1);

		for (int32 k = 0; k < (N - Abs(N - i - 1)); ++k)
		{
			Point index{ xi + k, yi - k };
			switch (visibilityMap[index])
			{
			case Visibility::Unseen:
				ToTile(index, N).draw(ColorF{ 0.0, 0.6 });
				break;
			case Visibility::Visible:
				break;
			}
		}
	}
}
void Battle::drawBuildings(const RectF& cameraView) const
{
	Array<Unit> buildings;
	for (const auto& group : { classBattle.listOfAllUnit, classBattle.listOfAllEnemyUnit })
	{
		for (const auto& item : group)
		{
			if (item.FlagBuilding)
			{
				for (const auto& u : item.ListClassUnit)
					buildings.push_back(u);
			}
		}
	}

	for (const auto& u : buildings)
	{
		if (!u.IsBattleEnable)
			continue;
		Vec2 pos = ToTileBottomCenter(Point(u.colBuilding, u.rowBuilding), N);
		if (!cameraView.intersects(pos))
			continue;

		TextureAsset(u.Image).draw(Arg::bottomCenter = pos.movedBy(0, -TileThickness));
		if (u.IsSelect)
			RectF(Arg::bottomCenter = pos.movedBy(0, -TileThickness), TextureAsset(u.Image).size()).drawFrame(3.0, Palette::Red);
	}
}
void Battle::drawUnits(const RectF& cameraView) const
{
	auto drawGroup = [&](const Array<ClassHorizontalUnit>& group, const String& ringA, const String& ringB)
		{
			for (const auto& item : group)
			{
				if (item.FlagBuilding || item.ListClassUnit.empty())
					continue;

				for (const auto& u : item.ListClassUnit)
				{
					if (!u.IsBattleEnable) continue;

					const Vec2 center = u.GetNowPosiCenter();

					TextureAsset(ringA).drawAt(center.movedBy(0, 8));
					TextureAsset(u.Image).draw(Arg::center = center);
					if (u.IsSelect)
						TextureAsset(u.Image).draw(Arg::center = center).drawFrame(3.0, Palette::Red);
					TextureAsset(ringB).drawAt(center.movedBy(0, 16));
					u.bLiquidBarBattle.draw(ColorF{ 0.9, 0.1, 0.1 }, ColorF{ 0.7, 0.05, 0.05 }, ColorF{ 0.9, 0.5, 0.1 });
				}
			}
		};

	drawGroup(classBattle.listOfAllUnit, U"ringA.png", U"ringB.png");
	drawGroup(classBattle.listOfAllEnemyUnit, U"ringA_E.png", U"ringB_E.png");
}
void Battle::drawHealthBars() const
{
	auto drawBars = [](const Array<ClassHorizontalUnit>& group)
		{
			for (const auto& item : group)
			{
				if (item.FlagBuilding || item.ListClassUnit.empty())
					continue;

				for (const auto& u : item.ListClassUnit)
				{
					if (u.IsBattleEnable)
						u.bLiquidBarBattle.draw(ColorF{ 0.9, 0.1, 0.1 }, ColorF{ 0.7, 0.05, 0.05 }, ColorF{ 0.9, 0.5, 0.1 });
				}
			}
		};
	drawBars(classBattle.listOfAllUnit);
	drawBars(classBattle.listOfAllEnemyUnit);
}
void Battle::drawSelectionRectangleOrArrow() const
{
	if (!MouseR.pressed())
		return;

	if (!IsBattleMove)
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
void Battle::drawSkillUI() const
{
	const int32 baseY = Scene::Size().y - renderTextureSkill.height() - underBarHeight;

	renderTextureSkill.draw(0, baseY);
	renderTextureSkillUP.draw(0, baseY);

	if (!nowSelectSkillSetumei.isEmpty())
	{
		rectSkillSetumei.draw(Palette::Black);
		fontSkill(nowSelectSkillSetumei).draw(rectSkillSetumei.stretched(-12), Palette::White);
	}
}
void Battle::drawBuildMenu() const
{
	if (!IsBuildMenuHome)
	{
		renderTextureBuildMenuEmpty.draw(Scene::Size().x - 328, Scene::Size().y - 328 - 30);
		return;
	}

	const int32 baseX = Scene::Size().x - 328;
	const int32 baseY = Scene::Size().y - 328 - underBarHeight;
	Rect(baseX - 64 - 6, baseY, 70, 328).drawFrame(4, 0, Palette::Black);
	auto drawBuildList = [&](const Array<cRightMenu>& items, double currentTime)
		{
			for (auto&& [i, item] : Indexed(items))
			{
				if (i == 0)
				{
					item.texture.resized(64).draw(baseX - 64, baseY + 4);
					if (!item.isMoved)
					{
						double progressRatio = Saturate(currentTime / item.time); // 0.0 ～ 1.0 に制限
						double gaugeRatio = Max(progressRatio, 0.1);
						double gaugeHeight = 64 * gaugeRatio;

						RectF{ baseX - 64, baseY + 4, 64, gaugeHeight }
						.draw(ColorF{ 0.0, 0.5 });
					}
				}
				else
				{
					item.texture.resized(32).draw(baseX - 32, baseY + 32 + (i * 32) + 4);
				}
			}
		};

	int index = buiSyu;
	if (!InRange(index, 0, (int32)buildMenus.size()))
		return;
	auto& menu = buildMenus[index];
	switch (buiSyu)
	{
	case 0:
		renderTextureBuildMenuHome.draw(baseX, baseY);
		drawBuildList(arrBuildMenuHomeYoyaku, menu.timer.sF());
		break;
	case 1:
		renderTextureBuildMenuThunderwalker.draw(baseX, baseY);
		drawBuildList(arrBuildMenuThunderwalkerYoyaku, menu.timer.sF());
		break;
	case 2:
		renderTextureBuildMenuKouhei.draw(baseX, baseY);
		drawBuildList(menu.reservations, menu.timer.sF());
		break;
	case 3:
		renderTextureBuildMenuKeisouHoheiT.draw(baseX, baseY);
		drawBuildList(arrBuildMenuKeisouYoyaku, menu.timer.sF());
		break;
	default:
		break;
	}
}
void Battle::drawResourcesUI() const
{
	const String goldText = U"Gold:{0}"_fmt(gold);
	const String trustText = U"Trust:{0}"_fmt(trust);
	const String foodText = U"Food:{0}"_fmt(food);

	int32 baseX = 0;
	int32 baseY = 0;

	if (longBuildSelectTragetId != -1)
	{
		// ビルド選択中 → 右下寄せに変更
		baseX = Scene::Size().x - 328 - int32(systemFont(goldText).region().w) - 64 - 6;
		baseY = Scene::Size().y - 328 - 30;
	}

	const Array<String> texts = { goldText, trustText, foodText };

	for (size_t i = 0; i < texts.size(); ++i)
	{
		const String& text = texts[i];
		const auto region = systemFont(text).region();
		Rect rect{ baseX, baseY + static_cast<int32>(i * region.h), static_cast<int32>(region.w), static_cast<int32>(region.h) };

		rect.draw(Palette::Black);
		systemFont(text).drawAt(rect.center(), Palette::White);
	}
}
void Battle::drawBuildTargetHighlight() const
{
	if (IsBuildSelectTraget)
	{
		if (const auto index = ToIndex(Cursor::PosF(), columnQuads, rowQuads))
		{
			// マウスカーソルがあるタイルを強調表示する
			ToTile(*index, N).draw(ColorF{ 1.0, 0.2 });
		}
	}
}
void Battle::drawBuildDescription() const
{
	if (nowSelectBuildSetumei != U"")
	{
		rectSetumei.draw(Palette::Black);
		fontSkill(nowSelectBuildSetumei).draw(rectSetumei.stretched(-12), Palette::White);
	}
}
void Battle::drawResourcePoints(const RectF& cameraView) const
{
	if (!classBattle.classMapBattle)
		return;

	const auto& mapData = classBattle.classMapBattle.value().mapData;

	const int32 mapSize = static_cast<int32>(mapData.size());

	for (int32 x = 0; x < mapSize; ++x)
	{
		for (int32 y = 0; y < static_cast<int32>(mapData[x].size()); ++y)
		{
			const auto& tile = mapData[x][y];

			if (!tile.isResourcePoint)
				continue;

			const Vec2 pos = ToTileBottomCenter(Point(x, y), N);

			if (!cameraView.intersects(pos))
				continue;

			// アイコンの描画
			TextureAsset(tile.resourcePointIcon).draw(Arg::bottomCenter = pos.movedBy(0, -TileThickness));

			// 所有者に応じて円枠の色を変える
			const ColorF circleColor = (tile.whichIsThePlayer == BattleWhichIsThePlayer::Sortie)
				? ColorF{ 0.0, 0.6 }
			: ColorF(Palette::Red);

			Circle(pos.movedBy(0, -TileThickness - TileOffset.y), 16).drawFrame(4, 0, circleColor);
		}
	}
}
void Battle::draw() const
{
	FsScene::draw();

	{
		// 2D カメラによる座標変換を適用する
		const auto tr = camera.createTransformer();
		// 乗算済みアルファ用のブレンドステートを適用する
		const ScopedRenderStates2D blend{ BlendState::Premultiplied };
		const RectF cameraView = getCameraView();

		drawTileMap(cameraView);
		drawFog(cameraView);
		drawBuildings(cameraView);
		drawUnits(cameraView);
		drawResourcePoints(cameraView);
		resourcePointTooltip.draw();
		drawSelectionRectangleOrArrow();
		drawBuildTargetHighlight();
	}

	renderTextureZinkei.draw(0, Scene::Size().y - renderTextureSkill.height() - renderTextureZinkei.height() - underBarHeight);
	drawSkillUI();
	drawBuildDescription();
	drawBuildMenu();
	drawResourcesUI();

	if (longBuildSelectTragetId == -1)
		DrawMiniMap(grid, camera.getRegion());
}

//ヘルパーメソッド

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
Vec2 Battle::ToTileBottomCenterTTT(const Point& index, const int32 N) const
{
	// index は (x, y) グリッド上のタイル位置
	const double posX = (index.x - index.y) * TileOffset.x;
	const double posY = (index.x + index.y) * TileOffset.y;

	return Vec2(posX, posY);
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
ClassHorizontalUnit Battle::getMovableUnits(Array<ClassHorizontalUnit>& source, BattleFormation bf)
{
	ClassHorizontalUnit result;

	for (auto& target : source)
		for (auto& unit : target.ListClassUnit)
		{
			if (unit.Formation == bf && unit.FlagMove == true && unit.IsBattleEnable == true)
				result.ListClassUnit.push_back(unit);
		}

	return result;
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

		Unit& cu = GetCU(unit->ID);
		cu.orderPosiLeft = Vec2(Floor(x), Floor(y)).movedBy(-(cu.yokoUnit / 2), -(cu.TakasaUnit / 2));
		cu.orderPosiLeftLast = cu.orderPosiLeft;
		cu.FlagMove = false;
		cu.FlagMoveAI = true;
	}
}
