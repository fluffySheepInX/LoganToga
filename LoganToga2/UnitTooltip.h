#pragma once
#include "stdafx.h"

class UnitTooltip
{
public:
	// Constructor to receive the font dependency
	explicit UnitTooltip(const Font& font);

	void show(const Vec2& pos, const String& text);

	void hide();

	void draw() const;

private:
	void updateRenderTexture();

	bool m_isVisible = false;
	Vec2 m_position;
	String m_content;
	RenderTexture m_renderTexture;
	Stopwatch m_fadeTimer;
	String m_lastRenderedContent;
	const Font& m_font;
};
