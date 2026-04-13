# pragma once
# include "MainSettings.hpp"

namespace SkyAppSupport
{
	namespace UiInternal
	{
		enum class EditorTextColorSlot
		{
			DarkPrimary,
			DarkSecondary,
			DarkAccent,
			LightPrimary,
			LightSecondary,
			LightAccent,
			CardPrimary,
			CardSecondary,
			SelectedPrimary,
			SelectedSecondary,
			Warning,
			Error,
		};

		inline constexpr ColorF DefaultPanelBackgroundColor{ 0.98, 0.95 };
		inline constexpr ColorF DefaultPanelFrameColor{ 0.25 };
		inline constexpr ColorF DefaultPanelTitleColor{ 0.12 };
		inline constexpr FilePathView DefaultPanelNinePatchPath = U"texture/camera_settings_panel.png";
		inline constexpr int32 DefaultPanelNinePatchPatchSize = 16;

		[[nodiscard]] inline const MainSupport::EditorTextColorSettings& EditorTextColors()
		{
			return MainSupport::GetEditorTextColorSettings();
		}

		[[nodiscard]] inline const ColorF& EditorTextOnDarkPrimaryColor()
		{
			return EditorTextColors().darkPrimary;
		}

		[[nodiscard]] inline const ColorF& EditorTextOnDarkSecondaryColor()
		{
			return EditorTextColors().darkSecondary;
		}

		[[nodiscard]] inline const ColorF& EditorTextOnDarkAccentColor()
		{
			return EditorTextColors().darkAccent;
		}

		[[nodiscard]] inline const ColorF& EditorTextOnLightPrimaryColor()
		{
			return EditorTextColors().lightPrimary;
		}

		[[nodiscard]] inline const ColorF& EditorTextOnLightSecondaryColor()
		{
			return EditorTextColors().lightSecondary;
		}

		[[nodiscard]] inline const ColorF& EditorTextOnLightAccentColor()
		{
			return EditorTextColors().lightAccent;
		}

		[[nodiscard]] inline const ColorF& EditorTextOnPanelPrimaryColor()
		{
			return EditorTextOnLightPrimaryColor();
		}

		[[nodiscard]] inline const ColorF& EditorTextOnPanelSecondaryColor()
		{
			return EditorTextOnLightSecondaryColor();
		}

		[[nodiscard]] inline const ColorF& EditorTextOnPanelAccentColor()
		{
			return EditorTextOnLightAccentColor();
		}

		[[nodiscard]] inline const ColorF& EditorTextOnCardPrimaryColor()
		{
			return EditorTextColors().cardPrimary;
		}

		[[nodiscard]] inline const ColorF& EditorTextOnCardSecondaryColor()
		{
			return EditorTextColors().cardSecondary;
		}

		[[nodiscard]] inline const ColorF& EditorTextOnSelectedPrimaryColor()
		{
			return EditorTextColors().selectedPrimary;
		}

		[[nodiscard]] inline const ColorF& EditorTextOnSelectedSecondaryColor()
		{
			return EditorTextColors().selectedSecondary;
		}

		[[nodiscard]] inline const ColorF& EditorTextWarningColor()
		{
			return EditorTextColors().warning;
		}

		[[nodiscard]] inline const ColorF& EditorTextErrorColor()
		{
			return EditorTextColors().error;
		}

		[[nodiscard]] inline StringView ToLabel(const EditorTextColorSlot slot)
		{
			switch (slot)
			{
			case EditorTextColorSlot::DarkPrimary:
				return U"Dark Primary";
			case EditorTextColorSlot::DarkSecondary:
				return U"Dark Secondary";
			case EditorTextColorSlot::DarkAccent:
				return U"Dark Accent";
			case EditorTextColorSlot::LightPrimary:
				return U"Light Primary";
			case EditorTextColorSlot::LightSecondary:
				return U"Light Secondary";
			case EditorTextColorSlot::LightAccent:
				return U"Light Accent";
			case EditorTextColorSlot::CardPrimary:
				return U"Card Primary";
			case EditorTextColorSlot::CardSecondary:
				return U"Card Secondary";
			case EditorTextColorSlot::SelectedPrimary:
				return U"Selected Primary";
			case EditorTextColorSlot::SelectedSecondary:
				return U"Selected Secondary";
			case EditorTextColorSlot::Warning:
				return U"Warning";
			case EditorTextColorSlot::Error:
			default:
				return U"Error";
			}
		}

		[[nodiscard]] inline ColorF ResolvePanelTitleColor(const ColorF& titleColor = DefaultPanelTitleColor)
		{
			return ((titleColor == DefaultPanelTitleColor) ? EditorTextOnLightPrimaryColor() : titleColor);
		}

		[[nodiscard]] inline bool DrawEditorIconButton(const Rect& rect, const StringView icon)
		{
			static const Font iconFont{ 16, Typeface::Bold };
			const bool hovered = rect.mouseOver();
			rect.rounded(6).draw(hovered ? ColorF{ 0.82, 0.90, 0.98, 0.96 } : ColorF{ 0.72, 0.80, 0.90, 0.92 })
				.drawFrame(1.0, 0.0, hovered ? ColorF{ 0.25, 0.40, 0.64, 0.96 } : ColorF{ 0.32, 0.38, 0.46, 0.92 });
			iconFont(icon).drawAt(rect.center(), EditorTextOnLightPrimaryColor());
			return hovered && MouseL.down();
		}

		[[nodiscard]] inline ColorF& GetEditorTextColor(MainSupport::EditorTextColorSettings& settings, const EditorTextColorSlot slot)
		{
			switch (slot)
			{
			case EditorTextColorSlot::DarkPrimary:
				return settings.darkPrimary;
			case EditorTextColorSlot::DarkSecondary:
				return settings.darkSecondary;
			case EditorTextColorSlot::DarkAccent:
				return settings.darkAccent;
			case EditorTextColorSlot::LightPrimary:
				return settings.lightPrimary;
			case EditorTextColorSlot::LightSecondary:
				return settings.lightSecondary;
			case EditorTextColorSlot::LightAccent:
				return settings.lightAccent;
			case EditorTextColorSlot::CardPrimary:
				return settings.cardPrimary;
			case EditorTextColorSlot::CardSecondary:
				return settings.cardSecondary;
			case EditorTextColorSlot::SelectedPrimary:
				return settings.selectedPrimary;
			case EditorTextColorSlot::SelectedSecondary:
				return settings.selectedSecondary;
			case EditorTextColorSlot::Warning:
				return settings.warning;
			case EditorTextColorSlot::Error:
			default:
				return settings.error;
			}
		}
	}
}
