# include "MapEditorInternal.hpp"
# include "MainScene.hpp"

namespace MapEditorDetail
{
	namespace
	{
		constexpr double MillSelectionRadius = 4.5;
		constexpr double TreeSelectionRadius = 2.2;
		constexpr double PineSelectionRadius = 2.2;
		constexpr double RockSelectionRadius = 2.8;
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

		case PlaceableModelType::Rock:
			return RockSelectionRadius;

		default:
			return 2.0;
		}
	}

	double GetNavPointSelectionRadius(const NavPoint& navPoint)
	{
		return Max(0.6, navPoint.radius);
	}

	bool IsValidPlacedModelIndex(const MapData& mapData, const Optional<size_t>& index)
	{
		return index && (*index < mapData.placedModels.size());
	}

	bool IsValidResourceAreaIndex(const MapData& mapData, const Optional<size_t>& index)
	{
		return index && (*index < mapData.resourceAreas.size());
	}

	bool IsValidNavPointIndex(const MapData& mapData, const Optional<size_t>& index)
	{
		return index && (*index < mapData.navPoints.size());
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

		case MapEditorTool::PlaceRock:
			return U"Rock 配置";

		case MapEditorTool::PlaceNavPoint:
			return U"NavPoint 配置";

		case MapEditorTool::LinkNavPoints:
			return U"NavLink 接続";

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

		case MapEditorTool::PlaceRock:
			return PlaceableModelType::Rock;

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
}
