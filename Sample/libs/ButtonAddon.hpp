#pragma once
# include <Siv3D.hpp>

class ButtonAddon : public IAddon
{
public:
	using ButtonHandle = uint64;
	using Command = int32;

	struct ButtonStyle
	{
		ColorF background = ColorF{ 0.15, 0.18, 0.22, 0.9 };
		ColorF hover = ColorF{ 0.25, 0.32, 0.40, 0.9 };
		ColorF frame = ColorF{ 0.7, 0.8, 0.9, 0.8 };
		ColorF text = ColorF{ 0.95, 0.95, 0.95 };
		double frameThickness = 2.0;
	};

	struct ButtonSpec
	{
		Rect rect;
		String text;
		Font font;
		Command command = 0;
		int32 zIndex = 0;
		ButtonStyle style = {};
	};

	static constexpr StringView AddonName{ U"ButtonAddon" };

	static void Register()
	{
		Addon::Register<ButtonAddon>(AddonName);
	}

	static ButtonHandle AddButton(ButtonSpec spec)
	{
		if (auto p = Addon::GetAddon<ButtonAddon>(AddonName))
		{
			return p->addButtonInternal(std::move(spec));
		}
		return 0;
	}

	static void ClearButtons()
	{
		if (auto p = Addon::GetAddon<ButtonAddon>(AddonName))
		{
			p->clearButtonsInternal();
		}
	}

	static void SetEnabled(ButtonHandle handle, bool enabled)
	{
		if (auto p = Addon::GetAddon<ButtonAddon>(AddonName))
		{
			p->setEnabledInternal(handle, enabled);
		}
	}

	static void SetOnCommand(std::function<void(Command)> handler)
	{
		if (auto p = Addon::GetAddon<ButtonAddon>(AddonName))
		{
			p->setOnCommandInternal(std::move(handler));
		}
	}

	static void RequestCommand(Command command)
	{
		if (auto p = Addon::GetAddon<ButtonAddon>(AddonName))
		{
			p->requestCommandInternal(command);
		}
	}

private:
	struct ButtonState
	{
		ButtonSpec spec;
		bool enabled = true;
		double hover = 0.0;
		double click = 0.0;
	};

	HashTable<ButtonHandle, ButtonState> m_buttons;
	Array<Command> m_pendingCommands;
	ButtonHandle m_nextHandle = 1;
	std::function<void(Command)> m_onCommand;

	bool init() override
	{
		return true;
	}

	bool update() override
	{
		const double deltaTime = Scene::DeltaTime();
		Optional<ButtonHandle> topHit;
		const auto handlesDesc = getSortedHandles(true);
		for (const auto handle : handlesDesc)
		{
			auto it = m_buttons.find(handle);
			if (it == m_buttons.end())
			{
				continue;
			}
			auto& state = it->second;
			if (state.enabled && state.spec.rect.mouseOver())
			{
				topHit = handle;
				break;
			}
		}

		for (const auto handle : handlesDesc)
		{
			auto it = m_buttons.find(handle);
			if (it == m_buttons.end())
			{
				continue;
			}
			auto& state = it->second;
			const bool isHover = state.enabled && topHit.has_value() && (*topHit == handle);

			if (isHover)
			{
				state.hover = Min(state.hover + deltaTime * 4.0, 1.0);
				Cursor::RequestStyle(CursorStyle::Hand);
			}
			else
			{
				state.hover = Max(state.hover - deltaTime * 4.0, 0.0);
			}

			if (isHover && state.spec.rect.leftClicked())
			{
				state.click = 1.0;
				if (m_onCommand)
				{
					m_onCommand(state.spec.command);
				}
			}

			if (state.click > 0.0)
			{
				state.click = Max(state.click - deltaTime * 4.0, 0.0);
			}
		}

		if (!m_pendingCommands.isEmpty())
		{
			for (const auto command : m_pendingCommands)
			{
				if (m_onCommand)
				{
					m_onCommand(command);
				}
			}
			m_pendingCommands.clear();
		}

		return true;
	}

	void draw() const override
	{
		const auto handlesAsc = getSortedHandles(false);
		for (const auto handle : handlesAsc)
		{
			auto it = m_buttons.find(handle);
			if (it == m_buttons.end())
			{
				continue;
			}
			const auto& state = it->second;
			const auto& style = state.spec.style;
			const ColorF background = style.background.lerp(style.hover, state.hover);
			state.spec.rect.draw(background);
			state.spec.rect.drawFrame(style.frameThickness, style.frame);
			state.spec.font(state.spec.text).drawAt(state.spec.rect.center(), style.text);
		}
	}

	ButtonHandle addButtonInternal(ButtonSpec spec)
	{
		const ButtonHandle handle = m_nextHandle++;
		m_buttons.emplace(handle, ButtonState{ .spec = std::move(spec) });
		return handle;
	}

	void clearButtonsInternal()
	{
		m_buttons.clear();
	}

	void setEnabledInternal(ButtonHandle handle, bool enabled)
	{
		if (auto it = m_buttons.find(handle); it != m_buttons.end())
		{
			it->second.enabled = enabled;
		}
	}

	void setOnCommandInternal(std::function<void(Command)> handler)
	{
		m_onCommand = std::move(handler);
	}

	void requestCommandInternal(Command command)
	{
		m_pendingCommands.push_back(command);
	}

	Array<ButtonHandle> getSortedHandles(bool desc) const
	{
		Array<ButtonHandle> handles;
		handles.reserve(m_buttons.size());
		for (const auto& [handle, _] : m_buttons)
		{
			handles.push_back(handle);
		}
		std::sort(handles.begin(), handles.end(), [&](ButtonHandle lhs, ButtonHandle rhs)
			{
				const auto itL = m_buttons.find(lhs);
				const auto itR = m_buttons.find(rhs);
				const int32 zl = (itL == m_buttons.end()) ? 0 : itL->second.spec.zIndex;
				const int32 zr = (itR == m_buttons.end()) ? 0 : itR->second.spec.zIndex;
				if (zl == zr)
				{
					return desc ? (lhs > rhs) : (lhs < rhs);
				}
				return desc ? (zl > zr) : (zl < zr);
			});
		return handles;
	}
};
