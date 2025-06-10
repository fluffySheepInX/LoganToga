#pragma once
# include <Siv3D.hpp>
#include "libs/CoTaskLib.hpp"
#include "ClassMap.h" 
#include "ClassMapBattle.h"

inline Optional<bool> downKeyEscapeResult;
inline Polygon EXITBTNRECT = {};
class SystemString {
public:
	String AppTitle;
	String TopMenuTitle;
	String lang;
	String configSave;
	String configLoad;
	String selectScenario;
	String selectScenario2;
	String selectChara1;
	String selectCard;
	String DoYouWantToQuitTheGame;
	Array<String> strategyMenu;
	String BattleMessage001;
	String BuyMessage001;
	String SelectCharMessage001;
	String StorySkip;

	String StatusName;
	String StatusRace;
	String StatusPrice;
	String StatusHp;
	String StatusMp;
	String StatusAttack;
	String StatusDefense;
	String StatusMagic;
	String StatusMagDef;
	String StatusSpeed;
	String StatusMove;
	String StatusSkill;
	String StatusSetumei;

	String SkillAttack;

};
inline SystemString systemString;

inline const String PATHBASE = U"000_Warehouse/";
inline String PATH_DEFAULT_GAME = U"000_DefaultGame/";
inline const String PathFont = PATHBASE + PATH_DEFAULT_GAME + U"005_Font";

inline const String PathImage = PATHBASE + PATH_DEFAULT_GAME + U"005_image0001";
inline const String PathMusic = PATHBASE + PATH_DEFAULT_GAME + U"music001";
inline const String PathSound = PATHBASE + PATH_DEFAULT_GAME + U"015_sound001";
inline const String PathLang = PATHBASE + PATH_DEFAULT_GAME;
inline const int32 INIT_WINDOW_SIZE_WIDTH = 800;
inline const int32 INIT_WINDOW_SIZE_HEIGHT = 800;
inline const int32 WINDOWSIZEWIDTH000 = 1600;
inline const int32 WINDOWSIZEHEIGHT000 = 900;
inline const int32 WINDOWSIZEWIDTH001 = 1200;
inline const int32 WINDOWSIZEHEIGHT001 = 600;
inline const String STRINGSLICE9000 = (PATHBASE + PATH_DEFAULT_GAME + U"/000_SystemImage" + U"/wnd0.png");
inline const String STRINGSLICE9001 = (PATHBASE + PATH_DEFAULT_GAME + U"/000_SystemImage" + U"/wnd1.png");
inline const String STRINGSLICE9002 = (PATHBASE + PATH_DEFAULT_GAME + U"/000_SystemImage" + U"/wnd2.png");
inline const String FONTJA = (PathFont + U"/x12y12pxMaruMinya.ttf");


class GameData
{
public:
	int score = 0;
	String playerName = U"Player";

	void increaseScore(int value)
	{
		score += value;
	}

	void printStatus() const
	{
		Console << U"Player: " << playerName << U", Score: " << score;
	}

	double cookies = 0.0;

	Array<int32> itemCounts;

	// シリアライズに対応させるためのメンバ関数を定義する
	template <class Archive>
	void SIV3D_SERIALIZE(Archive& archive)
	{
		archive(cookies, itemCounts);
	}
};

/// @brief 描画された最大のアルファ成分を保持するブレンドステートを作成する
/// @return 
inline BlendState MakeBlendState()
{
	BlendState blendState = BlendState::Default2D;
	blendState.srcAlpha = Blend::SrcAlpha;
	blendState.dstAlpha = Blend::DestAlpha;
	blendState.opAlpha = BlendOp::Max;
	return blendState;
}

struct RingEffect : IEffect
{
	Vec2 m_pos;

	ColorF m_color;

	// このコンストラクタ引数が .add<RingEffect>() の引数になる
	explicit RingEffect(const Vec2& pos)
		: m_pos{ pos }
		, m_color{ RandomColorF() } {
	}

	bool update(double t) override
	{
		// 時間に応じて大きくなる輪を描く
		Circle{ m_pos, (t * 100) }.drawFrame(4, m_color);

		// 1 秒未満なら継続する
		return (t < 1.0);
	}
};

