# pragma once
# include <Siv3D.hpp>

namespace ui
{
    inline const Font& DefaultFont()
    {
        static const Font font{ FontMethod::MSDF, 20, Typeface::Medium };
        return font;
    }

    struct Theme
    {
        ColorF panel{ 0.96, 0.97, 0.99, 0.92 };
        ColorF panelBorder{ 0.65, 0.72, 0.80, 1.0 };
        ColorF section{ 0.90, 0.93, 0.97, 0.96 };
        ColorF item{ 1.0, 1.0, 1.0, 0.96 };
        ColorF itemHovered{ 0.92, 0.96, 1.0, 1.0 };
        ColorF itemPressed{ 0.84, 0.91, 1.0, 1.0 };
        ColorF accent{ 0.25, 0.55, 0.95, 1.0 };
        ColorF text{ 0.12, 0.14, 0.18, 1.0 };
        ColorF textMuted{ 0.38, 0.42, 0.48, 1.0 };
    };

    inline const Theme& GetTheme()
    {
        static const Theme theme;
        return theme;
    }

    inline void Panel(const RectF& rect, const Theme& theme = GetTheme())
    {
        rect.rounded(10).draw(theme.panel);
        rect.rounded(10).drawFrame(1.5, theme.panelBorder);
    }

    inline void Section(const RectF& rect, const Theme& theme = GetTheme())
    {
        rect.rounded(8).draw(theme.section);
        rect.rounded(8).drawFrame(1.0, theme.panelBorder);
    }

    inline bool Button(const Font& font, StringView text, const RectF& rect, const Theme& theme = GetTheme())
    {
        const bool hovered = rect.mouseOver();
        const bool pressed = hovered && MouseL.pressed();
        const ColorF fill = pressed ? theme.itemPressed : (hovered ? theme.itemHovered : theme.item);

        rect.rounded(6).draw(fill);
        rect.rounded(6).drawFrame(1.0, theme.panelBorder);
        font(text).drawAt(rect.center(), theme.text);
        return rect.leftClicked();
    }

    inline double RadioListHeight(const size_t itemCount, const double rowHeight = 34.0)
    {
        return itemCount * rowHeight;
    }

    inline bool RadioList(const Font& font, size_t& index, const Array<String>& items, const RectF& rect,
        const double rowHeight = 34.0, const Theme& theme = GetTheme())
    {
        bool changed = false;

        for (size_t i = 0; i < items.size(); ++i)
        {
            const RectF row{ rect.x, rect.y + i * rowHeight, rect.w, rowHeight - 2 };
            const bool selected = (index == i);
            const bool hovered = row.mouseOver();
            const bool pressed = hovered && MouseL.pressed();
            const ColorF fill = selected
                ? (pressed ? ColorF{ 0.80, 0.89, 1.0, 1.0 } : ColorF{ 0.88, 0.94, 1.0, 1.0 })
                : (pressed ? theme.itemPressed : (hovered ? theme.itemHovered : theme.item));

            row.rounded(6).draw(fill);
            row.rounded(6).drawFrame(1.0, theme.panelBorder);

            const Circle bullet{ row.x + 16, row.centerY(), 8 };
            bullet.draw(Palette::White);
            bullet.drawFrame(2.0, theme.textMuted);
            if (selected)
            {
                Circle{ bullet.center, 4 }.draw(theme.accent);
            }

            font(items[i]).draw(row.x + 32, row.y + 6, theme.text);

            if (row.leftClicked())
            {
                index = i;
                changed = true;
            }
        }

        return changed;
    }

    struct SliderResult
    {
        bool changed = false;
        bool hovered = false;
        bool decreaseClicked = false;
        bool increaseClicked = false;
    };

