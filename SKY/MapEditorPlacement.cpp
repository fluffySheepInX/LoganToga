# include "MapEditorUpdateInternal.hpp"

namespace MapEditorUpdateDetail
{
	bool HandleRoadPlacement(MapEditorState& state, MapData& mapData)
	{
		if (state.selectedTool != MapEditorTool::PlaceRoad)
		{
			return false;
		}

		if (MouseL.down() && state.hoveredGroundPosition)
		{
			state.pendingRoadPlacementStartPosition = *state.hoveredGroundPosition;
			return true;
		}

		if (state.pendingRoadPlacementStartPosition && MouseL.up())
		{
			const Vec3 roadPosition = *state.pendingRoadPlacementStartPosition;
			const Vec3 roadDirectionTarget = state.hoveredGroundPosition.value_or(roadPosition);
			mapData.placedModels << MapEditorDetail::BuildRoadFromStartAndEnd(roadPosition, roadDirectionTarget, 8.0, 4.0, 0.0);
			state.pendingRoadPlacementStartPosition.reset();
			SelectPlacedModel(state, mapData.placedModels.size() - 1);
			MapEditorDetail::SetStatusMessage(state, U"Road を配置");
			return true;
		}

		return true;
	}

	namespace
	{
		constexpr double TireTrackInputSampleSpacing = 1.0;
		constexpr double TireTrackSegmentStep = 4.0;
		constexpr double TireTrackDecalWidth = 2.0;

		Vec3 CatmullRomVec3(const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3, const double t)
		{
			const double t2 = (t * t);
			const double t3 = (t2 * t);
			return 0.5 * (
				(2.0 * p1)
				+ ((-p0 + p2) * t)
				+ (((2.0 * p0) - (5.0 * p1) + (4.0 * p2) - p3) * t2)
				+ ((-p0 + (3.0 * p1) - (3.0 * p2) + p3) * t3));
		}

		Array<Vec3> BuildSmoothTireTrackPath(const Array<Vec3>& samples)
		{
			if (samples.size() < 2)
			{
				return samples;
			}

			Array<Vec3> out;
			out << samples.front();

			if (samples.size() == 2)
			{
				const double dist = samples[0].distanceFrom(samples[1]);
				const int32 steps = Max<int32>(1, static_cast<int32>(Math::Ceil(dist / TireTrackSegmentStep)));
				for (int32 i = 1; i <= steps; ++i)
				{
					out << samples[0].lerp(samples[1], (static_cast<double>(i) / steps));
				}
				return out;
			}

			for (size_t i = 0; (i + 1) < samples.size(); ++i)
			{
				const Vec3 p0 = (i == 0) ? samples[i] : samples[i - 1];
				const Vec3 p1 = samples[i];
				const Vec3 p2 = samples[i + 1];
				const Vec3 p3 = ((i + 2) < samples.size()) ? samples[i + 2] : samples[i + 1];
				const double dist = p1.distanceFrom(p2);
				const int32 steps = Max<int32>(1, static_cast<int32>(Math::Ceil(dist / TireTrackSegmentStep)));
				for (int32 s = 1; s <= steps; ++s)
				{
					const double t = (static_cast<double>(s) / steps);
					out << CatmullRomVec3(p0, p1, p2, p3, t);
				}
			}
			return out;
		}
	}

	bool HandleTireTrackPlacement(MapEditorState& state, MapData& mapData)
	{
		if (state.selectedTool != MapEditorTool::PlaceTireTrackDecal)
		{
			return false;
		}

		if (state.tireTrackPlacementMode == MapEditorTireTrackPlacementMode::Straight)
		{
			if (MouseL.down() && state.hoveredGroundPosition)
			{
				state.pendingTireTrackPlacementStartPosition = *state.hoveredGroundPosition;
				return true;
			}

			if (state.pendingTireTrackPlacementStartPosition && MouseL.up())
			{
				const Vec3 decalStart = *state.pendingTireTrackPlacementStartPosition;
				const Vec3 decalEnd = state.hoveredGroundPosition.value_or(decalStart);
				mapData.placedModels << MapEditorDetail::BuildTireTrackDecalFromStartAndEnd(decalStart, decalEnd, 6.0, TireTrackDecalWidth, 0.0);
				state.pendingTireTrackPlacementStartPosition.reset();
				SelectPlacedModel(state, mapData.placedModels.size() - 1);
				MapEditorDetail::SetStatusMessage(state, U"タイヤ跡デカールを配置 (直線)");
				return true;
			}

			return true;
		}

		if (MouseL.down() && state.hoveredGroundPosition)
		{
			state.pendingTireTrackSamples = Array<Vec3>{ *state.hoveredGroundPosition };
			return true;
		}

		if (state.pendingTireTrackSamples && MouseL.pressed() && state.hoveredGroundPosition)
		{
			const Vec3 hover = *state.hoveredGroundPosition;
			if (state.pendingTireTrackSamples->back().distanceFrom(hover) >= TireTrackInputSampleSpacing)
			{
				*state.pendingTireTrackSamples << hover;
			}
		}

		if (state.pendingTireTrackSamples && MouseL.up())
		{
			Array<Vec3> samples = *state.pendingTireTrackSamples;
			state.pendingTireTrackSamples.reset();

			if (state.hoveredGroundPosition)
			{
				const Vec3 hover = *state.hoveredGroundPosition;
				if (samples.empty() || (samples.back().distanceFrom(hover) > 1e-3))
				{
					samples << hover;
				}
			}

			if (samples.size() < 2)
			{
				return true;
			}

			const Array<Vec3> path = BuildSmoothTireTrackPath(samples);
			const size_t firstNewIndex = mapData.placedModels.size();
			for (size_t i = 0; (i + 1) < path.size(); ++i)
			{
				mapData.placedModels << MapEditorDetail::BuildTireTrackDecalFromStartAndEnd(
					path[i], path[i + 1], TireTrackSegmentStep, TireTrackDecalWidth, 0.0);
			}

			if (firstNewIndex < mapData.placedModels.size())
			{
				SelectPlacedModel(state, mapData.placedModels.size() - 1);
				MapEditorDetail::SetStatusMessage(state, U"タイヤ跡デカールを配置 ({} 区間)"_fmt(mapData.placedModels.size() - firstNewIndex));
			}
			return true;
		}

		return true;
	}

