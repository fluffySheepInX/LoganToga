# include "MapEditor.hpp"

namespace
{
	[[nodiscard]] Optional<Vec3> GetGroundIntersection(const DebugCamera3D& camera)
	{
		const Ray ray = camera.screenToRay(Cursor::PosF());
		const InfinitePlane groundPlane{ Float3{ 0, 0, 0 }, Float3{ 0, 1, 0 } };

		if (const auto distance = ray.intersects(groundPlane))
		{
			const Vec3 position = ray.point_at(*distance);
			return Vec3{ position.x, 0.0, position.z };
		}

		return none;
	}

	void SetStatusMessage(MapEditorState& state, const String& message)
	{
		state.statusMessage = message;
		state.statusMessageUntil = (Scene::Time() + 2.0);
	}

	bool DrawEditorButton(const Rect& rect, StringView label, const bool selected = false)
	{
		static const Font buttonFont{ 16, Typeface::Bold };
		const bool hovered = rect.mouseOver();
		const ColorF fillColor = selected
			? (hovered ? ColorF{ 0.52, 0.75, 0.95 } : ColorF{ 0.43, 0.67, 0.90 })
			: (hovered ? ColorF{ 0.84 } : ColorF{ 0.74 });
		rect.draw(fillColor).drawFrame(1, 0, ColorF{ 0.3 });
		buttonFont(label).drawAt(rect.center(), ColorF{ 0.12 });
		return hovered && MouseL.down();
	}

	[[nodiscard]] StringView ToLabel(const MapEditorTool tool)
	{
		switch (tool)
		{
		case MapEditorTool::SetSapperRallyPoint:
			return U"集結位置";

		case MapEditorTool::PlaceMill:
			return U"Mill 配置";

		case MapEditorTool::PlaceTree:
			return U"Tree 配置";

		case MapEditorTool::PlacePine:
			return U"Pine 配置";

		default:
			return U"Tool";
		}
	}

	[[nodiscard]] Optional<PlaceableModelType> ToPlaceableModelType(const MapEditorTool tool)
	{
		switch (tool)
		{
		case MapEditorTool::PlaceMill:
			return PlaceableModelType::Mill;

		case MapEditorTool::PlaceTree:
			return PlaceableModelType::Tree;

		case MapEditorTool::PlacePine:
			return PlaceableModelType::Pine;

		default:
			return none;
		}
	}
}

void UpdateMapEditor(MapEditorState& state, MapData& mapData, const DebugCamera3D& camera, const bool canHandleSceneInput)
{
	if (not state.enabled)
	{
		state.hoveredGroundPosition.reset();
		return;
	}

	state.hoveredGroundPosition = GetGroundIntersection(camera);

	if (not canHandleSceneInput)
	{
		return;
	}

	if (not state.hoveredGroundPosition || not MouseL.down())
	{
		return;
	}

	const Vec3 position = *state.hoveredGroundPosition;

	if (state.selectedTool == MapEditorTool::SetSapperRallyPoint)
	{
		mapData.sapperRallyPoint = position;
		SetStatusMessage(state, U"集結位置を更新");
		return;
	}

	if (const auto modelType = ToPlaceableModelType(state.selectedTool))
	{
		mapData.placedModels << PlacedModel{ .type = *modelType, .position = position };
		SetStatusMessage(state, U"モデルを配置");
	}
}

