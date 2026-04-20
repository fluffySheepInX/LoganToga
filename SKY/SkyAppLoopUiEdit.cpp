# include "SkyAppLoopInternal.hpp"
# include "MainScene.hpp"
# include "MainSettings.hpp"

using namespace MainSupport;
using namespace SkyAppSupport;

namespace SkyAppFlow
{
	namespace Detail::UiEditInput
	{
		[[nodiscard]] Point& GetUiPanelPosition(UiLayoutSettings& settings, const UiLayoutPanel panel)
		{
			switch (panel)
			{
			case UiLayoutPanel::MiniMap:
				return settings.miniMapPosition;

			case UiLayoutPanel::ModelHeight:
				return settings.modelHeightPosition;

			case UiLayoutPanel::TerrainVisualSettings:
				return settings.terrainVisualSettingsPosition;

			case UiLayoutPanel::FogSettings:
				return settings.fogSettingsPosition;

			case UiLayoutPanel::UnitEditor:
				return settings.unitEditorPosition;

			case UiLayoutPanel::ResourcePanel:
			default:
				return settings.resourcePanelPosition;
			}
		}

		[[nodiscard]] UiPanelDragState MakeUiPanelDragState(
			const UiLayoutPanel panel,
			const Rect& panelRect,
			const Point& cursorPosition,
			const UiLayoutSettings& layoutSettings,
			const bool resizing)
		{
			const Point panelPosition{ panelRect.x, panelRect.y };
			return UiPanelDragState{
				.panel = panel,
				.resizing = resizing,
				.grabOffset = resizing ? Point{ 0, 0 } : (cursorPosition - panelPosition),
				.startPosition = panelPosition,
				.startCursor = cursorPosition,
				.startSize = Point{ panelRect.w, panelRect.h },
				.layoutAtDragStart = layoutSettings,
				.moved = false,
			};
		}

		[[nodiscard]] bool TryBeginPanelInteraction(
			SkyAppState& state,
			const Point& cursorPosition,
			const UiLayoutPanel panel,
			const Rect& panelRect,
			const Rect& hitRect,
			const bool enabled = true,
			const bool resizing = false)
		{
			if ((not enabled) || (not hitRect.mouseOver()))
			{
				return false;
			}

			state.hud.uiPanelDrag = MakeUiPanelDragState(panel, panelRect, cursorPosition, state.hud.uiLayoutSettings, resizing);
			return true;
		}

		void TryBeginDrag(SkyAppState& state, const SkyAppFrameState& frame, const Point& cursorPosition)
		{
			if (state.hud.uiPanelDrag || (not MouseL.down()))
			{
				return;
			}

			const bool startedInteraction =
               TryBeginPanelInteraction(
					state,
					cursorPosition,
					UiLayoutPanel::MiniMap,
					frame.panels.miniMap,
					SkyAppUiLayout::MiniMapResizeHandle(frame.panels.miniMap),
					true,
					true)
				||
				TryBeginPanelInteraction(
					state,
					cursorPosition,
					UiLayoutPanel::ResourcePanel,
					frame.panels.resourcePanel,
					SkyAppUiLayout::ResourcePanelResizeHandle(frame.panels.resourcePanel),
					true,
					true)
				|| TryBeginPanelInteraction(
					state,
					cursorPosition,
					UiLayoutPanel::ResourcePanel,
					frame.panels.resourcePanel,
					frame.panels.resourcePanel)
				|| TryBeginPanelInteraction(
					state,
					cursorPosition,
					UiLayoutPanel::MiniMap,
					frame.panels.miniMap,
                   SkyAppUiLayout::AccordionHeaderRect(frame.panels.miniMap))
				|| TryBeginPanelInteraction(
					state,
					cursorPosition,
					UiLayoutPanel::ModelHeight,
					frame.panels.modelHeight,
					frame.panels.modelHeight,
					state.editor.modelHeightEditMode)
				|| TryBeginPanelInteraction(
					state,
					cursorPosition,
					UiLayoutPanel::TerrainVisualSettings,
					frame.panels.terrainSettings,
					SkyAppUiLayout::TerrainVisualPanelDragHandle(frame.panels.terrainSettings),
					state.hud.terrainVisualSettingsExpanded)
                || TryBeginPanelInteraction(
					state,
					cursorPosition,
					UiLayoutPanel::FogSettings,
					frame.panels.fogSettings,
					SkyAppUiLayout::FogSettingsPanelDragHandle(frame.panels.fogSettings),
					state.hud.fogSettingsExpanded)
				|| TryBeginPanelInteraction(
					state,
					cursorPosition,
					UiLayoutPanel::UnitEditor,
					frame.panels.unitEditor,
					frame.panels.unitEditor)
				|| TryBeginPanelInteraction(
					state,
					cursorPosition,
					UiLayoutPanel::UnitEditor,
					frame.panels.unitEditorList,
					frame.panels.unitEditorList);

			static_cast<void>(startedInteraction);
		}