	bool HandleWallPlacement(MapEditorState& state, MapData& mapData)
	{
		if (state.selectedTool != MapEditorTool::PlaceWall)
		{
			return false;
		}

		if (MouseL.down() && state.hoveredGroundPosition)
		{
			state.pendingWallPlacementStartPosition = *state.hoveredGroundPosition;
			return true;
		}

		if (state.pendingWallPlacementStartPosition && MouseL.up())
		{
			const Vec3 wallPosition = *state.pendingWallPlacementStartPosition;
			const Vec3 wallEndPosition = state.hoveredGroundPosition.value_or(wallPosition);
			mapData.placedModels << MapEditorDetail::BuildWallFromStartAndEnd(wallPosition, wallEndPosition, 10.0, 0.0);
			state.pendingWallPlacementStartPosition.reset();
			MapEditorDetail::SetStatusMessage(state, U"Wall を配置");
			return true;
		}

		return true;
	}

	void HandleGroundPlacement(MapEditorState& state, MapData& mapData, const MainSupport::AppCamera3D& camera)
	{
		if (not state.hoveredGroundPosition || not MouseL.down())
		{
			return;
		}

		const Vec3 position = *state.hoveredGroundPosition;

		if (state.selectedTool == MapEditorTool::SetPlayerBasePosition)
		{
			mapData.playerBasePosition = position;
			MapEditorDetail::SetStatusMessage(state, U"自軍拠点を更新");
			return;
		}

		if (state.selectedTool == MapEditorTool::SetEnemyBasePosition)
		{
			mapData.enemyBasePosition = position;
			MapEditorDetail::SetStatusMessage(state, U"敵拠点を更新");
			return;
		}

		if (state.selectedTool == MapEditorTool::SetSapperRallyPoint)
		{
			mapData.sapperRallyPoint = position;
			MapEditorDetail::SetStatusMessage(state, U"集結位置を更新");
			return;
		}

		if (const auto resourceType = MapEditorDetail::ToResourceType(state.selectedTool))
		{
			mapData.resourceAreas << ResourceArea{ .type = *resourceType, .position = position, .radius = MainSupport::ResourceAreaDefaultRadius };
			MapEditorDetail::SetStatusMessage(state, U"{}を追加"_fmt(MapEditorDetail::ToLabel(state.selectedTool)));
			return;
		}

		if (state.selectedTool == MapEditorTool::PlaceNavPoint)
		{
			mapData.navPoints << NavPoint{ .position = position, .radius = 1.4 };
			SelectNavPoint(state, mapData.navPoints.size() - 1);
			MapEditorDetail::SetStatusMessage(state, U"NavPoint を追加");
			return;
		}

		if (state.selectedTool == MapEditorTool::LinkNavPoints)
		{
			if (const auto hitNavPointIndex = MapEditorDetail::HitTestNavPoint(mapData.navPoints, camera))
			{
				if (not state.pendingNavLinkStartIndex)
				{
					state.pendingNavLinkStartIndex = *hitNavPointIndex;
					MapEditorDetail::SetStatusMessage(state, U"NavLink 開始点を選択");
					return;
				}

				if (*state.pendingNavLinkStartIndex == *hitNavPointIndex)
				{
					state.pendingNavLinkStartIndex.reset();
					MapEditorDetail::SetStatusMessage(state, U"NavLink 選択を解除");
					return;
				}

				const bool added = MapEditorDetail::ToggleNavLink(mapData, *state.pendingNavLinkStartIndex, *hitNavPointIndex);
				state.pendingNavLinkStartIndex.reset();
				MapEditorDetail::SetStatusMessage(state, added ? U"NavLink を追加" : U"NavLink を削除");
				return;
			}

			state.pendingNavLinkStartIndex.reset();
			return;
		}

		if (const auto modelType = MapEditorDetail::ToPlaceableModelType(state.selectedTool))
		{
            mapData.placedModels << PlacedModel{ .type = *modelType, .position = position, .ownerTeam = (state.selectedTool == MapEditorTool::PlaceMill ? state.placementMillOwnerTeam : MainSupport::UnitTeam::Player), .yaw = 0.0, .wallLength = 10.0 };
			MapEditorDetail::SetStatusMessage(state, U"モデルを配置");
		}
	}
}