/// @brief アニメーション処理をカプセル化したクラス
class AnimationEffect
{
public:
	// t の値に応じた描画処理を外部から渡す（再利用・カスタム可能）
	using DrawFunction = std::function<void(RectF, double)>;

	AnimationEffect(int32 a)
		:m_speed(0)
	{
		m_stopwatch.set(std::chrono::seconds(a));
	}

	void set(double speed, const DrawFunction& drawFunc)
	{
		m_speed = speed;
		m_drawFunc = drawFunc;
	}

	// 毎フレーム呼び出してアニメーション描画を行う
	void update(RectF re)
	{
		if (m_drawFunc == nullptr)
		{
			return;
		}

		// 経過時間に speed を掛けて t を計算
		const double t = m_stopwatch.sF() * m_speed;
		m_drawFunc(re, t);
	}

	// ボタン押下などでアニメーションをリセットする
	void trigger()
	{
		m_stopwatch.restart();
	}

private:
	Stopwatch m_stopwatch;  // アニメーション用タイマー
	double m_speed;         // アニメーションの進行速度
	DrawFunction m_drawFunc;
};

/*
UI周り
*/
enum class UIEventType
{
	Click,
	Hover,
	Drag
};

/// @brief 
struct UIElement
{
	UIElement()
		:animation(5),
		rect(RectF{}),
		color(ColorF{}),
		zIndex(0)
	{
	}

	String id;
	RectF rect;
	AnimationEffect animation;
	ColorF color;
	String text;
	String textureName;
	Font font = Font{ 32 };
	String evClick;
	int32 zIndex;

	std::unordered_map<UIEventType, std::function<void(void*)>> eventHandlers;

	// 指定したイベントタイプのハンドラを実行
	void triggerEvent(UIEventType eventType, void* data)
	{
		if (eventHandlers.count(eventType))
		{
			eventHandlers[eventType](data);
		}
		else
		{
			Print << U"Warning: No handler for event " << static_cast<int>(eventType);
		}
	}
};

inline void RegisterUIEvent(const String& evClick, UIEventType eventType, const std::function<void(void*)>& handler, Array<UIElement>& uiElements)
{
	for (auto& elem : uiElements)
	{
		if (elem.evClick == evClick)
		{
			elem.eventHandlers[eventType] = handler;
			break;
		}
	}
}

inline void HandleUIEvents(void* context, Array<UIElement>& uiElements)
{
	for (auto& elem : uiElements)
	{
		// マウスがUI要素の上にある場合
		if (elem.rect.mouseOver())
			elem.triggerEvent(UIEventType::Hover, context);

		// UI要素がクリックされた場合
		if (elem.rect.leftClicked())
		{
			elem.animation.trigger();
			elem.triggerEvent(UIEventType::Click, context);
		}

		// UI要素がドラッグされた場合
		if (MouseL.pressed() && elem.rect.leftPressed())
			elem.triggerEvent(UIEventType::Drag, context);
	}
}

inline HashTable<String, Array<UIElement>> LoadUISettings()
{
	HashTable<String, Array<UIElement>> uiElements;
	JSON json = JSON::Load(U"UISettings.json");
	if (!json) return uiElements;

	for (const auto& [key, elem3] : json)
	{
		Array<UIElement> arr;
		for (const auto& [key2, elem] : elem3)
		{
			UIElement ui;
			ui.id = key2;
			ui.rect = Rect(elem[U"x"].get<int>(), elem[U"y"].get<int>(), elem[U"w"].get<int>(), elem[U"h"].get<int>());
			ui.color = ColorF(elem[U"r"].get<double>(), elem[U"g"].get<double>(), elem[U"b"].get<double>(), elem[U"a"].get<double>());
			ui.text = elem[U"text"].getString();
			if (elem.hasElement(U"EvClick") == true)
				ui.evClick = elem[U"EvClick"].getString();
			ui.animation.set(5.0, [](RectF rect, double t) {
				// アニメーションの内容は元のコードと同様
				ColorF color{ 0.0 };

				// 四角形の拡大アニメーション（EaseOutBack で動きを付ける）
				if (0.0 <= t && t < 1.0)
				{
					rect = rect.scaled(EaseOutBack(t));
				}

				// 色の変化：t の値に応じて黒→白、または白→黒に変化
				if (0.58 <= t && t < 1.0)
				{
					color = ColorF((t - 0.58) / 0.42);  // 黒 → 白
				}
				else if (1.0 <= t && t < 1.8)
				{
					color = ColorF((1.8 - t) / 0.8);    // 白 → 黒
				}

				// 描画
				rect.draw(color);
			});
			if (elem.hasElement(U"zIndex") == true)
				ui.zIndex = elem[U"zIndex"].get<int32>();
			arr.push_back(ui);
		}
		uiElements.emplace(key, arr);
	}

	return uiElements;
}

