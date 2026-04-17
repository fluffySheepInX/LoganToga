# include "SkyAppRenderOverlayInternal.hpp"
# include "SkyAppUiPanelFrameInternal.hpp"
# include "MainSettings.hpp"

using namespace MainSupport;
using namespace SkyAppSupport;

namespace SkyAppFlow
{
	namespace
	{
		[[nodiscard]] bool DrawInitialResourceEditorRow(SkyAppState& state, const Rect& resourcePanel, const ResourceType type, const int32 yOffset)
		{
			double& initialValue = OverlayDetail::GetResourceValue(state.initialPlayerResources, type);
			double& currentValue = OverlayDetail::GetResourceValue(state.playerResources, type);
			const double step = OverlayDetail::GetResourceAdjustStep(type);
			const Rect minusButton{ (resourcePanel.rightX() - 84), (resourcePanel.y + yOffset), 32, 22 };
			const Rect plusButton{ (resourcePanel.rightX() - 44), (resourcePanel.y + yOffset), 32, 22 };
			bool changed = false;

			SimpleGUI::GetFont()(U"初期 {} {:.0f}"_fmt(OverlayDetail::ToDisplayLabel(type), initialValue)).draw(Vec2{ static_cast<double>(resourcePanel.x + 12), static_cast<double>(resourcePanel.y + yOffset + 1) }, OverlayDetail::GetResourceTextColor(type));

			if (DrawTextButton(minusButton, U"-"))
			{
				const double nextValue = Max(0.0, (initialValue - step));
				changed = (nextValue != initialValue);
				initialValue = nextValue;
				currentValue = initialValue;
			}

			if (DrawTextButton(plusButton, U"+"))
			{
				initialValue += step;
				currentValue = initialValue;
				changed = true;
			}

			return changed;
		}

		[[nodiscard]] bool DrawResourcePanelCameraHomeButton(const Rect& buttonRect)
		{
			const bool hovered = buttonRect.mouseOver();
			const ColorF backgroundColor = hovered
				? ColorF{ 0.82, 0.88, 0.96, 0.96 }
			: ColorF{ 0.72, 0.78, 0.86, 0.90 };
			const ColorF frameColor = hovered
				? ColorF{ 0.96, 0.98, 1.0, 0.98 }
			: ColorF{ 0.46, 0.52, 0.60, 0.95 };
			const ColorF iconColor{ 0.12, 0.15, 0.20, 0.96 };

			buttonRect.draw(backgroundColor).drawFrame(1, 0, frameColor);

			const Vec2 center = buttonRect.center();
			const Vec2 roofTop = center.movedBy(0, -10);
			const Vec2 roofLeft = center.movedBy(-10, -2);
			const Vec2 roofRight = center.movedBy(10, -2);
			Line{ roofLeft, roofTop }.draw(2.2, iconColor);
			Line{ roofTop, roofRight }.draw(2.2, iconColor);
			RectF{ Arg::center = center.movedBy(0, 7), 16, 12 }.drawFrame(2.0, iconColor);
			Line{ center.movedBy(0, 4), center.movedBy(0, 13) }.draw(2.0, iconColor);
			return hovered && MouseL.down();
		}

		void DrawBaseStatusLabel(const AppCamera3D& camera,
			const Vec3& basePosition,
			const double hitPoints,
			const ColorF& fillColor,
		   const ColorF& panelColor)
		{
			const Optional<Vec2> screenAnchor = OverlayDetail::ProjectToScreen(camera, basePosition.movedBy(0, 4.2, 0));

			if (not screenAnchor)
			{
				return;
			}

			const RectF panel{ Arg::center = screenAnchor->movedBy(0, -16), 156, 20 };
			panel.draw(panelColor).drawFrame(2, 0, ColorF{ 0.92, 0.96, 1.0, 0.72 });

			const RectF backRect{ Arg::center = panel.center(), 140, 8 };
			backRect.draw(ColorF{ 0.05, 0.05, 0.05, 0.88 });
			RectF{ backRect.pos, (backRect.w * Math::Saturate(hitPoints / BaseMaxHitPoints)), backRect.h }.draw(fillColor);
			backRect.drawFrame(1.0, ColorF{ 0.9, 0.95, 1.0, 0.55 });
		}
	}

	namespace OverlayDetail
	{
		void DrawResourceLoadWarnings(const SkyAppResources& resources)
		{
			if (resources.loadWarnings.isEmpty())
			{
				return;
			}

			const double lineHeight = 22.0;
			const double panelWidth = Min(680.0, Scene::Width() - 24.0);
			const double panelHeight = (18.0 + resources.loadWarnings.size() * lineHeight + 16.0);
			const RectF panel{ 12, 12, panelWidth, panelHeight };
			panel.rounded(10).draw(ColorF{ 0.18, 0.08, 0.08, 0.94 })
				.drawFrame(2.0, 0.0, ColorF{ 0.96, 0.54, 0.38, 0.96 });
			SimpleGUI::GetFont()(U"Resource load warning").draw(panel.x + 12, panel.y + 8, ColorF{ 1.0, 0.94, 0.84, 0.98 });

			for (size_t i = 0; i < resources.loadWarnings.size(); ++i)
			{
				SimpleGUI::GetFont()(resources.loadWarnings[i]).draw(panel.x + 12, panel.y + 30 + i * lineHeight, ColorF{ 1.0, 0.86, 0.78, 0.98 });
			}
		}

