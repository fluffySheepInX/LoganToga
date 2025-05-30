#pragma once
#include <Siv3D.hpp>

class GaussianFSAddon : public IAddon
{
public:
	/// @brief 状態を更新します。
	static void Condition(const Point& size);
	/// @brief アクティブかどうかを返します。
	/// @return アクティブな場合は true、それ以外の場合は false
	static bool IsActive();
	/// @brief アクティブにします。
	static void SetActive();
	/// @brief 非アクティブにします。
	static void SetNotActive();

	static void DragProcessWindow(Optional<std::pair<Point, Point>>& dragStart);
	static Shape2D GetStairs();
	static Polygon GetStairsAsPo();
	int32 WINDOWSIZEWIDTH000 = 0;
	int32 WINDOWSIZEHEIGHT000 = 0;
private:
	/// @brief 毎フレームの更新処理を行います。
	/// @return 更新が成功した場合は true、それ以外の場合は false
	bool update() override;
	/// @brief 毎フレームの描画処理を行います。
	void draw() const override;

	void DrawScene000() const;
	void DrawScene001() const;

	/// @brief 関数を格納するArrayを定義する
	Array<std::function<void()>> functions;
	/// @brief ガウスぼかし
	Size sceneSize = { 0,0 };
	RenderTexture gaussianA1 = {}, gaussianB1 = {};
	RenderTexture gaussianA4 = {}, gaussianB4 = {};
	RenderTexture gaussianA8 = {}, gaussianB8 = {};
	double a1 = 0.0, a4 = 0.0, a8 = 0.5;

	bool m_active = true;
	double m_SCALE = 1.0;
	Vec2 m_OFFSET = { 0, 0 };
	String m_NOWSCENE;

};
