# include "MapEditorInternal.hpp"
# include "MainScene.hpp"

namespace MapEditorDetail
{
	namespace
	{
		constexpr double MillSelectionRadius = 4.5;
		constexpr double TreeSelectionRadius = 2.2;
		constexpr double PineSelectionRadius = 2.2;
	}

   Optional<Vec3> GetGroundIntersection(const MainSupport::AppCamera3D& camera)
	{
		const Optional<Ray> ray = MainSupport::TryScreenToRay(camera, Cursor::PosF());
		if (not ray)
		{
			return none;
		}

		const InfinitePlane groundPlane{ Float3{ 0, 0, 0 }, Float3{ 0, 1, 0 } };

		if (const auto distance = ray->intersects(groundPlane))
		{
			const Vec3 position = ray->point_at(*distance);
			return Vec3{ position.x, 0.0, position.z };
		}

		return none;
	}

	void SetStatusMessage(MapEditorState& state, const String& message)
	{
		state.statusMessage = message;
		state.statusMessageUntil = (Scene::Time() + 2.0);
	}

	bool DrawEditorButton(const Rect& rect, const StringView label, const bool selected)
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

	double GetPlacedModelSelectionRadius(const PlacedModel& placedModel)
	{
		switch (placedModel.type)
		{
		case PlaceableModelType::Mill:
			return MillSelectionRadius;

		case PlaceableModelType::Tree:
			return TreeSelectionRadius;

		case PlaceableModelType::Pine:
			return PineSelectionRadius;

		default:
			return 2.0;
		}
	}

	bool IsValidPlacedModelIndex(const MapData& mapData, const Optional<size_t>& index)
	{
		return index && (*index < mapData.placedModels.size());
	}

    Optional<size_t> HitTestPlacedModel(const Array<PlacedModel>& placedModels, const MainSupport::AppCamera3D& camera)
	{
		const Optional<Ray> cursorRay = MainSupport::TryScreenToRay(camera, Cursor::PosF());
		if (not cursorRay)
		{
			return none;
		}

		double nearestDistance = Math::Inf;
		Optional<size_t> nearestIndex;

		for (size_t i = 0; i < placedModels.size(); ++i)
		{
			const Sphere interactionSphere{ placedModels[i].position.movedBy(0, 2.2, 0), GetPlacedModelSelectionRadius(placedModels[i]) };
			if (const auto distance = cursorRay->intersects(interactionSphere))
			{
				if (*distance < nearestDistance)
				{
					nearestDistance = *distance;
					nearestIndex = i;
				}
			}
		}

		return nearestIndex;
	}

	StringView ToLabel(const MapEditorTool tool)
	{
		switch (tool)
		{
		case MapEditorTool::SetPlayerBasePosition:
			return U"自軍拠点";

		case MapEditorTool::SetEnemyBasePosition:
			return U"敵拠点";

		case MapEditorTool::SetSapperRallyPoint:
			return U"集結位置";

		case MapEditorTool::SetBudgetArea:
			return U"予算エリア";

		case MapEditorTool::SetGunpowderArea:
			return U"火薬エリア";

		case MapEditorTool::SetManaArea:
			return U"魔力エリア";

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

	Optional<PlaceableModelType> ToPlaceableModelType(const MapEditorTool tool)
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

	Optional<MainSupport::ResourceType> ToResourceType(const MapEditorTool tool)
	{
		switch (tool)
		{
		case MapEditorTool::SetBudgetArea:
			return MainSupport::ResourceType::Budget;

		case MapEditorTool::SetGunpowderArea:
			return MainSupport::ResourceType::Gunpowder;

		case MapEditorTool::SetManaArea:
			return MainSupport::ResourceType::Mana;

		default:
			return none;
		}
	}

	ColorF GetResourceAreaColor(const MainSupport::ResourceType type)
	{
		switch (type)
		{
		case MainSupport::ResourceType::Budget:
			return ColorF{ 0.96, 0.82, 0.22, 0.60 };

		case MainSupport::ResourceType::Gunpowder:
			return ColorF{ 0.95, 0.42, 0.26, 0.60 };

		case MainSupport::ResourceType::Mana:
			return ColorF{ 0.42, 0.60, 0.98, 0.60 };

		default:
			return ColorF{ 0.25, 0.85, 0.98, 0.50 };
		}
	}

	int32 FindResourceAreaIndex(const Array<ResourceArea>& resourceAreas, const MainSupport::ResourceType type)
	{
		for (size_t i = 0; i < resourceAreas.size(); ++i)
		{
			if (resourceAreas[i].type == type)
			{
				return static_cast<int32>(i);
			}
		}

		return -1;
	}
}

using namespace MapEditorDetail;

void UpdateMapEditor(MapEditorState& state, MapData& mapData, const MainSupport::AppCamera3D& camera, const bool canHandleSceneInput)
{
	if (not state.enabled)
	{
		state.hoveredGroundPosition.reset();
		return;
	}

	state.hoveredGroundPosition = GetGroundIntersection(camera);
	if (not IsValidPlacedModelIndex(mapData, state.selectedPlacedModelIndex))
	{
		state.selectedPlacedModelIndex.reset();
	}

	if (not canHandleSceneInput)
	{
		return;
	}

	if (state.selectionMode)
	{
		if (not MouseL.down())
		{
			return;
		}

		if (const auto selectedIndex = HitTestPlacedModel(mapData.placedModels, camera))
		{
			state.selectedPlacedModelIndex = *selectedIndex;
			SetStatusMessage(state, U"{} を選択"_fmt(ToString(mapData.placedModels[*selectedIndex].type)));
			return;
		}

		if (state.selectedPlacedModelIndex && state.hoveredGroundPosition)
		{
			PlacedModel& placedModel = mapData.placedModels[*state.selectedPlacedModelIndex];
			placedModel.position = *state.hoveredGroundPosition;
			SetStatusMessage(state, U"{} を移動"_fmt(ToString(placedModel.type)));
			return;
		}

		state.selectedPlacedModelIndex.reset();
		SetStatusMessage(state, U"選択解除");
		return;
	}

	if (not state.hoveredGroundPosition || not MouseL.down())
	{
		return;
	}

	const Vec3 position = *state.hoveredGroundPosition;

	if (state.selectedTool == MapEditorTool::SetPlayerBasePosition)
	{
		mapData.playerBasePosition = position;
		SetStatusMessage(state, U"自軍拠点を更新");
		return;
	}

	if (state.selectedTool == MapEditorTool::SetEnemyBasePosition)
	{
		mapData.enemyBasePosition = position;
		SetStatusMessage(state, U"敵拠点を更新");
		return;
	}

	if (state.selectedTool == MapEditorTool::SetSapperRallyPoint)
	{
		mapData.sapperRallyPoint = position;
		SetStatusMessage(state, U"集結位置を更新");
		return;
	}

	if (const auto resourceType = ToResourceType(state.selectedTool))
	{
		const int32 index = FindResourceAreaIndex(mapData.resourceAreas, *resourceType);

		if (0 <= index)
		{
			mapData.resourceAreas[index].position = position;
		}
		else
		{
			mapData.resourceAreas << ResourceArea{ .type = *resourceType, .position = position, .radius = MainSupport::ResourceAreaDefaultRadius };
		}

		SetStatusMessage(state, U"{}を更新"_fmt(ToLabel(state.selectedTool)));
		return;
	}

	if (const auto modelType = ToPlaceableModelType(state.selectedTool))
	{
		mapData.placedModels << PlacedModel{ .type = *modelType, .position = position };
		SetStatusMessage(state, U"モデルを配置");
	}
}
