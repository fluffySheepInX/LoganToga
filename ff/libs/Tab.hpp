# pragma once
# include <Siv3D.hpp>

class ITab
{
public:

	ITab(const SizeF& tabSize, const Array<String>& items)
		: m_tabSize{ tabSize }
		, m_items{ items } {}

	virtual ~ITab() = default;

	virtual void draw(const Vec2& pos, const Font& font, const ColorF& color, const ColorF& outlineColor) const = 0;

	virtual size_t getTabCount() const noexcept
	{
		return m_items.size();
	}

	virtual size_t getActiveTabIndex() const noexcept
	{
		return m_activeIndex;
	}

	virtual void setActiveTabIndex(size_t index) noexcept
	{
		assert(index < m_items.size());
		m_activeIndex = index;
	}

	virtual void advance(int32 offset, bool wrapAround = false)
	{
		assert(InRange(offset, -1, 1));

		if (offset == -1)
		{
			if (m_activeIndex == 0)
			{
				if (wrapAround)
				{
					m_activeIndex = (m_items.size() - 1);
				}
			}
			else
			{
				--m_activeIndex;
			}
		}
		else if (offset == 1)
		{
			if (m_activeIndex == (m_items.size() - 1))
			{
				if (wrapAround)
				{
					m_activeIndex = 0;
				}
			}
			else
			{
				++m_activeIndex;
			}
		}
	}

protected:

	template <class F>
	void drawLabels(const Font& font, F&& getRect) const
	{
		for (size_t i = 0; i < m_items.size(); ++i)
		{
			const RectF tab = getRect(i);

			drawLabel(tab, font, m_items[i]);
		}
	}

	virtual void drawLabel(const RectF& tab, const Font& font, const String& text) const
	{
		font(text).drawAt(TextStyle::Shadow(Vec2{ 2.5, 2.5 }, ColorF{ 0.0, 0.6 }), 20, tab.center());
	}

	SizeF m_tabSize;

	Array<String> m_items;

	size_t m_activeIndex = 0;
};

class TabA : public ITab
{
public:

	TabA(const SizeF& tabSize, const Array<String>& items)
		: ITab{ tabSize, items }
		, m_step{ tabSize.x + tabSize.x * 0.14 } {}

	void draw(const Vec2& pos, const Font& font, const ColorF& color, const ColorF& outlineColor) const override
	{
		const auto getRect = [this, &pos](size_t i)
		{
			return RectF{ pos.x + (i * m_step), pos.y, m_tabSize };
		};

		for (size_t i = 0; i < m_items.size(); ++i)
		{
			const RectF tab = getRect(i);

			if (i == m_activeIndex)
			{
				tab.draw(color);
			}
			else
			{
				tab.drawFrame(3, 0, outlineColor);
			}
		}

		drawLabels(font, getRect);
	}

private:

	double m_step = 0.0;
};

class TabB : public ITab
{
public:

	TabB(const SizeF& tabSize, const Array<String>& items)
		: ITab{ tabSize, items }
		, m_step{ tabSize.x + tabSize.x * 0.14 } {}

	void draw(const Vec2& pos, const Font& font, const ColorF& color, const ColorF& outlineColor) const override
	{
		const double radius = (m_tabSize.y * 0.25);

		const auto getRect = [this, &pos](size_t i)
		{
			return RectF{ pos.x + (i * m_step), pos.y, m_tabSize };
		};

		for (size_t i = 0; i < m_items.size(); ++i)
		{
			const RectF tab = getRect(i);

			if (i == m_activeIndex)
			{
				tab.rounded(radius, radius, 0, 0).draw(color);
			}
			else
			{
				tab.stretched(-1.5).rounded(radius, radius, 0, 0).drawFrame(3, outlineColor);
			}
		}

		drawLabels(font, getRect);
	}

private:

	double m_step = 0.0;
};

class TabC : public ITab
{
public:

	TabC(const SizeF& tabSize, const Array<String>& items)
		: ITab{ tabSize, items } {}

