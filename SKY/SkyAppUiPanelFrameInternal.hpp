# pragma once
# include "SkyAppUiEditorTextColorsInternal.hpp"
# include "SkyAppUi.hpp"

namespace SkyAppSupport
{
	namespace UiInternal
	{
      [[nodiscard]] inline FilePath ResolvePanelNinePatchPath(const FilePathView configuredPath)
		{
			const FilePath requestedPath = String{ configuredPath };
			return (requestedPath.isEmpty() ? FilePath{ DefaultPanelNinePatchPath } : requestedPath);
		}

		[[nodiscard]] inline Optional<NinePatch> LoadPanelNinePatch(const FilePathView configuredPath)
		{
			const FilePath resolvedPath = ResolvePanelNinePatchPath(configuredPath);

			if (FileSystem::Exists(resolvedPath))
			{
				return NinePatch{ Texture{ resolvedPath }, DefaultPanelNinePatchPatchSize };
			}

			if ((resolvedPath != DefaultPanelNinePatchPath) && FileSystem::Exists(DefaultPanelNinePatchPath))
			{
				return NinePatch{ Texture{ DefaultPanelNinePatchPath }, DefaultPanelNinePatchPatchSize };
			}

			return none;
		}

       [[nodiscard]] inline const Optional<NinePatch>& GetConfiguredPanelNinePatch(const MainSupport::PanelSkinTarget target = MainSupport::PanelSkinTarget::Default)
		{
         static bool defaultInitialized = false;
			static FilePath defaultCachedConfiguredPath;
			static Optional<NinePatch> defaultNinePatch;
           static bool settingsInitialized = false;
			static FilePath settingsCachedConfiguredPath;
			static Optional<NinePatch> settingsNinePatch;
           static bool cameraSettingsInitialized = false;
			static FilePath cameraSettingsCachedConfiguredPath;
			static Optional<NinePatch> cameraSettingsNinePatch;
			static bool hudInitialized = false;
			static FilePath hudCachedConfiguredPath;
			static Optional<NinePatch> hudNinePatch;
			static bool mapEditorInitialized = false;
			static FilePath mapEditorCachedConfiguredPath;
			static Optional<NinePatch> mapEditorNinePatch;
			static bool unitEditorInitialized = false;
			static FilePath unitEditorCachedConfiguredPath;
			static Optional<NinePatch> unitEditorNinePatch;
			static bool toolModalInitialized = false;
			static FilePath toolModalCachedConfiguredPath;
			static Optional<NinePatch> toolModalNinePatch;

            bool* initialized = &defaultInitialized;
			FilePath* cachedConfiguredPath = &defaultCachedConfiguredPath;
			Optional<NinePatch>* ninePatch = &defaultNinePatch;

			switch (target)
			{
          case MainSupport::PanelSkinTarget::Settings:
				initialized = &settingsInitialized;
				cachedConfiguredPath = &settingsCachedConfiguredPath;
				ninePatch = &settingsNinePatch;
				break;

			case MainSupport::PanelSkinTarget::CameraSettings:
				initialized = &cameraSettingsInitialized;
				cachedConfiguredPath = &cameraSettingsCachedConfiguredPath;
				ninePatch = &cameraSettingsNinePatch;
				break;

			case MainSupport::PanelSkinTarget::Hud:
				initialized = &hudInitialized;
				cachedConfiguredPath = &hudCachedConfiguredPath;
				ninePatch = &hudNinePatch;
				break;

			case MainSupport::PanelSkinTarget::MapEditor:
				initialized = &mapEditorInitialized;
				cachedConfiguredPath = &mapEditorCachedConfiguredPath;
				ninePatch = &mapEditorNinePatch;
				break;

			case MainSupport::PanelSkinTarget::UnitEditor:
				initialized = &unitEditorInitialized;
				cachedConfiguredPath = &unitEditorCachedConfiguredPath;
				ninePatch = &unitEditorNinePatch;
				break;

			case MainSupport::PanelSkinTarget::ToolModal:
				initialized = &toolModalInitialized;
				cachedConfiguredPath = &toolModalCachedConfiguredPath;
				ninePatch = &toolModalNinePatch;
				break;

			case MainSupport::PanelSkinTarget::Default:
			default:
				break;
			}

			const FilePath configuredPath = MainSupport::GetEffectivePanelNinePatchPath(target);

          if ((not *initialized) || (*cachedConfiguredPath != configuredPath))
			{
             *initialized = true;
				*cachedConfiguredPath = configuredPath;
				*ninePatch = LoadPanelNinePatch(configuredPath);
			}

           return *ninePatch;
		}

		[[nodiscard]] inline const Optional<NinePatch>& GetPreviewPanelNinePatch(const FilePathView configuredPath)
		{
			static bool initialized = false;
			static FilePath cachedConfiguredPath;
			static Optional<NinePatch> ninePatch;
			const FilePath requestedPath = String{ configuredPath };

			if ((not initialized) || (cachedConfiguredPath != requestedPath))
			{
				initialized = true;
				cachedConfiguredPath = requestedPath;
				ninePatch = LoadPanelNinePatch(requestedPath);
			}

			return ninePatch;
		}

