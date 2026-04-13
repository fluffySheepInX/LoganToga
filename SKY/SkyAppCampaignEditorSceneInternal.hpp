# pragma once
# include "SkyAppInternal.hpp"
# include "SkyAppUiInternal.hpp"

namespace SkyAppInternal::CampaignEditorDetail
{
	enum class EditorActionButtonStyle
	{
		Primary,
		Secondary,
		Back,
	};

	inline void DrawEditorSection(const RectF& rect, const StringView title, const Font& font)
	{
		rect.rounded(18).draw(ColorF{ 0.10, 0.15, 0.24, 0.82 });
		rect.rounded(18).drawFrame(1.5, 0, ColorF{ 0.34, 0.46, 0.62, 0.74 });
        font(title).draw(rect.pos.movedBy(18, 14), SkyAppSupport::UiInternal::EditorTextOnDarkPrimaryColor());
	}

	inline void DrawEditorActionButton(const RectF& rect, const Font& font, const StringView label, const EditorActionButtonStyle style, const bool enabled = true)
	{
		const bool hovered = enabled && rect.mouseOver();
		ColorF fillColor;
		ColorF frameColor;
		ColorF textColor;

		if (not enabled)
		{
			fillColor = ColorF{ 0.16, 0.18, 0.22, 0.72 };
			frameColor = ColorF{ 0.38, 0.44, 0.52, 0.72 };
           textColor = SkyAppSupport::UiInternal::EditorTextOnDarkSecondaryColor();
		}
		else
		{
			switch (style)
			{
			case EditorActionButtonStyle::Primary:
				fillColor = (hovered ? ColorF{ 0.34, 0.52, 0.88 } : ColorF{ 0.26, 0.42, 0.74 });
				frameColor = ColorF{ 0.86, 0.93, 1.0, 0.92 };
              textColor = SkyAppSupport::UiInternal::EditorTextOnDarkPrimaryColor();
				break;

			case EditorActionButtonStyle::Back:
				fillColor = (hovered ? ColorF{ 0.14, 0.20, 0.30 } : ColorF{ 0.11, 0.16, 0.24 });
				frameColor = ColorF{ 0.56, 0.66, 0.80, 0.82 };
             textColor = SkyAppSupport::UiInternal::EditorTextOnDarkPrimaryColor();
				break;

			case EditorActionButtonStyle::Secondary:
			default:
				fillColor = (hovered ? ColorF{ 0.20, 0.30, 0.46 } : ColorF{ 0.15, 0.24, 0.38 });
				frameColor = ColorF{ 0.74, 0.84, 0.96, 0.86 };
             textColor = SkyAppSupport::UiInternal::EditorTextOnDarkPrimaryColor();
				break;
			}
		}

		rect.rounded(14).draw(fillColor);
		rect.rounded(14).drawFrame(2, 0, frameColor);
		font(label).drawAt(rect.center(), textColor);
	}

	inline void DrawEditorHintPopup(const RectF& rect, const Font& font, const Array<String>& lines)
	{
     if (lines.isEmpty())
		{
			return;
		}

		double maxTextWidth = 0.0;
		for (const auto& line : lines)
		{
			maxTextWidth = Max(maxTextWidth, font(line).region().w);
		}

		const double paddingX = 14.0;
		const double paddingY = 12.0;
		const double lineHeight = 24.0;
     const double width = Max(180.0, (maxTextWidth + paddingX * 2));
		const double height = (paddingY * 2 + lineHeight * lines.size());
		const double preferredX = (rect.centerX() - width + 28.0);
		const double preferredY = (rect.y - height - 10.0);
		const RectF popupRect{
			Clamp(preferredX, 12.0, (Scene::Width() - width - 12.0)),
			Clamp(preferredY, 12.0, (Scene::Height() - height - 12.0)),
			width,
			height,
		};
		popupRect.rounded(12).draw(ColorF{ 0.08, 0.10, 0.14, 0.98 });
		popupRect.rounded(12).drawFrame(1.5, 0, ColorF{ 0.74, 0.84, 0.96, 0.84 });

		for (size_t i = 0; i < lines.size(); ++i)
		{
            font(lines[i]).draw(popupRect.pos.movedBy(paddingX, paddingY + i * lineHeight), SkyAppSupport::UiInternal::EditorTextOnDarkPrimaryColor());
		}

       const Vec2 arrowBase{ Clamp(rect.centerX(), popupRect.x + 18.0, popupRect.rightX() - 18.0), popupRect.bottomY() };
		Triangle{ arrowBase.movedBy(-10, 0), arrowBase.movedBy(10, 0), arrowBase.movedBy(0, 10) }.draw(ColorF{ 0.08, 0.10, 0.14, 0.98 });
	}

	[[nodiscard]] inline String MakePreviewText(const StringView text, const size_t maxLength = 84)
	{
		String preview = String{ text };
		preview = preview.replaced(U'\n', U' ');
		preview = preview.replaced(U'\r', U' ');

		if (preview.size() <= maxLength)
		{
			return preview;
		}

		return (preview.substr(0, maxLength) + U"...");
	}

	[[nodiscard]] inline String MakeDialogueSummary(const StringView text)
	{
		const Array<String> lines = SplitDialogueText(text);
		if (lines.isEmpty())
		{
			return U"No dialogue";
		}

		return U"{} lines / {}"_fmt(lines.size(), MakePreviewText(lines.front(), 42));
	}

	struct WrappedEditorLine
	{
		String text;
		size_t startIndex = 0;
		size_t endIndex = 0;
	};

	[[nodiscard]] inline Array<WrappedEditorLine> BuildWrappedEditorLines(const StringView text, const Font& font, const double maxWidth)
	{
		Array<WrappedEditorLine> lines;
		String current;
		size_t currentStart = 0;
		size_t index = 0;

		for (const char32 ch : text)
		{
			if (ch == U'\r')
			{
				++index;
				continue;
			}

			if (ch == U'\n')
			{
				lines << WrappedEditorLine{ current, currentStart, index };
				current.clear();
				++index;
				currentStart = index;
				continue;
			}

			const String candidate = (current + ch);
			if ((not current.isEmpty()) && (font(candidate).region().w > maxWidth))
			{
				lines << WrappedEditorLine{ current, currentStart, index };
				current = String{ ch };
				currentStart = index;
				++index;
			}
			else
			{
				current = candidate;
				++index;
			}
		}

		lines << WrappedEditorLine{ current, currentStart, index };

		if (lines.isEmpty())
		{
			lines << WrappedEditorLine{};
		}

		return lines;
	}
}
