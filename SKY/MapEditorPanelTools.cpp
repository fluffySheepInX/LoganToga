# include "MapEditorPanelInternal.hpp"
# include "SkyAppUiInternal.hpp"

using namespace MapEditorDetail;

namespace MapEditorDetail
{
	[[nodiscard]] const Array<Color>& TerrainColorPalette()
	{
		static const Array<Color> palette{
			Color{ 255, 255, 255 },
			Color{ 198, 255, 180 },
			Color{ 255, 214, 160 },
			Color{ 196, 172, 152 },
			Color{ 188, 202, 220 },
			Color{ 148, 188, 152 },
		};
		return palette;
	}

	[[nodiscard]] bool IsTerrainPaintTool(const MapEditorTool tool)
	{
		return ToTerrainCellType(tool).has_value() || (tool == MapEditorTool::EraseTerrain);
	}

	[[nodiscard]] StringView ToLabel(const MapEditorToolCategory category)
	{
		switch (category)
		{
		case MapEditorToolCategory::Terrain:
			return U"地形";

		case MapEditorToolCategory::Placement:
			return U"配置";

		case MapEditorToolCategory::Navigation:
			return U"Nav";

		case MapEditorToolCategory::BasesAndResources:
		default:
			return U"拠点 / 資源";
		}
	}

	[[nodiscard]] MapEditorToolCategory ToCategory(const MapEditorTool tool)
	{
		switch (tool)
		{
		case MapEditorTool::SetPlayerBasePosition:
		case MapEditorTool::SetEnemyBasePosition:
		case MapEditorTool::SetSapperRallyPoint:
		case MapEditorTool::SetBudgetArea:
		case MapEditorTool::SetGunpowderArea:
		case MapEditorTool::SetManaArea:
			return MapEditorToolCategory::BasesAndResources;

		case MapEditorTool::PaintGrass:
		case MapEditorTool::PaintDirt:
		case MapEditorTool::PaintSand:
		case MapEditorTool::PaintRock:
		case MapEditorTool::EraseTerrain:
			return MapEditorToolCategory::Terrain;

		case MapEditorTool::PlaceMill:
		case MapEditorTool::PlaceTree:
		case MapEditorTool::PlacePine:
		case MapEditorTool::PlaceGrassPatch:
		case MapEditorTool::PlaceRock:
		case MapEditorTool::PlaceWall:
		case MapEditorTool::PlaceRoad:
        case MapEditorTool::PlaceTireTrackDecal:
			return MapEditorToolCategory::Placement;

		case MapEditorTool::PlaceNavPoint:
		case MapEditorTool::LinkNavPoints:
		default:
			return MapEditorToolCategory::Navigation;
		}
	}

	[[nodiscard]] Array<MapEditorTool> GetToolsForCategory(const MapEditorToolCategory category)
	{
		switch (category)
		{
		case MapEditorToolCategory::Terrain:
			return {
				MapEditorTool::PaintGrass,
				MapEditorTool::PaintDirt,
				MapEditorTool::PaintSand,
				MapEditorTool::PaintRock,
				MapEditorTool::EraseTerrain,
			};

		case MapEditorToolCategory::Placement:
			return {
				MapEditorTool::PlaceMill,
				MapEditorTool::PlaceTree,
				MapEditorTool::PlacePine,
				MapEditorTool::PlaceGrassPatch,
				MapEditorTool::PlaceRock,
				MapEditorTool::PlaceWall,
				MapEditorTool::PlaceRoad,
              MapEditorTool::PlaceTireTrackDecal,
			};

		case MapEditorToolCategory::Navigation:
			return {
				MapEditorTool::PlaceNavPoint,
				MapEditorTool::LinkNavPoints,
			};

		case MapEditorToolCategory::BasesAndResources:
		default:
			return {
				MapEditorTool::SetPlayerBasePosition,
				MapEditorTool::SetEnemyBasePosition,
				MapEditorTool::SetSapperRallyPoint,
				MapEditorTool::SetBudgetArea,
				MapEditorTool::SetGunpowderArea,
				MapEditorTool::SetManaArea,
			};
		}
	}

