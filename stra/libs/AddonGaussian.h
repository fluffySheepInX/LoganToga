#pragma once
#include <Siv3D.hpp>

class GaussianFSAddon : public IAddon
{
public:
	/// @brief 状態を更新します。
	static void Condition(const Point& size, const ColorF argC = ColorF{ U"#011B05" }, const ColorF argBarC = ColorF{ U"#158876" });
	/// @brief アクティブかどうかを返します。
	/// @return アクティブな場合は true、それ以外の場合は false
	static bool IsActive();
	/// @brief アクティブにします。
	static void SetActive();
	/// @brief 非アクティブにします。
	static void SetNotActive();
	static void SetLangSet(const Array<std::pair<String, String>>& langSet);
	static void SetLang(const String lang);
	static void SetSceneSet(const Array<Array<String>>& langSet);
	static void SetScene(const String lang);
	static double GetSCALE();
	static Vec2 GetOFFSET();
	/// @brief 
	/// @param dragStart 
	static void DragProcessWindow();
	/// @brief 
	/// @return 
	static bool IsGameEnd();
	/// @brief 
	/// @return 
	static bool IsHide();
	/// @brief 
	/// @param argS 
	static void SetSceneName(const String argS);

	static bool TriggerOrDisplayESC();
	static bool TriggerOrDisplayLang();
	static bool TriggerOrDisplaySceneSize();

	///// @brief 
	///// @param argFont 
	//static void SetFont(Font& argFont);

	static Point GetWindowSize();
	static Point GetInitWindowSize();
	static String GetLang();

	static void SetWindSize(int32 w, int32 h);
	static double CalculateScale(const Vec2& baseSize, const Vec2& currentSize);
	static Vec2 CalculateOffset(const Vec2& baseSize, const Vec2& currentSize);
	//static String s_current_lang;
private:
	/// @brief 毎フレームの更新処理を行います。
	/// @return 更新が成功した場合は true、それ以外の場合は false
	bool update() override;
	/// @brief 毎フレームの描画処理を行います。
	void draw() const override;

	void Draw000() const;
	void Draw001() const;
	void ProcessMultiLang() const;
	void ProcessSizeChange() const;

	/// @brief 関数を格納するArrayを定義する
	Array<std::function<void()>> functions;
#pragma region Font
	const String m_PATHBASE = U"001_Warehouse/";
	const String m_PathFont = m_PATHBASE + U"020_font/";
	const Font m_font = Font{ FontMethod::MSDF,12,(m_PathFont + U"x12y12pxMaruMinya.ttf") };
#pragma endregion

#pragma region GaussianBlur
	Size initSceneSize = { 400,600 };
	Size sceneSize = { 0,0 };
	RenderTexture gaussianA1 = {}, gaussianB1 = {};
	RenderTexture gaussianA4 = {}, gaussianB4 = {};
	RenderTexture gaussianA8 = {}, gaussianB8 = {};
	double a1 = 0.0, a4 = 0.0, a8 = 0.5;
#pragma endregion

	ColorF m_barColor;
	bool m_active = true;
	double m_SCALE = 1.0;
	Vec2 m_OFFSET = { 0, 0 };
	String m_NOWSCENE;
	String m_lastLang;
	HashTable<String, String> m_htLang;
	Optional<std::pair<Point, Point>> m_dragStartWindow;
	String m_lastSceneSize;
	Array<Array<String>> m_arraySceneSize;
	bool m_lockWindow = false;
	bool m_escPressed = false;
	bool m_langPressed = false;
	bool m_sceneSizePressed = false;

};