		void DrawUiEditGridOverlay(const SkyAppState& state)
		{
			if (not state.uiEditMode)
			{
				return;
			}

			for (int32 x = 0; x <= Scene::Width(); x += SkyAppUiLayout::UiEditGridCellSize)
			{
				const bool isMajorLine = (((x / SkyAppUiLayout::UiEditGridCellSize) % SkyAppUiLayout::UiEditGridMajorLineSpan) == 0);
				Line{ x, 0, x, Scene::Height() }.draw(1.0, isMajorLine ? ColorF{ 0.42, 0.55, 0.78, 0.16 } : ColorF{ 0.32, 0.40, 0.56, 0.08 });
			}

			for (int32 y = 0; y <= Scene::Height(); y += SkyAppUiLayout::UiEditGridCellSize)
			{
				const bool isMajorLine = (((y / SkyAppUiLayout::UiEditGridCellSize) % SkyAppUiLayout::UiEditGridMajorLineSpan) == 0);
				Line{ 0, y, Scene::Width(), y }.draw(1.0, isMajorLine ? ColorF{ 0.42, 0.55, 0.78, 0.16 } : ColorF{ 0.32, 0.40, 0.56, 0.08 });
			}
		}

		void DrawResourcePanelOverlay(SkyAppState& state, const SkyAppFrameState& frame)
		{
			const Rect resourcePanel = frame.panels.resourcePanel;
			const Rect resourcePanelCameraHomeButton = SkyAppUiLayout::ResourcePanelCameraHomeButton(resourcePanel);
			const Rect resourcePanelResizeHandle = SkyAppUiLayout::ResourcePanelResizeHandle(resourcePanel);
			bool resourceAdjustChanged = false;
			if (DrawResourcePanelCameraHomeButton(resourcePanelCameraHomeButton))
			{
				ResetCameraToPlayerBase(state);
			}

			SkyAppSupport::UiInternal::DrawPanelFrame(resourcePanel,
				   U"",
				   ColorF{ 0.08, 0.10, 0.12, 0.88 },
				   ColorF{ 0.75, 0.82, 0.90, 0.9 },
				   Palette::White,
				   MainSupport::PanelSkinTarget::Hud);
			SimpleGUI::GetFont()(state.uiEditMode ? U"資源 [Drag]" : (state.showResourceAdjustUi ? U"資源 / 初期値" : U"資源")).draw(SkyAppUiLayout::ResourcePanelTitlePosition(resourcePanel), Palette::White);
			SimpleGUI::GetFont()(U"現在 予算 {:.0f}"_fmt(state.playerResources.budget)).draw(Vec2{ static_cast<double>(resourcePanel.x + 12), static_cast<double>(resourcePanel.y + 28) }, ColorF{ 0.98, 0.90, 0.38 });
			SimpleGUI::GetFont()(U"現在 火薬 {:.0f}"_fmt(state.playerResources.gunpowder)).draw(Vec2{ static_cast<double>(resourcePanel.x + 12), static_cast<double>(resourcePanel.y + 48) }, ColorF{ 0.98, 0.56, 0.42 });
			SimpleGUI::GetFont()(U"現在 魔力 {:.0f}"_fmt(state.playerResources.mana)).draw(Vec2{ static_cast<double>(resourcePanel.x + 12), static_cast<double>(resourcePanel.y + 68) }, ColorF{ 0.58, 0.72, 1.0 });

			if (state.showResourceAdjustUi)
			{
				Line{ (resourcePanel.x + 12), (resourcePanel.y + 92), (resourcePanel.rightX() - 12), (resourcePanel.y + 92) }.draw(1.0, ColorF{ 0.72, 0.78, 0.86, 0.45 });
				SimpleGUI::GetFont()(U"テスト用 初期資源").draw(Vec2{ static_cast<double>(resourcePanel.x + 12), static_cast<double>(resourcePanel.y + 100) }, ColorF{ 0.90, 0.94, 0.98, 0.95 });
				resourceAdjustChanged = DrawInitialResourceEditorRow(state, resourcePanel, ResourceType::Budget, 122) || resourceAdjustChanged;
				resourceAdjustChanged = DrawInitialResourceEditorRow(state, resourcePanel, ResourceType::Gunpowder, 148) || resourceAdjustChanged;
				resourceAdjustChanged = DrawInitialResourceEditorRow(state, resourcePanel, ResourceType::Mana, 174) || resourceAdjustChanged;
				Line{ (resourcePanel.x + 12), (resourcePanel.y + 202), (resourcePanel.rightX() - 12), (resourcePanel.y + 202) }.draw(1.0, ColorF{ 0.72, 0.78, 0.86, 0.35 });

				const Rect resetButton{ (resourcePanel.x + 12), (resourcePanel.y + 212), (resourcePanel.w - 24), 28 };
				if (DrawTextButton(resetButton, U"初期資源をリセット"))
				{
					const ResourceStock defaultResources{ .budget = StartingResources };
					resourceAdjustChanged = ((state.initialPlayerResources.budget != defaultResources.budget)
						|| (state.initialPlayerResources.gunpowder != defaultResources.gunpowder)
						|| (state.initialPlayerResources.mana != defaultResources.mana))
						|| resourceAdjustChanged;
					state.initialPlayerResources = defaultResources;
					state.playerResources = defaultResources;
				}

				if (resourceAdjustChanged)
				{
					SaveInitialPlayerResources(state.initialPlayerResources);
				}
			}

			if (state.uiLayoutMessage.isVisible())
			{
				const Vec2 messagePosition{ static_cast<double>(resourcePanel.x), static_cast<double>(Min((resourcePanel.bottomY() + 8), (Scene::Height() - 28))) };
				SimpleGUI::GetFont()(state.uiLayoutMessage.text).draw(messagePosition, ColorF{ 0.98, 0.92, 0.72 });
			}

			if (state.uiEditMode)
			{
				const Rect resourcePanelResizeHandleOuterFrame = resourcePanelResizeHandle.stretched(3);
				resourcePanelResizeHandleOuterFrame.draw(ColorF{ 0.01, 0.02, 0.04, 0.82 }).drawFrame(2.0, 0.0, ColorF{ 0.90, 0.94, 1.0, 0.92 });
				resourcePanelResizeHandle.draw(ColorF{ 0.80, 0.86, 0.96, 0.22 }).drawFrame(1.2, 0.0, ColorF{ 0.74, 0.82, 0.94, 0.82 });
				Line{ resourcePanelResizeHandle.x + 6, resourcePanelResizeHandle.bottomY() - 2, resourcePanelResizeHandle.rightX() - 2, resourcePanelResizeHandle.y + 6 }.draw(1.4, ColorF{ 0.92, 0.96, 1.0, 0.88 });
				Line{ resourcePanelResizeHandle.x + 10, resourcePanelResizeHandle.bottomY() - 2, resourcePanelResizeHandle.rightX() - 2, resourcePanelResizeHandle.y + 10 }.draw(1.2, ColorF{ 0.92, 0.96, 1.0, 0.72 });
			}
		}