inline void SaveUISettings(UIElement& uiElements, String sname, bool add = false)
{
	JSON json = JSON::Load(U"UISettings.json");
	if (!json)
	{
		JSON target000 = JSON{
					{ U"x", uiElements.rect.x },
					{ U"y", uiElements.rect.y },
					{ U"w", uiElements.rect.w },
					{ U"h", uiElements.rect.h },
					{ U"r", uiElements.color.r },
					{ U"g", uiElements.color.g },
					{ U"b", uiElements.color.b },
					{ U"a", uiElements.color.a },
						{ U"zIndex", uiElements.zIndex},
					{ U"EvClick", U"EvClick"},
					{ U"text", uiElements.text} };
		JSON target001 = JSON{ {U"data",target000} };
		JSON target002 = JSON{ {sname,target001} };

		if (target002.save(U"UISettings.json") == false)
			throw Error{ U"Failed to save `UISettings.json`" };
	}
	else
	{
		if (add)
		{
			JSON target000 = JSON{
						{ U"x", uiElements.rect.x },
						{ U"y", uiElements.rect.y },
						{ U"w", uiElements.rect.w },
						{ U"h", uiElements.rect.h },
						{ U"r", uiElements.color.r },
						{ U"g", uiElements.color.g },
						{ U"b", uiElements.color.b },
						{ U"a", uiElements.color.a },
						{ U"zIndex", uiElements.zIndex},
						{ U"EvClick", uiElements.evClick},
						{ U"text", uiElements.text} };
			json[sname][uiElements.id] = target000;
		}
		else
		{
			for (auto&& [key, elem3] : json)
			{
				if (key != sname)
					continue;

				for (auto&& [key2, elem] : elem3)
				{
					if (key2 != uiElements.id)
						continue;

					elem[U"x"] = uiElements.rect.x;
					elem[U"y"] = uiElements.rect.y;
					elem[U"w"] = uiElements.rect.w;
					elem[U"h"] = uiElements.rect.h;
					elem[U"r"] = uiElements.color.r;
					elem[U"g"] = uiElements.color.g;
					elem[U"b"] = uiElements.color.b;
					elem[U"a"] = uiElements.color.a;
					elem[U"text"] = uiElements.text;
					elem[U"zIndex"] = uiElements.zIndex;
				}
			}

		}

		if (json.save(U"UISettings.json") == false)
			throw Error{ U"Failed to save `UISettings.json`" };
	}

}

inline void DragProcess(Optional<std::pair<Point, Point>>& dragStart, RectF& waku)
{
	if (waku.mouseOver() == true)
		Cursor::RequestStyle(CursorStyle::Hand);
	if (dragStart)
	{
		if (not MouseL.pressed())
		{
			dragStart.reset();
		}
		else
		{
			waku.pos = dragStart->second + (Cursor::ScreenPos() - dragStart->first);
		}
	}

	// ドラッグの開始
	if (waku.leftClicked())
		dragStart = { Cursor::ScreenPos(), waku.asRect().pos };
}

