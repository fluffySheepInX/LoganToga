# include "MapEditorInternal.hpp"
# include "MainScene.hpp"
# include "SkyAppUiInternal.hpp"

namespace MapEditorDetail
{
	namespace
	{
		constexpr double MillSelectionRadius = 4.5;
		constexpr double TreeSelectionRadius = 2.2;
		constexpr double PineSelectionRadius = 2.2;
     constexpr double GrassPatchSelectionRadius = 2.6;
		constexpr double RockSelectionRadius = 2.8;
           constexpr double WallSelectionPadding = 1.0;
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
        buttonFont(label).drawAt(rect.center(), selected ? SkyAppSupport::UiInternal::EditorTextOnSelectedPrimaryColor() : SkyAppSupport::UiInternal::EditorTextOnCardPrimaryColor());
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

		case PlaceableModelType::GrassPatch:
			return GrassPatchSelectionRadius;

		case PlaceableModelType::Rock:
			return RockSelectionRadius;

		case PlaceableModelType::Wall:
			return Max(1.6, (placedModel.wallLength * 0.5 + WallSelectionPadding));

		case PlaceableModelType::Road:
			return Max(2.4, (Max(placedModel.roadLength, placedModel.roadWidth) * 0.55));

		default:
			return 2.0;
		}
	}

	double GetNavPointSelectionRadius(const NavPoint& navPoint)
	{
		return Max(0.6, navPoint.radius);
	}

	double ComputeWallYaw(const Vec3& startPosition, const Vec3& endPosition, const double fallbackYaw)
	{
		const Vec2 direction{ (endPosition.x - startPosition.x), (endPosition.z - startPosition.z) };
		if (direction.lengthSq() <= 0.0001)
		{
			return fallbackYaw;
		}

		return Math::Atan2(direction.x, direction.y);
	}

	double ComputeWallLength(const Vec3& startPosition, const Vec3& endPosition, const double fallbackLength)
	{
		const double distance = startPosition.distanceFrom(endPosition);
		if (distance <= 0.0001)
		{
			return Clamp(fallbackLength, 2.0, 80.0);
		}

		return Clamp(distance, 2.0, 80.0);
	}

	PlacedModel BuildWallFromStartAndEnd(const Vec3& startPosition, const Vec3& endPosition, const double fallbackLength, const double fallbackYaw)
	{
		const double yaw = ComputeWallYaw(startPosition, endPosition, fallbackYaw);
		const double wallLength = ComputeWallLength(startPosition, endPosition, fallbackLength);
		const Vec3 direction{ Math::Sin(yaw), 0.0, Math::Cos(yaw) };
		return PlacedModel{
			.type = PlaceableModelType::Wall,
			.position = (startPosition + (direction * (wallLength * 0.5))),
			.yaw = yaw,
			.wallLength = wallLength,
		};
	}

	PlacedModel BuildRoadFromStartAndEnd(const Vec3& startPosition, const Vec3& endPosition, const double fallbackLength, const double fallbackWidth, const double fallbackYaw)
	{
		const double yaw = ComputeWallYaw(startPosition, endPosition, fallbackYaw);
		const double roadLength = ComputeWallLength(startPosition, endPosition, fallbackLength);
		const Vec3 direction{ Math::Sin(yaw), 0.0, Math::Cos(yaw) };
		return PlacedModel{
			.type = PlaceableModelType::Road,
			.position = (startPosition + (direction * (roadLength * 0.5))),
			.yaw = yaw,
			.roadLength = roadLength,
			.roadWidth = Clamp(fallbackWidth, 2.0, 80.0),
		};
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

		case MapEditorTool::PaintGrass:
			return U"草地 塗り";

		case MapEditorTool::PaintDirt:
			return U"土地 塗り";

		case MapEditorTool::PaintSand:
			return U"砂地 塗り";

		case MapEditorTool::PaintRock:
			return U"岩地 塗り";

		case MapEditorTool::EraseTerrain:
			return U"地表 削除";

		case MapEditorTool::PlaceMill:
			return U"Mill 配置";

		case MapEditorTool::PlaceTree:
			return U"Tree 配置";

		case MapEditorTool::PlacePine:
			return U"Pine 配置";

		case MapEditorTool::PlaceGrassPatch:
			return U"GrassTile 配置";

		case MapEditorTool::PlaceRock:
			return U"Rock 配置";

		case MapEditorTool::PlaceWall:
			return U"Wall 配置";

		case MapEditorTool::PlaceRoad:
			return U"Road 配置";

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

		case MapEditorTool::PlaceGrassPatch:
			return PlaceableModelType::GrassPatch;

		case MapEditorTool::PlaceRock:
			return PlaceableModelType::Rock;

		case MapEditorTool::PlaceWall:
			return PlaceableModelType::Wall;

		case MapEditorTool::PlaceRoad:
			return PlaceableModelType::Road;

		default:
			return none;
		}
	}

	Optional<TerrainCellType> ToTerrainCellType(const MapEditorTool tool)
	{
		switch (tool)
		{
		case MapEditorTool::PaintGrass:
			return TerrainCellType::Grass;

		case MapEditorTool::PaintDirt:
			return TerrainCellType::Dirt;

		case MapEditorTool::PaintSand:
			return TerrainCellType::Sand;

		case MapEditorTool::PaintRock:
			return TerrainCellType::Rock;

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