	[[nodiscard]] StringView GetOperationHint(const MapEditorState& state)
	{
		if (state.selectionMode)
		{
			return U"左クリック: 選択 / 地面クリック: 移動 / Road は先端回転・角ドラッグで伸縮";
		}

		if (state.selectedTool == MapEditorTool::PlaceWall)
		{
			return U"左ドラッグ: Wall の向きと長さを決定";
		}

		if (state.selectedTool == MapEditorTool::PlaceRoad)
		{
			return U"左ドラッグ: Road の角度と長さを決定";
		}

		if (state.selectedTool == MapEditorTool::PlaceTireTrackDecal)
		{
			return U"左ドラッグ: タイヤ跡デカールの向きと長さを決定";
		}

		if (IsTerrainPaintTool(state.selectedTool))
		{
           if (state.terrainPaintMode == MapEditorTerrainPaintMode::Area)
			{
				return U"左ドラッグ: 指定範囲をまとめて塗る / 削除";
			}

			return U"左ドラッグ: 1マスずつ地表を塗る / 削除";
		}

		return U"左クリック: 配置 / 接続 / 設定";
	}

	void ResetToolInteractionState(MapEditorState& state)
	{
		state.roadResizeDrag.reset();
		state.roadRotateDrag.reset();
		state.pendingWallPlacementStartPosition.reset();
		state.pendingRoadPlacementStartPosition.reset();
     state.pendingTireTrackPlacementStartPosition.reset();
		state.lastTerrainPaintCell.reset();
       state.pendingTerrainPaintRangeStartCell.reset();
	}

	void DrawMapEditorPanelSection(const Rect& rect)
	{
		rect.rounded(10).draw(ColorF{ 0.98, 0.99, 1.0, 0.34 });
		rect.rounded(10).drawFrame(1, 0, ColorF{ 0.76, 0.80, 0.88, 0.88 });
	}

