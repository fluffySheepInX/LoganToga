#pragma once
#include "../Gaussian.hpp"

/// @brief 
void GaussianFSAddon::Condition(const Point& size)
{
	if (auto p = Addon::GetAddon<GaussianFSAddon>(U"GaussianFSAddon"))
	{
		// ウィンドウの枠を非表示にする
		Window::SetStyle(WindowStyle::Frameless);
		// 中央に配置
		Window::Centering();
		// ウィンドウサイズ変更
		Window::Resize(size);

		p->sceneSize = size;
		p->gaussianA1 = RenderTexture{ p->sceneSize };
		p->gaussianB1 = RenderTexture{ p->sceneSize };
		p->gaussianA4 = RenderTexture{ p->sceneSize / 4 };
		p->gaussianB4 = RenderTexture{ p->sceneSize / 4 };
		p->gaussianA8 = RenderTexture{ p->sceneSize / 8 };
		p->gaussianB8 = RenderTexture{ p->sceneSize / 8 };
		p->WINDOWSIZEWIDTH000 = size.x;
		p->WINDOWSIZEHEIGHT000 = size.y;
	}
}

/// @brief アクティブかどうか
/// @return 
[[nodiscard]]
bool GaussianFSAddon::IsActive()
{
	if (auto p = Addon::GetAddon<GaussianFSAddon>(U"GaussianFSAddon"))
	{
		return p->m_active;
	}
	else
	{
		return false;
	}
}

/// @brief アクティブにします。
void GaussianFSAddon::SetActive()
{
	if (auto p = Addon::GetAddon<GaussianFSAddon>(U"GaussianFSAddon"))
		p->m_active = true;
}

/// @brief 非アクティブにします。
void GaussianFSAddon::SetNotActive()
{
	if (auto p = Addon::GetAddon<GaussianFSAddon>(U"GaussianFSAddon"))
		p->m_active = false;
}

/// @brief 
/// @param dragStart 
/// @param ww 
void GaussianFSAddon::DragProcessWindow(Optional<std::pair<Point, Point>>& dragStart)
{
	if (auto p = Addon::GetAddon<GaussianFSAddon>(U"GaussianFSAddon"))
	{
		// ドラッグ処理
		if (Rect{ 0, 0, p->WINDOWSIZEWIDTH000, 60 }.mouseOver() == true)
			Cursor::RequestStyle(U"MyCursorHand");
		if (dragStart)
			(MouseL.pressed() == true) ? Window::SetPos(dragStart->second + (Cursor::ScreenPos() - dragStart->first)) : dragStart.reset();
		// ドラッグの開始
		if (Rect{ 0, 0, p->WINDOWSIZEWIDTH000, 60 }.leftClicked())
			dragStart = { Cursor::ScreenPos(), Window::GetState().bounds.pos };
	}
}

Shape2D GaussianFSAddon::GetStairs()
{
	if (auto p = Addon::GetAddon<GaussianFSAddon>(U"GaussianFSAddon"))
	{
		return Shape2D::Stairs(Vec2{ p->WINDOWSIZEWIDTH000 - 5, p->WINDOWSIZEHEIGHT000 - 5 }, 20, 20, 4).draw(Palette::Black);
	}
	return Shape2D();
}
Polygon GaussianFSAddon::GetStairsAsPo()
{
	if (auto p = Addon::GetAddon<GaussianFSAddon>(U"GaussianFSAddon"))
	{
		return Shape2D::Stairs(Vec2{ p->WINDOWSIZEWIDTH000 - 5, p->WINDOWSIZEHEIGHT000 - 5 }, 20, 20, 4).draw(Palette::Black).asPolygon();
	}
	return Polygon();
}

[[nodiscard]]
bool GaussianFSAddon::update()
{
	if (not m_active)
		return true;
	return true;
}

void GaussianFSAddon::draw() const
{
	if (not m_active)
		return;

	DrawScene000();
	DrawScene001();
	// ガウスぼかし用テクスチャにもう一度シーンを描く
	{
		const ScopedRenderTarget2D target{ gaussianA1.clear(ColorF{ 0.0 }) };
		const ScopedRenderStates2D blend{ BlendState::Additive };
		DrawScene000();
		DrawScene001();
	}

	// オリジナルサイズのガウスぼかし (A1)
	// A1 を 1/4 サイズにしてガウスぼかし (A4)
	// A4 を 1/2 サイズにしてガウスぼかし (A8)
	Shader::GaussianBlur(gaussianA1, gaussianB1, gaussianA1);
	Shader::Downsample(gaussianA1, gaussianA4);
	Shader::GaussianBlur(gaussianA4, gaussianB4, gaussianA4);
	Shader::Downsample(gaussianA4, gaussianA8);
	Shader::GaussianBlur(gaussianA8, gaussianB8, gaussianA8);

	{
		const ScopedRenderStates2D blend{ BlendState::Additive };

		if (a1)
			gaussianA1.resized(sceneSize).draw(ColorF{ a1 });

		if (a4)
			gaussianA4.resized(sceneSize).draw(ColorF{ a4 });

		if (a8)
			gaussianA8.resized(sceneSize).draw(ColorF{ a8 });
	}

}

void GaussianFSAddon::DrawScene000() const
{
	if (m_NOWSCENE == U"SelectLang" || m_NOWSCENE == U"WinSizeScene")
	{
		Rect{ Scene::Size().x - 1, 0, 1, Scene::Size().y }.draw(Palette::Yellow);
		//Rect{ 0, Scene::Size().y - 1, Scene::Size().x, 1 }.draw(Palette::Yellow);
		Rect{ 0, 0, Scene::Size().x, 1 }.draw(Palette::Yellow);
		Rect{ 0, 0, 1, Scene::Size().y }.draw(Palette::Yellow);
		//下のエリア
		Rect{ 0, Scene::Size().y - 30, Scene::Size().x, 30 }.draw(Palette::Yellow);
	}
	else
	{
		// draw() とマウス座標にスケーリングを適用
		const Transformer2D screenScaling{ Mat3x2::Scale(m_SCALE).translated(m_OFFSET), TransformCursor::Yes };

		//
		Rect{ WINDOWSIZEWIDTH000 - 1, 0, 1, WINDOWSIZEHEIGHT000 }.draw(Palette::Yellow);
		//Rect{ 0, WINDOWSIZEHEIGHT000 - 1, WINDOWSIZEWIDTH000, 1 }.draw(Palette::Yellow);
		Rect{ 0, 0, WINDOWSIZEWIDTH000, 1 }.draw(Palette::Yellow);
		Rect{ 0, 0, 1, WINDOWSIZEHEIGHT000 }.draw(Palette::Yellow);
		//下のエリア
		Rect{ 0, WINDOWSIZEHEIGHT000 - 30, WINDOWSIZEWIDTH000, 30 }.draw(Palette::Yellow);
	}
}
void GaussianFSAddon::DrawScene001() const
{
	if (m_NOWSCENE == U"SelectLang" || m_NOWSCENE == U"WinSizeScene")
	{
		//exit
		GetStairs().draw(Palette::Black);
		Shape2D::Stairs(Vec2{ Scene::Size().x - 5, Scene::Size().y - 5 }, 20, 20, 4).draw(Palette::Black);
	}
	else
	{
		const Transformer2D screenScaling{ Mat3x2::Scale(m_SCALE).translated(m_OFFSET), TransformCursor::Yes };
		//exit
		GetStairs().draw(Palette::Black);
	}
}