void DrawMapEditorScene(const MapEditorState& state, const MapData& mapData)
{
	Cylinder{ mapData.sapperRallyPoint.movedBy(0, 0.28, 0), 0.18, 0.56 }.draw(ColorF{ 0.95, 0.82, 0.12 }.removeSRGBCurve());
	Sphere{ mapData.sapperRallyPoint.movedBy(0, 0.68, 0), 0.20 }.draw(ColorF{ 0.98, 0.92, 0.35 }.removeSRGBCurve());

	if (not state.enabled || not state.hoveredGroundPosition)
	{
		return;
	}

	const Vec3 hoverPosition = *state.hoveredGroundPosition;
	const ColorF previewColor = (state.selectedTool == MapEditorTool::SetSapperRallyPoint)
		? ColorF{ 0.98, 0.85, 0.25, 0.65 }
		: ColorF{ 0.25, 0.85, 0.98, 0.50 };
	Cylinder{ hoverPosition.movedBy(0, 0.06, 0), 0.35, 0.12 }.draw(previewColor.removeSRGBCurve());
	Sphere{ hoverPosition.movedBy(0, 0.26, 0), 0.12 }.draw(previewColor.removeSRGBCurve());
}

void DrawMapEditorPanel(MapEditorState& state, MapData& mapData, FilePathView path, const Rect& panelRect)
{
	static const Font font{ 16 };
	panelRect.draw(ColorF{ 0.98, 0.95 });
	panelRect.drawFrame(2, 0, ColorF{ 0.25 });
	font(U"Map Editor").draw((panelRect.x + 16), (panelRect.y + 12), ColorF{ 0.12 });
	font(U"左クリック: 配置 / 設定").draw((panelRect.x + 16), (panelRect.y + 36), ColorF{ 0.18 });

	const Array<MapEditorTool> tools{
		MapEditorTool::SetSapperRallyPoint,
		MapEditorTool::PlaceMill,
		MapEditorTool::PlaceTree,
		MapEditorTool::PlacePine,
	};

	for (size_t i = 0; i < tools.size(); ++i)
	{
		const int32 column = static_cast<int32>(i % 2);
		const int32 row = static_cast<int32>(i / 2);
		const Rect buttonRect{
			(panelRect.x + 16 + (column * 156)),
			(panelRect.y + 66 + (row * 42)),
			144,
			32,
		};

		if (DrawEditorButton(buttonRect, ToLabel(tools[i]), (state.selectedTool == tools[i])))
		{
			state.selectedTool = tools[i];
		}
	}

	const Rect undoButton{ (panelRect.x + 16), (panelRect.y + 154), 92, 30 };
	const Rect loadButton{ (panelRect.x + 116), (panelRect.y + 154), 92, 30 };
	const Rect saveButton{ (panelRect.x + 216), (panelRect.y + 154), 92, 30 };

	if (DrawEditorButton(undoButton, U"Undo"))
	{
		if (mapData.placedModels.isEmpty())
		{
			SetStatusMessage(state, U"削除対象なし");
		}
		else
		{
			mapData.placedModels.pop_back();
			SetStatusMessage(state, U"最後の配置を削除");
		}
	}

	if (DrawEditorButton(loadButton, U"Reload"))
	{
		mapData = LoadMapData(path);
		SetStatusMessage(state, U"TOML を再読込");
		state.hoveredGroundPosition.reset();
	}

	if (DrawEditorButton(saveButton, U"Save TOML"))
	{
		SetStatusMessage(state, SaveMapData(mapData, path) ? U"TOML 保存完了" : U"TOML 保存失敗");
	}

	font(U"Tool: {}"_fmt(ToLabel(state.selectedTool))).draw((panelRect.x + 16), (panelRect.y + 196), ColorF{ 0.15 });
	font(U"Objects: {}"_fmt(mapData.placedModels.size())).draw((panelRect.x + 16), (panelRect.y + 220), ColorF{ 0.15 });
	font(U"Rally: {:.1f}, {:.1f}, {:.1f}"_fmt(mapData.sapperRallyPoint.x, mapData.sapperRallyPoint.y, mapData.sapperRallyPoint.z)).draw((panelRect.x + 16), (panelRect.y + 244), ColorF{ 0.15 });

	if (Scene::Time() < state.statusMessageUntil)
	{
		font(state.statusMessage).draw((panelRect.x + 16), (panelRect.y + 268), ColorF{ 0.15 });
	}
}