inline bool EditUI(UIElement& uiElement,
			TextEditState& tes,
			Optional<std::pair<Point, Point>>& dragStart,
			Optional<std::pair<Point, Point>>& dragStartWaku,
			RectF& wakuMove,
			TextEditState& tesId,
			TextEditState& tesZIndex)
{
	int32 spaceX = 12;
	int32 spaceY = 12;

	// ドラッグによる移動
	DragProcess(dragStart, uiElement.rect);
	DragProcess(dragStartWaku, wakuMove);

	wakuMove.draw(Palette::Darkred);
	RectF waku = wakuMove.movedBy(0, 24);
	waku.w = 360;
	waku.h = 500;
	waku.draw(Palette::Black);
	waku.drawFrame(3, 0, Palette::Darkred);

	// w変更
	double value1 = 5.0;
	value1 = uiElement.rect.w;
	SimpleGUI::Slider(U"Width:{}"_fmt(value1), value1, 0.0, 400.0, Vec2(waku.x + spaceX, waku.y + spaceY), 110, 200);
	uiElement.rect.w = value1;
	// h変更
	double value2 = 5.0;
	value2 = uiElement.rect.h;
	SimpleGUI::Slider(U"Height:{}"_fmt(value2), value2, 0.0, 400.0, Vec2(waku.x + spaceX, waku.y + spaceY + (30 * 8) + (12 * 8)), 110, 200);
	uiElement.rect.h = value2;

	// テキスト編集
	SimpleGUI::TextBox(tes, Vec2(waku.x + spaceX, waku.y + spaceY + 30 + 12));
	uiElement.text = tes.text;

	// カラー変更
	ColorF color1{ 1.0 };
	color1 = uiElement.color;
	SimpleGUI::Slider(color1.r, Vec2{ waku.x + spaceX, waku.y + spaceY + (30 * 2) + (12 * 2) });
	SimpleGUI::Slider(color1.g, Vec2{ waku.x + spaceX, waku.y + spaceY + (30 * 3) + (12 * 3) });
	SimpleGUI::Slider(color1.b, Vec2{ waku.x + spaceX, waku.y + spaceY + (30 * 4) + (12 * 4) });
	uiElement.color = color1;

	SimpleGUI::TextBox(tesId, Vec2(waku.x + spaceX, waku.y + spaceY + (30 * 6) + (12 * 6)));
	uiElement.id = tesId.text;

	SimpleGUI::TextBox(tesZIndex, Vec2(waku.x + spaceX, waku.y + spaceY + (30 * 7) + (12 * 7)));
	if (tesZIndex.text.isEmpty() == false)
	{
		uiElement.zIndex = Parse<int32>(tesZIndex.text);
	}
	else
	{
		uiElement.zIndex = 0;
	}

	// 設定保存
	bool result = SimpleGUI::Button(U"Save UI", Vec2(waku.x + spaceX, waku.y + spaceY + (30 * 5) + (12 * 5)));

	return result;
}

/// @brief 
class FsScene : public Co::SceneBase
{
public:
	FsScene(String sceneName)
		: m_uiElements{ LoadUISettings() }
		, m_nowSName{ sceneName }
	{
	}
	virtual Co::Task<void> start() = 0;
private:
protected:
	void process()
	{
		if (KeyF3.down())
		{
			editMode = !editMode;

			// UI要素を zIndex の昇順でソート
			sortedElements = std::ref(m_uiElements[m_nowSName]);
			std::sort(sortedElements.begin(), sortedElements.end(), [](const UIElement& a, const UIElement& b) {
				return a.zIndex < b.zIndex;
			});
		}
	}
	void draw() const
	{
		//更新用loop
		for (auto elem : sortedElements)
		{
			elem.animation.update(elem.rect);
		}

		// 描画用loop
		for (const auto& elem : sortedElements)
		{
			elem.rect.draw(elem.color);
			elem.font(elem.text).draw(elem.rect, Palette::Black);
		}
	}
	bool editMode = false;
	TextEditState tes;
	TextEditState tesId;
	TextEditState tesZIndex;
	Optional<std::pair<Point, Point>> dragStart;
	Optional<std::pair<Point, Point>> dragStartWaku;
	size_t selectUiElement = 0;
	RectF waku = RectF{ 12,12,300,32 };
	String m_nowSName;
	HashTable<String, Array<UIElement>> m_uiElements;
	Array<UIElement> sortedElements;
	bool shouldExit = true;
};

