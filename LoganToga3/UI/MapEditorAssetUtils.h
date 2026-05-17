#pragma once
# include <Siv3D.hpp>
# include "MapEditorCoreTypes.h"

namespace LT3
{
	inline FilePath ResolveMapEditorAssetDirectory()
	{
		const FilePath fromApp = U"000_Warehouse/000_DefaultGame/015_BattleMapCellImage/";
		if (FileSystem::IsDirectory(fromApp))
		{
			return fromApp;
		}

		const FilePath fromRepo = U"App/000_Warehouse/000_DefaultGame/015_BattleMapCellImage/";
		if (FileSystem::IsDirectory(fromRepo))
		{
			return fromRepo;
		}

		return fromApp;
	}

	inline bool IsMapEditorImageFile(const FilePath& path)
	{
		const String extension = FileSystem::Extension(path).lowercased();
		return extension == U"png" || extension == U"jpg" || extension == U"jpeg" || extension == U"bmp" || extension == U"webp" || extension == U"gif";
	}

	inline bool IsMapEditorAnimatedGifFile(const FilePath& path)
	{
		return FileSystem::Extension(path).lowercased() == U"gif";
	}

	inline void PremultiplyImageAlpha(Image& image)
	{
		Color* p = image.data();
		const Color* const pEnd = (p + image.num_pixels());
		while (p != pEnd)
		{
			p->r = static_cast<uint8>((static_cast<uint16>(p->r) * p->a) / 255);
			p->g = static_cast<uint8>((static_cast<uint16>(p->g) * p->a) / 255);
			p->b = static_cast<uint8>((static_cast<uint16>(p->b) * p->a) / 255);
			++p;
		}
	}

	inline bool LoadMapEditorAssetVisual(MapEditorAsset& asset)
	{
		asset.imageSize = Size{ 0, 0 };
		asset.texture = Texture{};
		asset.isAnimatedGif = false;
		asset.animationFrames.clear();
		asset.animationFrameDelaysMillisec.clear();
		asset.animationDurationMillisec = 0;

		if (IsMapEditorAnimatedGifFile(asset.path))
		{
			AnimatedGIFReader reader{ asset.path };
			Array<Image> frames;
			Array<int32> frameDelaysMillisec;
			int32 durationMillisec = 0;
			if (reader && reader.read(frames, frameDelaysMillisec, durationMillisec) && !frames.isEmpty())
			{
				asset.animationFrames.reserve(frames.size());
				for (const auto& sourceFrame : frames)
				{
					Image frame = sourceFrame;
					PremultiplyImageAlpha(frame);
					asset.animationFrames << Texture{ frame };
				}

				asset.texture = asset.animationFrames.front();
				asset.imageSize = frames.front().size();
				asset.isAnimatedGif = true;
				asset.animationFrameDelaysMillisec = std::move(frameDelaysMillisec);
				asset.animationDurationMillisec = Max(durationMillisec, 1);
				return true;
			}
		}

		const Image image{ asset.path };
		if (!image)
		{
			return false;
		}

		Image premultipliedImage = image;
		PremultiplyImageAlpha(premultipliedImage);
		asset.imageSize = image.size();
		asset.texture = Texture{ premultipliedImage };
		return static_cast<bool>(asset.texture);
	}

	inline String TomlEscape(StringView text)
	{
		String result;
		for (const char32 ch : text)
		{
			if (ch == U'\\')
			{
				result += U"\\\\";
			}
			else if (ch == U'\"')
			{
				result += U"\\\"";
			}
			else
			{
				result += ch;
			}
		}
		return result;
	}

	inline int32 FindMapEditorAssetIndexByFileName(const MapEditorState& editor, StringView fileName)
	{
		for (int32 i = 0; i < static_cast<int32>(editor.assets.size()); ++i)
		{
			if (editor.assets[i].fileName == fileName)
			{
				return i;
			}
		}

		return InvalidMapEditorAsset;
	}

	inline bool IsDecalAssetFileName(StringView fileName)
	{
		return String{ fileName }.lowercased().starts_with(U"decal_");
	}

	inline bool IsMapEditorDecalAsset(const MapEditorState& editor, int32 assetIndex)
	{
		return (0 <= assetIndex)
			&& (assetIndex < static_cast<int32>(editor.assets.size()))
			&& (editor.assets[assetIndex].kind == MapEditorAssetKind::Object)
			&& IsDecalAssetFileName(editor.assets[assetIndex].fileName);
	}