		void UpdateHoverCursor(const SkyAppState& state, const SkyAppFrameState& frame)
		{
			if (not state.hud.uiEditMode)
			{
				return;
			}

			if (SkyAppUiLayout::MiniMapResizeHandle(frame.panels.miniMap).mouseOver()
				|| SkyAppUiLayout::ResourcePanelResizeHandle(frame.panels.resourcePanel).mouseOver())
			{
				Cursor::RequestStyle(CursorStyle::Hand);
			}
		}

		[[nodiscard]] bool UpdateDrag(
			SkyAppState& state,
			const Point& cursorPosition,
			const int32 sceneWidth,
			const int32 sceneHeight)
		{
			if ((not state.hud.uiPanelDrag) || (not MouseL.pressed()))
			{
				return false;
			}

			Point& panelPosition = GetUiPanelPosition(state.hud.uiLayoutSettings, state.hud.uiPanelDrag->panel);
			const Point previousPosition = panelPosition;
			const Point requestedPosition = SkyAppUiLayout::SnapToUiEditGrid(cursorPosition - state.hud.uiPanelDrag->grabOffset);
           const Point previousMiniMapSize = state.hud.uiLayoutSettings.miniMapSize;
			const Point previousResourcePanelSize = state.hud.uiLayoutSettings.resourcePanelSize;
			const bool resourcePanelExpandedForLayout = state.hud.showResourceAdjustUi;
			const bool resourcePanelShowStoredHeight = state.hud.uiEditMode;
			bool movedThisFrame = false;

			if ((state.hud.uiPanelDrag->panel == UiLayoutPanel::ResourcePanel) && state.hud.uiPanelDrag->resizing)
			{
				const Point requestedSize = SkyAppUiLayout::SnapToUiEditGrid(state.hud.uiPanelDrag->startSize + (cursorPosition - state.hud.uiPanelDrag->startCursor));
				const Point clampedSize = SkyAppUiLayout::ClampResourcePanelSize(
					requestedSize,
					state.hud.uiLayoutSettings.resourcePanelPosition,
					sceneWidth,
					sceneHeight,
					resourcePanelExpandedForLayout);
				state.hud.uiLayoutSettings.resourcePanelSize = clampedSize;
				movedThisFrame = (clampedSize != state.hud.uiPanelDrag->startSize);
			}
			else if (state.hud.uiPanelDrag->panel == UiLayoutPanel::MiniMap)
			{
               if (state.hud.uiPanelDrag->resizing)
				{
					const Point requestedSize = SkyAppUiLayout::SnapToUiEditGrid(state.hud.uiPanelDrag->startSize + (cursorPosition - state.hud.uiPanelDrag->startCursor));
					const Point clampedSize = SkyAppUiLayout::ClampMiniMapSize(
						requestedSize,
						state.hud.uiLayoutSettings.miniMapPosition,
						sceneWidth,
						sceneHeight,
						state.hud.miniMapExpanded);
					state.hud.uiLayoutSettings.miniMapSize = clampedSize;
					movedThisFrame = (clampedSize != state.hud.uiPanelDrag->startSize);
				}
				else
				{
					const Rect rect = SkyAppUiLayout::MiniMap(sceneWidth, sceneHeight, requestedPosition, state.hud.uiLayoutSettings.miniMapSize, state.hud.miniMapExpanded);
					panelPosition = Point{ rect.x, rect.y };
					movedThisFrame = (panelPosition != state.hud.uiPanelDrag->startPosition);
				}
			}
			else if (state.hud.uiPanelDrag->panel == UiLayoutPanel::ModelHeight)
			{
				const Rect rect = SkyAppUiLayout::ModelHeight(sceneWidth, sceneHeight, requestedPosition);
				panelPosition = Point{ rect.x, rect.y };
				movedThisFrame = (panelPosition != state.hud.uiPanelDrag->startPosition);
			}
			else if (state.hud.uiPanelDrag->panel == UiLayoutPanel::TerrainVisualSettings)
			{
				const Rect rect = SkyAppUiLayout::TerrainVisualSettings(sceneWidth, sceneHeight, requestedPosition);
				panelPosition = Point{ rect.x, rect.y };
				movedThisFrame = (panelPosition != state.hud.uiPanelDrag->startPosition);
			}
         else if (state.hud.uiPanelDrag->panel == UiLayoutPanel::FogSettings)
			{
				const Rect rect = SkyAppUiLayout::FogSettings(sceneWidth, sceneHeight, requestedPosition);
				panelPosition = Point{ rect.x, rect.y };
				movedThisFrame = (panelPosition != state.hud.uiPanelDrag->startPosition);
			}
			else if (state.hud.uiPanelDrag->panel == UiLayoutPanel::UnitEditor)
			{
				const Point delta = (requestedPosition - state.hud.uiPanelDrag->startPosition);
				const Rect startDetailRect = SkyAppUiLayout::UnitEditor(sceneWidth, sceneHeight, state.hud.uiPanelDrag->layoutAtDragStart.unitEditorPosition);
				const Rect startListRect = SkyAppUiLayout::UnitEditorList(sceneWidth, sceneHeight, state.hud.uiPanelDrag->layoutAtDragStart.unitEditorListPosition);
				const int32 clampedDeltaX = Clamp(delta.x,
					Max(-startDetailRect.x, -startListRect.x),
					Min(sceneWidth - startDetailRect.rightX(), sceneWidth - startListRect.rightX()));
				const int32 clampedDeltaY = Clamp(delta.y,
					Max(-startDetailRect.y, -startListRect.y),
					Min(sceneHeight - startDetailRect.bottomY(), sceneHeight - startListRect.bottomY()));
				state.hud.uiLayoutSettings.unitEditorPosition = (state.hud.uiPanelDrag->layoutAtDragStart.unitEditorPosition + Point{ clampedDeltaX, clampedDeltaY });
				state.hud.uiLayoutSettings.unitEditorListPosition = (state.hud.uiPanelDrag->layoutAtDragStart.unitEditorListPosition + Point{ clampedDeltaX, clampedDeltaY });
				panelPosition = state.hud.uiLayoutSettings.unitEditorPosition;
				movedThisFrame = ((clampedDeltaX != 0) || (clampedDeltaY != 0));
			}
			else
			{
				const Rect rect = SkyAppUiLayout::ResourcePanel(
					sceneWidth,
					sceneHeight,
					requestedPosition,
					state.hud.uiLayoutSettings.resourcePanelSize,
					resourcePanelExpandedForLayout,
					resourcePanelShowStoredHeight);
				panelPosition = Point{ rect.x, rect.y };
				movedThisFrame = (panelPosition != state.hud.uiPanelDrag->startPosition);
			}

			state.hud.uiPanelDrag->moved = state.hud.uiPanelDrag->moved || movedThisFrame;
			return (panelPosition != previousPosition)
               || (state.hud.uiLayoutSettings.miniMapSize != previousMiniMapSize)
				|| (state.hud.uiLayoutSettings.resourcePanelSize != previousResourcePanelSize)
				|| ((state.hud.uiPanelDrag->panel == UiLayoutPanel::UnitEditor)
					&& (state.hud.uiLayoutSettings.unitEditorListPosition != state.hud.uiPanelDrag->layoutAtDragStart.unitEditorListPosition));
		}