/// @brief 
/// @return 
inline Co::Task<bool> AskStop()
{
	const String Yes = U"はい";
	const Array<String> yesNoButtons{ Yes, U"いいえ" };

	const String choice = co_await Co::SimpleDialog(U"ゲームを止めますか？", yesNoButtons);

	bool re = (choice == Yes) ? true : false;
	co_return re;
}

/// @brief 
class TooltipExample : public Co::SequenceBase<>
{
public:
	TooltipExample(bool& escFlag)
		: m_escFlag{ escFlag }
	{
	}

private:
	const Rect HoverArea{ 0, 0, 200, 100 };
	bool& m_escFlag;
	Font m_font{ 20 };
	String m_tooltipText = U"ここがトゥールチップのエリアです！";
	Optional<Point> m_tooltipPosition;

	Co::Task<> start() override
	{
		while (true)
		{
			// マウスがエリア内に入るまで待機
			co_await Co::WaitUntilMouseOver(HoverArea);

			if (m_escFlag == true)
			{
				co_await Co::NextFrame();
				continue;
			}

			// トゥールチップを表示
			m_tooltipPosition = Cursor::Pos();
			co_await Co::WaitWhile([this] { return HoverArea.mouseOver(); });

			// トゥールチップを消す
			m_tooltipPosition = none;
		}
	}

	void draw() const override
	{
		// エリアを描画
		HoverArea.draw(Palette::Skyblue).drawFrame(2, 0, Palette::Navy);

		// トゥールチップを描画
		if (m_tooltipPosition)
		{
			m_font(m_tooltipText).drawAt(m_tooltipPosition->movedBy(10, -20), Palette::Black);
			RectF{ m_font(m_tooltipText).region().movedBy(m_tooltipPosition->movedBy(10, -20)) }
			.draw(Palette::Yellow.withAlpha(200));
		}
	}
};

inline bool downKeyEscape(Optional<bool> result)
{
	const auto runner = AskStop().runScoped(
		[&result](bool r) { result = r; },
		[] { Print << U"タスクがキャンセルされました"; }
	);

	while (System::Update())
	{
		if (runner.done())
			break;
	}

	if (result.has_value() == true)
		if (result.value() == true)
			return true;
	return false;
}

class LiquidBarBattle
{
public:

	LiquidBarBattle() = default;

	explicit LiquidBarBattle(const Rect& rect)
		: m_rect{ rect } {
	}

	void update(double targetHP)
	{
		m_targetHP = targetHP;
		m_liquidHP = Math::SmoothDamp(m_liquidHP, targetHP, m_liquidHPVelocity, LiquidSmoothTime);

		if (m_solidHP < targetHP)
		{
			m_solidHP = targetHP;
		}
		else
		{
			m_solidHP = Math::SmoothDamp(m_solidHP, targetHP, m_solidHPVelocity, SolidSmoothTime, MaxSolidBarSpeed);
		}
	}

	void ChangePoint(const Vec2 v)
	{
		m_rect.x = v.x;
		m_rect.y = v.y;
	}

