# include "MapEditorInternal.hpp"

using namespace MapEditorDetail;

void DrawMapEditorPanel(MapEditorState& state, MapData& mapData, const FilePathView path, const Rect& panelRect)
{
	static const Font font{ 16 };
	panelRect.draw(ColorF{ 0.98, 0.95 });
	panelRect.drawFrame(2, 0, ColorF{ 0.25 });
	font(U"Map Editor").draw((panelRect.x + 16), (panelRect.y + 12), ColorF{ 0.12 });
	const Rect selectionModeButton{ (panelRect.x + 180), (panelRect.y + 10), 140, 28 };
	if (DrawEditorButton(selectionModeButton, state.selectionMode ? U"選択モード: ON" : U"選択モード: OFF", state.selectionMode))
	{
		state.selectionMode = not state.selectionMode;
		if (not state.selectionMode)
		{
			state.selectedPlacedModelIndex.reset();
         state.selectedResourceAreaIndex.reset();
			state.selectedNavPointIndex.reset();
		}
		SetStatusMessage(state, state.selectionMode ? U"選択モードを有効化" : U"選択モードを無効化");
	}
 font(state.selectionMode ? U"左クリック: NavPoint/モデル/資源選択 / 地面クリックで移動" : U"左クリック: 配置 / 接続 / 設定").draw((panelRect.x + 16), (panelRect.y + 36), ColorF{ 0.18 });

	const Array<MapEditorTool> tools{
		MapEditorTool::SetPlayerBasePosition,
		MapEditorTool::SetEnemyBasePosition,
		MapEditorTool::SetSapperRallyPoint,
		MapEditorTool::SetBudgetArea,
		MapEditorTool::SetGunpowderArea,
		MapEditorTool::SetManaArea,
		MapEditorTool::PlaceMill,
		MapEditorTool::PlaceTree,
		MapEditorTool::PlacePine,
     MapEditorTool::PlaceRock,
		MapEditorTool::PlaceNavPoint,
		MapEditorTool::LinkNavPoints,
	};

	for (size_t i = 0; i < tools.size(); ++i)
	{
		const int32 column = static_cast<int32>(i % 2);
		const int32 row = static_cast<int32>(i / 2);
		const Rect buttonRect{
			(panelRect.x + 16 + (column * 156)),
            (panelRect.y + 62 + (row * 36)),
			144,
         30,
		};

		if (DrawEditorButton(buttonRect, ToLabel(tools[i]), (state.selectedTool == tools[i])))
		{
			state.selectedTool = tools[i];
           if (state.selectedTool != MapEditorTool::LinkNavPoints)
			{
				state.pendingNavLinkStartIndex.reset();
			}
		}
	}

   const Rect undoButton{ (panelRect.x + 16), (panelRect.y + 280), 92, 30 };
	const Rect loadButton{ (panelRect.x + 116), (panelRect.y + 280), 92, 30 };
	const Rect saveButton{ (panelRect.x + 216), (panelRect.y + 280), 92, 30 };

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
		SetStatusMessage(state, loadResult.message.isEmpty() ? U"保存済みマップを再読込" : loadResult.message);
		state.hoveredGroundPosition.reset();
	}

	if (DrawEditorButton(saveButton, U"Save TOML"))
	{
		SetStatusMessage(state, SaveMapData(mapData, path) ? U"TOML 保存完了" : U"TOML 保存失敗");
	}

    font(U"Mode: {} / Tool: {}"_fmt(state.selectionMode ? U"Select" : U"Edit", ToLabel(state.selectedTool))).draw((panelRect.x + 16), (panelRect.y + 320), ColorF{ 0.15 });
	font(U"Objects: {} / ResourceAreas: {}"_fmt(mapData.placedModels.size(), mapData.resourceAreas.size())).draw((panelRect.x + 16), (panelRect.y + 344), ColorF{ 0.15 });
	font(U"NavPoints: {} / NavLinks: {}"_fmt(mapData.navPoints.size(), mapData.navLinks.size())).draw((panelRect.x + 16), (panelRect.y + 368), ColorF{ 0.15 });
	font(U"Player: {:.1f}, {:.1f}, {:.1f}"_fmt(mapData.playerBasePosition.x, mapData.playerBasePosition.y, mapData.playerBasePosition.z)).draw((panelRect.x + 16), (panelRect.y + 392), ColorF{ 0.15 });
	font(U"Enemy: {:.1f}, {:.1f}, {:.1f}"_fmt(mapData.enemyBasePosition.x, mapData.enemyBasePosition.y, mapData.enemyBasePosition.z)).draw((panelRect.x + 16), (panelRect.y + 416), ColorF{ 0.15 });
	font(U"Rally: {:.1f}, {:.1f}, {:.1f}"_fmt(mapData.sapperRallyPoint.x, mapData.sapperRallyPoint.y, mapData.sapperRallyPoint.z)).draw((panelRect.x + 16), (panelRect.y + 440), ColorF{ 0.15 });
	if (IsValidNavPointIndex(mapData, state.pendingNavLinkStartIndex))
	{
		font(U"Link Start: NavPoint {}"_fmt(*state.pendingNavLinkStartIndex)).draw((panelRect.x + 16), (panelRect.y + 464), ColorF{ 0.15 });
	}
	else
	{
		font(U"Link Start: none").draw((panelRect.x + 16), (panelRect.y + 464), ColorF{ 0.15 });
	}

	if (IsValidNavPointIndex(mapData, state.selectedNavPointIndex))
	{
		NavPoint& navPoint = mapData.navPoints[*state.selectedNavPointIndex];
		const Rect deleteButton{ (panelRect.x + 216), (panelRect.y + 482), 92, 28 };
		const Rect radiusDownButton{ (panelRect.x + 216), (panelRect.y + 512), 28, 28 };
		const Rect radiusUpButton{ (panelRect.x + 280), (panelRect.y + 512), 28, 28 };
		font(U"Selected: NavPoint {} ({:.1f}, {:.1f}, {:.1f})"_fmt(*state.selectedNavPointIndex, navPoint.position.x, navPoint.position.y, navPoint.position.z)).draw((panelRect.x + 16), (panelRect.y + 490), ColorF{ 0.15 });
		font(U"Radius: {:.1f} / Links: {}"_fmt(navPoint.radius, CountNavLinksForPoint(mapData, *state.selectedNavPointIndex))).draw((panelRect.x + 16), (panelRect.y + 514), ColorF{ 0.15 });

		if (DrawEditorButton(deleteButton, U"削除"))
		{
			RemoveNavPointAt(mapData, *state.selectedNavPointIndex);
			state.selectedNavPointIndex.reset();
			state.pendingNavLinkStartIndex.reset();
			SetStatusMessage(state, U"選択中 NavPoint を削除");
		}

		if (DrawEditorButton(radiusDownButton, U"-"))
		{
			navPoint.radius = Clamp((navPoint.radius - 0.1), 0.5, 8.0);
		}

		if (DrawEditorButton(radiusUpButton, U"+"))
		{
			navPoint.radius = Clamp((navPoint.radius + 0.1), 0.5, 8.0);
		}
	}
	else if (IsValidPlacedModelIndex(mapData, state.selectedPlacedModelIndex))
	{
		const PlacedModel& placedModel = mapData.placedModels[*state.selectedPlacedModelIndex];
     const Rect deleteButton{ (panelRect.x + 216), (panelRect.y + 482), 92, 28 };
		font(U"Selected: {} ({:.1f}, {:.1f}, {:.1f})"_fmt(ToString(placedModel.type), placedModel.position.x, placedModel.position.y, placedModel.position.z)).draw((panelRect.x + 16), (panelRect.y + 490), ColorF{ 0.15 });

		if (DrawEditorButton(deleteButton, U"削除"))
		{
			mapData.placedModels.erase(mapData.placedModels.begin() + *state.selectedPlacedModelIndex);
			state.selectedPlacedModelIndex.reset();
			SetStatusMessage(state, U"選択中モデルを削除");
		}
	}
    else if (IsValidResourceAreaIndex(mapData, state.selectedResourceAreaIndex))
	{
		const ResourceArea& resourceArea = mapData.resourceAreas[*state.selectedResourceAreaIndex];
        const Rect deleteButton{ (panelRect.x + 216), (panelRect.y + 482), 92, 28 };
		font(U"Selected: {} ({:.1f}, {:.1f}, {:.1f}) r={:.1f}"_fmt(ToString(resourceArea.type), resourceArea.position.x, resourceArea.position.y, resourceArea.position.z, resourceArea.radius)).draw((panelRect.x + 16), (panelRect.y + 490), ColorF{ 0.15 });

		if (DrawEditorButton(deleteButton, U"削除"))
		{
			mapData.resourceAreas.erase(mapData.resourceAreas.begin() + *state.selectedResourceAreaIndex);
			state.selectedResourceAreaIndex.reset();
			SetStatusMessage(state, U"選択中資源エリアを削除");
		}
	}
	else
	{
      font(U"Selected: none").draw((panelRect.x + 16), (panelRect.y + 490), ColorF{ 0.15 });
	}

	if (Scene::Time() < state.statusMessageUntil)
	{
        font(state.statusMessage).draw((panelRect.x + 16), (panelRect.y + 548), ColorF{ 0.15 });
	}
}
