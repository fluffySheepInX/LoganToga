#pragma once

#include "Remake2Common.h"
#include <utility>

class WindowChromeAddon : public s3d::IAddon
{
public:
	static constexpr s3d::StringView AddonName = U"WindowChromeAddon";

	static void Configure(const String& title, const ColorF& frameColor = ColorF{ 0.32, 0.58, 0.92 }, const ColorF& glowColor = ColorF{ 0.92, 0.56, 0.78 })
	{
		if (auto* addon = s3d::Addon::GetAddon<WindowChromeAddon>(AddonName))
		{
			addon->m_title = title;
			addon->m_frameColor = frameColor;
			addon->m_glowColor = glowColor;
			Window::SetStyle(s3d::WindowStyle::Frameless);
			addon->ensureTextures(Scene::Size());
		}
	}

private:
	bool update() override
	{
		ensureTextures(Scene::Size());
		handleCloseButton();
		handleWindowDrag();
		return true;
	}

	void draw() const override
	{
		drawGlow();
		drawChrome();
	}

	void ensureTextures(const s3d::Size& sceneSize)
	{
		if ((m_sceneSize == sceneSize) && m_gaussianA1 && m_gaussianA4 && m_gaussianA8)
		{
			return;
		}

		m_sceneSize = sceneSize;
		m_gaussianA1 = s3d::RenderTexture{ m_sceneSize };
		m_gaussianB1 = s3d::RenderTexture{ m_sceneSize };
		m_gaussianA4 = s3d::RenderTexture{ m_sceneSize / 4 };
		m_gaussianB4 = s3d::RenderTexture{ m_sceneSize / 4 };
		m_gaussianA8 = s3d::RenderTexture{ m_sceneSize / 8 };
		m_gaussianB8 = s3d::RenderTexture{ m_sceneSize / 8 };
	}

	[[nodiscard]] RectF getBottomBarRect() const
	{
		return RectF{ 0, static_cast<double>(m_sceneSize.y) - m_bottomBarHeight, static_cast<double>(m_sceneSize.x), m_bottomBarHeight };
	}

	[[nodiscard]] RectF getLeftBorderRect() const
	{
		return RectF{ 0, 0, m_borderThickness, static_cast<double>(m_sceneSize.y) };
	}

	[[nodiscard]] RectF getRightBorderRect() const
	{
		return RectF{ static_cast<double>(m_sceneSize.x) - m_borderThickness, 0, m_borderThickness, static_cast<double>(m_sceneSize.y) };
	}

	[[nodiscard]] RectF getCloseButtonRect() const
	{
		return RectF{ static_cast<double>(m_sceneSize.x) - m_closeButtonSize - 6.0, static_cast<double>(m_sceneSize.y) - m_bottomBarHeight + 3.0, m_closeButtonSize, m_bottomBarHeight - 6.0 };
	}

	[[nodiscard]] RectF getDragRect() const
	{
		return RectF{ 0, static_cast<double>(m_sceneSize.y) - m_bottomBarHeight, static_cast<double>(m_sceneSize.x) - (m_closeButtonSize + 12.0), m_bottomBarHeight };
	}

	void handleCloseButton()
	{
		if (getCloseButtonRect().leftClicked())
		{
			s3d::System::Exit();
		}
	}

	void handleWindowDrag()
	{
		const RectF dragRect = getDragRect();
		if (!m_dragStartWindow && MouseL.down() && dragRect.mouseOver() && !getCloseButtonRect().mouseOver())
		{
			m_dragStartWindow = std::pair{ Cursor::ScreenPos(), Window::GetState().bounds.pos };
		}

		if (!m_dragStartWindow)
		{
			return;
		}

		if (MouseL.pressed())
		{
			Window::SetPos(m_dragStartWindow->second + (Cursor::ScreenPos() - m_dragStartWindow->first));
		}
		else
		{
			m_dragStartWindow.reset();
		}
	}

	void drawGlow() const
	{
		if (!m_gaussianA1)
		{
			return;
		}

		drawGlowSource();

		{
			const s3d::ScopedRenderTarget2D target{ m_gaussianA1.clear(ColorF{ 0.0 }) };
			const s3d::ScopedRenderStates2D blend{ s3d::BlendState::Additive };
			drawGlowSource();
		}

		s3d::Shader::GaussianBlur(m_gaussianA1, m_gaussianB1, m_gaussianA1);
		s3d::Shader::Downsample(m_gaussianA1, m_gaussianA4);
		s3d::Shader::GaussianBlur(m_gaussianA4, m_gaussianB4, m_gaussianA4);
		s3d::Shader::Downsample(m_gaussianA4, m_gaussianA8);
		s3d::Shader::GaussianBlur(m_gaussianA8, m_gaussianB8, m_gaussianA8);

		{
			const s3d::ScopedRenderStates2D blend{ s3d::BlendState::Additive };
			if (m_a1) m_gaussianA1.resized(m_sceneSize).draw(ColorF{ m_a1 });
			if (m_a4) m_gaussianA4.resized(m_sceneSize).draw(ColorF{ m_a4 });
			if (m_a8) m_gaussianA8.resized(m_sceneSize).draw(ColorF{ m_a8 });
		}
	}

	void drawGlowSource() const
	{
		//right area
		Rect{ m_sceneSize.x - 1, 0, 30, m_sceneSize.y }.draw(m_frameColor);
		//top area
		Rect{ 0, -29, m_sceneSize.x, 30 }.draw(m_frameColor);
		//left area
		Rect{ -29, 0, 30, m_sceneSize.y }.draw(m_frameColor);
		//bottom area
		Rect{ 0, m_sceneSize.y - 30, m_sceneSize.x, 30 }.draw(m_frameColor);
	}

	void drawChrome() const
	{
		const RectF bottomBarRect = getBottomBarRect();
		const RectF closeButtonRect = getCloseButtonRect();
		const bool closeHovered = closeButtonRect.mouseOver();

		closeButtonRect.draw(closeHovered ? ColorF{ 0.60, 0.16, 0.20, 0.95 } : ColorF{ 0.20, 0.11, 0.16, 0.90 });
		closeButtonRect.drawFrame(1.0, m_frameColor);
		Line{ closeButtonRect.tl().movedBy(5, 4), closeButtonRect.br().movedBy(-5, -4) }.draw(1.5, Palette::White);
		Line{ closeButtonRect.tr().movedBy(-5, 4), closeButtonRect.bl().movedBy(5, -4) }.draw(1.5, Palette::White);
	}

	String m_title = U"LoganToga2Remake2";
	ColorF m_frameColor{ 0.32, 0.58, 0.92 };
	ColorF m_glowColor{ 0.92, 0.56, 0.78, 0.85 };
	double m_bottomBarHeight = 24.0;
	double m_borderThickness = 2.0;
	double m_innerGlowThickness = 8.0;
	double m_closeButtonSize = 30.0;
	s3d::Size m_sceneSize{ 0, 0 };
	s3d::RenderTexture m_gaussianA1;
	s3d::RenderTexture m_gaussianB1;
	s3d::RenderTexture m_gaussianA4;
	s3d::RenderTexture m_gaussianB4;
	s3d::RenderTexture m_gaussianA8;
	s3d::RenderTexture m_gaussianB8;
	double m_a1 = 0.0;
	double m_a4 = 0.0;
	double m_a8 = 0.5;
	Font m_titleFont{ FontMethod::MSDF, 12, Typeface::Medium };
	Optional<std::pair<s3d::Point, s3d::Point>> m_dragStartWindow;
};
