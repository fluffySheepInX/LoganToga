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

			state.uiPanelDrag = MakeUiPanelDragState(panel, panelRect, cursorPosition, state.uiLayoutSettings, resizing);
			return true;
		}

		void TryBeginDrag(SkyAppState& state, const SkyAppFrameState& frame, const Point& cursorPosition)
		{
			if (state.uiPanelDrag || (not MouseL.down()))
			{
				return;
			}

			const bool startedInteraction =
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
					frame.panels.miniMap)
				|| TryBeginPanelInteraction(
					state,
					cursorPosition,
					UiLayoutPanel::ModelHeight,
					frame.panels.modelHeight,
					frame.panels.modelHeight,
					state.modelHeightEditMode)
				|| TryBeginPanelInteraction(
					state,
					cursorPosition,
					UiLayoutPanel::TerrainVisualSettings,
					frame.panels.terrainSettings,
					SkyAppUiLayout::TerrainVisualPanelDragHandle(frame.panels.terrainSettings),
					state.terrainVisualSettingsExpanded)
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

		[[nodiscard]] bool UpdateDrag(
			SkyAppState& state,
			const Point& cursorPosition,
			const int32 sceneWidth,
			const int32 sceneHeight)
		{
			if ((not state.uiPanelDrag) || (not MouseL.pressed()))
			{
				return false;
			}

			Point& panelPosition = GetUiPanelPosition(state.uiLayoutSettings, state.uiPanelDrag->panel);
			const Point previousPosition = panelPosition;
			const Point requestedPosition = SkyAppUiLayout::SnapToUiEditGrid(cursorPosition - state.uiPanelDrag->grabOffset);
			const Point previousResourcePanelSize = state.uiLayoutSettings.resourcePanelSize;
			const bool resourcePanelExpandedForLayout = state.showResourceAdjustUi;
			const bool resourcePanelShowStoredHeight = state.uiEditMode;
			bool movedThisFrame = false;

			if ((state.uiPanelDrag->panel == UiLayoutPanel::ResourcePanel) && state.uiPanelDrag->resizing)
			{
				const Point requestedSize = SkyAppUiLayout::SnapToUiEditGrid(state.uiPanelDrag->startSize + (cursorPosition - state.uiPanelDrag->startCursor));
				const Point clampedSize = SkyAppUiLayout::ClampResourcePanelSize(
					requestedSize,
					state.uiLayoutSettings.resourcePanelPosition,
					sceneWidth,
					sceneHeight,
					resourcePanelExpandedForLayout);
				state.uiLayoutSettings.resourcePanelSize = clampedSize;
				movedThisFrame = (clampedSize != state.uiPanelDrag->startSize);
			}
			else if (state.uiPanelDrag->panel == UiLayoutPanel::MiniMap)
			{
				const Rect rect = SkyAppUiLayout::MiniMap(sceneWidth, sceneHeight, requestedPosition, state.miniMapExpanded);
				panelPosition = Point{ rect.x, rect.y };
				movedThisFrame = (panelPosition != state.uiPanelDrag->startPosition);
			}
			else if (state.uiPanelDrag->panel == UiLayoutPanel::ModelHeight)
			{
				const Rect rect = SkyAppUiLayout::ModelHeight(sceneWidth, sceneHeight, requestedPosition);
				panelPosition = Point{ rect.x, rect.y };
				movedThisFrame = (panelPosition != state.uiPanelDrag->startPosition);
			}
			else if (state.uiPanelDrag->panel == UiLayoutPanel::TerrainVisualSettings)
			{
				const Rect rect = SkyAppUiLayout::TerrainVisualSettings(sceneWidth, sceneHeight, requestedPosition);
				panelPosition = Point{ rect.x, rect.y };
				movedThisFrame = (panelPosition != state.uiPanelDrag->startPosition);
			}
			else if (state.uiPanelDrag->panel == UiLayoutPanel::UnitEditor)
			{
				const Point delta = (requestedPosition - state.uiPanelDrag->startPosition);
				const Rect startDetailRect = SkyAppUiLayout::UnitEditor(sceneWidth, sceneHeight, state.uiPanelDrag->layoutAtDragStart.unitEditorPosition);
				const Rect startListRect = SkyAppUiLayout::UnitEditorList(sceneWidth, sceneHeight, state.uiPanelDrag->layoutAtDragStart.unitEditorListPosition);
				const int32 clampedDeltaX = Clamp(delta.x,
					Max(-startDetailRect.x, -startListRect.x),
					Min(sceneWidth - startDetailRect.rightX(), sceneWidth - startListRect.rightX()));
				const int32 clampedDeltaY = Clamp(delta.y,
					Max(-startDetailRect.y, -startListRect.y),
					Min(sceneHeight - startDetailRect.bottomY(), sceneHeight - startListRect.bottomY()));
				state.uiLayoutSettings.unitEditorPosition = (state.uiPanelDrag->layoutAtDragStart.unitEditorPosition + Point{ clampedDeltaX, clampedDeltaY });
				state.uiLayoutSettings.unitEditorListPosition = (state.uiPanelDrag->layoutAtDragStart.unitEditorListPosition + Point{ clampedDeltaX, clampedDeltaY });
				panelPosition = state.uiLayoutSettings.unitEditorPosition;
				movedThisFrame = ((clampedDeltaX != 0) || (clampedDeltaY != 0));
			}
			else
			{
				const Rect rect = SkyAppUiLayout::ResourcePanel(
					sceneWidth,
					sceneHeight,
					requestedPosition,
					state.uiLayoutSettings.resourcePanelSize,
					resourcePanelExpandedForLayout,
					resourcePanelShowStoredHeight);
				panelPosition = Point{ rect.x, rect.y };
				movedThisFrame = (panelPosition != state.uiPanelDrag->startPosition);
			}

			state.uiPanelDrag->moved = state.uiPanelDrag->moved || movedThisFrame;
			return (panelPosition != previousPosition)
				|| (state.uiLayoutSettings.resourcePanelSize != previousResourcePanelSize)
				|| ((state.uiPanelDrag->panel == UiLayoutPanel::UnitEditor)
					&& (state.uiLayoutSettings.unitEditorListPosition != state.uiPanelDrag->layoutAtDragStart.unitEditorListPosition));
		}

		void EndDrag(SkyAppState& state)
		{
			if ((not state.uiPanelDrag) || (not MouseL.up()))
			{
				return;
			}

			const bool moved = state.uiPanelDrag->moved;
			state.uiPanelDrag.reset();

			if (moved)
			{
				state.uiLayoutMessage.show(SaveUiLayoutSettings(state.uiLayoutSettings)
					? U"Saved: {}"_fmt(UiLayoutSettingsPath)
					: U"UI layout save failed");
			}
		}

		[[nodiscard]] bool Handle(SkyAppState& state, SkyAppFrameState& frame)
		{
			if ((not state.uiEditMode) || frame.showEscMenu)
			{
				state.uiPanelDrag.reset();
				return false;
			}

			const Point cursorPosition = Cursor::Pos();
			const int32 sceneWidth = Scene::Width();
			const int32 sceneHeight = Scene::Height();
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