		inline void DrawPanelNinePatchPreview(const RectF& rect,
			const FilePathView configuredPath,
			const StringView title = U"Panel Preview")
		{
          if (const auto& ninePatch = GetPreviewPanelNinePatch(configuredPath))
			{
              ninePatch->draw(rect);
			}
			else
			{
				rect.draw(DefaultPanelBackgroundColor);
			}

			rect.drawFrame(2, 0, DefaultPanelFrameColor);
			SimpleGUI::GetFont()(title).draw(rect.pos.movedBy(16, 12), ResolvePanelTitleColor());
			SimpleGUI::GetFont()(U"Sample body text").draw(rect.pos.movedBy(16, 42), EditorTextOnLightSecondaryColor());
			SimpleGUI::GetFont()(U"Selected / hover preview").draw(rect.pos.movedBy(16, 66), EditorTextOnLightAccentColor());

			const RectF selectButton{ rect.x + 16, rect.bottomY() - 44, 124, 26 };
			selectButton.rounded(6).draw(ColorF{ 0.33, 0.53, 0.82, 0.96 }).drawFrame(1.0, 0.0, ColorF{ 0.20, 0.32, 0.52, 0.96 });
			SimpleGUI::GetFont()(U"Select...").drawAt(selectButton.center(), EditorTextOnSelectedPrimaryColor());
		}

		inline void DrawNinePatchPanelFrame(const Rect& panelRect,
			StringView title = U"",
			const ColorF& backgroundColor = DefaultPanelBackgroundColor,
			const ColorF& frameColor = DefaultPanelFrameColor,
          const ColorF& titleColor = DefaultPanelTitleColor,
			const MainSupport::PanelSkinTarget target = MainSupport::PanelSkinTarget::Default)
		{
         if (const auto& ninePatch = GetConfiguredPanelNinePatch(target))
			{
				const ColorF resolvedTitleColor = ResolvePanelTitleColor(titleColor);
				ninePatch->draw(RectF{ panelRect });
				panelRect.drawFrame(2, 0, frameColor);

				if (not title.isEmpty())
				{
					SimpleGUI::GetFont()(title).draw((panelRect.x + 16), (panelRect.y + 12), resolvedTitleColor);
				}

				return;
			}

			const ColorF resolvedTitleColor = ResolvePanelTitleColor(titleColor);
			panelRect.draw(backgroundColor);
			panelRect.drawFrame(2, 0, frameColor);

			if (not title.isEmpty())
			{
				SimpleGUI::GetFont()(title).draw((panelRect.x + 16), (panelRect.y + 12), resolvedTitleColor);
			}
		}

		inline void DrawPanelFrame(const Rect& panelRect,
			StringView title = U"",
			const ColorF& backgroundColor = DefaultPanelBackgroundColor,
			const ColorF& frameColor = DefaultPanelFrameColor,
          const ColorF& titleColor = DefaultPanelTitleColor,
			const MainSupport::PanelSkinTarget target = MainSupport::PanelSkinTarget::Default)
		{
           if (const auto& ninePatch = GetConfiguredPanelNinePatch(target))
			{
				const ColorF resolvedTitleColor = ResolvePanelTitleColor(titleColor);
				ninePatch->draw(RectF{ panelRect });
				panelRect.drawFrame(2, 0, frameColor);

				if (not title.isEmpty())
				{
					SimpleGUI::GetFont()(title).draw((panelRect.x + 16), (panelRect.y + 12), resolvedTitleColor);
				}

				return;
			}

			const ColorF resolvedTitleColor = ResolvePanelTitleColor(titleColor);
			panelRect.draw(backgroundColor);
			panelRect.drawFrame(2, 0, frameColor);

			if (not title.isEmpty())
			{
				SimpleGUI::GetFont()(title).draw((panelRect.x + 16), (panelRect.y + 12), resolvedTitleColor);
			}
		}

		[[nodiscard]] inline bool DrawAccordionHeader(const Rect& panelRect,
			StringView title,
			const bool isExpanded,
			const ColorF& backgroundColor = DefaultPanelBackgroundColor,
			const ColorF& frameColor = DefaultPanelFrameColor,
          const ColorF& titleColor = DefaultPanelTitleColor,
			const MainSupport::PanelSkinTarget target = MainSupport::PanelSkinTarget::Default)
		{
			const ColorF resolvedTitleColor = ResolvePanelTitleColor(titleColor);
			const Rect headerRect = SkyAppUiLayout::AccordionHeaderRect(panelRect);
			const bool hovered = headerRect.mouseOver();

			if (const auto& ninePatch = GetConfiguredPanelNinePatch(target))
			{
				ninePatch->draw(RectF{ headerRect });
				if (hovered)
				{
					headerRect.draw(ColorF{ 1.0, 1.0, 1.0, 0.10 });
				}
			}
			else
			{
				headerRect.draw(hovered ? backgroundColor.lerp(ColorF{ 0.92 }, 0.25) : backgroundColor);
			}

			headerRect.drawFrame(2, 0, frameColor);
			SimpleGUI::GetFont()(isExpanded ? U"▼" : U"▶").draw((headerRect.x + 12), (headerRect.y + 7), resolvedTitleColor);
			SimpleGUI::GetFont()(title).draw((headerRect.x + 34), (headerRect.y + 7), resolvedTitleColor);
			return hovered && MouseL.down();
		}
	}
}