	void draw(const Vec2& pos, const Font& font, const ColorF& color, const ColorF& outlineColor) const override
	{
		constexpr double Thickness = 3.0;
		const double radius = (m_tabSize.y * 0.5);

		const auto getRect = [this, &pos](size_t i)
		{
			return RectF{ (pos.x + i * (m_tabSize.x - Thickness)), pos.y, m_tabSize };
		};

		for (size_t i = 0; i < m_items.size(); ++i)
		{
			const RectF tab = getRect(i);

			if (i == 0) // ⊂
			{
				tab.stretched(-Thickness * 0.5).rounded(radius, 0, 0, radius).drawFrame(Thickness, outlineColor);
			}
			else if (i == (m_items.size() - 1)) // ⊃
			{
				tab.stretched(-Thickness * 0.5).rounded(0, radius, radius, 0).drawFrame(Thickness, outlineColor);
			}
			else // □
			{
				tab.drawFrame(Thickness, 0, outlineColor);
			}
		}

		for (size_t i = 0; i < m_items.size(); ++i)
		{
			const RectF tab = getRect(i);

			if (i == m_activeIndex)
			{
				if (i == 0) // ⊂
				{
					tab.rounded(radius, 0, 0, radius).draw(color);
				}
				else if (i == (m_items.size() - 1)) // ⊃
				{
					tab.rounded(0, radius, radius, 0).draw(color);
				}
				else // □
				{
					tab.draw(color);
				}
			}
		}

		drawLabels(font, getRect);
	}
};

class TabD : public ITab
{
public:

	TabD(const SizeF& tabSize, const Array<String>& items)
		: ITab{ tabSize, items } {}

	void draw(const Vec2& pos, const Font& font, const ColorF& color, const ColorF& outlineColor) const override
	{
		constexpr double Thickness = 3.0;
		const double radius = (m_tabSize.y * 0.5);
		const double smallRadius = (m_tabSize.y * 0.1);
		const double width = ((m_items.size() - 1) * (m_tabSize.x - Thickness) + m_tabSize.x);

		const auto getRect = [this, &pos](size_t i)
		{
			return RectF{ (pos.x + i * (m_tabSize.x - Thickness)), pos.y, m_tabSize };
		};

		RectF{ pos.x, pos.y + 3, width, m_tabSize.y - 6 }.stretched(-Thickness * 0.5).rounded(radius).drawFrame(Thickness, outlineColor);

		for (size_t i = 0; i < m_items.size(); ++i)
		{
			const RectF tab = getRect(i);

			if (i == m_activeIndex)
			{
				if (i == 0) // ⊂
				{
					tab.rounded(radius, smallRadius, smallRadius, radius).draw(color);
				}
				else if (i == (m_items.size() - 1)) // ⊃
				{
					tab.rounded(smallRadius, radius, radius, smallRadius).draw(color);
				}
				else // □
				{
					tab.rounded(smallRadius).draw(color);
				}
			}
		}

		drawLabels(font, getRect);
	}
};

class TabE : public ITab
{
public:

	TabE(const SizeF& tabSize, const Array<String>& items)
		: ITab{ tabSize, items } {}

	void draw(const Vec2& pos, const Font& font, const ColorF& color, const ColorF& outlineColor) const override
	{
		constexpr double Thickness = 3.0;
		const double shear = (m_tabSize.y * 0.2);
		const double radius = (m_tabSize.y * 0.5);
		const double width = ((m_items.size() - 1) * (m_tabSize.x - Thickness) + m_tabSize.x);

		const auto getRect = [this, &pos](size_t i)
		{
			return RectF{ (pos.x + i * (m_tabSize.x - Thickness)), pos.y, m_tabSize };
		};

		RectF{ pos.x, pos.y + 3, width, m_tabSize.y - 6 }.stretched(-Thickness * 0.5).rounded(radius).drawFrame(Thickness, outlineColor);

		for (size_t i = 0; i < m_items.size(); ++i)
		{
			const RectF tab = getRect(i);

			if (i == m_activeIndex)
			{
				if (i == 0) // ⊂
				{
					tab.stretched(0, -shear, 0, 0).rounded(radius, 0, 0, radius).draw(color);
					Triangle{ tab.tr().movedBy(-shear, 0),  tab.tr().movedBy(shear, 0), tab.br().movedBy(-shear, 0) }.draw(color);
				}
				else if (i == (m_items.size() - 1)) // ⊃
				{
					Triangle{ tab.tl().movedBy(shear, 0), tab.bl().movedBy(shear, 0), tab.bl().movedBy(-shear, 0) }.draw(color);
					tab.stretched(0, 0, 0, -shear).rounded(0, radius, radius, 0).draw(color);
				}
				else // □
				{
					tab.shearedX(shear).draw(color);
				}
			}
		}

		drawLabels(font, getRect);
	}

protected:

	void drawLabel(const RectF& tab, const Font& font, const String& text) const override
	{
		const Transformer2D tr{ Mat3x2::ShearX(0.35).translated(tab.center()) };

		font(text).drawAt(TextStyle::Shadow(Vec2{ 2.5, 2.5 }, ColorF{ 0.0, 0.6 }), 20, Vec2{ 0, 0 });
	}
};
