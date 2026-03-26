#pragma once
# include <Siv3D.hpp>

class LabelAddon : public IAddon
{
public:
	using LabelHandle = uint64;

	struct LabelSpec
	{
		String text;
		Vec2 pos{ 0.0, 0.0 };
		Font font;
		ColorF color = ColorF{ 1.0 };
		int32 zIndex = 0;
		bool center = false;
		bool visible = true;
	};

	struct VerticalStyle
	{
		Vec2 origin{ 0.0, 0.0 };
		double lineHeight = 24.0;
		double spacing = 0.0;
		Font font{ 14 };
		ColorF color = ColorF{ 1.0 };
	};

	struct VerticalLayout
	{
		Vec2 origin{ 0.0, 0.0 };
		double lineHeight = 24.0;
		double spacing = 0.0;
		int32 cursor = 0;
		Font defaultFont{ 14 };
		ColorF defaultColor = ColorF{ 1.0 };

		static VerticalLayout FromStyle(const VerticalStyle& style)
		{
			return VerticalLayout{}
				.WithOrigin(style.origin)
				.WithLineHeight(style.lineHeight)
				.WithSpacing(style.spacing)
				.WithDefaultFont(style.font)
				.WithDefaultColor(style.color);
		}

		VerticalLayout WithOrigin(const Vec2& value) const
		{
			auto layout = *this;
			layout.origin = value;
			return layout;
		}

		VerticalLayout WithLineHeight(double value) const
		{
			auto layout = *this;
			layout.lineHeight = value;
			return layout;
		}

		VerticalLayout WithSpacing(double value) const
		{
			auto layout = *this;
			layout.spacing = value;
			return layout;
		}

		VerticalLayout WithDefaultFont(const Font& value) const
		{
			auto layout = *this;
			layout.defaultFont = value;
			return layout;
		}

		VerticalLayout WithDefaultColor(const ColorF& value) const
		{
			auto layout = *this;
			layout.defaultColor = value;
			return layout;
		}

		Vec2 GetPos(int32 index) const
		{
			return origin + Vec2{ 0.0, (lineHeight + spacing) * index };
		}

		Vec2 NextPos()
		{
			return GetPos(cursor++);
		}

		void Reset()
		{
			cursor = 0;
		}

		LabelSpec MakeSpec(String text, int32 index, int32 zIndex = 0, bool center = false, bool visible = true) const
		{
			return { .text = std::move(text), .pos = GetPos(index), .font = defaultFont, .color = defaultColor, .zIndex = zIndex, .center = center, .visible = visible };
		}

		LabelSpec MakeSpec(String text, int32 index, const Font& font, const ColorF& color, int32 zIndex = 0, bool center = false, bool visible = true) const
		{
			return { .text = std::move(text), .pos = GetPos(index), .font = font, .color = color, .zIndex = zIndex, .center = center, .visible = visible };
		}

		LabelSpec MakeSpecAtNext(String text, int32 zIndex = 0, bool center = false, bool visible = true)
		{
			return MakeSpec(std::move(text), cursor++, zIndex, center, visible);
		}

		LabelSpec MakeSpecAtNext(String text, const Font& font, const ColorF& color, int32 zIndex = 0, bool center = false, bool visible = true)
		{
			return MakeSpec(std::move(text), cursor++, font, color, zIndex, center, visible);
		}

		void Update(LabelHandle handle, String text, int32 index, int32 zIndex = 0, bool center = false, bool visible = true) const
		{
			LabelAddon::UpdateLabel(handle, MakeSpec(std::move(text), index, zIndex, center, visible));
		}

		void Update(LabelHandle handle, String text, int32 index, const Font& font, const ColorF& color, int32 zIndex = 0, bool center = false, bool visible = true) const
		{
			LabelAddon::UpdateLabel(handle, MakeSpec(std::move(text), index, font, color, zIndex, center, visible));
		}
	};

	static constexpr StringView AddonName{ U"LabelAddon" };

	static void Register()
	{
		Addon::Register<LabelAddon>(AddonName);
	}

	static LabelHandle AddLabel(LabelSpec spec)
	{
		if (auto p = Addon::GetAddon<LabelAddon>(AddonName))
		{
			return p->addLabelInternal(std::move(spec));
		}
		return 0;
	}

	static void UpdateLabel(LabelHandle handle, LabelSpec spec)
	{
		if (auto p = Addon::GetAddon<LabelAddon>(AddonName))
		{
			p->updateLabelInternal(handle, std::move(spec));
		}
	}

	static void RemoveLabel(LabelHandle handle)
	{
		if (auto p = Addon::GetAddon<LabelAddon>(AddonName))
		{
			p->removeLabelInternal(handle);
		}
	}

	static void ClearLabels()
	{
		if (auto p = Addon::GetAddon<LabelAddon>(AddonName))
		{
			p->clearLabelsInternal();
		}
	}

private:
	HashTable<LabelHandle, LabelSpec> m_labels;
	LabelHandle m_nextHandle = 1;

	bool init() override
	{
		return true;
	}

	bool update() override
	{
		return true;
	}

	void draw() const override
	{
		const auto handlesAsc = getSortedHandles(false);
		for (const auto handle : handlesAsc)
		{
			auto it = m_labels.find(handle);
			if (it == m_labels.end())
			{
				continue;
			}
			const auto& spec = it->second;
			if (!spec.visible)
			{
				continue;
			}
			if (spec.center)
			{
				spec.font(spec.text).drawAt(spec.pos, spec.color);
			}
			else
			{
				spec.font(spec.text).draw(spec.pos, spec.color);
			}
		}
	}

	LabelHandle addLabelInternal(LabelSpec spec)
	{
		const LabelHandle handle = m_nextHandle++;
		m_labels.emplace(handle, std::move(spec));
		return handle;
	}

	void updateLabelInternal(LabelHandle handle, LabelSpec spec)
	{
		if (auto it = m_labels.find(handle); it != m_labels.end())
		{
			it->second = std::move(spec);
		}
	}

	void removeLabelInternal(LabelHandle handle)
	{
		m_labels.erase(handle);
	}

	void clearLabelsInternal()
	{
		m_labels.clear();
	}

	Array<LabelHandle> getSortedHandles(bool desc) const
	{
		Array<LabelHandle> handles;
		handles.reserve(m_labels.size());
		for (const auto& [handle, _] : m_labels)
		{
			handles.push_back(handle);
		}
		std::sort(handles.begin(), handles.end(), [&](LabelHandle lhs, LabelHandle rhs)
			{
				const auto itL = m_labels.find(lhs);
				const auto itR = m_labels.find(rhs);
				const int32 zl = (itL == m_labels.end()) ? 0 : itL->second.zIndex;
				const int32 zr = (itR == m_labels.end()) ? 0 : itR->second.zIndex;
				if (zl == zr)
				{
					return desc ? (lhs > rhs) : (lhs < rhs);
				}
				return desc ? (zl > zr) : (zl < zr);
			});
		return handles;
	}
};
