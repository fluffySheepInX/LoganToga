# include "MapEditorPanelInternal.hpp"
# include "SkyAppUiInternal.hpp"

using namespace MapEditorDetail;

namespace MapEditorDetail
{
	void DrawMapEditorCommandSection(MapEditorState& state, MapData& mapData, const FilePathView path, const Rect& panelRect)
	{
		const int32 commandY = (panelRect.y + 320);
		const Rect undoButton{ (panelRect.x + 16), commandY, 92, 30 };
		const Rect loadButton{ (panelRect.x + 116), commandY, 92, 30 };
		const Rect saveButton{ (panelRect.x + 216), commandY, 92, 30 };

		if (DrawEditorButton(undoButton, U"Undo"))
		{
			if (mapData.placedModels.isEmpty())
			{
				SetStatusMessage(state, U"削除対象なし");
			}
			else
			{
				mapData.placedModels.pop_back();
				if (not IsValidPlacedModelIndex(mapData, state.selectedPlacedModelIndex))
				{
					state.selectedPlacedModelIndex.reset();
					state.roadResizeDrag.reset();
					state.roadRotateDrag.reset();
				}
				SetStatusMessage(state, U"最後の配置を削除");
			}
		}

		if (DrawEditorButton(loadButton, U"保存済み再読込"))
		{
			const MapDataLoadResult loadResult = LoadMapDataWithStatus(path);
			mapData = loadResult.mapData;
			state.selectedPlacedModelIndex.reset();
			state.selectedResourceAreaIndex.reset();
			state.selectedNavPointIndex.reset();
			state.pendingNavLinkStartIndex.reset();
			ResetToolInteractionState(state);
			state.roadResizeDrag.reset();
			state.roadRotateDrag.reset();
			SetStatusMessage(state, loadResult.message.isEmpty() ? U"保存済みマップを再読込" : loadResult.message);
			state.hoveredGroundPosition.reset();
		}

		if (DrawEditorButton(saveButton, U"Save TOML"))
		{
			SetStatusMessage(state, SaveMapData(mapData, path) ? U"TOML 保存完了" : U"TOML 保存失敗");
		}
	}

	void DrawMapEditorInfoSection(const MapEditorState& state, const MapData& mapData, const Rect& panelRect, const Font& font)
	{
		const Rect infoSectionRect{ (panelRect.x + 12), (panelRect.y + 364), (panelRect.w - 24), 84 };
		const int32 infoY = (infoSectionRect.y + 10);
		DrawMapEditorPanelSection(infoSectionRect);
		font(U"Mode: {} / Tool: {}"_fmt(state.selectionMode ? U"Select" : U"Edit", ToLabel(state.selectedTool))).draw((infoSectionRect.x + 10), infoY, SkyAppSupport::UiInternal::EditorTextOnLightPrimaryColor());
		font(U"Obj {} / Area {} / Nav {} / Link {} / Terrain {}"_fmt(mapData.placedModels.size(), mapData.resourceAreas.size(), mapData.navPoints.size(), mapData.navLinks.size(), mapData.terrainCells.size())).draw((infoSectionRect.x + 10), (infoY + 20), SkyAppSupport::UiInternal::EditorTextOnLightPrimaryColor());
		font(U"P {:.0f}, {:.0f}, {:.0f} / E {:.0f}, {:.0f}, {:.0f}"_fmt(mapData.playerBasePosition.x, mapData.playerBasePosition.y, mapData.playerBasePosition.z, mapData.enemyBasePosition.x, mapData.enemyBasePosition.y, mapData.enemyBasePosition.z)).draw((infoSectionRect.x + 10), (infoY + 40), SkyAppSupport::UiInternal::EditorTextOnLightPrimaryColor());
		font(U"Rally {:.0f}, {:.0f}, {:.0f}"_fmt(mapData.sapperRallyPoint.x, mapData.sapperRallyPoint.y, mapData.sapperRallyPoint.z)).draw((infoSectionRect.x + 10), (infoY + 60), SkyAppSupport::UiInternal::EditorTextOnLightPrimaryColor());
		if (IsValidNavPointIndex(mapData, state.pendingNavLinkStartIndex))
		{
			font(U"Link Start: NavPoint {}"_fmt(*state.pendingNavLinkStartIndex)).draw((infoSectionRect.x + 172), (infoY + 60), SkyAppSupport::UiInternal::EditorTextOnPanelPrimaryColor());
		}
		else
		{
			font(U"Link Start: none").draw((infoSectionRect.x + 172), (infoY + 60), SkyAppSupport::UiInternal::EditorTextOnPanelPrimaryColor());
		}
	}

	void DrawMapEditorStatusMessage(const MapEditorState& state, const Rect& panelRect, const Font& font)
	{
		if (Scene::Time() < state.statusMessageUntil)
		{
			font(state.statusMessage).draw((panelRect.x + 16), (panelRect.bottomY() - 28), SkyAppSupport::UiInternal::EditorTextOnPanelPrimaryColor());
		}
	}
}