	inline bool PromoteMapEditorAssetToDecal(MapEditorState& editor, int32 assetIndex)
	{
		if ((assetIndex < 0) || (assetIndex >= static_cast<int32>(editor.assets.size())))
		{
			return false;
		}

		MapEditorAsset& asset = editor.assets[assetIndex];
		if (asset.kind != MapEditorAssetKind::Object)
		{
			return false;
		}

		if (IsDecalAssetFileName(asset.fileName))
		{
			return true;
		}

		const FilePath targetPath = FileSystem::ParentPath(asset.path) + U"decal_" + asset.fileName;
		const String targetFileName = FileSystem::FileName(targetPath);
		if (FileSystem::Exists(targetPath))
		{
			editor.statusText = U"Decal rename failed: already exists {}"_fmt(targetFileName);
			return false;
		}

		if (!FileSystem::Rename(asset.path, targetPath))
		{
			editor.statusText = U"Decal rename failed: {}"_fmt(asset.fileName);
			return false;
		}

		asset.path = targetPath;
		asset.fileName = targetFileName;
		return true;
	}

	inline void NormalizeDecalSettings(MapEditorAsset& asset)
	{
		asset.decalOpacity = Clamp(asset.decalOpacity, 0.0, 1.0);
		asset.decalScale = Clamp(asset.decalScale, 0.1, 4.0);
		asset.decalOpacityMin = Clamp(asset.decalOpacityMin, 0.0, 1.0);
		asset.decalOpacityMax = Clamp(asset.decalOpacityMax, 0.0, 1.0);
		if (asset.decalOpacityMax < asset.decalOpacityMin)
		{
			std::swap(asset.decalOpacityMin, asset.decalOpacityMax);
		}
		asset.decalScaleMin = Clamp(asset.decalScaleMin, 0.1, 4.0);
		asset.decalScaleMax = Clamp(asset.decalScaleMax, 0.1, 4.0);
		if (asset.decalScaleMax < asset.decalScaleMin)
		{
			std::swap(asset.decalScaleMin, asset.decalScaleMax);
		}
	}

	inline void ApplyDecalAssetToCell(MapEditorCell& cell, const MapEditorAsset& asset)
	{
		cell.decalOpacity = asset.useRandomDecalOpacity
			? Random(asset.decalOpacityMin, asset.decalOpacityMax)
			: asset.decalOpacity;
		cell.decalScale = asset.useRandomDecalScale
			? Random(asset.decalScaleMin, asset.decalScaleMax)
			: asset.decalScale;
	}

	inline MapEditorDecalPlacement CreateMapEditorDecalPlacement(int32 assetIndex, const MapEditorAsset& asset)
	{
		return MapEditorDecalPlacement{
			assetIndex,
			asset.useRandomDecalOpacity ? Random(asset.decalOpacityMin, asset.decalOpacityMax) : asset.decalOpacity,
			asset.useRandomDecalScale ? Random(asset.decalScaleMin, asset.decalScaleMax) : asset.decalScale
		};
	}

	inline void AddDecalAssetToCell(MapEditorCell& cell, int32 assetIndex, const MapEditorAsset& asset)
	{
		const MapEditorDecalPlacement placement = CreateMapEditorDecalPlacement(assetIndex, asset);
		cell.decals << placement;
		cell.decalOpacity = placement.opacity;
		cell.decalScale = placement.scale;
	}

	inline void SyncLegacyDecalFieldsFromStack(MapEditorCell& cell)
	{
		if (cell.decals.isEmpty())
		{
			cell.decalOpacity = 1.0;
			cell.decalScale = 1.0;
			return;
		}

		const MapEditorDecalPlacement& top = cell.decals.back();
		cell.decalOpacity = top.opacity;
		cell.decalScale = top.scale;
	}

	inline Rect NormalizeMapEditorCellRect(const Point& a, const Point& b)
	{
		const int32 left = Min(a.x, b.x);
		const int32 top = Min(a.y, b.y);
		const int32 right = Max(a.x, b.x);
		const int32 bottom = Max(a.y, b.y);
		return Rect{ left, top, right - left + 1, bottom - top + 1 };
	}

	inline int32 MaxDecalStackSizeInRect(const MapEditorState& editor, const Rect& cellRect)
	{
		int32 maxStackSize = 0;
		for (int32 y = cellRect.y; y < cellRect.y + cellRect.h; ++y)
		{
			for (int32 x = cellRect.x; x < cellRect.x + cellRect.w; ++x)
			{
				if ((x < 0) || (y < 0) || (x >= editor.mapWidth) || (y >= editor.mapHeight))
				{
					continue;
				}
				const size_t cellIndex = static_cast<size_t>(y * editor.mapWidth + x);
				maxStackSize = Max(maxStackSize, static_cast<int32>(editor.cells[cellIndex].decals.size()));
			}
		}

		return maxStackSize;
	}