	const LiquidBarBattle& draw(const ColorF& liquidColorFront, const ColorF& liquidColorBack, const ColorF& solidColor) const
	{
		// バーの背景を描く
		m_rect.draw(ColorF{ 0.2, 0.15, 0.25 });

		// バーの枠を描く
		m_rect.drawFrame(2, 0);

		const Point basePos = m_rect.pos.movedBy(FrameThickness, FrameThickness);
		const int32 height = (m_rect.h - (FrameThickness * 2));
		const double width = (m_rect.w - (FrameThickness * 2));

		const double solidWidth = Min(Max((width * m_solidHP) + (height * 0.5 * 0.3), 0.0), width);
		const double liquidWidth = (width * m_liquidHP);

		// 固体バーを描く
		{
			const RectF solidBar{ basePos, solidWidth, height };
			const double alpha = ((0.005 < AbsDiff(m_targetHP, m_solidHP)) ? 1.0 : (AbsDiff(m_targetHP, m_solidHP) / 0.005));
			solidBar.draw(ColorF{ solidColor, alpha });
		}

		// 液体バーを描く
		{
			const double t = Scene::Time();
			const double offsetScale = ((m_liquidHP < 0.05) ? (m_liquidHP / 0.05) : (0.98 < m_liquidHP) ? 0.0 : 1.0);

			// 背景の液体バーを描く
			for (int32 i = 0; i < height; ++i)
			{
				const Vec2 pos = basePos.movedBy(0, i);
				const double waveOffset = (i * 0.3)
					+ (Math::Sin(i * 17_deg + t * 800_deg) * 0.8)
					+ (Math::Sin(i * 11_deg + t * 700_deg) * 1.2)
					+ (Math::Sin(i * 7_deg + t * 550_deg) * 1.6);
				const RectF rect{ pos, Clamp((liquidWidth + waveOffset * offsetScale), 0.0, width), 1 };

				const double distance = Clamp(1.0 - i / (height - 1.0) + 0.7, 0.0, 1.0);
				HSV hsv{ liquidColorBack };
				hsv.v *= Math::Pow(distance, 2.0);
				rect.draw(hsv);
			}

			// 前景の液体バーを描く
			for (int32 i = 0; i < height; ++i)
			{
				const Vec2 pos = basePos.movedBy(0, i);
				const double waveOffset = (i * 0.3)
					+ (Math::Sin(i * 17_deg - t * 800_deg) * 0.8)
					+ (Math::Sin(i * 11_deg - t * 700_deg) * 1.2)
					+ (Math::Sin(i * 7_deg - t * 550_deg) * 1.6);
				const RectF rect{ pos, Clamp((liquidWidth + waveOffset * offsetScale), 0.0, width), 1 };

				const double distance = Clamp(1.0 - i / (height - 1.0) + 0.7, 0.0, 1.0);
				HSV hsv{ liquidColorFront };
				hsv.v *= Math::Pow(distance, 2.0);
				rect.draw(hsv);
			}
		}
		return *this;
	}
	// 左揃えでテキストを描く
	const LiquidBarBattle& withLabel(const DrawableText drawableText, double fontSize, const ColorF& textColor, double offsetX, const Vec2& textPosOffset = Vec2(0, 0)) const
	{
		const RectF textSize = drawableText.region();
		const Point textPos = (Vec2(m_rect.x + offsetX, m_rect.y + (m_rect.h - textSize.y) / 2) + textPosOffset).asPoint();
		drawableText.draw(fontSize, textPos, ColorF(0.1));
		drawableText.draw(fontSize, textPos, textColor);
		return *this;
	}

private:

	// 液体バーが減少するときの平滑化時間（小さいと早く目標に到達）
	static constexpr double LiquidSmoothTime = 0.03;

	// 固体バーが減少するときの平滑化時間（小さいと早く目標に到達）
	static constexpr double SolidSmoothTime = 0.5;

	// 固体バーが減少するときの最大の速さ
	static constexpr double MaxSolidBarSpeed = 0.25;

	static constexpr int32 FrameThickness = 2;

	Rect m_rect = Rect::Empty();

	double m_targetHP = 1.0;
	double m_liquidHP = 1.0;
	double m_solidHP = 1.0;
	double m_liquidHPVelocity = 0.0;
	double m_solidHPVelocity = 0.0;
};
class ClassStaticCommonMethod {
public:
	static String MoldingScenarioText(String target)
	{
		return target
			.replaced(U"$", U"\r\n")
			.replaced(U"\\", U"")
			.replaced(U"\t", U"")
			.replaced(U"@", U" ");
	}
	static String ReplaceNewLine(String target)
	{
		return target.replaced(U"\r\n", U"")
			.replaced(U"\r", U"")
			.replaced(U"\n", U"")
			.replaced(U" ", U"")
			.replaced(U"\t", U"");
	}
	static ClassMapBattle GetClassMapBattle(ClassMap cm){
		ClassMapBattle cmb;
		cmb.name = cm.name;
		for (String aaa : cm.data)
		{
			Array<MapDetail> aMd;
			Array s = aaa.split(U',');
			for (auto bbb : s)
			{
				MapDetail md;

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

private:
	ClassStaticCommonMethod();
};
