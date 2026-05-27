#pragma once
# include <Siv3D.hpp>
# include "MapEditorMapLayersDraw.h"
# include "QuarterView.h"

namespace LT3
{
	struct DecalAmbientSoundCandidate
	{
		FilePath path;
		double volume = 0.6;
		double weight = 1.0;
	};

	inline Array<DecalAmbientSoundCandidate> CollectDecalAmbientSoundCandidates(const MapEditorState& editor, const Vec2& worldAnchor, double radius)
	{
		Array<DecalAmbientSoundCandidate> candidates;
		if (editor.cells.isEmpty() || radius <= 0.0)
		{
			return candidates;
		}

		const double radiusSq = radius * radius;
		for (int32 y = 0; y < editor.mapHeight; ++y)
		{
			for (int32 x = 0; x < editor.mapWidth; ++x)
			{
				const MapEditorCell& cell = editor.cells[MapEditorCellIndex(editor, x, y)];
				if (cell.decals.isEmpty())
				{
					continue;
				}

				const Vec2 center = MapEditorCellCenter(x, y);
				const double distanceSq = center.distanceFromSq(worldAnchor);
				if (distanceSq > radiusSq)
				{
					continue;
				}

				for (const MapEditorDecalPlacement& decal : cell.decals)
				{
					if (!(0 <= decal.assetIndex && decal.assetIndex < static_cast<int32>(editor.assets.size())))
					{
						continue;
					}

					const MapEditorAsset& asset = editor.assets[decal.assetIndex];
					if (asset.decalAmbientSound.isEmpty())
					{
						continue;
					}

					const FilePath path = ResolveDecalAmbientSoundPath(asset.decalAmbientSound);
					if (path.isEmpty() || !FileSystem::Exists(path))
					{
						continue;
					}

					const double t = 1.0 - Clamp(Sqrt(distanceSq) / radius, 0.0, 1.0);
					const double weight = Max(0.05, t);
					candidates << DecalAmbientSoundCandidate{ path, Clamp(asset.decalAmbientVolume, 0.0, 1.0), weight };
				}
			}
		}

		return candidates;
	}

	inline Array<DecalAmbientSoundCandidate> CollectDecalAmbientSoundCandidatesNearMouseOrCenter(const MapEditorState& editor)
	{
		constexpr double MouseRadius = 180.0;
		constexpr double CenterRadius = 240.0;

		Array<DecalAmbientSoundCandidate> merged = CollectDecalAmbientSoundCandidates(editor, ToQuarterWorld(Cursor::PosF()), MouseRadius);
		const Array<DecalAmbientSoundCandidate> center = CollectDecalAmbientSoundCandidates(editor, ToQuarterWorld(Scene::Center()), CenterRadius);
		merged.insert(merged.end(), center.begin(), center.end());
		return merged;
	}
}
