﻿# include <Siv3D.hpp> // Siv3D v0.6.14
# include "000_SystemString.h"
# include "001_GLOBAL.h"
# include "005_GameUIToolkit.h"
# include "100_StructEffect.h"
# include "101_StructHomography.h"
# include "102_StructGameData.h"
# include "150_EnumClassLanguageSteamFullSuport.h"
# include "151_EnumSelectCharStatus.h"
# include "158_EnumFormBuyDisplayStatus.h"
# include "159_EnumBattleStatus.h"
# include "200_ClassGaussianClass.h"
# include "201_ClassPauseWindow.h"
# include "205_ClassScenario.h" 
# include "210_ClassStaticCommonMethod.h" 
# include "270_ClassMapCreator.h" 
# include "275_ClassAStar.h" 
# include "280_ClassExecuteSkills.h" 

using App = SceneManager<String, GameData>;
std::unique_ptr<GaussianClass> m_gaussianClass;
PauseWindow m_pauseWindow = PauseWindow();
LanguageSteamFullSuport language;
SystemString systemString;
MapCreator mapCreator;
Array<Quad> columnQuads;
Array<Quad> rowQuads;
auto randomFade()
{
	Array<std::function<std::unique_ptr<IFade>()>> makeFadeFuncs = {
		[]() -> std::unique_ptr<IFade> { return std::make_unique<Fade4>(); },
	};

	return Sample(makeFadeFuncs)();
}
void DrawUnder000()
{
	//枠
	Rect{ Scene::Size().x - 1, 0, 1, Scene::Size().y }.draw(Palette::Yellow);
	Rect{ 0, 0, Scene::Size().x , 1 }.draw(Palette::Yellow);
	Rect{ 0, 0, 1, Scene::Size().y }.draw(Palette::Yellow);
	//下のエリア
	Rect{ 0, Scene::Size().y - 30, Scene::Size().x, 30 }.draw(Palette::Yellow);
}
void DrawUnder001()
{
	//exit
	Shape2D::Cross(10, 5, Vec2{ Scene::Size().x - 10, Scene::Size().y - 10 }).draw(Palette::Black);
}
// 描画された最大のアルファ成分を保持するブレンドステートを作成する
BlendState MakeBlendState()
{
	BlendState blendState = BlendState::Default2D;
	blendState.srcAlpha = Blend::SrcAlpha;
	blendState.dstAlpha = Blend::DestAlpha;
	blendState.opAlpha = BlendOp::Max;
	return blendState;
}
void SetWindSize(int32 w, int32 h)
{
	// ゲームのシーンサイズ
	const Size sceneSize{ w, h };
	// 必要なシーンサイズよりやや大きめの領域（タイトルバーやフレームを考慮）
	const Size requiredAreadSize{ sceneSize + Size{ 60, 10 } };
	// プレイヤーのワークエリア（画面サイズからタスクバーを除いた領域）のサイズ
	const Size workAreaSize = System::GetCurrentMonitor().workArea.size;
	// OS の UI のスケール（多くの場合 1.0～2.0）
	const double uiScaling = Window::GetState().scaling;
	// UI スケールを考慮したワークエリアサイズ
	const Size availableWorkAreaSize = (SizeF{ workAreaSize } / uiScaling).asPoint();
	// ゲームのシーンサイズがプレイヤーのワークエリア内に収まるか
	const bool ok = (requiredAreadSize.x <= availableWorkAreaSize.x) && (requiredAreadSize.y <= availableWorkAreaSize.y);

	if (ok)
	{
		Window::Resize(sceneSize);
	}
	else
	{
		// UI 倍率 1.0 相当でリサイズ
		Scene::SetResizeMode(ResizeMode::Keep);
		Scene::Resize(sceneSize);
		Window::ResizeActual(sceneSize);
	}
}
// オリジナルのシーンを何倍すればよいかを返す関数
double CalculateScale(const Vec2& baseSize, const Vec2& currentSize)
{
	return Min((currentSize.x / baseSize.x), (currentSize.y / baseSize.y));
}
// 画面の中央に配置するためのオフセットを返す関数
Vec2 CalculateOffset(const Vec2& baseSize, const Vec2& currentSize)
{
	return ((currentSize - baseSize * CalculateScale(baseSize, currentSize)) / 2.0);
}
void DragProcess(Optional<std::pair<Point, Point>>& dragStart)
{
	// ドラッグ処理
	if (Rect{ 0, 0, WINDOWSIZEWIDTH000 / 2, 60 }.mouseOver() == true)
	{
		Cursor::RequestStyle(U"MyCursorHand");
	}
	if (dragStart)
	{
		if (not MouseL.pressed())
		{
			dragStart.reset();
		}
		else
		{
			Window::SetPos(dragStart->second + (Cursor::ScreenPos() - dragStart->first));
		}
	}

	// ドラッグの開始
	if (Rect{ 0, 0, WINDOWSIZEWIDTH000 / 2, 60 }.leftClicked())
	{
		dragStart = { Cursor::ScreenPos(), Window::GetState().bounds.pos };
	}
}
void WriteIni(INI ini)
{
	if (not ini) // もし読み込みに失敗したら
	{
		ini.write(U"data", U"winSize", 1600);
		ini.write(U"data", U"winSizeCheck", false);
		ini.write(U"data", U"FluExit", true);
		ini.save(U"data.ini");
	}
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
int32 BattleMoveAStar(Array<ClassHorizontalUnit>& target,
						Array<ClassHorizontalUnit>& enemy,
						MapCreator mapCreator,
						Array<Array<MapDetail>> mapData,
						ClassGameStatus& classGameStatus,
						Array<Array<Point>>& debugRoot,
						Array<ClassAStar*>& list,
						const std::atomic<bool>& abort,
						const std::atomic<bool>& pause
)
{
	while (true)
	{
		if (abort == true)
		{
			break;
		}
		if (pause == true)
		{
			continue;
		}
		////アスターアルゴリズムで移動経路取得
		for (auto& aaa : target)
		{
			if (abort == true)
			{
				break;
			}

			if (aaa.FlagBuilding == true)
			{
				continue;
			}
			for (auto& bbb : aaa.ListClassUnit)
			{
				if (abort == true)
				{
					break;
				}

				if (bbb.IsBuilding == true && bbb.mapTipObjectType == MapTipObjectType::WALL2)
				{
					continue;
				}
				if (bbb.IsBattleEnable == false)
				{
					continue;
				}

				Array<Point> listRoot;

				//まず現在のマップチップを取得
				s3d::Optional<Size> nowIndex = mapCreator.ToIndex(bbb.GetNowPosiCenter(), columnQuads, rowQuads);
				if (nowIndex.has_value() == false)
				{
					continue;
				}

				//最寄りの敵の座標を取得
				Vec2 xy1 = bbb.GetNowPosiCenter();
				xy1.x = xy1.x * xy1.x;
				xy1.y = xy1.y * xy1.y;
				double disA = xy1.x + xy1.y;
				HashTable<double, ClassUnit> dicDis;
				try
				{
					for (auto& ccc : enemy)
					{
						for (auto& ddd : ccc.ListClassUnit)
						{
							if (ddd.IsBuilding == true && ddd.mapTipObjectType == MapTipObjectType::WALL2)
							{
								continue;
							}
							if (ddd.IsBattleEnable == false)
							{
								continue;
							}
							Vec2 xy2 = ddd.GetNowPosiCenter();
							xy2.x = xy2.x * xy2.x;
							xy2.y = xy2.y * xy2.y;
							double disB = xy2.x + xy2.y;
							//hashが被ることあり
							dicDis.emplace(disA - disB, ddd);
						}
					}
				}
				catch (const std::exception&)
				{
					throw;
				}

				if (dicDis.size() == 0)
				{
					continue;
				}

				auto minElement = dicDis.begin();
				for (auto it = dicDis.begin(); it != dicDis.end(); ++it) {
					if (it->first < minElement->first) {
						minElement = it;
					}
				}

				bool flagGetEscapeRange = false;
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
						minElement->second.nowPosiLeft = Vec2(xC, yC);
						flagGetEscapeRange = true;
					}
				}

				if (bbb.FlagMoving == true && flagGetEscapeRange == false)
				{
					continue;
				}
				if (bbb.FlagMovingEnd == false && flagGetEscapeRange == false)
				{
					continue;
				}

				//最寄りの敵のマップチップを取得
				s3d::Optional<Size> nowIndexEnemy = mapCreator.ToIndex(minElement->second.GetNowPosiCenter(), columnQuads, rowQuads);
				if (nowIndexEnemy.has_value() == false)
				{
					continue;
				}

				if (nowIndexEnemy.value() == nowIndex.value())
				{
					continue;
				}

				////現在地を開く
				ClassAStarManager classAStarManager(nowIndexEnemy.value().x, nowIndexEnemy.value().y);
				Optional<ClassAStar*> startAstar = classAStarManager.OpenOne(nowIndex.value().x, nowIndex.value().y, 0, nullptr, mapCreator.N);
				MicrosecClock mc;
				////移動経路取得
				while (true)
				{
					try
					{
						if (abort == true)
						{
							break;
						}

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
														mapCreator.N
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

				if (listRoot.size() != 0)
				{
					classGameStatus.aiRoot[bbb.ID] = listRoot;
					//debugRoot.push_back(listRoot);
				}
			}
		}
	}

	return -1;
}
int32 BattleMoveAStarMyUnits(Array<ClassHorizontalUnit>& target,
						Array<ClassHorizontalUnit>& enemy,
						MapCreator mapCreator,
						Array<Array<MapDetail>> mapData,
						ClassGameStatus& classGameStatus,
						Array<Array<Point>>& debugRoot,
						Array<ClassAStar*>& list,
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
			s3d::Optional<Size> nowIndexEnemy = mapCreator.ToIndex(listClassUnit.orderPosiLeft, columnQuads, rowQuads);
			if (nowIndexEnemy.has_value() == false)
				continue;

			////現在地を開く
			ClassAStarManager classAStarManager(nowIndexEnemy.value().x, nowIndexEnemy.value().y);

			//現在のマップチップを取得
			s3d::Optional<Size> nowIndex = mapCreator.ToIndex(listClassUnit.GetNowPosiCenter(), columnQuads, rowQuads);
			if (nowIndex.has_value() == false)
				continue;

			Optional<ClassAStar*> startAstar = classAStarManager.OpenOne(nowIndex.value().x, nowIndex.value().y, 0, nullptr, mapCreator.N);
			MicrosecClock mc;
			////移動経路取得
			while (true)
			{
				try
				{
					if (abort == true)
					{
						break;
					}

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
													mapCreator.N
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

			if (listRoot.size() != 0)
			{
				classGameStatus.aiRoot[listClassUnit.ID] = listRoot;
				debugRoot.push_back(listRoot);
				listClassUnit.FlagMoveAI = false;
			}
		}
	}
	return -1;
}

class ClassSelectLang {
public:
	String lang = U"";
	RectF btnRectF = RectF{};
	bool isDisplayBtnRectF = true;
	int32 SortKey = 0;
};
template <class T>
struct TexturedCollider
{
	T Collider;
};

// タイトルシーン
/// @brief 言語選択シーン
class SelectLang : public App::Scene
{
public:
	// コンストラクタ（必ず実装）
	SelectLang(const InitData& init)
		: IScene{ init }
	{
		int32 counterAll = 0;
		int32 counter = -1;
		int32 counterCol = 0;
		for (auto ttt : LANDIS)
		{
			ClassSelectLang csl;
			csl.SortKey = counterAll;
			csl.isDisplayBtnRectF = true;
			csl.lang = ttt.second;
			RectF rectText = getData().fontNormal(ttt.second).region();
			if (counterCol % 3 == 0)
			{
				counterCol = 0;
				counter++;
			}
			else
			{
			}

			csl.btnRectF = RectF{ 100 / 2,(300) + (counterAll * (rectText.h + 40)) ,WINDOWSIZEWIDTH000 / 2 - 100,rectText.h + 40 };
			//csl.btnRectF = RectF{ 100 / 2,(300) + (counterAll * (rectText.h + 20)) ,WINDOWSIZEWIDTH000 / 2 - 100,rectText.h + 20 };

			acsl.push_back(csl);
			counterCol++;
			counterAll++;
		}

		vbar001.emplace(SasaGUI::Orientation::Vertical);

		//仮置き
		language = LanguageSteamFullSuport::English;


		EXITBTNPOLYGON = Shape2D::Cross(10, 5, Vec2{ INIT_WINDOW_SIZE_WIDTH - 10, INIT_WINDOW_SIZE_HEIGHT - 10 }).asPolygon();
		EXITBTNRECT = Rect{ Arg::center(EXITBTNPOLYGON.centroid().asPoint()),20,20 };
	}
	// 更新関数（オプション）
	void update() override
	{
		for (auto& ttt : acsl)
		{
			if (ttt.btnRectF.leftClicked())
			{
				AudioAsset(U"click").setLoop(false);
				AudioAsset(U"click").play();

				LangFunc(ttt.lang);
				TextureSet();

				Optional<INI> ini{ std::in_place, U"/data.ini" };
				INI ini2 = INI(U"data.ini");
				WriteIni(ini2);
				INI ini3 = INI(U"data.ini");
				ini.emplace(ini3);

				bool temp = Parse<bool>(ini.value()[U"data.winSizeCheck"]);
				if (temp)
				{
					changeScene(U"TitleScene");
				}
				else
				{
					changeScene(U"WinSizeScene");
				}

			}
		}
	}

	void LangFunc(String lang)
	{
		const JSON jsonLang = JSON::Load(PathLang + U"/SystemString.json");

		if (not jsonLang)throw Error{ U"Failed to load `SystemString.json`" };

		for (const auto& [key, value] : jsonLang[U"lang"]) {

			if (
				lang == U"日本語" && (value[U"lang"].getString() == U"Japan")
				)
			{
				SystemString ss;
				ss.TopMenuTitle = value[U"TopMenuTitle"].getString();
				ss.AppTitle = value[U"AppTitle"].getString();
				ss.configSave = value[U"configSave"].get<String>();
				ss.configLoad = value[U"configLoad"].get<String>();
				ss.selectScenario = value[U"selectScenario"].get<String>();
				ss.selectScenario2 = value[U"selectScenario2"].get<String>();
				ss.selectChara1 = value[U"selectChara1"].get<String>();
				ss.selectCard = value[U"selectCard"].get<String>();
				ss.DoYouWantToQuitTheGame = value[U"DoYouWantToQuitTheGame"].get<String>();
				ss.strategyMenu.push_back(value[U"strategyMenu000"].get<String>());
				ss.strategyMenu.push_back(value[U"strategyMenu001"].get<String>());
				ss.strategyMenu.push_back(value[U"strategyMenu002"].get<String>());
				ss.strategyMenu.push_back(value[U"strategyMenu003"].get<String>());
				ss.strategyMenu.push_back(value[U"strategyMenu004"].get<String>());
				ss.strategyMenu.push_back(value[U"strategyMenu005"].get<String>());
				ss.strategyMenu.push_back(value[U"strategyMenu006"].get<String>());
				ss.strategyMenu.push_back(value[U"strategyMenu007"].get<String>());
				ss.strategyMenu.push_back(value[U"strategyMenu008"].get<String>());
				ss.strategyMenu.push_back(value[U"strategyMenu009"].get<String>());
				//ss.strategyMenu000 = value[U"strategyMenu000"].get<String>();
				//ss.strategyMenu001 = value[U"strategyMenu001"].get<String>();
				//ss.strategyMenu002 = value[U"strategyMenu002"].get<String>();
				//ss.strategyMenu003 = value[U"strategyMenu003"].get<String>();
				//ss.strategyMenu004 = value[U"strategyMenu004"].get<String>();
				//ss.strategyMenu005 = value[U"strategyMenu005"].get<String>();
				//ss.strategyMenu006 = value[U"strategyMenu006"].get<String>();
				//ss.strategyMenu007 = value[U"strategyMenu007"].get<String>();
				//ss.strategyMenu008 = value[U"strategyMenu008"].get<String>();
				//ss.strategyMenu009 = value[U"strategyMenu009"].get<String>();
				ss.BattleMessage001 = value[U"BattleMessage001"].get<String>();
				ss.BuyMessage001 = value[U"BuyMessage001"].get<String>();
				ss.SelectCharMessage001 = value[U"SelectCharMessage001"].get<String>();
				ss.StorySkip = value[U"StorySkip"].get<String>();
				ss.StatusName = value[U"StatusName"].get<String>();
				ss.StatusRace = value[U"StatusRace"].get<String>();
				ss.StatusPrice = value[U"StatusPrice"].get<String>();
				ss.StatusHp = value[U"StatusHp"].get<String>();
				ss.StatusMp = value[U"StatusMp"].get<String>();
				ss.StatusAttack = value[U"StatusAttack"].get<String>();
				ss.StatusDefense = value[U"StatusDefense"].get<String>();
				ss.StatusMagic = value[U"StatusMagic"].get<String>();
				ss.StatusMagDef = value[U"StatusMagDef"].get<String>();
				ss.StatusSpeed = value[U"StatusSpeed"].get<String>();
				ss.StatusMove = value[U"StatusMove"].get<String>();
				ss.StatusSkill = value[U"StatusSkill"].get<String>();
				ss.StatusSetumei = value[U"StatusSetumei"].get<String>();
				ss.SkillAttack = value[U"SkillAttack"].get<String>();
				systemString = ss;
			}
		}
	}

	void TextureSet()
	{
	}

	// 描画関数（オプション） 
	void draw() const override
	{
		for (auto& ttt : acsl)
		{
			getData().slice9Cy.draw(ttt.btnRectF.asRect());
			getData().fontLine(ttt.lang).draw(Arg::center = ttt.btnRectF.center(), Palette::Antiquewhite);
		}
	}

	void drawFadeIn(double t) const override
	{
		draw();

		m_fadeInFunction->fade(1 - t);
	}

	void drawFadeOut(double t) const override
	{
		draw();

		m_fadeOutFunction->fade(t);
	}
private:
	Optional<SasaGUI::ScrollBar> vbar001;
	Array<ClassSelectLang> acsl;
	std::unique_ptr<IFade> m_fadeInFunction = randomFade();
	std::unique_ptr<IFade> m_fadeOutFunction = randomFade();
};
class WinSizeScene : public App::Scene {
public:
	WinSizeScene(const InitData& init) : IScene(init) {
		Size tempSize = { 600,300 };
		Window::Resize(tempSize);

		// シーンの拡大倍率を計算する
		const Size BaseSceneSize{ WINDOWSIZEWIDTH000, WINDOWSIZEHEIGHT000 };
		SCALE = CalculateScale(BaseSceneSize, tempSize);
		OFFSET = CalculateOffset(BaseSceneSize, tempSize);

		EXITBTNPOLYGON = Shape2D::Cross(10, 5, Vec2{ tempSize.x - 10, tempSize.y - 10 }).asPolygon();
		EXITBTNRECT = Rect{ Arg::center(EXITBTNPOLYGON.centroid().asPoint()),20,20 };

		INI aa = INI(U"data.ini");
		WriteIni(aa);
		ini.emplace(aa);  // .emplace() で再代入

		m_gaussianClass->SetSize(Scene::Size());

		System::Update();
	}
	void update() override {
		//if (m_000Button.leftClicked() || m_001Button.leftClicked()) {
		if (m_000Button.leftClicked()) {
			if (m_000Button.leftClicked())
			{
				ini->write(U"data", U"winSize", 1600);
			}
			else
			{
				ini->write(U"data", U"winSize", 1200);
			}
			ini->write(U"data", U"winSizeCheck", checked0);
			ini->save(U"data.ini");

			changeScene(U"TitleScene");
		}
		SimpleGUI::CheckBox(checked0, U"もう表示しない", m_001Button.movedBy(0, 60).pos);
	}

	void draw() const override {
		getData().fontNormal(U"Window Size").draw(12, 10, Palette::White);
		m_000Button.draw(Palette::Skyblue).drawFrame(2, Palette::Black);
		//m_001Button.draw(Palette::Skyblue).drawFrame(2, Palette::Black);

		//仮置き
		getData().fontMini(U"1600*900").drawAt(m_000Button.center(), Palette::Black);
		//getData().fontMini(U"1200*600").drawAt(m_001Button.center(), Palette::Black);
	}
private:
	Optional<INI> ini{ std::in_place, U"/data.ini" };
	Rect m_000Button = Rect(12, 100, 200, 60);
	Rect m_001Button = Rect(12, 160 + 10, 200, 60);
	bool checked0 = false;
};
class TitleScene : public App::Scene {
public:
	TitleScene(const InitData& init) : IScene(init) {
		INI ini = INI(U"data.ini");
		WriteIni(ini);
		int32 tempWinSize = Parse<int32>(ini[U"data.winSize"]);
		Size ss;
		if (tempWinSize == 1600)
		{
			ss = { WINDOWSIZEWIDTH000, WINDOWSIZEHEIGHT000 };
			SetWindSize(WINDOWSIZEWIDTH000, WINDOWSIZEHEIGHT000);
		}
		else
		{
			ss = { WINDOWSIZEWIDTH001, WINDOWSIZEHEIGHT001 };
			SetWindSize(WINDOWSIZEWIDTH001, WINDOWSIZEHEIGHT001);
		}

		EXITBTNPOLYGON = Shape2D::Cross(10, 5, Vec2{ WINDOWSIZEWIDTH000 - 10, Scene::Size().y - 10 }).asPolygon();
		EXITBTNRECT = Rect{ Arg::center(EXITBTNPOLYGON.centroid().asPoint()),20,20 };

		// シーンの拡大倍率を計算する
		const Size BaseSceneSize{ WINDOWSIZEWIDTH000, WINDOWSIZEHEIGHT000 };
		SCALE = CalculateScale(BaseSceneSize, ss);
		OFFSET = CalculateOffset(BaseSceneSize, ss);
		m_gaussianClass->SetSize(Scene::Size());

		System::Update();

		// TOML ファイルからデータを読み込む
		const TOMLReader tomlConfig{ PATHBASE + PATH_DEFAULT_GAME + U"/config.toml" };

		if (not tomlConfig) // もし読み込みに失敗したら
		{
			throw Error{ U"Failed to load `config.toml`" };
		}

		TitleMenuX = tomlConfig[U"config.TitleMenuX"].get<int32>();
		TitleMenuY = tomlConfig[U"config.TitleMenuY"].get<int32>();
		space = tomlConfig[U"config.TitleMenuSpace"].get<int32>();

		for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/000_SystemImage/000_TitleMenuImage/"))
		{
			String filename = FileSystem::FileName(filePath);
			TextureAsset::Register(filename, filePath);
		}
		easyRect = Rect(TitleMenuX, TitleMenuY, 160, 40);
		normalRect = Rect(TitleMenuX, (TitleMenuY + (40 * 1)) + space * 1, 160, 40);
		hardRect = Rect(TitleMenuX, (TitleMenuY + (40 * 2)) + space * 2, 160, 40);
		lunaRect = Rect(TitleMenuX, (TitleMenuY + (40 * 3)) + space * 3, 160, 40);

		for (auto&& e : getData().classGameStatus.arrayClassBGM | std::views::filter([&](auto&& e) { return e.op == true; }))
		{
			getData().audio = AudioAsset(e.tag);
			getData().audio.setLoop(true);
			getData().audio.play();
		}
	}
	void update() override {
		if (easyRect.leftClicked() == true)
		{
			changeScene(U"ScenarioMenu", 0.9s);
		}
		if (normalRect.leftClicked() == true)
		{
			changeScene(U"ScenarioMenu", 0.9s);
		}
		if (hardRect.leftClicked() == true)
		{
			changeScene(U"ScenarioMenu", 0.9s);
		}
		if (lunaRect.leftClicked() == true)
		{
			changeScene(U"ScenarioMenu", 0.9s);
		}
	}

	void draw() const override {
		TextureAsset(U"0001_easy.png").draw(TitleMenuX, TitleMenuY);
		TextureAsset(U"0002_normal.png").draw(TitleMenuX, (TitleMenuY + (40 * 1)) + space * 1);
		TextureAsset(U"0003_hard.png").draw(TitleMenuX, (TitleMenuY + (40 * 2)) + space * 2);
		TextureAsset(U"0004_luna.png").draw(TitleMenuX, (TitleMenuY + (40 * 3)) + space * 3);
		easyRect.draw(ColorF{ 0, 0, 0, 0 });
		normalRect.draw(ColorF{ 0, 0, 0, 0 });
		hardRect.draw(ColorF{ 0, 0, 0, 0 });
		lunaRect.draw(ColorF{ 0, 0, 0, 0 });
	}
private:
	int32 TitleMenuX = 0;
	int32 TitleMenuY = 0;
	int32 space = 30;
	Rect easyRect;
	Rect normalRect;
	Rect hardRect;
	Rect lunaRect;
};
class ScenarioMenu : public App::Scene
{
public:
	// コンストラクタ（必ず実装）
	ScenarioMenu(const InitData& init)
		: IScene{ init }
	{
		for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/070_Scenario/InfoScenario/"))
		{
			String filename = FileSystem::FileName(filePath);
			const JSON jsonScenario = JSON::Load(filePath);
			if (not jsonScenario) // もし読み込みに失敗したら
			{
				continue;
			}

			for (const auto& [key, value] : jsonScenario[U"Scenario"]) {
				ClassScenario cs;
				cs.ButtonType = value[U"ButtonType"].getString();
				cs.ScenarioName = value[U"ScenarioName"].getString();
				cs.SortKey = Parse<int32>(value[U"Sortkey"].getString());
				if (value.hasElement(U"power") == true)
				{
					String sPower = value[U"power"].getString();
					if (sPower.contains(',') == true)
					{
						cs.ArrayPower = sPower.split(',');
					}
					else
					{
						cs.ArrayPower.push_back(sPower);
					}
				}
				if (value.hasElement(U"SelectCharaFrameImageLeft") == true)
				{
					cs.SelectCharaFrameImageLeft = value[U"SelectCharaFrameImageLeft"].getString();
				}
				if (value.hasElement(U"SelectCharaFrameImageRight") == true)
				{
					cs.SelectCharaFrameImageRight = value[U"SelectCharaFrameImageRight"].getString();
				}
				if (value.hasElement(U"HelpString") == true)
				{
					cs.HelpString = value[U"HelpString"].getString();
				}
				if (value.hasElement(U"Mail") == true)
				{
					cs.Mail = value[U"Mail"].getString();
				}
				if (value.hasElement(U"Internet") == true)
				{
					cs.Internet = value[U"Internet"].getString();
				}
				cs.Text = ClassStaticCommonMethod::MoldingScenarioText(value[U"Text"].getString());
				cs.btnRectF = {};
				arrayClassScenario.push_back(std::move(cs));
			}
		}

		for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/000_SystemImage/005_ScenarioBackImage/"))
		{
			String filename = FileSystem::FileName(filePath);
			if (filename = U"pre0.jpg")
			{
				TextureAsset::Register(filename, filePath);
			}
		}

		vbar001.emplace(SasaGUI::Orientation::Vertical);
		vbar002.emplace(SasaGUI::Orientation::Vertical);

		int32 Scenario1YStart = 64;
		Scenario2X = WINDOWSIZEWIDTH000 / 2;
		Scenario2Y = WINDOWSIZEHEIGHT000 / 2;
		ScenarioTextX = WINDOWSIZEWIDTH000 / 2;
		ScenarioTextY = 80;
		auto max_it = std::max_element(arrayClassScenario.begin(), arrayClassScenario.end(),
									   [](const ClassScenario& a, const ClassScenario& b) {
										   return a.ScenarioName.size() < b.ScenarioName.size();
									   });
		if (max_it != arrayClassScenario.end())
		{
			ClassScenario& scenario_with_longest_name = *max_it;
			RectF re = getData().fontMini(scenario_with_longest_name.ScenarioName).region();
			re.w = re.w + (32 * 2);
			re.h = WINDOWSIZEHEIGHT000 / 2 - Scenario1YStart;
			re.x = 64;
			re.y = Scenario1YStart;
			scenario_with_longest_name_x = (int32)re.x;
			scenario_with_longest_name_y = (int32)re.y;
			scenario_with_longest_name_w = (int32)re.w;
			scenario_with_longest_name_h = (int32)re.h;

			rectFScenario1X = re.asRect();
			rectFScenario2X = { (int32)re.x ,(int32)re.y + (int32)re.h + 32,(int32)re.w ,(int32)re.h - 32 };

			vbar001.value().updateLayout({
				(int32)(re.x + re.w + SasaGUI::ScrollBar::Thickness), (int32)re.y,
				SasaGUI::ScrollBar::Thickness,
				(int32)re.h
			});
			vbar001.value().updateConstraints(0.0, 2000.0, Scene::Height());

			vbar002.value().updateLayout({
				(int32)(re.x + re.w + SasaGUI::ScrollBar::Thickness), (int32)Scenario2Y + 32,
				SasaGUI::ScrollBar::Thickness,
				(int32)re.h
			});
			vbar002.value().updateConstraints(0.0, 2000.0, Scene::Height());

			rectFScenarioTextX = Rect{ (int32)(re.x + re.w + SasaGUI::ScrollBar::Thickness) + 32,(int32)re.y ,ScenarioTextX - 100,770 };
		}

		Scenario1 = RenderTexture{ Size{ scenario_with_longest_name_w, scenario_with_longest_name_h }, ColorF{ 0.5, 0.0 } };
		RenderWrite1();

		Scenario2 = RenderTexture{ Size{ scenario_with_longest_name_w, scenario_with_longest_name_h }, ColorF{ 0.5, 0.0 } };
		RenderWrite2();

		for (auto&& e : arrayClassScenario)
		{
			getData().fontLine.preload(e.Text);
		}
	}
	// 更新関数（オプション）
	void update() override
	{
		RenderWrite1();
		RenderWrite2();

		for (auto&& e : arrayClassScenario)
		{
			if (e.ButtonType == U"Scenario")
			{
				{
					const Transformer2D transformer{ Mat3x2::Identity(), Mat3x2::Translate(scenario_with_longest_name_x, scenario_with_longest_name_y) };

					if (e.btnRectF.mouseOver())
					{
						tempText = ClassStaticCommonMethod::MoldingScenarioText(e.Text);
					}

					if (e.btnRectF.leftClicked() == true)
					{
						getData().selectClassScenario = e;
						changeScene(U"SelectChar", 0.9s);
					}
				}
			}
			else
			{
				{
					const Transformer2D transformer{ Mat3x2::Identity(), Mat3x2::Translate(scenario_with_longest_name_x, rectFScenario2X.y + 32) };

					if (e.btnRectF.mouseOver())
					{
						tempText = ClassStaticCommonMethod::MoldingScenarioText(e.Text);
					}

					if (e.btnRectF.leftClicked() == true)
					{
						if (e.ButtonType == U"Mail")
						{
							String ttt = U"start \"aaa\" \"https://mail.google.com/mail/u/0/?tf=cm&fs=1&to";
							ttt += e.Mail;
							ttt += U"&su=game%E3%81%AE%E4%BB%B6&body=%E3%81%B5%E3%82%8F%E3%81%B5%E3%82%8F%EF%BD%9E%E3%80%82%E3%82%B2%E3%83%BC%E3%83%A0%E3%81%AE%E4%BB%B6%E3%81%A7%E8%81%9E%E3%81%8D%E3%81%9F%E3%81%84%E3%81%AE%E3%81%A7%E3%81%99%E3%81%8C%E4%BB%A5%E4%B8%8B%E8%A8%98%E8%BF%B0\"";
							std::system(ttt.narrow().c_str());
							//process = ChildProcess{ U"C:/Program Files (x86)/Google/Chrome/Application/chrome.exe", U"https://mail.google.com/mail/u/0/?tf=cm&fs=1&to" + e.Mail + U"&su=game%E3%81%AE%E4%BB%B6&body=%E3%81%B5%E3%82%8F%E3%81%B5%E3%82%8F%EF%BD%9E%E3%80%82%E3%82%B2%E3%83%BC%E3%83%A0%E3%81%AE%E4%BB%B6%E3%81%A7%E8%81%9E%E3%81%8D%E3%81%9F%E3%81%84%E3%81%AE%E3%81%A7%E3%81%99%E3%81%8C%E4%BB%A5%E4%B8%8B%E8%A8%98%E8%BF%B0" };
						}
						else if (e.ButtonType == U"Internet")
						{
							System::LaunchBrowser(e.Internet);
						}
					}
				}
			}
		}

		if (rectFScenario1X.mouseOver() == true)
		{
			vbar001.value().scroll(Mouse::Wheel() * 60);
		}
		if (rectFScenario2X.mouseOver() == true)
		{
			vbar002.value().scroll(Mouse::Wheel() * 60);
		}
		vbar001.value().update();
		vbar002.value().update();
	}
	// 描画関数（オプション）
	void draw() const override
	{
		TextureAsset(U"pre0.jpg").resized(WINDOWSIZEWIDTH000, WINDOWSIZEHEIGHT000).draw(Arg::center = Scene::Center());

		getData().slice9.draw(rectFScenario1X);
		getData().slice9.draw(rectFScenario2X);
		getData().slice9.draw(rectFScenarioTextX);
		getData().fontLine(tempText).draw(rectFScenarioTextX.stretched(-32), ColorF{ 0.85 });

		Scenario1.draw(scenario_with_longest_name_x, scenario_with_longest_name_y);
		Scenario2.draw(scenario_with_longest_name_x, rectFScenario2X.y + 32);

		vbar001.value().draw();
		vbar002.value().draw();
	}
	void RenderWrite1()
	{
		int32 counterScenario1 = 0;
		{
			const ScopedRenderTarget2D target{ Scenario1.clear(ColorF{0.5, 0.0}) };

			// 描画された最大のアルファ成分を保持するブレンドステート
			const ScopedRenderStates2D blend{ MakeBlendState() };

			int32 counterScenario1 = 0;
			for (auto&& e : arrayClassScenario | std::views::filter([](auto&& e) { return e.SortKey < 0; }))
			{
				RectF re = getData().fontMini(e.ScenarioName).region();
				re.h = re.h + 16 * 2;

				int32 yyy = 32 + (counterScenario1 * re.h) + (counterScenario1 * Scenario1YBetween) - vbar001.value().value();

				re.w = scenario_with_longest_name_w - 32;
				re.x = 16;
				re.y = yyy;

				e.btnRectF = re;

				getData().slice9.draw(e.btnRectF.asRect());
				getData().fontMini(e.ScenarioName).drawAt(e.btnRectF.center(), ColorF{ 0.85 });

				counterScenario1++;
			}
		}

	}
	void RenderWrite2()
	{
		{
			const ScopedRenderTarget2D target{ Scenario2.clear(ColorF{0.5, 0.0}) };

			// 描画された最大のアルファ成分を保持するブレンドステート
			const ScopedRenderStates2D blend{ MakeBlendState() };

			int32 counterScenario2 = 0;

			for (auto&& e : arrayClassScenario | std::views::filter([](auto&& e) { return e.SortKey >= 0; }))
			{
				RectF re = getData().fontMini(e.ScenarioName).region();
				re.h = re.h + 16 * 2;

				int32 yyy = 32 + (counterScenario2 * re.h) + (counterScenario2 * Scenario1YBetween) - vbar002.value().value();

				re.w = scenario_with_longest_name_w - 32;
				re.x = 16;
				re.y = yyy;

				e.btnRectF = re;

				getData().slice9.draw(e.btnRectF.asRect());
				getData().fontMini(e.ScenarioName).drawAt(e.btnRectF.center(), ColorF{ 0.85 });

				counterScenario2++;
			}
		}

	}
	void drawFadeIn(double t) const override
	{
		draw();

		m_fadeInFunction->fade(1 - t);
	}
	void drawFadeOut(double t) const override
	{
		draw();

		m_fadeOutFunction->fade(t);
	}
private:
	Optional<SasaGUI::ScrollBar> vbar001;
	Optional<SasaGUI::ScrollBar> vbar002;
	String tempText = U"";
	int32 Scenario1YBetween = 32;
	int32 Scenario1X = 0;
	int32 Scenario2X;
	int32 Scenario2Y;
	int32 ScenarioTextX;
	int32 ScenarioTextY;
	Rect rectFScenario1X = {};
	Rect rectFScenario2X = {};
	Rect rectFScenarioTextX = {};
	Array <ClassScenario> arrayClassScenario;
	std::unique_ptr<IFade> m_fadeInFunction = randomFade();
	std::unique_ptr<IFade> m_fadeOutFunction = randomFade();
	RenderTexture Scenario1 = {};
	RenderTexture Scenario2 = {};
	int32 scenario_with_longest_name_x = 0;
	int32 scenario_with_longest_name_y = 0;
	int32 scenario_with_longest_name_w = 0;
	int32 scenario_with_longest_name_h = 0;
};
class SelectChar : public App::Scene
{
public:
	// コンストラクタ（必ず実装）
	SelectChar(const InitData& init)
		: IScene{ init }
	{
		for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/030_SelectCharaImage/"))
		{
			String filename = FileSystem::FileName(filePath);
			TextureAsset::Register(filename, filePath);
		}

		String sc = systemString.selectChara1;
		RectF re1 = getData().fontNormal(sc).region();
		re1.x = WINDOWSIZEWIDTH000 / 2 - (re1.w / 2);
		re1.y = basePointY;
		arrayRectFSystem.push_back(re1);
		Rect re2 = { 0,0,WINDOWSIZEWIDTH000,256 };
		re2.x = WINDOWSIZEWIDTH000 / 2 - (800);
		re2.y = (re1.h + basePointY + 20) + 450 + 20;
		arrayRectSystem.push_back(re2);

		int32 arrayPowerSize = getData().selectClassScenario.ArrayPower.size();
		int32 xxx = 0;
		xxx = ((arrayPowerSize * 169) / 2);
		int32 counter = 0;
		for (auto ttt : getData().selectClassScenario.ArrayPower)
		{
			for (auto&& e : getData().classGameStatus.arrayClassPower | std::views::filter([&](auto&& e) { return e.PowerTag == ttt; }))
			{
				RectF rrr = {};
				rrr = { (WINDOWSIZEWIDTH000 / 2 - xxx) + counter * 169,re1.h + basePointY + 20,169,450 };
				e.RectF = rrr;
			}
			counter++;
		}

		//Scene::SetBackground(Color{ 126,87,194,255 });
	}
	// 更新関数（オプション）
	void update() override
	{
		switch (selectCharStatus)
		{
		case SelectCharStatus::SelectChar:
		{
			for (const auto ttt : getData().classGameStatus.arrayClassPower)
			{
				if (ttt.RectF.leftClicked() == true)
				{
					// TOML ファイルからデータを読み込む
					const TOMLReader tomlInfoProcess{ PATHBASE + PATH_DEFAULT_GAME + U"/070_Scenario/InfoProcess/" + ttt.PowerTag + U".toml" };

					if (not tomlInfoProcess) // もし読み込みに失敗したら
					{
						selectCharStatus = SelectCharStatus::Message;
						Message001 = true;
						break;
					}

					for (const auto& table : tomlInfoProcess[U"Process"].tableArrayView()) {
						String map = table[U"map"].get<String>();
						getData().classGameStatus.arrayInfoProcessSelectCharaMap = map.split(U',');
						//for (auto& map : getData().classGameStatus.arrayInfoProcessSelectCharaMap)
						//{
						//	String ene = table[map].get<String>();
						//	getData().classGameStatus.arrayInfoProcessSelectCharaEnemyUnit = ene.split(U',');
						//}
					}
					getData().classGameStatus.nowPowerTag = ttt.PowerTag;
					getData().NovelPower = ttt.PowerTag;
					getData().NovelNumber = 0;

					for (auto& aaa : getData().classGameStatus.arrayClassPower)
					{
						if (aaa.PowerTag == ttt.PowerTag)
						{
							getData().selectClassPower = aaa;
							getData().Money = aaa.Money;
						}
					}

					changeScene(U"Novel", 0.9s);
				}
			}
		}
		break;
		case SelectCharStatus::Message:
		{
			if (Message001)
			{
				sceneMessageBoxImpl.set();
				if (sceneMessageBoxImpl.m_buttonC.mouseOver())
				{
					Cursor::RequestStyle(CursorStyle::Hand);

					if (MouseL.down())
					{
						Message001 = false;
						selectCharStatus = SelectCharStatus::SelectChar;
					}
				}
			}
		}
		break;
		case SelectCharStatus::Event:
			break;
		default:
			break;
		}
	}
	// 描画関数（オプション）
	void draw() const override
	{
		TextureAsset(getData().selectClassScenario.SelectCharaFrameImageLeft).draw();
		TextureAsset(getData().selectClassScenario.SelectCharaFrameImageRight)
			.draw(WINDOWSIZEWIDTH000 - TextureAsset(getData().selectClassScenario.SelectCharaFrameImageRight).width(), 0);

		arrayRectFSystem[0].draw();
		getData().fontMini(systemString.selectChara1).draw(arrayRectFSystem[0], ColorF{ 0.25 });
		arrayRectFSystem[0].drawFrame(3, 0, Palette::Orange);

		getData().slice9.draw(arrayRectSystem[0]);

		for (const auto ttt : getData().classGameStatus.arrayClassPower)
		{
			ttt.RectF(TextureAsset(ttt.Image).resized(169, 450)).draw();
			if (ttt.RectF.mouseOver() == true)
			{
				getData().fontLine(ttt.Text).draw(arrayRectSystem[0].stretched(-10), ColorF{ 0.85 });
			}
		}

		switch (selectCharStatus)
		{
		case SelectCharStatus::SelectChar:
			break;
		case SelectCharStatus::Message:
			sceneMessageBoxImpl.show(systemString.SelectCharMessage001);
			break;
		case SelectCharStatus::Event:
			break;
		default:
			break;
		}
	}

	void drawFadeIn(double t) const override
	{
		draw();

		m_fadeInFunction->fade(1 - t);
	}
	void drawFadeOut(double t) const override
	{
		draw();

		m_fadeOutFunction->fade(t);
	}
private:
	int32 basePointY = 50;
	/// @brief classConfigString.selectChara1の枠　など
	Array<Rect> arrayRectSystem;
	/// @brief mouseOver時のテキストエリア　など
	Array<RectF> arrayRectFSystem;
	bool Message001 = false;
	SelectCharStatus selectCharStatus = SelectCharStatus::SelectChar;
	s3dx::SceneMessageBoxImpl sceneMessageBoxImpl;
	std::unique_ptr<IFade> m_fadeInFunction = randomFade();
	std::unique_ptr<IFade> m_fadeOutFunction = randomFade();
};
class Novel : public App::Scene
{
public:
	// コンストラクタ（必ず実装）
	Novel(const InitData& init)
		: IScene{ init }
	{
		getData().audio.stop();
		//getData().audio = AudioAsset(e.tag);
		//AudioAsset(U"BGM").setVolume(0.2);
		//AudioAsset(U"BGM").play();

		String np = getData().NovelPower;
		int32 nn = getData().NovelNumber;
		String path = PATHBASE + PATH_DEFAULT_GAME + U"/070_Scenario/InfoStory/" + np + U"+" + Format(nn) + U".csv";
		csv = CSV{ path };
		if (not csv) // もし読み込みに失敗したら
		{
			throw Error{ U"Failed to load " + np + U"+" + Format(nn) + U".csv" };
		}

		nowRow = 0;
		if (csv[nowRow][0].c_str()[0] == '#')
		{
			nowRow++;
		}

		rectText = { 50,WINDOWSIZEHEIGHT000 - (256 + 32),WINDOWSIZEWIDTH000 - 100,256 };
		rectHelp = { 70,WINDOWSIZEHEIGHT000 - 325 - 70,400,70 };
		rectFace = { WINDOWSIZEWIDTH000 - 100 - 206 - 50,WINDOWSIZEHEIGHT000 - 325 + 50,206,206 };
		rectSkip = { WINDOWSIZEWIDTH000 - 400 - 50,WINDOWSIZEHEIGHT000 - 325 - 70,400,70 };
		stopwatch = Stopwatch{ StartImmediately::Yes };
	}
	// 更新関数（オプション）
	void update() override
	{
		if (csv[nowRow][7].length() != length)
		{
			length = stopwatch.sF() / 0.05;
		}
		if (csv[nowRow][0].substr(0, 3) == U"end" || rectSkip.leftClicked() == true)
		{
			if (getData().Wave >= getData().selectClassPower.Wave)
			{
				changeScene(U"TitleScene", 0.9s);
			}
			else
			{
				changeScene(U"Buy", 0.9s);
			}
		}

		if (csv[nowRow][0].c_str()[0] == '#')
		{
			nowRow++;
		}

		if (MouseL.down() == true && rectText.mouseOver() == true)
		{
			if (csv[nowRow][7].length() == length)
			{
				stopwatch.restart();
				length = 0;
				nowRow++;
			}
			else
			{
				length = csv[nowRow][7].length();
			}
		}
	}
	// 描画関数（オプション）
	void draw() const override
	{
		if (csv.rows() == nowRow)
		{
			return;
		}
		if (csv[nowRow][4] == U"0")
		{
			Scene::SetBackground(ColorF{ U"#000000" });
		}
		if (csv[nowRow][4] != U"-1" && csv[nowRow][4] != U"0")
		{
			TextureAsset(csv[nowRow][4]).resized(WINDOWSIZEWIDTH000, WINDOWSIZEHEIGHT000).drawAt(Scene::Center());
		}

		getData().slice9.draw(rectText);
		getData().slice9.draw(rectSkip);
		getData().fontLine(systemString.StorySkip).draw(rectSkip.stretched(-10), ColorF{ 0.85 });

		if (csv[nowRow][3] != U"-1")
		{
			rectFace(TextureAsset(csv[nowRow][3])).draw();
		}
		if (csv[nowRow][0].c_str()[0] != '#')
		{
			getData().fontLine(csv[nowRow][7].substr(0, length)).draw(rectText.stretched(-10), ColorF{ 0.85 });
		}
		if (csv[nowRow][1] != U"-1")
		{
			getData().slice9.draw(rectHelp);
			String he = U"";
			if (csv[nowRow][2] != U"-1")
			{
				he = csv[nowRow][1] + U" " + csv[nowRow][2];
			}
			else
			{
				he = csv[nowRow][1];
			}
			getData().fontLine(he).draw(rectHelp.stretched(-10), ColorF{ 0.85 });
		}
	}

	void drawFadeIn(double t) const override
	{
		draw();

		m_fadeInFunction->fade(1 - t);
	}

	void drawFadeOut(double t) const override
	{
		draw();

		m_fadeOutFunction->fade(t);
	}
private:
	Rect rectText = {};
	Rect rectFace = {};
	Rect rectHelp = {};
	Rect rectSkip = {};
	int32 nowRow = 0;
	int32 length = 0;
	CSV csv;
	Stopwatch stopwatch;
	std::unique_ptr<IFade> m_fadeInFunction = randomFade();
	std::unique_ptr<IFade> m_fadeOutFunction = randomFade();
};
class Buy : public App::Scene
{
public:
	// コンストラクタ（必ず実装）
	Buy(const InitData& init)
		: IScene{ init }
	{
		getData().audio.stop();
		for (auto&& e : getData().classGameStatus.arrayClassBGM | std::views::filter([&](auto&& e) { return e.prepare == true; }))
		{
			getData().audio = AudioAsset(e.tag);
			getData().audio.setLoop(true);
			getData().audio.play();
			break;
		}

		//getData().audio = AudioAsset(e.tag);
		//getData().audio.play();

		//中央上
		arrayRectMenuBack.push_back(Rect{ 432,16,500,500 });

		////メニューボタン
		//for (auto index : Range(0, getData().classGameStatus.NumMenus - 1)) {
		//	if (getData().classGameStatus.strategyMenus[index]) {
		//		htMenuBtn.push_back(std::make_tuple(index, Rect{ 16,16 + (index * 64),300,64 }));
		//	}
		//}

		renderTextureRight = MSRenderTexture{ 400,800, ColorF{0.5, 0.0} };
		renderTextureRight.clear(ColorF{ 0.5, 0.0 });

		renderTextureMenuBtn = MSRenderTexture{ 400,800, ColorF{0.5, 0.0} };
		renderTextureMenuBtn.clear(ColorF{ 0.5, 0.0 });
		{
			// レンダーターゲットを renderTextureDetail に変更する
			const ScopedRenderTarget2D target{ renderTextureMenuBtn };

			// 描画された最大のアルファ成分を保持するブレンドステート
			const ScopedRenderStates2D blend{ MakeBlendState() };

			{
				getData().slice9.draw(Rect{ 0,0,400,800 });

				for (auto index : Range(0, getData().classGameStatus.NumMenus - 1)) {
					if (getData().classGameStatus.strategyMenus[index]) {
						{
							htMenuBtn.push_back(std::make_tuple(index, Rect{ 16,16 + (index * 64),300,64 }));
							getData().slice9.draw(std::get<1>(htMenuBtn.back()));
							getData().fontLine(systemString.strategyMenu[index]).draw(std::get<1>(htMenuBtn.back()).stretched(-10));
						}
					}
				}
			}

			Graphics2D::Flush();
			renderTextureMenuBtn.resolve();
		}

		//初期化
		{
			for (auto ttt : htMenuBtnDisplay)
				ttt.second = false;

			int32 counter = 0;
			for (auto& ttt : getData().classGameStatus.arrayClassUnit)
			{
				ttt.rectExecuteBtnStrategyMenu = Rect{ 448,548 + (counter * 64),300,64 };
				ttt.rectExecuteBtnStrategyMenuLeader = Rect{ 432 + 500 - 164,548 + (counter * 64),64,64 };
				ttt.rectExecuteBtnStrategyMenuMember = Rect{ 432 + 500 - 80,548 + (counter * 64),64,64 };
				counter++;
			}

			vbar001.emplace(SasaGUI::Orientation::Vertical);;
			vbar002.emplace(SasaGUI::Orientation::Vertical);;
			vbarRight.emplace(SasaGUI::Orientation::Vertical);;
			vbar001.value().updateLayout({
				(int32)(432 + 500 + SasaGUI::ScrollBar::Thickness), (int32)(16),
				SasaGUI::ScrollBar::Thickness,
				(int32)500
			});
			vbar002.value().updateLayout({
				(int32)(432 + 500 + SasaGUI::ScrollBar::Thickness), (int32)516,
				SasaGUI::ScrollBar::Thickness,
				(int32)500
			});
			vbarRight.value().updateLayout({
				(int32)(1396 + SasaGUI::ScrollBar::Thickness), (int32)16,
				SasaGUI::ScrollBar::Thickness,
				(int32)800
			});
			vbar001.value().updateConstraints(0.0, 2000.0, Scene::Height());
			vbar002.value().updateConstraints(0.0, 2000.0, Scene::Height());
			vbarRight.value().updateConstraints(0.0, 2000.0, Scene::Height());
		}
	}
	// 更新関数（オプション）
	void update() override
	{
		Cursor::RequestStyle(U"MyCursor");

		switch (formBuyDisplayStatus)
		{
		case FormBuyDisplayStatus::Normal:
		{
			for (auto& ttt : htMenuBtn)
			{
				const Quad Button1Quad = projection.transformRect(std::get<1>(ttt));
				if (Button1Quad.mouseOver())
				{
					for (auto&& [i, re] : htMenuBtnDisplay)
						htMenuBtnDisplay[i] = false;
					htMenuBtnDisplay[std::get<0>(ttt)] = true;
				}

				switch (std::get<0>(ttt))
				{
				case 9:
				{
					if (Button1Quad.leftClicked() == true)
					{
						//バトル前準備
						processBeforeBattle();
						changeScene(U"Battle", 0.9s);
					}
				}
				break;
				default:
					break;
				}
			}

			//trueなら表示
			if (std::any_of(htMenuBtnDisplay.begin(), htMenuBtnDisplay.end(),
				[](const auto& ttt) { return ttt.second; }))
			{
				rectExecuteBtn = Rect{ 432, 516, 500, 500 };
			}

			displayConscriptionUnitInfo();
			//徴兵処理
			conscriptionUnit();
			//ユニット表示エリアでスクロールした時、位置を調整する
			fixDisplayUnit();
			//徴兵可能ユニット表示エリアでスクロールした時、位置を調整する
			fixDisplayUnitConscriptionUnit();
			//ユニットをクリック時に、その他のユニットの対象フラグを初期化する
			resetFlagsUnit();

			displayUnitInfo();
			//スクロールバー関係
			processBar();
		}
		break;
		case FormBuyDisplayStatus::Message:
		{
			if (Message001)
			{
				sceneMessageBoxImpl.set();
				if (sceneMessageBoxImpl.m_buttonC.mouseOver())
				{
					Cursor::RequestStyle(CursorStyle::Hand);

					if (MouseL.down())
					{
						Message001 = false;
						formBuyDisplayStatus = FormBuyDisplayStatus::Normal;
					}
				}
			}
		}
		break;
		case FormBuyDisplayStatus::Event:
			break;
		default:
			break;
		}
	}
	// 描画関数（オプション）
	void draw() const override
	{
		getData().slice9.draw(rectExecuteBtn);

		for (auto& ttt : arrayRectMenuBack)
			getData().slice9.draw(ttt);
		for (auto& ttt : getData().classGameStatus.arrayPlayerUnit)
		{
			for (auto& aaa : ttt.ListClassUnit)
			{
				if (aaa.rectDetailStrategyMenu.y > (13 * 32) + 16 + 16 || aaa.rectDetailStrategyMenu.y < 16 + 15)
				{

				}
				else
				{
					if (aaa.pressedDetailStrategyMenu == true)
					{
						aaa.rectDetailStrategyMenu(TextureAsset(aaa.Image)).draw().drawFrame(0, 3, Palette::Red);
					}
					else
					{
						aaa.rectDetailStrategyMenu(TextureAsset(aaa.Image)).draw().drawFrame(0, 3, Palette::Orange);
					}
				}
			}
		}

		//強制表示メニュー
		{
			const ScopedRenderStates2D sampler{ SamplerState::ClampAniso };
			Shader::QuadWarp(TargetQuad, renderTextureMenuBtn);
		}

		//右詳細
		{
			const ScopedRenderStates2D sampler{ SamplerState::ClampAniso };
			Shader::QuadWarp(TargetQuadRight, renderTextureRight);
		}
		if (flagVbarRight)
			vbarRight.value().draw();

		for (auto& ttt : htMenuBtnDisplay)
		{
			switch (ttt.first)
			{
			case 0:
				if (ttt.second == true)
				{
					//徴兵出来るユニット表示
					for (auto nowHtRectPlusUnit : getData().classGameStatus.arrayClassUnit)
					{
						if (nowHtRectPlusUnit.rectExecuteBtnStrategyMenu.y > Scene::Size().y - 12 || nowHtRectPlusUnit.rectExecuteBtnStrategyMenu.y < 548 - 12)
						{

						}
						else
						{
							getData().slice9.draw(nowHtRectPlusUnit.rectExecuteBtnStrategyMenu);
							getData().fontLine(nowHtRectPlusUnit.Name).draw(nowHtRectPlusUnit.rectExecuteBtnStrategyMenu.stretched(-10));
							getData().slice9.draw(nowHtRectPlusUnit.rectExecuteBtnStrategyMenuLeader);
							getData().fontLine(U"L").draw(nowHtRectPlusUnit.rectExecuteBtnStrategyMenuLeader.stretched(-10));
							getData().slice9.draw(nowHtRectPlusUnit.rectExecuteBtnStrategyMenuMember);
							getData().fontLine(U"M").draw(nowHtRectPlusUnit.rectExecuteBtnStrategyMenuMember.stretched(-10));
						}
					}
					vbar002.value().draw();
				}
				break;
			case 1:
				break;
			case 2:
				break;
			case 3:
				break;
			case 4:
				break;
			case 5:
				break;
			case 6:
				break;
			case 7:
				break;
			case 8:
				break;
			case 9:
				break;
			default:
				break;
			}
		}

		vbar001.value().draw();

		switch (formBuyDisplayStatus)
		{
		case FormBuyDisplayStatus::Normal:
			break;
		case FormBuyDisplayStatus::Message:
			sceneMessageBoxImpl.show(systemString.BuyMessage001);
			break;
		case FormBuyDisplayStatus::Event:
			break;
		default:
			break;
		}
	}
	void drawFadeIn(double t) const override
	{
		draw();

		m_fadeInFunction->fade(1 - t);
	}
	void drawFadeOut(double t) const override
	{
		draw();

		m_fadeOutFunction->fade(t);
	}
private:
	/// @brief ユニットをクリック時に、その他のユニットの対象フラグを初期化する
	void resetFlagsUnit()
	{
		for (auto& nowArrayPlayerUnit : getData().classGameStatus.arrayPlayerUnit)
		{
			for (auto& aaa : nowArrayPlayerUnit.ListClassUnit)
			{
				if (aaa.rectDetailStrategyMenu.leftClicked() == true)
				{
					for (auto& nowarrayPlayerUnit : getData().classGameStatus.arrayPlayerUnit)
					{
						for (auto& bbb : nowarrayPlayerUnit.ListClassUnit)
						{
							bbb.pressedDetailStrategyMenu = false;
						}
					}

					aaa.pressedDetailStrategyMenu = true;
					break;
				}
			}
		}
	}
	/// @brief ユニットをマウスオーバー時に、そのユニットの情報を表示する
	void displayUnitInfo()
	{
		for (auto& nowArrayPlayerUnit : getData().classGameStatus.arrayPlayerUnit)
		{
			for (auto& aaa : nowArrayPlayerUnit.ListClassUnit)
			{
				if (aaa.rectDetailStrategyMenu.mouseOver() == true)
				{
					writeRenderRight(aaa);
					flagVbarRight = true;
				}
			}
		}
	}
	void displayConscriptionUnitInfo()
	{
		for (auto& nowHtRectPlusUnit : getData().classGameStatus.arrayClassUnit)
		{
			if (nowHtRectPlusUnit.rectExecuteBtnStrategyMenu.mouseOver() == true)
			{
				writeRenderRight(nowHtRectPlusUnit);
				flagVbarRight = true;
			}
		}
	}
	void writeRenderRight(ClassUnit& nowHtRectPlusUnit)
	{
		{
			// レンダーターゲットを tempRight に変更する
			const ScopedRenderTarget2D target{ tempRight.clear(ColorF{0.5, 0.0}) };

			// 描画された最大のアルファ成分を保持するブレンドステート
			const ScopedRenderStates2D blend{ MakeBlendState() };

			String temp = U"【" + systemString.StatusName + U"】" + nowHtRectPlusUnit.Name + U"\r\n"
				+ U"【" + systemString.StatusPrice + U"】" + Format(nowHtRectPlusUnit.Price) + U"\r\n"
				+ U"【" + systemString.StatusRace + U"】" + Format(nowHtRectPlusUnit.Race) + U"\r\n"
				+ U"【" + systemString.StatusHp + U"】" + Format(nowHtRectPlusUnit.Hp) + U"\r\n"
				+ U"【" + systemString.StatusMp + U"】" + Format(nowHtRectPlusUnit.Mp) + U"\r\n"
				+ U"【" + systemString.StatusMagic + U"】" + Format(nowHtRectPlusUnit.Magic) + U"\r\n"
				+ U"【" + systemString.StatusMove + U"】" + Format(nowHtRectPlusUnit.Move) + U"\r\n"
				+ U"【" + systemString.StatusAttack + U"】" + Format(nowHtRectPlusUnit.Attack) + U"\r\n"
				+ U"【" + systemString.StatusDefense + U"】" + Format(nowHtRectPlusUnit.Defense) + U"\r\n"
				+ U"【" + systemString.StatusSpeed + U"】" + Format(nowHtRectPlusUnit.Speed) + U"\r\n"
				+ U"【" + systemString.StatusSetumei + U"】" + U"\r\n"
				;

			getData().fontLine(temp).draw(12, 12);

			while (not getData().fontLine(nowHtRectPlusUnit.Help).draw(BaseRectRightSetumei.stretched(-12), ColorF{ 0.0 }))
			{
				BaseRectRightSetumei.h = BaseRectRightSetumei.h + 12;
			}
			BaseRectRightSetumei.y = getData().fontLine(temp).region().h + 12;
			getData().slice9.draw(BaseRectRightSetumei);
			getData().fontLine(nowHtRectPlusUnit.Help).draw(BaseRectRightSetumei.stretched(-12));

			Graphics2D::Flush();
			tempRight.resolve();
		}

		{
			// レンダーターゲットを renderTextureRight に変更する
			const ScopedRenderTarget2D target{ renderTextureRight.clear(ColorF{0.5, 0.0}) };

			// 描画された最大のアルファ成分を保持するブレンドステート
			const ScopedRenderStates2D blend{ MakeBlendState() };

			getData().slice9.draw(Rect{ 0,0,400,800 });

			tempRight.draw();

			Graphics2D::Flush();
			renderTextureRight.resolve();
		}
	}
	/// @brief スクロールバー関係
	void processBar()
	{
		double mw = Mouse::Wheel();

		if (arrayRectMenuBack[0].mouseOver() == true)
			vbar001.value().scroll(Mouse::Wheel() * 60);
		if (rectExecuteBtn.mouseOver() == true)
			vbar002.value().scroll(Mouse::Wheel() * 60);

		if (flagVbarRight && TargetQuadRight.mouseOver() == true)
		{
			vbarRight.value().scroll(Mouse::Wheel() * 60);

			{
				// レンダーターゲットを renderTextureDetail に変更する
				const ScopedRenderTarget2D target{ renderTextureRight.clear(ColorF{0.5, 0.0}) };

				// 描画された最大のアルファ成分を保持するブレンドステート
				const ScopedRenderStates2D blend{ MakeBlendState() };

				getData().slice9.draw(Rect{ 0,0,400,800 });

				tempRight(0, vbarRight.value().value(), 400, 800).draw();

				Graphics2D::Flush();
				renderTextureRight.resolve();
			}
		}

		vbar001.value().update();
		vbar002.value().update();
		vbarRight.value().update();
	}
	/// @brief ユニット表示エリアでスクロールした時、位置を調整する
	void fixDisplayUnit()
	{
		int32 counterUnit = 0;
		for (auto& nowarrayPlayerUnit : getData().classGameStatus.arrayPlayerUnit)
		{
			int32 yyy = 16 + 16 + (counterUnit * 32) - vbar001.value().value();
			for (auto& aaa : nowarrayPlayerUnit.ListClassUnit)
			{
				aaa.rectDetailStrategyMenu.y = yyy;
			}
			counterUnit++;
		}
	}
	/// @brief 徴兵可能ユニット表示エリアでスクロールした時、位置を調整する
	void fixDisplayUnitConscriptionUnit()
	{
		int32 counterUnit = 0;
		for (auto& nowarrayPlayerUnit : getData().classGameStatus.arrayClassUnit)
		{
			int32 yyy = 548 + (counterUnit * 64) - vbar002.value().value();

			nowarrayPlayerUnit.rectExecuteBtnStrategyMenu.y = yyy;
			nowarrayPlayerUnit.rectExecuteBtnStrategyMenuLeader.y = yyy;
			nowarrayPlayerUnit.rectExecuteBtnStrategyMenuMember.y = yyy;
			counterUnit++;
		}
	}
	/// @brief 徴兵処理
	void conscriptionUnit()
	{
		for (auto& nowHtRectPlusUnit : getData().classGameStatus.arrayClassUnit)
		{
			//リーダー追加時処理
			if (nowHtRectPlusUnit.rectExecuteBtnStrategyMenuLeader.leftClicked() == true)
			{
				if (nowHtRectPlusUnit.Price > getData().Money)
				{
					formBuyDisplayStatus = FormBuyDisplayStatus::Message;
					Message001 = true;
					break;
				}

				getData().Money = getData().Money - nowHtRectPlusUnit.Price;

				ClassHorizontalUnit chu;
				ClassUnit cu = nowHtRectPlusUnit;
				cu.ID = getData().classGameStatus.getIDCount();
				cu.rectDetailStrategyMenu = Rect{ 432 + 16,16 + 16 + (getData().classGameStatus.arrayPlayerUnit.size() * 32),32,32 };
				chu.ListClassUnit.push_back(cu);
				getData().classGameStatus.arrayPlayerUnit.push_back(chu);
			}

			//メンバー追加処理
			if (nowHtRectPlusUnit.rectExecuteBtnStrategyMenuMember.leftClicked() == true)
			{
				int32 rowCounter = 0;
				for (auto che : getData().classGameStatus.arrayPlayerUnit)
				{
					for (auto che2 : che.ListClassUnit)
					{
						if (che2.pressedDetailStrategyMenu == false)
							continue;

						if (nowHtRectPlusUnit.Price > getData().Money)
						{
							formBuyDisplayStatus = FormBuyDisplayStatus::Message;
							Message001 = true;
							break;
						}

						getData().Money = getData().Money - nowHtRectPlusUnit.Price;

						ClassUnit cu = nowHtRectPlusUnit;
						cu.ID = getData().classGameStatus.getIDCount();
						cu.rectDetailStrategyMenu = Rect{ 432 + 16 + (getData().classGameStatus.arrayPlayerUnit[rowCounter].ListClassUnit.size() * 32),16 + 16 + (rowCounter * 32),32,32 };
						getData().classGameStatus.arrayPlayerUnit[rowCounter].ListClassUnit.push_back(cu);
						break;
					}
					rowCounter++;
				}
			}

			//一括追加の予定
			if (nowHtRectPlusUnit.rectExecuteBtnStrategyMenu.rightClicked() == true)
			{
			}
		}
	}
	/// @brief バトル前準備
	void processBeforeBattle()
	{
		String targetMap = getData().classGameStatus.arrayInfoProcessSelectCharaMap[getData().Wave];

		const TOMLReader tomlMap{ PATHBASE + PATH_DEFAULT_GAME + U"/016_BattleMap/" + targetMap };
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

		ClassBattle cb;
		const TOMLReader tomlInfoProcess{ PATHBASE + PATH_DEFAULT_GAME + U"/070_Scenario/InfoProcess/" + getData().classGameStatus.nowPowerTag + U".toml" };
		if (not tomlInfoProcess) // もし読み込みに失敗したら
			throw Error{ U"Failed to load `tomlInfoProcess`" };

		//建物関係
		cb.classMapBattle = ClassStaticCommonMethod::GetClassMapBattle(sM);
		ClassHorizontalUnit chuSor;
		ClassHorizontalUnit chuDef;
		ClassHorizontalUnit chuNa;
		chuSor.FlagBuilding = true;
		chuDef.FlagBuilding = true;
		chuNa.FlagBuilding = true;
		for (size_t indexRow = 0; indexRow < cb.classMapBattle.value().mapData.size(); ++indexRow)
		{
			for (size_t indexCol = 0; indexCol < cb.classMapBattle.value().mapData[indexRow].size(); ++indexCol)
			{
				for (auto& bui : cb.classMapBattle.value().mapData[indexRow][indexCol].building)
				{
					String key = std::get<0>(bui);
					BattleWhichIsThePlayer bw = std::get<2>(bui);

					// arrayClassObjectMapTip から適切な ClassObjectMapTip オブジェクトを見つける
					for (const auto& mapTip : getData().classGameStatus.arrayClassObjectMapTip)
					{
						if (mapTip.nameTag == key)
						{
							// ClassUnit の設定を行う
							ClassUnit unitBui;
							unitBui.IsBuilding = true;
							unitBui.ID = getData().classGameStatus.getIDCount();
							std::get<1>(bui) = unitBui.ID;
							unitBui.mapTipObjectType = mapTip.type;
							unitBui.NoWall2 = mapTip.noWall2;
							unitBui.HPCastle = mapTip.castle;
							unitBui.CastleDefense = mapTip.castleDefense;
							unitBui.CastleMagdef = mapTip.castleMagdef;
							unitBui.Image = mapTip.nameTag;
							unitBui.rowBuilding = indexRow;
							unitBui.colBuilding = indexCol;

							if (bw == BattleWhichIsThePlayer::Sortie)
							{
								chuSor.ListClassUnit.push_back(unitBui);
							}
							else if (bw == BattleWhichIsThePlayer::Def)
							{
								chuDef.ListClassUnit.push_back(unitBui);
							}
							else
							{
								chuNa.ListClassUnit.push_back(unitBui);
							}
							break; // 適切なオブジェクトが見つかったのでループを抜ける
						}
					}
				}
			}
		}
		cb.sortieUnitGroup.push_back(chuSor);
		cb.defUnitGroup.push_back(chuDef);
		cb.neutralUnitGroup.push_back(chuNa);

		//敵兵
		for (auto&& [i, ttt] : Indexed(sM.data))
		{
			Array<String> arrayMapUnit = ttt.split(U',');
			for (auto&& [j, unitYoko] : Indexed(arrayMapUnit))
			{
				Array<String> cellInfo = unitYoko.split(U'*');
				if (cellInfo[2] == U"" || cellInfo[2] == U"-1")
				{
					continue;
				}
				ClassHorizontalUnit chu;
				Array<String> unitInfo = cellInfo[2].split(U':');

				//部隊編成を取得
				auto it = std::find_if(getData().classGameStatus.arrayClassEnemy.begin(), getData().classGameStatus.arrayClassEnemy.end(),
							[&](const ClassEnemy& unit) { return unit.name == unitInfo[0]; });
				if (it == getData().classGameStatus.arrayClassEnemy.end())
				{
					continue;
				}
				for (auto& ce : it->type)
				{
					//ユニットの情報を取得
					auto it2 = std::find_if(getData().classGameStatus.arrayClassUnit.begin(), getData().classGameStatus.arrayClassUnit.end(),
								[&](const ClassUnit& unit) { return unit.NameTag == ce; });
					if (it2 == getData().classGameStatus.arrayClassUnit.end())
					{
						continue;
					}
					it2->ID = getData().classGameStatus.getIDCount();
					it2->houkou = unitInfo[1];
					it2->initXY = Point(i, j);
					chu.ListClassUnit.push_back(*it2);
				}
				cb.defUnitGroup.push_back(chu);
			}
		}

		mapCreator.N = cb.classMapBattle.value().mapData.size();

		//敵兵位置移動
		for (auto&& [i, item] : IndexedRef(cb.defUnitGroup))
		{
			if (!item.FlagBuilding &&
				!item.ListClassUnit.empty())
			{
				Point start;
				Point end;

				if (item.ListClassUnit[0].houkou == U"東")
				{
					start = mapCreator.ToTileBottomCenter(item.ListClassUnit[0].initXY.movedBy(-1, 0), mapCreator.N).asPoint();
					end = mapCreator.ToTileBottomCenter(item.ListClassUnit[0].initXY, mapCreator.N).asPoint();
				}
				else if (item.ListClassUnit[0].houkou == U"西")
				{
					start = mapCreator.ToTileBottomCenter(item.ListClassUnit[0].initXY.movedBy(1, 0), mapCreator.N).asPoint();
					end = mapCreator.ToTileBottomCenter(item.ListClassUnit[0].initXY, mapCreator.N).asPoint();
				}
				else if (item.ListClassUnit[0].houkou == U"北")
				{
					start = mapCreator.ToTileBottomCenter(item.ListClassUnit[0].initXY.movedBy(0, -1), mapCreator.N).asPoint();
					end = mapCreator.ToTileBottomCenter(item.ListClassUnit[0].initXY, mapCreator.N).asPoint();
				}
				else if (item.ListClassUnit[0].houkou == U"南")
				{
					start = mapCreator.ToTileBottomCenter(item.ListClassUnit[0].initXY.movedBy(0, 1), mapCreator.N).asPoint();
					end = mapCreator.ToTileBottomCenter(item.ListClassUnit[0].initXY, mapCreator.N).asPoint();
				}
				else {
					start = mapCreator.ToTileBottomCenter(item.ListClassUnit[0].initXY.movedBy(-1, 0), mapCreator.N).asPoint();
					end = mapCreator.ToTileBottomCenter(item.ListClassUnit[0].initXY, mapCreator.N).asPoint();
				}

				//その部隊の人数を取得
				int32 unitCount = item.ListClassUnit.size();
				//商の数
				int32 result = unitCount / 2;
				//角度
				// X軸との角度を計算
				//θ'=直線とx軸のなす角度
				double angle2 = Math::Atan2(end.y - start.y,
									   end.x - start.x);
				//θ
				double angle = Math::Pi / 2 - angle2;
				//偶奇判定
				if (unitCount % 2 == 1)
				{
					////奇数の場合
					int32 counter = 0;
					for (auto& unit : item.ListClassUnit)
					{
						//px+(b-切り捨て商)＊dcosθ+a＊d'cosθ’
						double xPos = end.x
							+ (
								(counter - (result))
								* (getData().classGameStatus.DistanceBetweenUnit * Math::Cos(angle))
								)
							-
							(i * (getData().classGameStatus.DistanceBetweenUnitTate * Math::Cos(angle2)));
						//py+(b-切り捨て商)＊dsinθ-a＊d'sinθ’
						double yPos = end.y
							- (
							(counter - (result))
							* (getData().classGameStatus.DistanceBetweenUnit * Math::Sin(angle))

							)
							-
							(i * (getData().classGameStatus.DistanceBetweenUnitTate * Math::Sin(angle2)));

						//移動
						unit.nowPosiLeft = Vec2(xPos, yPos);
						counter++;
					}
				}
				else
				{
					int32 counter = 0;
					for (auto& unit : item.ListClassUnit)
					{
						//px+(b-切り捨て商)＊dcosθ+a＊d'cosθ’
						double xPos = end.x
							+ (
								(counter - (result))
								* (getData().classGameStatus.DistanceBetweenUnit * Math::Cos(angle))
								)
							-
							(i * (getData().classGameStatus.DistanceBetweenUnitTate * Math::Cos(angle2)));
						//py+(b-切り捨て商)＊dsinθ-a＊d'sinθ’
						double yPos = end.y
							- (
							(counter - (result))
							* (getData().classGameStatus.DistanceBetweenUnit * Math::Sin(angle))

							)
							-
							(i * (getData().classGameStatus.DistanceBetweenUnitTate * Math::Sin(angle2)));

						//移動
						unit.nowPosiLeft = Vec2(xPos, yPos);
						counter++;
					}
				}
			}
		}

		cb.battleWhichIsThePlayer = BattleWhichIsThePlayer::Sortie;
		//C++11以降では、std::move を使ってコピーを避け、効率的に要素を追加することもできます。
		//これは特に大きな Array オブジェクトを扱う場合に有用です
		cb.sortieUnitGroup.append(std::move(getData().classGameStatus.arrayPlayerUnit));

		getData().classGameStatus.classBattle = cb;

	}
	/// @brief 左上メニュー、画面上部ユニット群の強制表示枠
	Array <Rect> arrayRectMenuBack;
	/// @brief 画面下部の詳細実行枠
	Rect rectExecuteBtn{ 0,0,0,0 };

	VertexShader vs;
	PixelShader ps;
	const Rect BaseRect{ 0,0, 400, 800 };
	const Quad TargetQuad{ Vec2{ 0, 0 },Vec2{ 400, 64 },Vec2{ 400, 800 },Vec2{ 0, 900 } };
	const Mat3x3 projection = Mat3x3::Homography(BaseRect.asQuad(), TargetQuad);
	const Rect BaseRectRight{ 932,0, 400, 800 };
	const Quad TargetQuadRight{ Vec2{ 996, 64 },Vec2{ 1396, 0 },Vec2{ 1396, 900 },Vec2{ 996, 800 } };
	const Mat3x3 projectionRight = Mat3x3::Homography(BaseRectRight.asQuad(), TargetQuadRight);
	Rect BaseRectRightSetumei{ 0,0, 400, 0 };

	Array<std::tuple<int32, Rect>> htMenuBtn;
	MSRenderTexture renderTextureMenuBtn;
	MSRenderTexture renderTextureRight;
	MSRenderTexture tempRight = MSRenderTexture{ 400,1600, ColorF{0.5, 0.0} };

	HashTable<int32, bool> htMenuBtnDisplay;
	std::unique_ptr<IFade> m_fadeInFunction = randomFade();
	std::unique_ptr<IFade> m_fadeOutFunction = randomFade();
	Optional<SasaGUI::ScrollBar> vbar001;
	Optional<SasaGUI::ScrollBar> vbar002;
	Optional<SasaGUI::ScrollBar> vbarRight;
	bool flagVbarRight = false;
	bool Message001 = false;
	FormBuyDisplayStatus formBuyDisplayStatus = FormBuyDisplayStatus::Normal;
	s3dx::SceneMessageBoxImpl sceneMessageBoxImpl;

};
class Battle : public App::Scene
{
public:
	// コンストラクタ（必ず実装）
	Battle(const InitData& init)
		: IScene{ init }
	{
		getData().audio.stop();
		for (auto&& e : getData().classGameStatus.arrayClassBGM | std::views::filter([&](auto&& e) { return e.battle == true; }))
		{
			getData().audio = AudioAsset(e.tag);
			getData().audio.setLoop(true);
			getData().audio.play();
			break;
		}

		////マップ
		// maptip フォルダ内のファイルを列挙する
		for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/015_BattleMapCellImage/"))
			TextureAsset::Register(FileSystem::FileName(filePath), filePath);
		for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/040_ChipImage/"))
			TextureAsset::Register(FileSystem::FileName(filePath), filePath);
		for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/041_ChipImageSkill/"))
			TextureAsset::Register(FileSystem::FileName(filePath), filePath);

		mapCreator.N = getData().classGameStatus.classBattle.classMapBattle.value().mapData.size();
		columnQuads = mapCreator.MakeColumnQuads(mapCreator.N);
		rowQuads = mapCreator.MakeRowQuads(mapCreator.N);
		Grid<int32> gridWork(Size{ mapCreator.N, mapCreator.N });
		mapCreator.grid = gridWork;

		int32 counterXSor = 0;
		int32 counterYSor = 0;
		bool flagSor = false;
		for (const auto target : getData().classGameStatus.classBattle.classMapBattle.value().mapData)
		{
			for (const auto wid : target)
			{
				if (wid.kougekiButaiNoIti == true)
				{
					flagSor = true;
					break;
				}
				else
				{
					counterYSor++;
				}
			}

			if (flagSor == true)
			{
				break;
			}
			counterYSor = 0;
			counterXSor++;
		}
		int32 counterXDef = 0;
		int32 counterYDef = 0;
		bool flagDef = false;
		for (const auto target : getData().classGameStatus.classBattle.classMapBattle.value().mapData)
		{
			for (const auto wid : target)
			{
				if (wid.boueiButaiNoIti == true)
				{
					flagDef = true;
					break;
				}
				else
				{
					counterYDef++;
				}
			}

			if (flagDef == true)
			{
				break;
			}
			counterYDef = 0;
			counterXDef++;
		}

		//始点設定
		viewPos = mapCreator.ToTileBottomCenter(Point(counterXSor, counterYSor), mapCreator.N);

		//ユニットの初期位置設定
		bool ran = true;
		for (auto& item : getData().classGameStatus.classBattle.sortieUnitGroup)
		{
			if (!item.FlagBuilding &&
				!item.ListClassUnit.empty())
			{
				for (auto& itemUnit : item.ListClassUnit)
				{
					Point pt = Point(counterXSor, counterYSor);
					Vec2 reV = mapCreator.ToTileBottomCenter(pt, mapCreator.N);
					if (ran == true)
					{
						itemUnit.nowPosiLeft = Vec2(reV.x + Random(-50, 50), reV.y + Random(0, 50));
					}
					else
					{
						itemUnit.nowPosiLeft = Vec2(reV.x, reV.y - itemUnit.TakasaUnit - 15);
					}
				}
			}
		}

		//ユニット体力バーの設定
		for (auto& item : getData().classGameStatus.classBattle.sortieUnitGroup)
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
		for (auto& item : getData().classGameStatus.classBattle.defUnitGroup)
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

		// buiの初期位置
		for (auto& item : getData().classGameStatus.classBattle.sortieUnitGroup)
		{
			if (item.FlagBuilding == true &&
				!item.ListClassUnit.empty())
			{
				for (auto& itemUnit : item.ListClassUnit)
				{
					Point pt = Point(itemUnit.rowBuilding, itemUnit.colBuilding);
					Vec2 vv = mapCreator.ToTileBottomCenter(pt, mapCreator.N);
					vv = { vv.x,vv.y - (25 + 15) };
					itemUnit.nowPosiLeft = vv;
				}
			}
		}
		for (auto& item : getData().classGameStatus.classBattle.defUnitGroup)
		{
			if (item.FlagBuilding == true &&
				!item.ListClassUnit.empty())
			{
				for (auto& itemUnit : item.ListClassUnit)
				{
					Point pt = Point(itemUnit.rowBuilding, itemUnit.colBuilding);
					Vec2 vv = mapCreator.ToTileBottomCenter(pt, mapCreator.N);
					vv = { vv.x,vv.y - (25 + 15) };
					itemUnit.nowPosiLeft = vv;
				}
			}
		}

		rtMap.clear(ColorF{ 0.5, 0.0 });
		rtMap = RenderTexture{ (uint32)(mapCreator.N * mapCreator.TileOffset.x) * 2,(uint32)(mapCreator.N * mapCreator.TileOffset.y) * 2 + 15, Palette::Red };
		{
			const auto tr = camera.createTransformer();
			const ScopedRenderTarget2D target{ rtMap };
			const ScopedRenderStates2D blend{ MakeBlendState() };

			//for (int32 i = 0; i < (mapCreator.N * 2 - 1); ++i)
			//{
			//	// x の開始インデックス
			//	const int32 xi = (i < (mapCreator.N - 1)) ? 0 : (i - (mapCreator.N - 1));

			//	// y の開始インデックス
			//	const int32 yi = (i < (mapCreator.N - 1)) ? i : (mapCreator.N - 1);

			//	// 左から順にタイルを描く
			//	for (int32 k = 0; k < (mapCreator.N - Abs(mapCreator.N - i - 1)); ++k)
			//	{
			//		// タイルのインデックス
			//		const Point index{ (xi + k), (yi - k) };

			//		// そのタイルの底辺中央の座標
			//		const int32 i = index.manhattanLength();
			//		const int32 xi = (i < (mapCreator.N - 1)) ? 0 : (i - (mapCreator.N - 1));
			//		const int32 yi = (i < (mapCreator.N - 1)) ? i : (mapCreator.N - 1);
			//		const int32 k2 = (index.manhattanDistanceFrom(Point{ xi, yi }) / 2);
			//		const double posX = ((i < (mapCreator.N - 1)) ? (i * -mapCreator.TileOffset.x) : ((i - 2 * mapCreator.N + 2) * mapCreator.TileOffset.x));
			//		const double posY = (i * mapCreator.TileOffset.y);
			//		Vec2 pos = { (posX + mapCreator.TileOffset.x * 2 * k2), posY };

			//		// 底辺中央を基準にタイルを描く
			//		String tip = getData().classGameStatus.classBattle.classMapBattle.value().mapData[index.x][index.y].tip;
			//		TextureAsset(tip + U".png").draw(Arg::bottomCenter = pos);

			//	}
			//}

		}


		renderTextureSkill = RenderTexture{ 320,320 };
		renderTextureSkill.clear(ColorF{ 0.5, 0.0 });
		{
			const ScopedRenderTarget2D target{ renderTextureSkill.clear(ColorF{ 0.8, 0.8, 0.8,0.5 }) };

			// 描画された最大のアルファ成分を保持するブレンドステート
			const ScopedRenderStates2D blend{ MakeBlendState() };

			//skill抽出
			Array<ClassSkill> table;
			for (auto& item : getData().classGameStatus.classBattle.sortieUnitGroup)
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
			table.sort_by([](const ClassSkill& a, const ClassSkill& b)
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
		renderTextureSkillUP.clear(ColorF{ 0.5, 0.0, 0.0, 0.0 });

		renderTextureSelektUnit = RenderTexture{ 160,200 };
		renderTextureSelektUnit.clear(ColorF{ 0.5, 0.0 });
		RectSelectUnit.push_back(Rect{ 4,4,200,40 });
		RectSelectUnit.push_back(Rect{ 4,44,160,40 });
		RectSelectUnit.push_back(Rect{ 4,84,160,40 });
		RectSelectUnit.push_back(Rect{ 4,124,160,40 });
		{
			const ScopedRenderTarget2D target{ renderTextureSelektUnit.clear(ColorF{ 0.8, 0.8, 0.8,0.5 }) };

			// 描画された最大のアルファ成分を保持するブレンドステート
			const ScopedRenderStates2D blend{ MakeBlendState() };

			for (auto&& [i, ttt] : Indexed(RectSelectUnit))
			{
				if (i == 0)
				{
					getData().fontMini(U"選択").draw(ttt, Palette::Black);
					continue;
				}
				else if (i == 1)
				{
					ttt.draw(Palette::Red);
					getData().fontMini(U"前衛").draw(ttt);
					continue;
				}
				else if (i == 2)
				{
					ttt.draw(Palette::Blue);
					getData().fontMini(U"後衛").draw(ttt);
					continue;
				}
				else if (i == 3)
				{
					ttt.draw(Palette::Aliceblue);
					getData().fontMini(U"騎兵").draw(ttt, Palette::Black);
					continue;
				}
			}

			Rect df = Rect(160, 200);
			df.drawFrame(4, 0, ColorF{ 0.5 });
		}

		rectZinkei.push_back(Rect{ 8,8,60,40 });
		rectZinkei.push_back(Rect{ 76,8,60,40 });
		rectZinkei.push_back(Rect{ 144,8,60,40 });
		renderTextureZinkei = RenderTexture{ 320,60 };
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
					ttt.draw(Palette::Aliceblue);
					getData().fontLine(U"密集").draw(ttt, Palette::Black);
					continue;
				}
				else if (i == 1)
				{
					ttt.draw(Palette::Aliceblue);
					getData().fontLine(U"横列").draw(ttt, Palette::Black);
					continue;
				}
				else if (i == 2)
				{
					ttt.draw(Palette::Aliceblue);
					getData().fontLine(U"正方").draw(ttt, Palette::Black);
					continue;
				}
			}
		}

		rectOrderSkill.push_back(Rect{ 8,8,80,40 });
		rectOrderSkill.push_back(Rect{ 96,8,60,40 });
		renderTextureOrderSkill = RenderTexture{ 320,60 };
		renderTextureOrderSkill.clear(ColorF{ 0.5, 0.0 });
		{
			const ScopedRenderTarget2D target{ renderTextureOrderSkill.clear(ColorF{ 0.8, 0.8, 0.8,0.5 }) };

			// 描画された最大のアルファ成分を保持するブレンドステート
			const ScopedRenderStates2D blend{ MakeBlendState() };

			Rect df = Rect(320, 60);
			df.drawFrame(4, 0, ColorF{ 0.5 });

			for (auto&& [i, ttt] : Indexed(rectOrderSkill))
			{
				if (i == 0)
				{
					ttt.draw(Palette::Aliceblue);
					getData().fontLine(U"必殺技").draw(ttt, Palette::Black);
					continue;
				}
				else if (i == 1)
				{
					ttt.draw(Palette::Aliceblue);
					getData().fontLine(U"通常").draw(ttt, Palette::Black);
					continue;
				}
			}

		}

		getData().classGameStatus.arrayBattleZinkei.push_back(false);
		getData().classGameStatus.arrayBattleZinkei.push_back(false);
		getData().classGameStatus.arrayBattleZinkei.push_back(false);
		getData().classGameStatus.arrayBattleCommand.push_back(false);
		getData().classGameStatus.arrayBattleCommand.push_back(false);

		task = Async(BattleMoveAStar,
				std::ref(getData().classGameStatus.classBattle.defUnitGroup),
				std::ref(getData().classGameStatus.classBattle.sortieUnitGroup),
				std::ref(mapCreator),
				std::ref(getData().classGameStatus.classBattle.classMapBattle.value().mapData),
				std::ref(getData().classGameStatus),
				std::ref(debugRoot), std::ref(debugAstar),
				std::ref(abort), std::ref(pauseTask));
	}
	~Battle()
	{
		abort = true;
		abortMyUnits = true;
	}
	// 更新関数（オプション）
	void update() override
	{
		Cursor::RequestStyle(U"MyCursor");

		// 2D カメラを更新する
		camera.update();

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

				//カメラ移動
				if (MouseL.pressed() == true)
				{
					const auto viewPos = (camera.getTargetCenter() - Cursor::Delta());
					camera.jumpTo(viewPos, camera.getTargetScale());
				}
				if (MouseR.pressed() == false && getData().classGameStatus.IsBattleMove == false)
				{
					if (MouseR.up() == false)
					{
						cursPos = Cursor::Pos();
					}
				}
				else if (MouseR.pressed() == false && getData().classGameStatus.IsBattleMove == true)
				{
					if (MouseR.up() == false)
					{
						cursPos = Cursor::Pos();
					}
				}
				else if (MouseR.down() == true && getData().classGameStatus.IsBattleMove == true)
				{
					cursPos = Cursor::Pos();
				}

				//部隊を選択状態にする。もしくは既に選択状態なら経路を算出する
				if (MouseR.up() == true)
				{
					Point start = cursPos;
					Point end = Cursor::Pos();

					//ターゲットを抽出
					Array<ClassHorizontalUnit> lisClassHorizontalUnit;
					switch (getData().classGameStatus.classBattle.battleWhichIsThePlayer)
					{
					case BattleWhichIsThePlayer::Sortie:
					{
						for (auto& temp : getData().classGameStatus.classBattle.sortieUnitGroup)
						{
							ClassHorizontalUnit chu;
							for (auto& temptemp : temp.ListClassUnit)
							{
								if (temptemp.mapTipObjectType == MapTipObjectType::GATE)
									continue;
								if (temptemp.mapTipObjectType == MapTipObjectType::WALL2)
									continue;

								if (temptemp.IsBattleEnable == true)
								{
									chu.ListClassUnit.push_back(temptemp);
								}
							}
							if (chu.ListClassUnit.size() > 0)
							{
								lisClassHorizontalUnit.push_back(chu);
							}
						}
						//lisClassHorizontalUnit = getData().classGameStatus.classBattle.sortieUnitGroup;
					}
					break;
					case BattleWhichIsThePlayer::Def:
						lisClassHorizontalUnit = getData().classGameStatus.classBattle.defUnitGroup;
						break;
					case BattleWhichIsThePlayer::None:
						//AI同士の戦いにフラグは立てない
						return;
					default:
						return;
					}

					if (getData().classGameStatus.IsBattleMove == true)
					{
						Array<ClassUnit*> lisUnit;
						for (auto& target : lisClassHorizontalUnit)
							for (auto& unit : target.ListClassUnit)
								if (unit.FlagMove == true && unit.IsBattleEnable == true)
									lisUnit.push_back(&unit);

						if (lisUnit.size() == 1)
						{
							ClassUnit* cu = lisUnit[0];
							// 移動先の座標算出
							Vec2 nor = Vec2(end - start).normalized();
							Vec2 moved = cu->nowPosiLeft + Vec2(nor.x * cu->Speed, nor.y * cu->Speed);
							// 移動先が有効かどうかチェック || 本来は経路探索で移動可能かどうか調べるべき
							auto index = mapCreator.ToIndex(moved, columnQuads, rowQuads);
							if (not index.has_value())
							{
								cu->FlagMove = false;
								return;
							}

							//移動
							ClassUnit& cuu = GetCU(cu->ID);
							cuu.vecMove = Vec2(cu->orderPosiLeft - cu->nowPosiLeft).normalized();
							cuu.orderPosiLeft = end;
							cuu.FlagMove = false;
							cuu.FlagMoving = true;
						}

						if (getData().classGameStatus.arrayBattleZinkei[0] == true)
						{
							//密集

							for (auto& target : lisClassHorizontalUnit)
								for (auto& unit : target.ListClassUnit)
								{
									ClassUnit& cuu = GetCU(unit.ID);
									cuu.orderPosiLeft = end.movedBy(Random(-10, 10), Random(-10, 10));
									cuu.orderPosiLeftLast = cuu.orderPosiLeft;
									cuu.FlagMove = false;
									cuu.FlagMoveAI = true;
								}
						}
						else if (getData().classGameStatus.arrayBattleZinkei[1] == true)
						{
							//横列

							ClassHorizontalUnit liZenei;
							for (auto& target : lisClassHorizontalUnit)
								for (auto& unit : target.ListClassUnit)
								{
									if (unit.Formation == BattleFormation::F && unit.FlagMove == true && unit.IsBattleEnable == true)
										liZenei.ListClassUnit.push_back(unit);
								}

							ClassHorizontalUnit liKouei;
							for (auto& target : lisClassHorizontalUnit)
								for (auto& unit : target.ListClassUnit)
								{
									if (unit.Formation == BattleFormation::B && unit.FlagMove == true && unit.IsBattleEnable == true)
										liKouei.ListClassUnit.push_back(unit);
								}

							ClassHorizontalUnit liKihei;
							for (auto& target : lisClassHorizontalUnit)
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
								Array<ClassUnit*> target;
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
												* (getData().classGameStatus.DistanceBetweenUnit * Math::Cos(angle))
												)
											-
											(i * (getData().classGameStatus.DistanceBetweenUnitTate * Math::Cos(angle2)));
										//py+(b-切り捨て商)＊dsinθ-a＊d'sinθ’
										double yPos = end.y
											- (
											(ii - (result))
											* (getData().classGameStatus.DistanceBetweenUnit * Math::Sin(angle))

											)
											-
											(i * (getData().classGameStatus.DistanceBetweenUnitTate * Math::Sin(angle2)));

										ClassUnit& cuu = GetCU(unit->ID);
										cuu.orderPosiLeft = Vec2(xPos, yPos);
										cuu.orderPosiLeftLast = Vec2(xPos, yPos);
										cuu.FlagMove = false;
										cuu.FlagMoveAI = true;

										//unit->orderPosiLeft = Vec2(xPos, yPos);
										//unit->orderPosiLeftLast = Vec2(xPos, yPos);
										//unit->FlagMove = false;
										//unit->FlagMoveAI = true;

										auto index = mapCreator.ToIndex(unit->orderPosiLeft, columnQuads, rowQuads);
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
												* (getData().classGameStatus.DistanceBetweenUnit * Math::Cos(angle))
												)
											-
											(i * (getData().classGameStatus.DistanceBetweenUnitTate * Math::Cos(angle2)));
										//py+(b-切り捨て商)＊dsinθ-a＊d'sinθ’
										double yPos = end.y
											- (
											(ii - (result))
											* (getData().classGameStatus.DistanceBetweenUnit * Math::Sin(angle))

											)
											-
											(i * (getData().classGameStatus.DistanceBetweenUnitTate * Math::Sin(angle2)));

										ClassUnit& cuu = GetCU(unit->ID);
										cuu.orderPosiLeft = Vec2(xPos, yPos);
										cuu.orderPosiLeftLast = Vec2(xPos, yPos);
										cuu.FlagMove = false;
										cuu.FlagMoveAI = true;

										//unit->orderPosiLeft = Vec2(xPos, yPos);
										//unit->orderPosiLeftLast = Vec2(xPos, yPos);
										//unit->FlagMove = false;
										//unit->FlagMoveAI = true;

										auto index = mapCreator.ToIndex(unit->orderPosiLeft, columnQuads, rowQuads);
									}
								}
							}
						}
						else
						{
							//正方

							Array<ClassHorizontalUnit> lisClassHorizontalUnitLoop;
							for (auto& target : lisClassHorizontalUnit)
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
								Array<ClassUnit*> target;
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
												* (getData().classGameStatus.DistanceBetweenUnit * Math::Cos(angle))
												)
											-
											(i * (getData().classGameStatus.DistanceBetweenUnitTate * Math::Cos(angle2)));
										//py+(b-切り捨て商)＊dsinθ-a＊d'sinθ’
										double yPos = end.y
											- (
											(ii - (result))
											* (getData().classGameStatus.DistanceBetweenUnit * Math::Sin(angle))

											)
											-
											(i * (getData().classGameStatus.DistanceBetweenUnitTate * Math::Sin(angle2)));

										ClassUnit& cuu = GetCU(unit->ID);
										cuu.orderPosiLeft = Vec2(xPos, yPos);
										cuu.orderPosiLeftLast = Vec2(xPos, yPos);
										cuu.FlagMove = false;
										cuu.FlagMoveAI = true;

										//unit->orderPosiLeft = Vec2(xPos, yPos);
										//unit->orderPosiLeftLast = Vec2(xPos, yPos);
										//unit->FlagMove = false;
										//unit->FlagMoveAI = true;

										auto index = mapCreator.ToIndex(unit->orderPosiLeft, columnQuads, rowQuads);
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
												* (getData().classGameStatus.DistanceBetweenUnit * Math::Cos(angle))
												)
											-
											(i * (getData().classGameStatus.DistanceBetweenUnitTate * Math::Cos(angle2)));
										//py+(b-切り捨て商)＊dsinθ-a＊d'sinθ’
										double yPos = end.y
											- (
											(ii - (result))
											* (getData().classGameStatus.DistanceBetweenUnit * Math::Sin(angle))

											)
											-
											(i * (getData().classGameStatus.DistanceBetweenUnitTate * Math::Sin(angle2)));

										ClassUnit& cuu = GetCU(unit->ID);
										cuu.orderPosiLeft = Vec2(xPos, yPos);
										cuu.orderPosiLeftLast = Vec2(xPos, yPos);
										cuu.FlagMove = false;
										cuu.FlagMoveAI = true;

										//unit->orderPosiLeft = Vec2(xPos, yPos);
										//unit->orderPosiLeftLast = Vec2(xPos, yPos);
										//unit->FlagMove = false;
										//unit->FlagMoveAI = true;

										auto index = mapCreator.ToIndex(unit->orderPosiLeft, columnQuads, rowQuads);
									}
								}
							}
						}

						getData().classGameStatus.IsBattleMove = false;

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
										std::ref(getData().classGameStatus.classBattle.sortieUnitGroup),
										std::ref(getData().classGameStatus.classBattle.defUnitGroup),
										std::ref(mapCreator),
										std::ref(getData().classGameStatus.classBattle.classMapBattle.value().mapData),
										std::ref(getData().classGameStatus),
										std::ref(debugRoot), std::ref(debugAstar),
										std::ref(abortMyUnits), std::ref(pauseTaskMyUnits));
					}
					else
					{
						//範囲選択
						for (auto& target : lisClassHorizontalUnit)
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
											ClassUnit& cuu = GetCU(unit.ID);
											cuu.FlagMove = true;
											getData().classGameStatus.IsBattleMove = true;
										}
										else
										{
											ClassUnit& cuu = GetCU(unit.ID);
											cuu.FlagMove = false;
										}
									}
									else
									{
										//下
										if (gnpc.x >= end.x && gnpc.x <= start.x
											&& gnpc.y >= start.y && gnpc.y <= end.y)
										{
											ClassUnit& cuu = GetCU(unit.ID);
											cuu.FlagMove = true;
											getData().classGameStatus.IsBattleMove = true;
										}
										else
										{
											ClassUnit& cuu = GetCU(unit.ID);
											cuu.FlagMove = false;
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
											ClassUnit& cuu = GetCU(unit.ID);
											cuu.FlagMove = true;
											getData().classGameStatus.IsBattleMove = true;
										}
										else
										{
											ClassUnit& cuu = GetCU(unit.ID);
											cuu.FlagMove = false;
										}
									}
									else
									{
										//下
										if (gnpc.x >= start.x && gnpc.x <= end.x
											&& gnpc.y >= start.y && gnpc.y <= end.y)
										{
											ClassUnit& cuu = GetCU(unit.ID);
											cuu.FlagMove = true;
											getData().classGameStatus.IsBattleMove = true;
										}
										else
										{
											ClassUnit& cuu = GetCU(unit.ID);
											cuu.FlagMove = false;
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
						for (auto& item : getData().classGameStatus.classBattle.sortieUnitGroup)
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

						while (not getData().fontMiniMini(nowSelectSkillSetumei).draw(rectSkillSetumei.stretched(-12), ColorF{ 0.0 }))
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

			//移動指定
			{
				const Transformer2D transformer{ Mat3x2::Identity(), Mat3x2::Translate(316, WINDOWSIZEHEIGHT000 - 200) };

				for (auto&& [i, re] : Indexed(RectSelectUnit))
				{
					if (re.leftClicked())
					{
						BattleFormation bbb = BattleFormation::F;
						if (i == 1)
						{
							bbb = BattleFormation::F;
						}
						else if (i == 2)
						{
							bbb = BattleFormation::B;
						}
						else if (i == 3)
						{
							bbb = BattleFormation::M;
						}

						//ターゲットを抽出
						Array<ClassHorizontalUnit> lisClassHorizontalUnit;
						switch (getData().classGameStatus.classBattle.battleWhichIsThePlayer)
						{
						case BattleWhichIsThePlayer::Sortie:
						{
							for (auto& temp : getData().classGameStatus.classBattle.sortieUnitGroup)
							{
								ClassHorizontalUnit chu;
								for (auto& temptemp : temp.ListClassUnit)
								{
									if (temptemp.mapTipObjectType == MapTipObjectType::GATE)
										continue;
									if (temptemp.mapTipObjectType == MapTipObjectType::WALL2)
										continue;

									if (temptemp.IsBattleEnable == true)
									{
										chu.ListClassUnit.push_back(temptemp);
									}
								}
								if (chu.ListClassUnit.size() > 0)
								{
									lisClassHorizontalUnit.push_back(chu);
								}
							}
							//lisClassHorizontalUnit = getData().classGameStatus.classBattle.sortieUnitGroup;
						}
						break;
						case BattleWhichIsThePlayer::Def:
							lisClassHorizontalUnit = getData().classGameStatus.classBattle.defUnitGroup;
							break;
						case BattleWhichIsThePlayer::None:
							//AI同士の戦いにフラグは立てない
							return;
						default:
							return;
						}

						for (auto& target : lisClassHorizontalUnit)
						{
							for (auto& unit : target.ListClassUnit)
							{
								if (unit.Formation == bbb)
								{
									ClassUnit& cuu = GetCU(unit.ID);
									cuu.FlagMove = true;
									getData().classGameStatus.IsBattleMove = true;
								}
							}
						}
					}
				}
			}

			//陣形処理
			{
				const Transformer2D transformer{ Mat3x2::Identity(), Mat3x2::Translate(0,Scene::Size().y - 440 - 30) };

				for (auto&& [j, ttt] : Indexed(rectZinkei))
				{
					if (ttt.leftClicked())
					{
						getData().classGameStatus.arrayBattleZinkei.clear();
						for (size_t k = 0; k < rectZinkei.size(); k++)
						{
							getData().classGameStatus.arrayBattleZinkei.push_back(false);
						}
						getData().classGameStatus.arrayBattleZinkei[j] = true;

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
									getData().fontLine(U"密集").draw(ttt, Palette::Black);
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
									getData().fontLine(U"横列").draw(ttt, Palette::Black);
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
									getData().fontLine(U"正方").draw(ttt, Palette::Black);
									continue;
								}
							}
						}

					}
				}
			}

			//コマンド処理
			{
				const Transformer2D transformer{ Mat3x2::Identity(), Mat3x2::Translate(0,Scene::Size().y - 380 - 30) };

				for (auto&& [j, ttt] : Indexed(rectOrderSkill))
				{
					if (ttt.leftClicked())
					{
						getData().classGameStatus.arrayBattleCommand.clear();
						for (size_t k = 0; k < rectOrderSkill.size(); k++)
						{
							getData().classGameStatus.arrayBattleCommand.push_back(false);
						}
						getData().classGameStatus.arrayBattleCommand[j] = true;

						renderTextureOrderSkill.clear(ColorF{ 0.5, 0.0 });
						{
							const ScopedRenderTarget2D target{ renderTextureOrderSkill.clear(ColorF{ 0.8, 0.8, 0.8,0.5 }) };

							// 描画された最大のアルファ成分を保持するブレンドステート
							const ScopedRenderStates2D blend{ MakeBlendState() };

							Rect df = Rect(320, 60);
							df.drawFrame(4, 0, ColorF{ 0.5 });

							for (auto&& [i, ttt] : Indexed(rectOrderSkill))
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
									getData().fontLine(U"必殺技").draw(ttt, Palette::Black);
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
									getData().fontLine(U"通常").draw(ttt, Palette::Black);
									continue;
								}
							}

						}

					}
				}
			}

			//pause処理
			{
				if (KeySpace.down())
				{
					PauseFlag = !PauseFlag;
					if (PauseFlag == false)
					{
						pauseTask = false;
						pauseTaskMyUnits = false;
					}
					else
					{
						pauseTask = true;
						pauseTaskMyUnits = true;
						return;
					}
				}
				if (PauseFlag == true)
					return;
			}

			//移動処理
			{
				for (auto& item : getData().classGameStatus.classBattle.defUnitGroup)
				{
					for (auto& itemUnit : item.ListClassUnit)
					{
						if (itemUnit.IsBuilding == true && itemUnit.mapTipObjectType == MapTipObjectType::WALL2)
						{
							continue;
						}
						if (itemUnit.IsBattleEnable == false)
						{
							continue;
						}
						if (itemUnit.FlagMoving == true)
						{
							itemUnit.nowPosiLeft = itemUnit.nowPosiLeft + (itemUnit.vecMove * ((itemUnit.Move + itemUnit.cts.Speed) / 100));

							Circle c = { itemUnit.nowPosiLeft ,1 };
							Circle cc = { itemUnit.orderPosiLeft ,1 };

							if (c.intersects(cc))
							{
								itemUnit.FlagMoving = false;
							}
							//if (itemUnit.nowPosiLeft.x <= itemUnit.orderPosiLeft.x + 10 && itemUnit.nowPosiLeft.x >= itemUnit.orderPosiLeft.x - 10
							//	&& itemUnit.nowPosiLeft.y <= itemUnit.orderPosiLeft.y + 10 && itemUnit.nowPosiLeft.y >= itemUnit.orderPosiLeft.y - 10)
							//{
							//	itemUnit.FlagMoving = false;
							//}
							continue;
						}
						//auto rootPo = getData().classGameStatus.aiRoot;
						//for (auto [key, value] : rootPo)
						//{
						//	Print << key << U": " << value;
						//}
						if (getData().classGameStatus.aiRoot[itemUnit.ID].isEmpty() == true)
						{
							continue;
						}
						if (getData().classGameStatus.aiRoot[itemUnit.ID].size() == 1)
						{
							itemUnit.FlagMovingEnd = true;
							itemUnit.FlagMoving = false;
							continue;
						}

						// タイルのインデックス
						Point index;
						try
						{
							getData().classGameStatus.aiRoot[itemUnit.ID].pop_front();
							auto rthrthrt = getData().classGameStatus.aiRoot[itemUnit.ID];
							if (rthrthrt.size() > 0)
							{
								index = rthrthrt[0];
							}
						}
						catch (const std::exception&)
						{
							continue;
						}

						// そのタイルの底辺中央の座標
						const int32 i = index.manhattanLength();
						const int32 xi = (i < (mapCreator.N - 1)) ? 0 : (i - (mapCreator.N - 1));
						const int32 yi = (i < (mapCreator.N - 1)) ? i : (mapCreator.N - 1);
						const int32 k2 = (index.manhattanDistanceFrom(Point{ xi, yi }) / 2);
						const double posX = ((i < (mapCreator.N - 1)) ? (i * -mapCreator.TileOffset.x) : ((i - 2 * mapCreator.N + 2) * mapCreator.TileOffset.x));
						const double posY = (i * mapCreator.TileOffset.y) - mapCreator.TileThickness;
						const Vec2 pos = { (posX + mapCreator.TileOffset.x * 2 * k2) - (itemUnit.yokoUnit / 2), posY - itemUnit.TakasaUnit - 15 };

						itemUnit.orderPosiLeft = pos;
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
				for (auto& item : getData().classGameStatus.classBattle.sortieUnitGroup)
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


						if (getData().classGameStatus.aiRoot[itemUnit.ID].isEmpty() == true)
							continue;


						//潜在的バグ有り
						if (getData().classGameStatus.aiRoot[itemUnit.ID].size() == 1)
						{
							itemUnit.FlagMovingEnd = true;
							itemUnit.FlagMoving = false;
							continue;
						}

						// タイルのインデックス
						Point index;
						try
						{
							getData().classGameStatus.aiRoot[itemUnit.ID].pop_front();
							auto rthrthrt = getData().classGameStatus.aiRoot[itemUnit.ID];
							index = getData().classGameStatus.aiRoot[itemUnit.ID][0];
						}
						catch (const std::exception&)
						{
							throw;
							continue;
						}

						if (getData().classGameStatus.aiRoot[itemUnit.ID].size() == 1)
						{
							itemUnit.orderPosiLeft = itemUnit.orderPosiLeftLast;
						}
						else
						{
							// そのタイルの底辺中央の座標
							const int32 i = index.manhattanLength();
							const int32 xi = (i < (mapCreator.N - 1)) ? 0 : (i - (mapCreator.N - 1));
							const int32 yi = (i < (mapCreator.N - 1)) ? i : (mapCreator.N - 1);
							const int32 k2 = (index.manhattanDistanceFrom(Point{ xi, yi }) / 2);
							const double posX = ((i < (mapCreator.N - 1)) ? (i * -mapCreator.TileOffset.x) : ((i - 2 * mapCreator.N + 2) * mapCreator.TileOffset.x));
							const double posY = (i * mapCreator.TileOffset.y) - mapCreator.TileThickness;
							const Vec2 pos = { (posX + mapCreator.TileOffset.x * 2 * k2) - (itemUnit.yokoUnit / 2), posY - itemUnit.TakasaUnit - 15 };

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
				for (auto& item : getData().classGameStatus.classBattle.neutralUnitGroup)
				{
					for (auto& itemUnit : item.ListClassUnit)
					{
						if (itemUnit.IsBuilding == true && itemUnit.mapTipObjectType == MapTipObjectType::WALL2)
						{
							continue;
						}
						if (itemUnit.IsBattleEnable == false)
						{
							continue;
						}
						if (itemUnit.FlagMoving == true)
						{
							itemUnit.nowPosiLeft = itemUnit.nowPosiLeft + (itemUnit.vecMove * (itemUnit.Move / 100));

							Circle c = { itemUnit.nowPosiLeft ,1 };
							Circle cc = { itemUnit.orderPosiLeft ,1 };

							if (c.intersects(cc))
							{
								itemUnit.FlagMoving = false;
							}
							continue;
						}
						if (getData().classGameStatus.aiRoot[itemUnit.ID].isEmpty() == true)
						{
							continue;
						}
						if (getData().classGameStatus.aiRoot[itemUnit.ID].size() == 1)
						{
							itemUnit.FlagMovingEnd = true;
							itemUnit.FlagMoving = false;
							continue;
						}

						// タイルのインデックス
						Point index;
						try
						{
							getData().classGameStatus.aiRoot[itemUnit.ID].pop_front();
							auto rthrthrt = getData().classGameStatus.aiRoot[itemUnit.ID];
							index = getData().classGameStatus.aiRoot[itemUnit.ID][0];
						}
						catch (const std::exception&)
						{
							throw;
							continue;
						}

						// そのタイルの底辺中央の座標
						const int32 i = index.manhattanLength();
						const int32 xi = (i < (mapCreator.N - 1)) ? 0 : (i - (mapCreator.N - 1));
						const int32 yi = (i < (mapCreator.N - 1)) ? i : (mapCreator.N - 1);
						const int32 k2 = (index.manhattanDistanceFrom(Point{ xi, yi }) / 2);
						const double posX = ((i < (mapCreator.N - 1)) ? (i * -mapCreator.TileOffset.x) : ((i - 2 * mapCreator.N + 2) * mapCreator.TileOffset.x));
						const double posY = (i * mapCreator.TileOffset.y) - mapCreator.TileThickness;
						const Vec2 pos = { (posX + mapCreator.TileOffset.x * 2 * k2) - (itemUnit.yokoUnit / 2), posY - itemUnit.TakasaUnit - 15 };

						itemUnit.orderPosiLeft = pos;
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
			for (auto& item : getData().classGameStatus.classBattle.sortieUnitGroup)
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
			for (auto& item : getData().classGameStatus.classBattle.defUnitGroup)
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

			//skill処理
			SkillProcess(getData().classGameStatus.classBattle.sortieUnitGroup, getData().classGameStatus.classBattle.defUnitGroup, m_Battle_player_skills);
			SkillProcess(getData().classGameStatus.classBattle.defUnitGroup, getData().classGameStatus.classBattle.sortieUnitGroup, m_Battle_enemy_skills);

			//skill実行処理
			const double time = Scene::Time();
			{
				Array<ClassExecuteSkills> deleteCES;
				for (ClassExecuteSkills& loop_Battle_player_skills : m_Battle_player_skills)
				{
					Array<int32> arrayNo;
					for (auto& target : loop_Battle_player_skills.ArrayClassBullet)
					{
						target.lifeTime += Scene::DeltaTime();

						switch (loop_Battle_player_skills.classSkill.SkillType)
						{
						case SkillType::missileAdd:
						{
							//時間が過ぎたら補正を終了する
							if (target.lifeTime > target.duration)
							{
								loop_Battle_player_skills.classUnit->cts.Speed = 0.0;
								loop_Battle_player_skills.classUnit->cts.plus_speed_time = 0.0;
							}
						}
						break;
						default:
							break;
						}

						if (target.lifeTime > target.duration)
						{
							loop_Battle_player_skills.classUnit->FlagMovingSkill = false;

							//消滅
							arrayNo.push_back(target.No);

							break;
						}
						else
						{
							//イージングによって速度を変更させる
							if (loop_Battle_player_skills.classSkill.Easing.has_value() == true)
							{
								// 移動の割合 0.0～1.0
								const double t = Min(target.stopwatch.sF(), 1.0);
								switch (loop_Battle_player_skills.classSkill.Easing.value())
								{
								case SkillEasing::easeOutExpo:
								{
									const double ea = EaseOutExpo(t);
									target.NowPosition = Vec2((target.NowPosition.x + (target.MoveVec.x * (loop_Battle_player_skills.classSkill.speed / 100) * (ea * loop_Battle_player_skills.classSkill.EasingRatio))),
																(target.NowPosition.y + (target.MoveVec.y * (loop_Battle_player_skills.classSkill.speed / 100) * (ea * loop_Battle_player_skills.classSkill.EasingRatio))));
								}
								break;
								default:
									break;
								}
							}
							else
							{
								switch (loop_Battle_player_skills.classSkill.MoveType)
								{
								case MoveType::line:
								{
									if (loop_Battle_player_skills.classSkill.SkillCenter == SkillCenter::end)
									{
										Vec2 offPos = { -1,-1 };
										for (size_t i = 0; i < getData().classGameStatus.classBattle.sortieUnitGroup.size(); i++)
										{
											for (size_t j = 0; j < getData().classGameStatus.classBattle.sortieUnitGroup[i].ListClassUnit.size(); j++)
											{
												if (loop_Battle_player_skills.UnitID == getData().classGameStatus.classBattle.sortieUnitGroup[i].ListClassUnit[j].ID)
												{
													offPos = getData().classGameStatus.classBattle.sortieUnitGroup[i].ListClassUnit[j].GetNowPosiCenter();
												}
											}
										}
										target.NowPosition = offPos;
									}
									else
									{
										target.NowPosition = Vec2((target.NowPosition.x + (target.MoveVec.x * (loop_Battle_player_skills.classSkill.speed / 100))),
																	(target.NowPosition.y + (target.MoveVec.y * (loop_Battle_player_skills.classSkill.speed / 100))));
									}
								}
								break;
								case MoveType::circle:
								{
									Vec2 offPos = { -1,-1 };
									for (size_t i = 0; i < getData().classGameStatus.classBattle.sortieUnitGroup.size(); i++)
									{
										for (size_t j = 0; j < getData().classGameStatus.classBattle.sortieUnitGroup[i].ListClassUnit.size(); j++)
										{
											if (loop_Battle_player_skills.UnitID == getData().classGameStatus.classBattle.sortieUnitGroup[i].ListClassUnit[j].ID)
											{
												offPos = getData().classGameStatus.classBattle.sortieUnitGroup[i].ListClassUnit[j].GetNowPosiCenter();
											}
										}
									}

									const double theta = (target.RushNo * 60_deg + time * (loop_Battle_player_skills.classSkill.speed * Math::Pi / 180.0));
									const Vec2 pos = OffsetCircular{ offPos, loop_Battle_player_skills.classSkill.radius, theta };

									target.NowPosition = pos;
								}
								break;
								case MoveType::swing:
								{
									const float degg = target.degree + (loop_Battle_player_skills.classSkill.speed / 100);
									if (loop_Battle_player_skills.classSkill.range + target.initDegree > degg)
									{
										//範囲内
										target.degree = degg;
										target.radian = ToRadians(target.degree);
									}
									else
									{

									}
								}
								break;
								default:
									target.NowPosition = Vec2((target.NowPosition.x + (target.MoveVec.x * (loop_Battle_player_skills.classSkill.speed / 100))),
																(target.NowPosition.y + (target.MoveVec.y * (loop_Battle_player_skills.classSkill.speed / 100))));
									break;
								}
							}
						}

						//衝突したらunitのHPを減らし、消滅
						RectF rrr = { Arg::bottomCenter(target.NowPosition),(double)loop_Battle_player_skills.classSkill.w,(double)loop_Battle_player_skills.classSkill.h };
						TexturedCollider tc1 = { rrr };
						TexturedCollider tc2 = { Circle{ target.NowPosition.x,target.NowPosition.y,loop_Battle_player_skills.classSkill.w / 2 } };

						bool bombCheck = false;
						if (loop_Battle_player_skills.classSkill.SkillType == SkillType::heal)
						{
							ColliderCheckHeal(rrr, target, loop_Battle_player_skills, arrayNo, loop_Battle_player_skills.classUnitHealTarget);
						}
						else
						{
							ColliderCheck(rrr, target, loop_Battle_player_skills, arrayNo, getData().classGameStatus.classBattle.defUnitGroup);
						}
					}

					loop_Battle_player_skills.ArrayClassBullet.remove_if([&](const ClassBullets& cb)
						{
							if (arrayNo.includes(cb.No))
							{
								//Print << U"suc";
								return true;
							}
							else
							{
								//Print << U"no";
								return false;
							}
						});
					arrayNo.clear();
				}
				m_Battle_player_skills.remove_if([&](const ClassExecuteSkills& a) { return a.ArrayClassBullet.size() == 0; });
			}
			{
				Array<ClassExecuteSkills> deleteCES;
				for (ClassExecuteSkills& loop_Battle_player_skills : m_Battle_enemy_skills)
				{
					Array<int32> arrayNo;
					for (auto& target : loop_Battle_player_skills.ArrayClassBullet)
					{
						target.lifeTime += Scene::DeltaTime();

						if (target.lifeTime > target.duration)
						{
							loop_Battle_player_skills.classUnit->FlagMovingSkill = false;

							//消滅
							arrayNo.push_back(target.No);

							break;
						}
						else
						{
							//イージングによって速度を変更させる
							if (loop_Battle_player_skills.classSkill.Easing.has_value() == true)
							{
								// 移動の割合 0.0～1.0
								const double t = Min(target.stopwatch.sF(), 1.0);
								switch (loop_Battle_player_skills.classSkill.Easing.value())
								{
								case SkillEasing::easeOutExpo:
								{
									const double ea = EaseOutExpo(t);
									target.NowPosition = Vec2((target.NowPosition.x + (target.MoveVec.x * (loop_Battle_player_skills.classSkill.speed / 100) * (ea * loop_Battle_player_skills.classSkill.EasingRatio))),
																(target.NowPosition.y + (target.MoveVec.y * (loop_Battle_player_skills.classSkill.speed / 100) * (ea * loop_Battle_player_skills.classSkill.EasingRatio))));
								}
								break;
								default:
									break;
								}
							}
							else
							{
								switch (loop_Battle_player_skills.classSkill.MoveType)
								{
								case MoveType::line:
								{
									if (loop_Battle_player_skills.classSkill.SkillCenter == SkillCenter::end)
									{
										Vec2 offPos = { -1,-1 };
										for (size_t i = 0; i < getData().classGameStatus.classBattle.defUnitGroup.size(); i++)
										{
											for (size_t j = 0; j < getData().classGameStatus.classBattle.defUnitGroup[i].ListClassUnit.size(); j++)
											{
												if (loop_Battle_player_skills.UnitID == getData().classGameStatus.classBattle.defUnitGroup[i].ListClassUnit[j].ID)
												{
													offPos = getData().classGameStatus.classBattle.defUnitGroup[i].ListClassUnit[j].GetNowPosiCenter();
												}
											}
										}
										target.NowPosition = offPos;
									}
									else
									{
										target.NowPosition = Vec2((target.NowPosition.x + (target.MoveVec.x * (loop_Battle_player_skills.classSkill.speed / 100))),
																	(target.NowPosition.y + (target.MoveVec.y * (loop_Battle_player_skills.classSkill.speed / 100))));
									}
								}
								break;
								case MoveType::circle:
								{
									Vec2 offPos = { -1,-1 };
									for (size_t i = 0; i < getData().classGameStatus.classBattle.defUnitGroup.size(); i++)
									{
										for (size_t j = 0; j < getData().classGameStatus.classBattle.defUnitGroup[i].ListClassUnit.size(); j++)
										{
											if (loop_Battle_player_skills.UnitID == getData().classGameStatus.classBattle.defUnitGroup[i].ListClassUnit[j].ID)
											{
												offPos = getData().classGameStatus.classBattle.defUnitGroup[i].ListClassUnit[j].GetNowPosiCenter();
											}
										}
									}

									const double theta = (target.RushNo * 60_deg + time * (loop_Battle_player_skills.classSkill.speed * Math::Pi / 180.0));
									const Vec2 pos = OffsetCircular{ offPos, loop_Battle_player_skills.classSkill.radius, theta };

									target.NowPosition = pos;
								}
								break;
								case MoveType::swing:
								{
									const float degg = target.degree + (loop_Battle_player_skills.classSkill.speed / 100);
									if (loop_Battle_player_skills.classSkill.range + target.initDegree > degg)
									{
										//範囲内
										target.degree = degg;
										target.radian = ToRadians(target.degree);
									}
									else
									{

									}
								}
								break;
								default:
									target.NowPosition = Vec2((target.NowPosition.x + (target.MoveVec.x * (loop_Battle_player_skills.classSkill.speed / 100))),
																(target.NowPosition.y + (target.MoveVec.y * (loop_Battle_player_skills.classSkill.speed / 100))));
									break;
								}
							}
						}

						//衝突したらunitのHPを減らし、消滅
						RectF rrr = { Arg::bottomCenter(target.NowPosition),(double)loop_Battle_player_skills.classSkill.w,(double)loop_Battle_player_skills.classSkill.h };
						TexturedCollider tc1 = { rrr };
						TexturedCollider tc2 = { Circle{ target.NowPosition.x,target.NowPosition.y,loop_Battle_player_skills.classSkill.w / 2 } };

						bool bombCheck = false;
						if (loop_Battle_player_skills.classSkill.SkillType == SkillType::heal)
						{
							ColliderCheck(rrr, target, loop_Battle_player_skills, arrayNo, getData().classGameStatus.classBattle.defUnitGroup);
						}
						else
						{
							ColliderCheck(rrr, target, loop_Battle_player_skills, arrayNo, getData().classGameStatus.classBattle.sortieUnitGroup);
						}
					}
					loop_Battle_player_skills.ArrayClassBullet.remove_if([&](const ClassBullets& cb)
						{
							if (arrayNo.includes(cb.No))
							{
								//Print << U"suc";
								return true;
							}
							else
							{
								//Print << U"no";
								return false;
							}
						});
					arrayNo.clear();
				}
				m_Battle_enemy_skills.remove_if([&](const ClassExecuteSkills& a) { return a.ArrayClassBullet.size() == 0; });
			}

			//体力が無くなったunit削除処理
			for (auto& item : getData().classGameStatus.classBattle.sortieUnitGroup)
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
			for (auto& item : getData().classGameStatus.classBattle.defUnitGroup)
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
			for (auto& item : getData().classGameStatus.classBattle.neutralUnitGroup)
			{
				for (auto& itemUnit : item.ListClassUnit)
				{
					if (itemUnit.isValidBuilding() == true)
					{
						if (itemUnit.HPCastle <= 0)
						{
							itemUnit.IsBattleEnable = false;
						}
					}
					else
					{
						if (itemUnit.Hp <= 0)
						{
							itemUnit.IsBattleEnable = false;
						}
					}
				}
			}

			//戦闘終了条件を確認
			int32 countSortieUnitGroup = 0;
			for (auto& item : getData().classGameStatus.classBattle.sortieUnitGroup)
			{
				for (auto& itemListClassUnit : item.ListClassUnit)
				{
					if (itemListClassUnit.IsBattleEnable == true)
					{
						countSortieUnitGroup++;
					}
				}
			}

			if (countSortieUnitGroup == 0)
			{
				changeScene(U"Buy", 0.9s);
			}

			int32 countDefUnitGroup = 0;
			for (auto& item : getData().classGameStatus.classBattle.defUnitGroup)
			{
				for (auto& itemListClassUnit : item.ListClassUnit)
				{
					if (itemListClassUnit.IsBattleEnable == true)
						countDefUnitGroup++;
				}
			}

			if (countDefUnitGroup == 0)
			{
				getData().NovelNumber = getData().NovelNumber + 1;
				getData().Wave = getData().Wave + 1;

				changeScene(U"Card", 0.9s);
			}
		}
		break;
		case BattleStatus::Message:
		{
			const auto t = camera.createTransformer();

			if (BattleMessage001)
			{
				sceneMessageBoxImpl.set(camera);
				if (sceneMessageBoxImpl.m_buttonC.mouseOver())
				{
					Cursor::RequestStyle(CursorStyle::Hand);

					if (MouseL.down())
					{
						BattleMessage001 = false;
						battleStatus = BattleStatus::Battle;

						camera.jumpTo(viewPos, camera.getTargetScale());
					}
				}
			}
		}
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
	}

	void ColliderCheck(RectF rrr, ClassBullets& target, ClassExecuteSkills& loop_Battle_player_skills, Array<int32>& arrayNo, Array<ClassHorizontalUnit>& chu)
	{
		TexturedCollider tc1 = { rrr };
		TexturedCollider tc2 = { Circle{ target.NowPosition.x,target.NowPosition.y,loop_Battle_player_skills.classSkill.w / 2 } };

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
						vv = mapCreator.ToTileBottomCenter(pt, mapCreator.N);
						vv = { vv.x,vv.y - (25 + 15) };
					}
					break;
					default:
					{
						Point pt = Point(itemTarget.rowBuilding, itemTarget.colBuilding);
						vv = mapCreator.ToTileBottomCenter(pt, mapCreator.N);
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
					&& tc1.Collider.rotatedAt(tc1.Collider.bottomCenter(), target.radian + Math::ToRadians(90)).intersects(cTar) == true)
				{
					if (ProcessCollid(bombCheck, arrayNo, target, loop_Battle_player_skills, itemTarget))break;
				}
				else if (tc2.Collider.intersects(cTar) == true)
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
	void ColliderCheckHeal(RectF rrr, ClassBullets& target, ClassExecuteSkills& loop_Battle_player_skills, Array<int32>& arrayNo, ClassUnit* itemTarget)
	{
		TexturedCollider tc1 = { rrr };
		TexturedCollider tc2 = { Circle{ target.NowPosition.x,target.NowPosition.y,loop_Battle_player_skills.classSkill.w / 2 } };

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
				vv = mapCreator.ToTileBottomCenter(pt, mapCreator.N);
				vv = { vv.x,vv.y - (25 + 15) };
			}
			break;
			default:
			{
				Point pt = Point(itemTarget->rowBuilding, itemTarget->colBuilding);
				vv = mapCreator.ToTileBottomCenter(pt, mapCreator.N);
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
			&& tc1.Collider.rotatedAt(tc1.Collider.bottomCenter(), target.radian + Math::ToRadians(90)).intersects(cTar) == true)
		{
			ProcessCollid(bombCheck, arrayNo, target, loop_Battle_player_skills, *itemTarget);
		}
		else if (tc2.Collider.intersects(cTar) == true)
		{
			ProcessCollid(bombCheck, arrayNo, target, loop_Battle_player_skills, *itemTarget);
		}
	}
	void CalucDamage(ClassUnit& itemTarget, double strTemp, ClassExecuteSkills& ces)
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
	bool ProcessCollid(bool& bombCheck, Array<int32>& arrayNo, ClassBullets& target, ClassExecuteSkills& loop_Battle_player_skills, ClassUnit& itemTarget)
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

	// 描画関数（オプション）
	void draw() const override
	{
		{
			const auto t = camera.createTransformer();

			//rtMap.drawAt(camera.getCenter());

			for (int32 i = 0; i < (mapCreator.N * 2 - 1); ++i)
			{
				// x の開始インデックス
				const int32 xi = (i < (mapCreator.N - 1)) ? 0 : (i - (mapCreator.N - 1));

				// y の開始インデックス
				const int32 yi = (i < (mapCreator.N - 1)) ? i : (mapCreator.N - 1);

				// 左から順にタイルを描く
				for (int32 k = 0; k < (mapCreator.N - Abs(mapCreator.N - i - 1)); ++k)
				{
					// タイルのインデックス
					const Point index{ (xi + k), (yi - k) };

					// そのタイルの底辺中央の座標
					const int32 i = index.manhattanLength();
					const int32 xi = (i < (mapCreator.N - 1)) ? 0 : (i - (mapCreator.N - 1));
					const int32 yi = (i < (mapCreator.N - 1)) ? i : (mapCreator.N - 1);
					const int32 k2 = (index.manhattanDistanceFrom(Point{ xi, yi }) / 2);
					const double posX = ((i < (mapCreator.N - 1)) ? (i * -mapCreator.TileOffset.x) : ((i - 2 * mapCreator.N + 2) * mapCreator.TileOffset.x));
					const double posY = (i * mapCreator.TileOffset.y);
					Vec2 pos = { (posX + mapCreator.TileOffset.x * 2 * k2), posY };

					// 底辺中央を基準にタイルを描く
					String tip = getData().classGameStatus.classBattle.classMapBattle.value().mapData[index.x][index.y].tip;
					TextureAsset(tip + U".png").draw(Arg::bottomCenter = pos);

					//PutText(U"{}"_fmt(index), pos.movedBy(0, -mapCreator.TileOffset.y - 15));
					//PutText(U"{}"_fmt(pos), pos.movedBy(0, -mapCreator.TileOffset.y - 0));
				}
			}

			if (debugMap.size() != 0)
			{
				//// タイルのインデックス
				//const Point index{ (debugMap.back().begin()->GetRow()),(debugMap.back().begin()->GetCol()) };

				//// そのタイルの底辺中央の座標
				//const int32 i = index.manhattanLength();
				//const int32 xi = (i < (mapCreator.N - 1)) ? 0 : (i - (mapCreator.N - 1));
				//const int32 yi = (i < (mapCreator.N - 1)) ? i : (mapCreator.N - 1);
				//const int32 k2 = (index.manhattanDistanceFrom(Point{ xi, yi }) / 2);
				//const double posX = ((i < (mapCreator.N - 1)) ? (i * -mapCreator.TileOffset.x) : ((i - 2 * mapCreator.N + 2) * mapCreator.TileOffset.x));
				//const double posY = (i * mapCreator.TileOffset.y) - mapCreator.TileThickness;
				//const Vec2 pos = { (posX + mapCreator.TileOffset.x * 2 * k2), posY };

				//Circle ccccc = Circle{ Arg::bottomCenter = pos,30 };
				//ccccc.draw();
			}
			if (debugRoot.size() != 0)
			{
				//for (auto abcd : debugRoot.back())
				//{
				//	// タイルのインデックス
				//	const Point index{ abcd.x,abcd.y };

				//	// そのタイルの底辺中央の座標
				//	const int32 i = index.manhattanLength();
				//	const int32 xi = (i < (mapCreator.N - 1)) ? 0 : (i - (mapCreator.N - 1));
				//	const int32 yi = (i < (mapCreator.N - 1)) ? i : (mapCreator.N - 1);
				//	const int32 k2 = (index.manhattanDistanceFrom(Point{ xi, yi }) / 2);
				//	const double posX = ((i < (mapCreator.N - 1)) ? (i * -mapCreator.TileOffset.x) : ((i - 2 * mapCreator.N + 2) * mapCreator.TileOffset.x));
				//	const double posY = (i * mapCreator.TileOffset.y) - mapCreator.TileThickness;
				//	const Vec2 pos = { (posX + mapCreator.TileOffset.x * 2 * k2), posY };

				//	Circle ccccc = Circle{ Arg::bottomCenter = pos,30 };
				//	ccccc.draw(Palette::Red);
				//}
			}
			if (debugAstar.size() != 0)
			{
				//for (auto hfdfsjf : debugAstar)
				//{
				//	// タイルのインデックス
				//	const Point index{ hfdfsjf->GetRow(),hfdfsjf->GetCol()};

				//	// そのタイルの底辺中央の座標
				//	const int32 i = index.manhattanLength();
				//	const int32 xi = (i < (mapCreator.N - 1)) ? 0 : (i - (mapCreator.N - 1));
				//	const int32 yi = (i < (mapCreator.N - 1)) ? i : (mapCreator.N - 1);
				//	const int32 k2 = (index.manhattanDistanceFrom(Point{ xi, yi }) / 2);
				//	const double posX = ((i < (mapCreator.N - 1)) ? (i * -mapCreator.TileOffset.x) : ((i - 2 * mapCreator.N + 2) * mapCreator.TileOffset.x));
				//	const double posY = (i * mapCreator.TileOffset.y) - mapCreator.TileThickness;
				//	const Vec2 pos = { (posX + mapCreator.TileOffset.x * 2 * k2), posY };

				//	Circle ccccc = Circle{ Arg::bottomCenter = pos,15 };
				//	ccccc.draw(Palette::Yellow);
				//}
			}

			//体力ゲージ
			for (auto& item : getData().classGameStatus.classBattle.sortieUnitGroup)
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
			for (auto& item : getData().classGameStatus.classBattle.defUnitGroup)
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

			////unit
			//
			for (auto& item : getData().classGameStatus.classBattle.sortieUnitGroup)
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
			for (auto& item : getData().classGameStatus.classBattle.sortieUnitGroup)
			{
				if (!item.FlagBuilding &&
					!item.ListClassUnit.empty())
				{
					for (auto& itemUnit : item.ListClassUnit)
					{
						if (itemUnit.IsBattleEnable == false)
							continue;
						if (itemUnit.FlagMove == true)
						{
							TextureAsset(itemUnit.Image).drawAt(itemUnit.GetNowPosiCenter()).drawFrame(3, 3, Palette::Orange);
						}
						else
						{
							TextureAsset(itemUnit.Image).drawAt(itemUnit.GetNowPosiCenter());
						}

						//s3d::Optional<Size> nowIndexEnemy = mapCreator.ToIndex(itemUnit.orderPosiLeft, columnQuads, rowQuads);
						//if (nowIndexEnemy.has_value() == false)
						//	continue;
						//Circle ccccc = Circle{ Arg::bottomCenter = nowIndexEnemy.value(),30 };
						//ccccc.draw(Palette::Blue);
					}
				}
			}
			for (auto& item : getData().classGameStatus.classBattle.sortieUnitGroup)
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

			for (auto& item : getData().classGameStatus.classBattle.defUnitGroup)
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
			for (auto& item : getData().classGameStatus.classBattle.defUnitGroup)
			{
				if (!item.FlagBuilding &&
					!item.ListClassUnit.empty())
				{
					for (auto& itemUnit : item.ListClassUnit)
					{
						if (itemUnit.IsBattleEnable == false)
							continue;
						if (itemUnit.FlagMove == true)
						{
							TextureAsset(itemUnit.Image).drawAt(itemUnit.GetNowPosiCenter()).drawFrame(3, 3, Palette::Orange);
						}
						else
						{
							TextureAsset(itemUnit.Image).drawAt(itemUnit.GetNowPosiCenter());
						}
					}
				}
			}
			for (auto& item : getData().classGameStatus.classBattle.defUnitGroup)
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


			Array<ClassUnit> bui;
			bui.append(getData().classGameStatus.classBattle.sortieUnitGroup[0].ListClassUnit);
			bui.append(getData().classGameStatus.classBattle.defUnitGroup[0].ListClassUnit);
			bui.append(getData().classGameStatus.classBattle.neutralUnitGroup[0].ListClassUnit);
			Array<std::pair<Vec2, String>> buiTex;

			for (int32 i = 0; i < (mapCreator.N * 2 - 1); ++i)
			{
				// x の開始インデックス
				const int32 xi = (i < (mapCreator.N - 1)) ? 0 : (i - (mapCreator.N - 1));

				// y の開始インデックス
				const int32 yi = (i < (mapCreator.N - 1)) ? i : (mapCreator.N - 1);

				// 左から順にタイルを描く
				for (int32 k = 0; k < (mapCreator.N - Abs(mapCreator.N - i - 1)); ++k)
				{
					// タイルのインデックス
					const Point index{ (xi + k), (yi - k) };

					// そのタイルの底辺中央の座標
					const int32 i = index.manhattanLength();
					const int32 xi = (i < (mapCreator.N - 1)) ? 0 : (i - (mapCreator.N - 1));
					const int32 yi = (i < (mapCreator.N - 1)) ? i : (mapCreator.N - 1);
					const int32 k2 = (index.manhattanDistanceFrom(Point{ xi, yi }) / 2);
					const double posX = ((i < (mapCreator.N - 1)) ? (i * -mapCreator.TileOffset.x) : ((i - 2 * mapCreator.N + 2) * mapCreator.TileOffset.x));
					const double posY = (i * mapCreator.TileOffset.y);

					Vec2 pos = { (posX + mapCreator.TileOffset.x * 2 * k2), posY };
					for (auto& abc : bui)
					{
						if (abc.IsBattleEnable == false)
						{
							continue;
						}

						if (abc.rowBuilding == (xi + k) && abc.colBuilding == (yi - k))
						{
							std::pair<Vec2, String> hhhh = { pos.movedBy(0,-15), abc.Image + U".png" };
							buiTex.push_back(hhhh);
						}
					}
				}
			}

			//bui
			for (auto aaa : buiTex)
			{
				TextureAsset(aaa.second).draw(Arg::bottomCenter = aaa.first);
			}

			//触れたらもう一度unitをdrawする。2重に描写することで、目的を達する


			//範囲指定もしくは移動先矢印
			if (MouseR.pressed())
			{
				if (getData().classGameStatus.IsBattleMove == false)
				{
					const double thickness = 3.0;
					double offset = 0.0;

					offset += (Scene::DeltaTime() * 10);

					const Rect rect{ cursPos, Cursor::Pos() - cursPos };
					rect.top().draw(LineStyle::SquareDot(offset), thickness);
					rect.right().draw(LineStyle::SquareDot(offset), thickness);
					rect.bottom().draw(LineStyle::SquareDot(offset), thickness);
					rect.left().draw(LineStyle::SquareDot(offset), thickness);
				}
				else
				{
					Line{ cursPos, Cursor::Pos() }
					.drawArrow(10, Vec2{ 40, 80 }, Palette::Orange);
				}
			}

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
		}

		renderTextureSkill.draw(0, Scene::Size().y - 320 - 30);
		renderTextureSkillUP.draw(0, Scene::Size().y - 320 - 30);
		renderTextureSelektUnit.draw(316, WINDOWSIZEHEIGHT000 - 200);

		renderTextureZinkei.draw(0, Scene::Size().y - 440 - 30);
		renderTextureOrderSkill.draw(0, Scene::Size().y - 380 - 30);

		if (flagDisplaySkillSetumei == true)
		{
			getData().slice9.draw(rectSkillSetumei);
			getData().fontMiniMini(nowSelectSkillSetumei).draw(rectSkillSetumei.stretched(-12));
		}

		//ClearPrint();
		////for (auto& target : getData().classGameStatus.classBattle.sortieUnitGroup)
		////	for (auto& unit : target.ListClassUnit)
		////		if (unit.FlagMove == true)
		////			Print << Format(unit.ID) + U":" + unit.NameTag;
		//for (auto& target : getData().classGameStatus.classBattle.sortieUnitGroup)
		//	for (auto& unit : target.ListClassUnit)
		//		if (unit.FlagMoveAI == true)
		//			Print << U"【" + Format(unit.ID) + U":" + unit.NameTag;

		switch (battleStatus)
		{
		case BattleStatus::Battle:
			break;
		case BattleStatus::Message:
		{
			const auto t = camera.createTransformer();
			sceneMessageBoxImpl.show(systemString.BattleMessage001);
		}
		break;
		case BattleStatus::Event:
			break;
		default:
			break;
		}
	}

	void drawFadeIn(double t) const override
	{
		draw();

		m_fadeInFunction->fade(1 - t);
	}

	void drawFadeOut(double t) const override
	{
		draw();

		m_fadeOutFunction->fade(t);
	}
private:
	ClassUnit& GetCU(long ID)
	{
		for (auto& temp : getData().classGameStatus.classBattle.sortieUnitGroup)
		{
			for (auto& temptemp : temp.ListClassUnit)
			{
				if (temptemp.ID == ID)
				{
					return temptemp;
				}
			}
		}
	}
	bool PauseFlag = false;
	RenderTexture rtMap;
	Array<ClassAStar*> debugAstar;
	AsyncTask<int32> task;
	AsyncTask<int32> taskMyUnits;

	std::atomic<bool> abort{ false };
	std::atomic<bool> abortMyUnits{ false };
	std::atomic<bool> pauseTask{ false };
	std::atomic<bool> pauseTaskMyUnits{ false };

	RenderTexture renderTextureSkill;
	RenderTexture renderTextureSkillUP;
	HashTable<String, Rect> htSkill;
	Array<String> nowSelectSkill;
	bool flagDisplaySkillSetumei = false;
	String nowSelectSkillSetumei = U"";
	Rect rectSkillSetumei = { 0,0,320,320 };

	RenderTexture renderTextureZinkei;
	Array<Rect> rectZinkei;

	RenderTexture renderTextureOrderSkill;
	Array<Rect> rectOrderSkill;

	RenderTexture renderTextureSelektUnit;
	Array<Rect> RectSelectUnit;

	Array<ClassHorizontalUnit> bui;
	Vec2 viewPos;
	Point cursPos = Cursor::Pos();
	std::unique_ptr<IFade> m_fadeInFunction = randomFade();
	std::unique_ptr<IFade> m_fadeOutFunction = randomFade();
	Camera2D camera{ Vec2{ 0, 0 },1.0,CameraControl::Wheel };
	bool BattleMessage001 = true;
	BattleStatus battleStatus = BattleStatus::Message;
	s3dx::SceneMessageBoxImpl sceneMessageBoxImpl;
	Array<ClassExecuteSkills> m_Battle_player_skills;
	Array<ClassExecuteSkills> m_Battle_enemy_skills;
	Array<ClassExecuteSkills> m_Battle_neutral_skills;
	Array<Array<ClassAStar>> debugMap;
	Array<Array<Point>> debugRoot;
	Vec2 ConvertVecX(double rad, double x, double y, double charX, double charY)
	{
		//キャラクタを基に回転させないと、バグる

		double x2 = 0;
		double y2 = 0;
		x2 = ((x - charX) * s3d::Math::Cos(rad)) - ((y - charY) * s3d::Math::Sin(rad));
		y2 = ((x - charX) * s3d::Math::Sin(rad)) + ((y - charY) * s3d::Math::Cos(rad));
		return Vec2(x2 + charX, y2 + charY);
	}
	bool NearlyEqual(double a, double b)
	{
		return abs(a - b) < DBL_EPSILON;
	}

	void SkillProcess(Array<ClassHorizontalUnit>& ach, Array<ClassHorizontalUnit>& achTarget, Array<ClassExecuteSkills>& aces)
	{
		for (auto& item : ach)
		{
			for (auto& itemUnit : item.ListClassUnit)
			{
				//発動中もしくは死亡ユニットはスキップ
				if (itemUnit.FlagMovingSkill == true || itemUnit.IsBattleEnable == false)
					continue;

				auto temp = itemUnit.Skill.filter([&](const ClassSkill& itemSkill) {return nowSelectSkill.contains(itemSkill.nameTag); });

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
					for (ClassSkill& itemSkill : itemUnit.Skill.sort_by([](const auto& item1, const auto& item2) { return  item1.sortKey < item2.sortKey; }))
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
	bool SkillProcess002(Array<ClassHorizontalUnit>& aaatarget, Vec2 xA, ClassUnit& itemUnit, ClassSkill& itemSkill, Array<ClassExecuteSkills>& aces)
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

				int32 random = getData().classGameStatus.getBattleIDCount();
				int singleAttackNumber = random;

				itemUnit.FlagMovingSkill = true;

				//rush数だけ実行する
				int32 rushBase = 1;
				if (itemSkill.rush > 1) rushBase = itemSkill.rush;

				ClassExecuteSkills ces;
				ces.No = getData().classGameStatus.getDeleteCESIDCount();
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

};
class Card : public App::Scene
{
public:
	// コンストラクタ（必ず実装）
	Card(const InitData& init)
		: IScene{ init }
	{
		// カードの確率分布
		{
			Array<int> probabilities;
			for (size_t i = 0; i < getData().classGameStatus.arrayClassCard.size(); i++)
			{
				probabilities.push_back(1);
			}
			distributionCard = DiscreteDistribution(probabilities.begin(), probabilities.end());
		}

		String sc = U"Select card.";
		RectF re1 = getData().fontMini(sc).region();
		re1.x = Scene::Center().x - (re1.w / 2);
		re1.y = basePointY;
		arrayRectFSystem.push_back(re1);
		RectF re2 = { 0,0,1600,300 };
		re2.x = Scene::Center().x - (800);
		re2.y = (re1.h + basePointY + 20) + 550 + 20;
		arrayRectFSystem.push_back(re2);

		int32 arraySize = 3;
		int32 xxx = 0;
		xxx = ((arraySize * 206) / 2);
		int32 counter = 0;
		for (size_t i = 0; i < 3; i++)
		{
			ClassCard cc = DiscreteSample(getData().classGameStatus.arrayClassCard, distributionCard);
			RectF rrr = {};
			rrr = { (Scene::Center().x - xxx) + counter * 206,re1.h + basePointY + 20,206,550 };
			cc.rectF = rrr;

			arrayCard.push_back(cc);
			counter++;
		}
	}
	// 更新関数（オプション）
	void update() override
	{
		for (auto& ccc : arrayCard)
		{
			if (ccc.rectF.leftClicked() == false)
			{
				continue;
			}



			changeScene(U"Novel", 0.9s);
		}
	}
	// 描画関数（オプション）
	void draw() const override
	{
		getData().slice9.draw(arrayRectFSystem[0].asRect());
		getData().fontNormal(U"Select card.").draw(arrayRectFSystem[0].stretched(-10), ColorF{ 0.85 });
		getData().slice9.draw(arrayRectFSystem[1].asRect());

		for (auto& ttt : arrayCard)
		{
			if (ttt.rectF.mouseOver() == true)
			{
				getData().fontMini(ttt.help).draw(arrayRectFSystem[1].stretched(-10), ColorF{ 0.85 });
			}

			for (size_t i = 0; i < ttt.icon.size(); i++)
			{
				String filename = U"/006_CardImage/" + ttt.icon[i];
				ttt.rectF(TextureAsset(filename)).draw();
			}
		}
	}

	void drawFadeIn(double t) const override
	{
		draw();

		m_fadeInFunction->fade(1 - t);
	}

	void drawFadeOut(double t) const override
	{
		draw();

		m_fadeOutFunction->fade(t);
	}
private:
	int32 basePointY = 50;
	Array<RectF> arrayRectFSystem;
	Array<ClassCard> arrayCard;
	DiscreteDistribution distributionCard;
	std::unique_ptr<IFade> m_fadeInFunction = randomFade();
	std::unique_ptr<IFade> m_fadeOutFunction = randomFade();
};


void Init(App& manager)
{
	for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/010_FaceImage/"))
	{
		String filename = FileSystem::FileName(filePath);
		TextureAsset::Register(filename, filePath);
	}
	for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/006_CardImage/"))
	{
		String filename = U"/006_CardImage/" + FileSystem::FileName(filePath);
		TextureAsset::Register(filename, filePath);
	}
	for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/005_BackgroundImage/"))
	{
		String filename = FileSystem::FileName(filePath);
		TextureAsset::Register(filename, filePath);
	}
	for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/040_ChipImage/"))
	{
		String filename = FileSystem::FileName(filePath);
		TextureAsset::Register(filename, filePath);
	}
	for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/070_Scenario/InfoPower/"))
	{
		String filename = FileSystem::FileName(filePath);
		const JSON jsonPower = JSON::Load(filePath);
		if (not jsonPower) // もし読み込みに失敗したら
		{
			continue;
		}

		for (const auto& [key, value] : jsonPower[U"Power"])
		{
			ClassPower cp;
			cp.PowerTag = value[U"PowerTag"].getString();
			cp.PowerName = value[U"PowerName"].getString();
			cp.HelpString = value[U"Help"].getString();
			cp.SortKey = Parse<int32>(value[U"SortKey"].getString());
			cp.Image = value[U"Image"].getString();
			cp.Text = value[U"Text"].getString();
			cp.Diff = value[U"Diff"].getString();
			cp.Money = Parse<long>(value[U"Money"].getString());
			cp.Wave = Parse<int32>(value[U"Wave"].getString());
			manager.get().get()->classGameStatus.arrayClassPower.push_back(std::move(cp));
		}
	}
	for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/070_Scenario/InfoBGM/"))
	{
		String filename = FileSystem::FileName(filePath);
		const JSON jsonPower = JSON::Load(filePath);
		if (not jsonPower) continue;

		for (const auto& [key, value] : jsonPower[U"BGM"])
		{
			ClassBGM cp;
			if (value.hasElement(U"BGMTag") == true)
			{
				cp.tag = value[U"BGMTag"].getString();
			}
			if (value.hasElement(U"BGMName") == true)
			{
				cp.name = value[U"BGMName"].getString();
			}
			if (value.hasElement(U"OP") == true)
			{
				cp.op = true;
			}
			if (value.hasElement(U"PREPARE") == true)
			{
				cp.prepare = true;
			}
			if (value.hasElement(U"BATTLE") == true)
			{
				cp.battle = true;
			}

			AudioAsset::Register(cp.tag, PATHBASE + PATH_DEFAULT_GAME + U"/045_BGM/" + cp.tag);

			manager.get().get()->classGameStatus.arrayClassBGM.push_back(std::move(cp));
		}
	}
	for (const auto& filePath : FileSystem::DirectoryContents(PATHBASE + PATH_DEFAULT_GAME + U"/070_Scenario/InfoCard/"))
	{
		String filename = FileSystem::FileName(filePath);
		const JSON jsonPower = JSON::Load(filePath);
		if (not jsonPower) continue;

		for (const auto& [key, value] : jsonPower[U"Card"])
		{
			ClassCard cc;
			cc.nameTag = value[U"name_tag"].getString();
			cc.sortKey = Parse<int32>(value[U"sortkey"].getString());
			cc.func = value[U"func"].getString();
			const Array<String> strArray = value[U"icon"].getString().split(U',');
			for (auto& s : strArray)
			{
				cc.icon.push_back(s.replaced(U" ", U"").replaced(U"\t", U""));
			}
			cc.icon.reverse();

			cc.name = value[U"name"].getString();
			cc.help = value[U"help"].getString();
			cc.attackMyUnit = Parse<int32>(value[U"attackMyUnit"].getString());
			cc.defMyUnit = Parse<int32>(value[U"defMyUnit"].getString());
			cc.moveMyUnit = Parse<int32>(value[U"moveMyUnit"].getString());
			cc.costMyUnit = Parse<int32>(value[U"costMyUnit"].getString());
			cc.hpMyUnit = Parse<int32>(value[U"hpMyUnit"].getString());
			manager.get().get()->classGameStatus.arrayClassCard.push_back(std::move(cc));
		}
	}

	// Unit.jsonからデータを読み込む
	{
		const JSON jsonUnit = JSON::Load(PATHBASE + PATH_DEFAULT_GAME + U"/070_Scenario/InfoUnit/Unit.json");

		if (not jsonUnit) // もし読み込みに失敗したら
			throw Error{ U"Failed to load `Unit.json`" };

		Array<ClassUnit> arrayClassUnit;
		for (const auto& [key, value] : jsonUnit[U"Unit"]) {
			ClassUnit cu;
			cu.NameTag = value[U"name_tag"].getString();
			cu.Name = value[U"name"].getString();
			cu.Image = value[U"image"].getString();
			cu.Hp = Parse<int32>(value[U"hp"].getString());
			cu.Mp = Parse<int32>(value[U"mp"].getString());
			cu.HpMAX = cu.Hp;
			cu.Attack = Parse<int32>(value[U"attack"].getString());
			cu.Defense = Parse<int32>(value[U"defense"].getString());
			cu.Magic = Parse<int32>(value[U"magic"].getString());
			cu.MagDef = Parse<int32>(value[U"magDef"].getString());
			cu.Speed = Parse<double>(value[U"speed"].getString());
			cu.Price = Parse<int32>(value[U"price"].getString());
			cu.Move = Parse<double>(value[U"move"].getString());
			cu.Escape_range = Parse<int32>(value[U"escape_range"].getString());
			if (value.hasElement(U"help") == true)
			{
				cu.Help = (value[U"help"].getString());
			}
			if (value.hasElement(U"bf") == true)
			{
				int32 temp = Parse<int32>(value[U"bf"].getString());

				if (temp == 0)
				{
					cu.Formation = BattleFormation::F;
				}
				else if (temp == 1)
				{
					cu.Formation = BattleFormation::B;
				}
				else if (temp == 2)
				{
					cu.Formation = BattleFormation::M;
				}
				else
				{
					cu.Formation = BattleFormation::F;
				}
			}
			cu.Race = (value[U"race"].getString());
			String sNa = value[U"skill"].getString();
			if (sNa.contains(',') == true)
			{
				cu.SkillName = sNa.split(',');
			}
			else
			{
				cu.SkillName.push_back(sNa);
			}
			arrayClassUnit.push_back(std::move(cu));
		}

		// unitのスキルを読み込み
		const JSON skillData = JSON::Load(PATHBASE + PATH_DEFAULT_GAME + +U"/070_Scenario/InfoSkill/skill.json");

		if (not skillData) // もし読み込みに失敗したら
		{
			throw Error{ U"Failed to load `skill.json`" };
		}

		Array<ClassSkill> arrayClassSkill;
		for (const auto& [key, value] : skillData[U"Skill"]) {
			ClassSkill cu;
			if (value.hasElement(U"sortkey") == true)
				cu.sortKey = Parse<int32>(value[U"sortkey"].get<String>());
			if (value.hasElement(U"help") == true)
				cu.help = ClassStaticCommonMethod::ReplaceNewLine(value[U"help"].get<String>());

			{
				if (value[U"func"].get<String>() == U"missile")
				{
					cu.SkillType = SkillType::missile;
				}
				if (value[U"func"].get<String>() == U"missileAdd")
				{
					cu.SkillType = SkillType::missileAdd;
				}
				if (value[U"func"].get<String>() == U"heal")
				{
					cu.SkillType = SkillType::heal;
				}
			}
			{
				if (value[U"MoveType"].get<String>() == U"throw")
				{
					cu.MoveType = MoveType::thr;
				}
				if (value[U"MoveType"].get<String>() == U"circle")
				{
					cu.MoveType = MoveType::circle;
				}
				if (value[U"MoveType"].get<String>() == U"swing")
				{
					cu.MoveType = MoveType::swing;
				}
			}
			if (value.hasElement(U"Easing") == true)
				if (value[U"Easing"].get<String>() == U"easeOutExpo")
					cu.Easing = SkillEasing::easeOutExpo;
			if (value.hasElement(U"EasingRatio") == true)
			{
				cu.EasingRatio = Parse<int32>(value[U"EasingRatio"].get<String>());
			}
			if (value.hasElement(U"icon") == true)
			{
				cu.icon = (value[U"icon"].get<String>().split(','));
			}
			if (value.hasElement(U"slow_per") == true)
			{
				cu.slowPer = Parse<int32>(value[U"slow_per"].get<String>());
			}
			else
			{
				cu.slowPer = none;
			}
			if (value.hasElement(U"slow_time") == true)
			{
				cu.slowTime = Parse<int32>(value[U"slow_time"].get<String>());
			}
			else
			{
				cu.slowTime = none;
			}
			if (value.hasElement(U"center") == true)
			{
				if (value[U"center"].get<String>() == U"on")
				{
					cu.SkillCenter = SkillCenter::on;
				}
				else if (value[U"center"].get<String>() == U"off")
				{
					cu.SkillCenter = SkillCenter::off;
				}
				else if (value[U"center"].get<String>() == U"end")
				{
					cu.SkillCenter = SkillCenter::end;
				}
			}
			if (value.hasElement(U"bom") == true)
			{
				if (value[U"bom"].get<String>() == U"on")
				{
					cu.SkillBomb = SkillBomb::on;
				}
				else if (value[U"bom"].get<String>() == U"off")
				{
					cu.SkillBomb = SkillBomb::off;
				}
			}
			if (value.hasElement(U"height") == true)
			{
				cu.height = Parse<int32>(value[U"height"].get<String>());
			}
			if (value.hasElement(U"radius") == true)
			{
				cu.radius = Parse<double>(value[U"radius"].get<String>());
			}
			if (value.hasElement(U"rush") == true)
			{
				cu.rush = Parse<int32>(value[U"rush"].get<String>());
			}
			if (value.hasElement(U"rush_random_degree") == true)
			{
				cu.rushRandomDegree = Parse<int32>(value[U"rush_random_degree"].get<String>());
			}
			cu.image = value[U"image"].get<String>();
			if (value.hasElement(U"d360") == true)
			{
				if (value[U"d360"].get<String>() == U"on")
				{
					cu.SkillD360 = SkillD360::on;
					TextureAsset::Register(cu.image + U".png", PATHBASE + PATH_DEFAULT_GAME + U"/042_ChipImageSkillEffect/" + cu.image + U".png");
				}
			}
			else
			{
				TextureAsset::Register(cu.image + U"NW.png", PATHBASE + PATH_DEFAULT_GAME + U"/042_ChipImageSkillEffect/" + cu.image + U"NW.png");
				TextureAsset::Register(cu.image + U"N.png", PATHBASE + PATH_DEFAULT_GAME + U"/042_ChipImageSkillEffect/" + cu.image + U"N.png");
			}
			if (value.hasElement(U"force_ray") == true)
			{
				if (value[U"force_ray"].get<String>() == U"on")
				{
					cu.SkillForceRay = SkillForceRay::on;
				}
				else
				{
					cu.SkillForceRay = SkillForceRay::off;
				}
			}
			if (value.hasElement(U"ray") == true)
			{
				for (auto& temp : value[U"ray"].get<String>().split(','))
				{
					cu.ray.push_back(Parse<double>(temp.trimmed()) / 255);
				}
			}
			if (value.hasElement(U"ray_strokeThickness") == true)
			{
				cu.rayStrokeThickness = Parse<int32>(value[U"ray_strokeThickness"].get<String>());
			}
			if (value.hasElement(U"plus_speed") == true)
			{
				cu.plus_speed = Parse<double>(value[U"plus_speed"].get<String>());
			}
			if (value.hasElement(U"plus_speed_time") == true)
			{
				cu.plus_speed_time = Parse<double>(value[U"plus_speed_time"].get<String>());
			}
			cu.nameTag = value[U"name_tag"].get<String>();
			cu.name = value[U"name"].get<String>();
			cu.range = Parse<int32>(value[U"range"].get<String>());
			cu.w = Parse<int32>(value[U"w"].get<String>());
			cu.h = Parse<int32>(value[U"h"].get<String>());
			cu.str = Parse<int32>(value[U"str"].get<String>());
			if (value[U"str_kind"].get<String>() == U"attack")
			{
				cu.SkillStrKind = SkillStrKind::attack;
			}
			else if (value[U"str_kind"].get<String>() == U"off")
			{
				cu.SkillStrKind = SkillStrKind::attack;
			}
			else if (value[U"str_kind"].get<String>() == U"end")
			{
				cu.SkillStrKind = SkillStrKind::attack;
			}
			else if (value[U"str_kind"].get<String>() == U"attack_magic")
			{
				cu.SkillStrKind = SkillStrKind::attack_magic;
			}

			cu.speed = Parse<double>(value[U"speed"].get<String>());
			if (value.hasElement(U"attr") == true)
			{
				if (value[U"attr"].get<String>() == U"mp")
				{
					cu.attr = U"mp";
				}
				else if (value[U"attr"].get<String>() == U"hp")
				{
					cu.attr = U"hp";
				}
			}

			arrayClassSkill.push_back(std::move(cu));
		}

		//unitのスキル名からスキルクラスを探し、unitに格納
		for (auto& itemUnit : arrayClassUnit)
			for (const auto& itemSkillName : itemUnit.SkillName)
				for (const auto& skill : arrayClassSkill)
					if (skill.nameTag == itemSkillName)
					{
						itemUnit.Skill.emplace_back(skill);
						break;
					}

		manager.get().get()->classGameStatus.arrayClassUnit = arrayClassUnit;
	}
	// obj.jsonからデータを読み込む
	{
		const JSON objData = JSON::Load(PATHBASE + PATH_DEFAULT_GAME + U"/070_Scenario/InfoObject/obj.json");

		if (not objData) // もし読み込みに失敗したら
			throw Error{ U"Failed to load `obj.json`" };

		Array<ClassObjectMapTip> arrayClassObj;
		for (const auto& [key, value] : objData[U"obj"]) {
			ClassObjectMapTip cu;
			cu.nameTag = value[U"name"].get<String>();
			String ty = value[U"type"].get<String>();
			if (ty == U"wall2")
			{
				cu.type = MapTipObjectType::WALL2;
			}
			else if (ty == U"gate")
			{
				cu.type = MapTipObjectType::GATE;
			}
			cu.noWall2 = value[U"no_wall2"].get<int32>();
			cu.castle = value[U"castle"].get<int32>();
			cu.castleDefense = value[U"castle_defense"].get<int32>();
			cu.castleMagdef = value[U"castle_magdef"].get<int32>();

			arrayClassObj.push_back(std::move(cu));
		}
		manager.get().get()->classGameStatus.arrayClassObjectMapTip = arrayClassObj;
	}
	//enemy
	{
		const JSON jsonUnit = JSON::Load(PATHBASE + PATH_DEFAULT_GAME + U"/070_Scenario/InfoEnemy/enemy.json");

		if (not jsonUnit) // もし読み込みに失敗したら
			throw Error{ U"Failed to load `Unit.json`" };

		Array<ClassEnemy> arrayClassUnit;
		for (const auto& [key, value] : jsonUnit[U"enemy"]) {
			ClassEnemy ce;
			ce.name = value[U"name"].getString();
			ce.type = value[U"value"].getString().split(',');
			arrayClassUnit.push_back(std::move(ce));
		}
		manager.get().get()->classGameStatus.arrayClassEnemy = arrayClassUnit;
	}
	// config.tomlからデータを読み込む
	{
		const TOMLReader tomlConfig{ PATHBASE + PATH_DEFAULT_GAME + U"/config.toml" };
		if (not tomlConfig) // もし読み込みに失敗したら
			throw Error{ U"Failed to load `config.toml`" };
		manager.get().get()->classGameStatus.DistanceBetweenUnit = tomlConfig[U"config.DistanceBetweenUnit"].get<int32>();
		manager.get().get()->classGameStatus.DistanceBetweenUnitTate = tomlConfig[U"config.DistanceBetweenUnitTate"].get<int32>();
	}

}

void Main()
{
	// ウィンドウの枠を非表示にする
	Window::SetStyle(WindowStyle::Frameless);

	//中央に配置
	Window::Centering();

	// 背景の色を設定する | Set the background color
	Scene::SetBackground(ColorF(U"#0F040D"));

	Size tempSize = { INIT_WINDOW_SIZE_WIDTH,INIT_WINDOW_SIZE_HEIGHT };
	Window::Resize(tempSize);

	App manager;
	manager.add<TitleScene>(U"TitleScene");
	manager.add<WinSizeScene>(U"WinSizeScene");
	manager.add<SelectLang>(U"SelectLang");
	manager.add<ScenarioMenu>(U"ScenarioMenu");
	manager.add<SelectChar>(U"SelectChar");
	manager.add<Novel>(U"Novel");
	manager.add<Buy>(U"Buy");
	manager.add<Card>(U"Card");
	manager.add<Battle>(U"Battle");

	// 関数を格納するArrayを定義する
	Array<std::function<void()>> functions;
	functions.push_back(DrawUnder000);
	functions.push_back(DrawUnder001);
	m_gaussianClass = std::make_unique<GaussianClass>(Scene::Size(), functions);

	Init(manager);
	manager.init(U"SelectLang");

	Optional<std::pair<Point, Point>> dragStart;

	while (System::Update())
	{
		if (IS_SCENE_MODAL_PAUSED)
		{
			// ポーズ状態ならポーズウィンドウを描画
			m_pauseWindow.draw(manager.get().get()->fontLine);
			return;
		}

		if (not manager.update())
		{
			break;
		}

		m_gaussianClass->Show();

		//タイトル表示
		manager.get().get()->fontLine(systemString.AppTitle).draw(5, Scene::Size().y - 30, Palette::Black);

		if (EXITBTNRECT.leftClicked() == true)
		{
			break;
		}

		DragProcess(dragStart);
	}

	m_gaussianClass.reset();

}