	void DrawMapEditorToolSection(MapEditorState& state, const Rect& panelRect, const Font& font)
	{
		const Array<MapEditorToolCategory> categories{
			MapEditorToolCategory::BasesAndResources,
			MapEditorToolCategory::Terrain,
			MapEditorToolCategory::Placement,
			MapEditorToolCategory::Navigation,
		};
		const Rect selectionModeButton{ (panelRect.x + 180), (panelRect.y + 10), 140, 28 };
		const Rect navPointVisibilityButton{ (panelRect.x + 16), (panelRect.y + 36), 144, 28 };
		const Rect navLinkVisibilityButton{ (panelRect.x + 176), (panelRect.y + 36), 144, 28 };
     const Rect terrainPaintSingleCellButton{ (panelRect.x + 176), (panelRect.y + 88), 68, 28 };
		const Rect terrainPaintAreaButton{ (panelRect.x + 252), (panelRect.y + 88), 68, 28 };
		const Rect toolSectionRect{ (panelRect.x + 12), (panelRect.y + 186), (panelRect.w - 24), 126 };
		const Rect toolButtonArea{ (toolSectionRect.x + 4), (toolSectionRect.y + 34), (toolSectionRect.w - 8), 90 };
		const auto selectTool = [&](const MapEditorTool tool)
			{
				state.selectedTool = tool;
				state.activeToolCategory = ToCategory(tool);
				ResetToolInteractionState(state);
				if (state.selectedTool != MapEditorTool::LinkNavPoints)
				{
					state.pendingNavLinkStartIndex.reset();
				}
			};

		if (DrawEditorButton(selectionModeButton, state.selectionMode ? U"選択モード: ON" : U"選択モード: OFF", state.selectionMode))
		{
			state.selectionMode = not state.selectionMode;
			if (not state.selectionMode)
			{
				state.selectedPlacedModelIndex.reset();
				state.selectedResourceAreaIndex.reset();
				state.selectedNavPointIndex.reset();
			}
			ResetToolInteractionState(state);
			SetStatusMessage(state, state.selectionMode ? U"選択モードを有効化" : U"選択モードを無効化");
		}

		if (DrawEditorButton(navPointVisibilityButton, state.showNavPoints ? U"NavPoint: 表示" : U"NavPoint: 非表示", state.showNavPoints))
		{
			state.showNavPoints = not state.showNavPoints;
          if (not state.showNavPoints)
			{
				state.selectedNavPointIndex.reset();
				state.pendingNavLinkStartIndex.reset();
			}
			SetStatusMessage(state, state.showNavPoints ? U"NavPoint を表示" : U"NavPoint を非表示");
		}

		if (DrawEditorButton(navLinkVisibilityButton, state.showNavLinks ? U"NavLink: 表示" : U"NavLink: 非表示", state.showNavLinks))
		{
			state.showNavLinks = not state.showNavLinks;
         if (not state.showNavLinks)
			{
				state.pendingNavLinkStartIndex.reset();
			}
			SetStatusMessage(state, state.showNavLinks ? U"NavLink を表示" : U"NavLink を非表示");
		}

		font(GetOperationHint(state)).draw((panelRect.x + 16), (panelRect.y + 68), SkyAppSupport::UiInternal::EditorTextOnLightSecondaryColor());
       if ((state.activeToolCategory == MapEditorToolCategory::Terrain) || IsTerrainPaintTool(state.selectedTool))
		{
			if (DrawEditorButton(terrainPaintSingleCellButton, U"1マス", (state.terrainPaintMode == MapEditorTerrainPaintMode::SingleCell)))
			{
				state.terrainPaintMode = MapEditorTerrainPaintMode::SingleCell;
				ResetToolInteractionState(state);
				SetStatusMessage(state, U"地形塗りを 1マス モードに変更");
			}

			if (DrawEditorButton(terrainPaintAreaButton, U"範囲", (state.terrainPaintMode == MapEditorTerrainPaintMode::Area)))
			{
				state.terrainPaintMode = MapEditorTerrainPaintMode::Area;
				ResetToolInteractionState(state);
				SetStatusMessage(state, U"地形塗りを 範囲 モードに変更");
			}
		}

		font(U"Category").draw((panelRect.x + 16), (panelRect.y + 92), SkyAppSupport::UiInternal::EditorTextOnLightSecondaryColor());

		for (size_t i = 0; i < categories.size(); ++i)
		{
			const int32 column = static_cast<int32>(i % 2);
			const int32 row = static_cast<int32>(i / 2);
			const Rect categoryButton{
				(panelRect.x + 16 + (column * 152)),
				(panelRect.y + 116 + (row * 34)),
				144,
				28,
			};

			if (DrawEditorButton(categoryButton, ToLabel(categories[i]), (state.activeToolCategory == categories[i])))
			{
				state.activeToolCategory = categories[i];
				state.toolCategoryScrollRow = 0;
			}
		}

		DrawMapEditorPanelSection(toolSectionRect);
		font(U"Tools: {}"_fmt(ToLabel(state.activeToolCategory))).draw((toolSectionRect.x + 10), (toolSectionRect.y + 8), SkyAppSupport::UiInternal::EditorTextOnLightPrimaryColor());

		const Array<MapEditorTool> tools = GetToolsForCategory(state.activeToolCategory);
		constexpr int32 VisibleToolRows = 3;
		const int32 totalToolRows = static_cast<int32>((tools.size() + 1) / 2);
		const int32 maxToolScrollRow = Max(0, (totalToolRows - VisibleToolRows));
		state.toolCategoryScrollRow = Clamp(state.toolCategoryScrollRow, 0, maxToolScrollRow);
		if (toolSectionRect.mouseOver())
		{
			const double wheel = Mouse::Wheel();
			if (wheel != 0.0)
			{
				state.toolCategoryScrollRow = Clamp((state.toolCategoryScrollRow - static_cast<int32>(Math::Round(wheel))), 0, maxToolScrollRow);
			}
		}
		if (0 < maxToolScrollRow)
		{
			font(U"Wheel Scroll {}/{}"_fmt((state.toolCategoryScrollRow + 1), (maxToolScrollRow + 1))).draw((toolSectionRect.x + 178), (toolSectionRect.y + 8), SkyAppSupport::UiInternal::EditorTextOnLightSecondaryColor());
		}

		for (size_t i = 0; i < tools.size(); ++i)
		{
			const int32 column = static_cast<int32>(i % 2);
			const int32 absoluteRow = static_cast<int32>(i / 2);
			const int32 visibleRow = (absoluteRow - state.toolCategoryScrollRow);
			if ((visibleRow < 0) || (VisibleToolRows <= visibleRow))
			{
				continue;
			}

			const Rect buttonRect{
				(toolButtonArea.x + (column * 150)),
				(toolButtonArea.y + (visibleRow * 30)),
				142,
				26,
			};

			if (DrawEditorButton(buttonRect, ToLabel(tools[i]), (state.selectedTool == tools[i])))
			{
				selectTool(tools[i]);
			}
		}
	}
}