		void DrawBaseStatusOverlays(SkyAppState& state, const SkyAppFrameState& frame)
		{
			DrawBaseStatusLabel(state.camera,
				state.mapData.playerBasePosition,
				state.playerBaseHitPoints,
				ColorF{ 0.36, 0.92, 0.46, 0.95 },
			   ColorF{ 0.08, 0.10, 0.12, 0.88 });

			if (frame.isEditorMode || IsFogExploredAt(state.fogOfWar, state.mapData.enemyBasePosition))
			{
				DrawBaseStatusLabel(state.camera,
					state.mapData.enemyBasePosition,
					state.enemyBaseHitPoints,
					ColorF{ 0.96, 0.28, 0.24, 0.95 },
				ColorF{ 0.12, 0.08, 0.08, 0.88 });
			}
		}

      void DrawModelHeightOverlay(SkyAppResources& resources, SkyAppState& state, const SkyAppFrameState& frame)
		{
			if (state.modelHeightEditMode)
			{
				if (state.uiEditMode)
				{
					frame.panels.modelHeight.drawFrame(2, 0, ColorF{ 0.78, 0.84, 0.96, 0.72 });
				}
               DrawModelHeightEditor(state.modelHeightSettings,
					state.modelHeightTarget,
					state.modelHeightTextureMode,
					state.tireTrackTextureTarget,
                  state.modelHeightPreviewAnimationRole,
					resources.GetUnitRenderModel(state.modelHeightTarget, state.modelHeightPreviewAnimationRole),
					state.modelHeightMessage.text,
					state.modelHeightMessage.until,
					frame.panels.modelHeight,
					frame.previewRenderPositions);
			}
		}

		void DrawMatchResultOverlay(const SkyAppState& state)
		{
			if (state.playerWon)
			{
				const Rect overlay{ Arg::center = Scene::Center(), 340, 140 };
				overlay.draw(ColorF{ 0.06, 0.06, 0.08, 0.88 }).drawFrame(3, 0, *state.playerWon ? ColorF{ 0.45, 0.92, 0.56 } : ColorF{ 0.96, 0.38, 0.30 });
				SimpleGUI::GetFont()(*state.playerWon ? U"Victory" : U"Defeat").drawAt(overlay.center().movedBy(0, -18), Palette::White);
				SimpleGUI::GetFont()(*state.playerWon ? U"Enemy base destroyed" : U"Player base destroyed").drawAt(overlay.center().movedBy(0, 18), ColorF{ 0.92 });
				SimpleGUI::GetFont()(U"Press Enter or Click").drawAt(overlay.center().movedBy(0, 48), ColorF{ 0.92, 0.92, 0.82, 0.95 });
			}
		}
	}
}