	inline int32 MoveDecalZOrderInRect(MapEditorState& editor, const Rect& cellRect, int32 stackIndex, int32 delta)
	{
		int32 changed = 0;
		for (int32 y = cellRect.y; y < cellRect.y + cellRect.h; ++y)
		{
			for (int32 x = cellRect.x; x < cellRect.x + cellRect.w; ++x)
			{
				if ((x < 0) || (y < 0) || (x >= editor.mapWidth) || (y >= editor.mapHeight))
				{
					continue;
				}

				const size_t cellIndex = static_cast<size_t>(y * editor.mapWidth + x);
				MapEditorCell& cell = editor.cells[cellIndex];
				const int32 nextIndex = stackIndex + delta;
				if ((stackIndex < 0) || (nextIndex < 0) || (stackIndex >= static_cast<int32>(cell.decals.size())) || (nextIndex >= static_cast<int32>(cell.decals.size())))
				{
					continue;
				}

				std::swap(cell.decals[stackIndex], cell.decals[nextIndex]);
				SyncLegacyDecalFieldsFromStack(cell);
				++changed;
			}
		}

		return changed;
	}

	inline int32 MoveDecalToBackInRect(MapEditorState& editor, const Rect& cellRect, int32 stackIndex)
	{
		int32 changed = 0;
		for (int32 y = cellRect.y; y < cellRect.y + cellRect.h; ++y)
		{
			for (int32 x = cellRect.x; x < cellRect.x + cellRect.w; ++x)
			{
				if ((x < 0) || (y < 0) || (x >= editor.mapWidth) || (y >= editor.mapHeight))
				{
					continue;
				}

				const size_t cellIndex = static_cast<size_t>(y * editor.mapWidth + x);
				MapEditorCell& cell = editor.cells[cellIndex];
				if ((stackIndex <= 0) || (stackIndex >= static_cast<int32>(cell.decals.size())))
				{
					continue;
				}

				for (int32 i = stackIndex; i > 0; --i)
				{
					std::swap(cell.decals[i], cell.decals[i - 1]);
				}

				SyncLegacyDecalFieldsFromStack(cell);
				++changed;
			}
		}
		return changed;
	}

	inline int32 MoveDecalToFrontInRect(MapEditorState& editor, const Rect& cellRect, int32 stackIndex)
	{
		int32 changed = 0;
		for (int32 y = cellRect.y; y < cellRect.y + cellRect.h; ++y)
		{
			for (int32 x = cellRect.x; x < cellRect.x + cellRect.w; ++x)
			{
				if ((x < 0) || (y < 0) || (x >= editor.mapWidth) || (y >= editor.mapHeight))
				{
					continue;
				}

				const size_t cellIndex = static_cast<size_t>(y * editor.mapWidth + x);
				MapEditorCell& cell = editor.cells[cellIndex];
				const int32 lastIndex = static_cast<int32>(cell.decals.size()) - 1;
				if ((stackIndex < 0) || (stackIndex >= lastIndex))
				{
					continue;
				}

				for (int32 i = stackIndex; i < lastIndex; ++i)
				{
					std::swap(cell.decals[i], cell.decals[i + 1]);
				}

				SyncLegacyDecalFieldsFromStack(cell);
				++changed;
			}
		}
		return changed;
	}

	inline int32 ApplyDecalAssetToPlacedCells(MapEditorState& editor, int32 assetIndex)
	{
		if (!IsMapEditorDecalAsset(editor, assetIndex))
		{
			return 0;
		}

		const MapEditorAsset& asset = editor.assets[assetIndex];
		int32 appliedCount = 0;
		for (auto& cell : editor.cells)
		{
			bool appliedToCell = false;
			for (auto& decal : cell.decals)
			{
				if (decal.assetIndex != assetIndex)
				{
					continue;
				}

				const MapEditorDecalPlacement placement = CreateMapEditorDecalPlacement(assetIndex, asset);
				decal.opacity = placement.opacity;
				decal.scale = placement.scale;
				appliedToCell = true;
			}

			if (!appliedToCell)
			{
				continue;
			}

			SyncLegacyDecalFieldsFromStack(cell);
			++appliedCount;
		}

		return appliedCount;
	}
}