		void EndDrag(SkyAppState& state)
		{
			if ((not state.hud.uiPanelDrag) || (not MouseL.up()))
			{
				return;
			}

			const bool moved = state.hud.uiPanelDrag->moved;
			state.hud.uiPanelDrag.reset();

			if (moved)
			{
				state.messages[SkyAppSupport::MessageChannel::UiLayout].show(SaveUiLayoutSettings(state.hud.uiLayoutSettings)
					? U"Saved: {}"_fmt(UiLayoutSettingsPath)
					: U"UI layout save failed");
			}
		}

		[[nodiscard]] bool Handle(SkyAppState& state, SkyAppFrameState& frame)
		{
			if ((not state.hud.uiEditMode) || frame.showEscMenu)
			{
				state.hud.uiPanelDrag.reset();
				return false;
			}

			const Point cursorPosition = Cursor::Pos();
			const int32 sceneWidth = Scene::Width();
			const int32 sceneHeight = Scene::Height();
         UpdateHoverCursor(state, frame);
			TryBeginDrag(state, frame, cursorPosition);
			const bool layoutChanged = UpdateDrag(state, cursorPosition, sceneWidth, sceneHeight);
			EndDrag(state);

			if (layoutChanged)
			{
				frame = BuildFrameState(state);
			}

			return layoutChanged;
		}
	}
}