    inline SliderResult SliderHEx(StringView label, double& value, const double min, const double max,
        const Vec2& pos, const double labelWidth = 130.0, const double sliderWidth = 200.0,
        const Theme& theme = GetTheme())
    {
        const Font& font = DefaultFont();
        const RectF labelRect{ pos, labelWidth, 34 };
        const double rightAreaX = (pos.x + labelWidth + 10.0);
        const double buttonSize = 24.0;
        const double buttonGap = 4.0;
        const double trackWidth = Max(44.0, sliderWidth - ((buttonSize * 2.0) + (buttonGap * 3.0)));
        const RectF trackRect{ rightAreaX, pos.y + 14, trackWidth, 6 };
        const RectF decreaseRect{ trackRect.x + trackRect.w + buttonGap, pos.y + 5, buttonSize, buttonSize };
        const RectF increaseRect{ decreaseRect.x + buttonSize + buttonGap, pos.y + 5, buttonSize, buttonSize };

        const double clampedValue = Clamp(value, min, max);
        const double t = (max > min) ? ((clampedValue - min) / (max - min)) : 0.0;
        const Vec2 knobCenter{ trackRect.x + trackRect.w * t, trackRect.centerY() };
        const Circle knob{ knobCenter, 10 };

        static Optional<size_t> activeSlider;
        const size_t sliderId = reinterpret_cast<size_t>(&value);
        const RectF hitRect = trackRect.stretched(12, 12);

        if (MouseL.down() && (labelRect.mouseOver() || hitRect.mouseOver() || knob.mouseOver()))
        {
            activeSlider = sliderId;
        }

        SliderResult result;
        if (activeSlider && (*activeSlider == sliderId))
        {
            if (MouseL.pressed())
            {
                const double nt = Clamp((Cursor::PosF().x - trackRect.x) / trackRect.w, 0.0, 1.0);
                const double newValue = Min(max, Max(min, (min + (max - min) * nt)));
                if (newValue != value)
                {
                    value = newValue;
                    result.changed = true;
                }
            }
            else
            {
                activeSlider.reset();
            }
        }

        const bool hovered = labelRect.mouseOver() || hitRect.mouseOver() || knob.mouseOver() || decreaseRect.mouseOver() || increaseRect.mouseOver()
            || (activeSlider && (*activeSlider == sliderId));
        result.hovered = hovered;
        result.decreaseClicked = decreaseRect.leftClicked();
        result.increaseClicked = increaseRect.leftClicked();

        labelRect.rounded(6).draw(theme.item);
        labelRect.rounded(6).drawFrame(1.0, theme.panelBorder);
        font(label).draw(labelRect.x + 10, labelRect.y + 6, theme.text);

        trackRect.rounded(3).draw(ColorF{ 0.78, 0.82, 0.88, 1.0 });
        RectF{ trackRect.pos, Max(0.0, knobCenter.x - trackRect.x), trackRect.h }.rounded(3).draw(theme.accent);
        knob.draw(hovered ? ColorF{ 1.0, 1.0, 1.0, 1.0 } : ColorF{ 0.96, 0.98, 1.0, 1.0 });
        knob.drawFrame(2.0, theme.panelBorder);

        for (const auto& buttonInfo : { std::pair{ decreaseRect, U"-" }, std::pair{ increaseRect, U"+" } })
        {
            const bool buttonHovered = buttonInfo.first.mouseOver();
            const bool pressed = buttonHovered && MouseL.pressed();
            const ColorF fill = pressed ? theme.itemPressed : (buttonHovered ? theme.itemHovered : theme.item);
            buttonInfo.first.rounded(5).draw(fill);
            buttonInfo.first.rounded(5).drawFrame(1.0, theme.panelBorder);
            font(buttonInfo.second).drawAt(buttonInfo.first.center(), theme.text);
        }

        return result;
    }

    inline bool SliderH(StringView label, double& value, const double min, const double max,
        const Vec2& pos, const double labelWidth = 130.0, const double sliderWidth = 200.0,
        const Theme& theme = GetTheme())
    {
        return SliderHEx(label, value, min, max, pos, labelWidth, sliderWidth, theme).changed;
    }

    inline void Tooltip(const Font& font, StringView text, Vec2 pos)
    {
        const RectF textRegion = font(text).region();
        RectF tooltip{ pos, (textRegion.w + 18), (textRegion.h + 14) };
        tooltip.x = Min(tooltip.x, (Scene::Width() - tooltip.w - 12));
        tooltip.y = Min(tooltip.y, (Scene::Height() - tooltip.h - 12));
        tooltip.x = Max(tooltip.x, 12.0);
        tooltip.y = Max(tooltip.y, 12.0);

        tooltip.rounded(8).draw(ColorF{ 0.10, 0.13, 0.18, 0.94 });
        tooltip.rounded(8).drawFrame(1.0, ColorF{ 0.70, 0.80, 0.96, 0.95 });
        font(text).draw(tooltip.pos.movedBy(9, 7), Palette::White);
    }
}
