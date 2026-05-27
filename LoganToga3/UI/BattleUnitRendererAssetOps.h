#pragma once
# include <Siv3D.hpp>
# include "../Systems/BattleSystems.h"
# include "../Data/BattleAssetPaths.h"
# include "../Data/UnitCatalog.h"
# include "QuarterView.h"

namespace LT3
{
	struct UnitVisualInfo
	{
		String image;
		String kind;
		bool unique = false;
		String portraitImage;
		double uniqueSpeechIntervalSec = 6.0;
		double uniqueSpeechVisibleSec = 2.2;
		double uniqueSpeechBubbleWidth = 180.0;
		double uniqueSpeechBubbleHeight = 54.0;
		Array<String> uniqueSpeechLines;
		double visualScale = 1.0;
		Point visualOffset{ 0, 0 };
		Point shadowOffset{ 0, 0 };
		double shadowScale = 1.0;
		double shadowOpacity = 0.28;
		UnitPlacementAnchor placementAnchor = UnitPlacementAnchor::Center;
		UnitRenderSizeMode renderSizeMode = UnitRenderSizeMode::Gameplay;
		double gameplaySizeMul = 2.2;
		UnitArtWidthReference artWidthReference = UnitArtWidthReference::Cell;
		double artWidthValue = 1.0;
		double artWidthValueLineHorizontal = 1.0;
		double artWidthValueLineDiagUpRight = 1.0;
		double artWidthValueLineDiagUpLeft = 1.0;
		bool artKeepAspect = true;
		Point lineIconHorizontalOffset{ 0, 0 };
		Point lineIconDiagUpRightOffset{ 0, 0 };
		Point lineIconDiagUpLeftOffset{ 0, 0 };
		String lineIconHorizontalName;
		String lineIconDiagUpRightName;
		String lineIconDiagUpLeftName;
	};

	struct BattleRenderAssets
	{
		HashTable<String, UnitVisualInfo> unitVisualByTag;
		mutable HashTable<String, Texture> unitTextureCache;
		mutable HashTable<String, Array<Texture>> unitGifFrameCache;
		mutable HashTable<String, Array<int32>> unitGifFrameDelaysMillisecCache;
		mutable HashTable<String, int32> unitGifDurationMillisecCache;
		mutable HashTable<String, Texture> portraitTextureCache;
		mutable HashTable<String, Texture> iconTextureCache;
		mutable HashTable<String, Texture> resourceTextureCache;
	};

	inline BattleRenderAssets BuildBattleRenderAssets(const UnitCatalog& catalog, const DefinitionStores* defs = nullptr)
	{
		BattleRenderAssets assets;
		for (const auto& entry : catalog.entries)
		{
			if (!entry.unit_id.isEmpty())
			{
				UnitVisualInfo info{
					entry.image,
					entry.kind,
					entry.unique,
					entry.portraitImage,
					entry.uniqueSpeechIntervalSec,
					entry.uniqueSpeechVisibleSec,
					entry.uniqueSpeechBubbleWidth,
					entry.uniqueSpeechBubbleHeight,
					entry.uniqueSpeechLines,
					entry.visualScale,
					entry.visualOffset,
					entry.shadowOffset,
					entry.shadowScale,
					entry.shadowOpacity,
					entry.placementAnchor,
					entry.renderSizeMode,
					entry.gameplaySizeMul,
					entry.artWidthReference,
					entry.artWidthValue,
					entry.artWidthValueLineHorizontal,
					entry.artWidthValueLineDiagUpRight,
					entry.artWidthValueLineDiagUpLeft,
					entry.artKeepAspect,
					entry.lineIconHorizontalOffset,
					entry.lineIconDiagUpRightOffset,
					entry.lineIconDiagUpLeftOffset
				};

				if (defs)
				{
					for (const auto& action : defs->buildActions)
					{
						if (action.ownerTag == entry.unit_id || action.spawnTag == entry.unit_id || action.resultTag == entry.unit_id)
						{
							info.lineIconHorizontalName = action.lineIconHorizontal;
							info.lineIconDiagUpRightName = action.lineIconDiagUpRight;
							info.lineIconDiagUpLeftName = action.lineIconDiagUpLeft;
							break;
						}
					}
				}

				assets.unitVisualByTag[entry.unit_id] = info;
			}
		}
		return assets;
	}

	inline UnitVisualInfo FindUnitVisualInfoByTag(const BattleRenderAssets& assets, const String& unitTag)
	{
		if (assets.unitVisualByTag.contains(unitTag))
		{
			return assets.unitVisualByTag.at(unitTag);
		}
		return UnitVisualInfo{};
	}

	inline SizeF ResolveUnitTextureDrawSize(const Texture& texture, const UnitDef& def, const UnitVisualInfo& visual, bool isBuildingVisual)
	{
		if (visual.renderSizeMode == UnitRenderSizeMode::Art)
		{
			const double cellWidth = QuarterTileOffset.x * 2.0;
			const double artBaseWidth = (visual.artWidthReference == UnitArtWidthReference::Pixel)
				? visual.artWidthValue
				: (cellWidth * visual.artWidthValue);
			const double width = Max(1.0, artBaseWidth * visual.visualScale);
			if (visual.artKeepAspect && texture.width() > 0)
			{
				const double height = Max(1.0, width * (static_cast<double>(texture.height()) / static_cast<double>(texture.width())));
				return SizeF{ width, height };
			}
			return SizeF{ width, width };
		}

		const double mul = Max(0.2, visual.gameplaySizeMul);
		const double baseSize = def.radius * mul * visual.visualScale;
		const double imageSize = Max(1.0, baseSize);
		return SizeF{ imageSize, imageSize };
	}

	inline const Texture* ResolveUnitDisplayTexture(const BattleRenderAssets& assets, const FilePath& unitPath, bool animateGif)
	{
		const String extension = FileSystem::Extension(unitPath).lowercased();
		if (extension != U"gif")
		{
			if (!assets.unitTextureCache.contains(unitPath))
			{
				assets.unitTextureCache.emplace(unitPath, Texture{ unitPath });
			}
			return &assets.unitTextureCache.at(unitPath);
		}

		if (!assets.unitGifDurationMillisecCache.contains(unitPath))
		{
			Array<Texture> frames;
			Array<int32> delaysMillisec;
			int32 durationMillisec = 0;
			AnimatedGIFReader reader{ unitPath };
			Array<Image> images;
			if (reader && reader.read(images, delaysMillisec, durationMillisec) && !images.isEmpty())
			{
				frames.reserve(images.size());
				for (const auto& image : images)
				{
					frames << Texture{ image };
				}
			}

			assets.unitGifFrameCache.emplace(unitPath, std::move(frames));
			assets.unitGifFrameDelaysMillisecCache.emplace(unitPath, std::move(delaysMillisec));
			assets.unitGifDurationMillisecCache.emplace(unitPath, Max(durationMillisec, 1));
		}

		if (assets.unitGifFrameCache.contains(unitPath))
		{
			const Array<Texture>& frames = assets.unitGifFrameCache.at(unitPath);
			if (!frames.isEmpty())
			{
				if (!animateGif)
				{
					return &frames.front();
				}

				if (assets.unitGifFrameDelaysMillisecCache.contains(unitPath) && assets.unitGifDurationMillisecCache.contains(unitPath))
				{
					const Array<int32>& delaysMillisec = assets.unitGifFrameDelaysMillisecCache.at(unitPath);
					const int32 durationMillisec = assets.unitGifDurationMillisecCache.at(unitPath);
					if (!delaysMillisec.isEmpty() && durationMillisec > 0)
					{
						const size_t frameIndex = AnimatedGIFReader::GetFrameIndex(Scene::Time(), delaysMillisec, durationMillisec);
						return &frames[Min(frameIndex, frames.size() - 1)];
					}
				}

				return &frames.front();
			}
		}

		if (!assets.unitTextureCache.contains(unitPath))
		{
			assets.unitTextureCache.emplace(unitPath, Texture{ unitPath });
		}
		return &assets.unitTextureCache.at(unitPath);
	}

	inline bool DrawUnitTexture(const BattleRenderAssets& assets, const UnitDef& def, const Vec2& pos, bool isMoving, StringView iconOverride = U"", const Vec2& iconOverrideOffset = Vec2{ 0, 0 })
	{
		const UnitVisualInfo visual = FindUnitVisualInfoByTag(assets, def.unit_id);
		const auto resolveOverrideArtWidth = [&](const String& overrideName) -> double
		{
			if (!visual.lineIconHorizontalName.isEmpty() && overrideName == visual.lineIconHorizontalName)
			{
				return visual.artWidthValueLineHorizontal;
			}
			if (!visual.lineIconDiagUpRightName.isEmpty() && overrideName == visual.lineIconDiagUpRightName)
			{
				return visual.artWidthValueLineDiagUpRight;
			}
			if (!visual.lineIconDiagUpLeftName.isEmpty() && overrideName == visual.lineIconDiagUpLeftName)
			{
				return visual.artWidthValueLineDiagUpLeft;
			}
			return visual.artWidthValue;
		};
		const auto drawByAnchor = [&](const TextureRegion& region, const Vec2& drawPos, UnitPlacementAnchor anchor)
		{
			if (anchor == UnitPlacementAnchor::BottomCenter)
			{
				region.draw(Arg::bottomCenter = drawPos);
			}
			else
			{
				region.drawAt(drawPos);
			}
		};

		if (!iconOverride.isEmpty())
		{
			const String overrideName = String{ iconOverride };
			const Array<FilePath> overrideCandidates = {
				ResolveBuildIconPath(overrideName),
				ResolveUnitChipPath(overrideName),
				ResolveBuildingChipPath(overrideName)
			};

			for (const auto& overridePath : overrideCandidates)
			{
				if (!FileSystem::Exists(overridePath))
				{
					continue;
				}

				if (!assets.unitTextureCache.contains(overridePath))
				{
					assets.unitTextureCache.emplace(overridePath, Texture{ overridePath });
				}

				const Texture& texture = assets.unitTextureCache.at(overridePath);
				UnitVisualInfo overrideVisual = visual;
				overrideVisual.artWidthValue = resolveOverrideArtWidth(overrideName);
				const SizeF drawSize = ResolveUnitTextureDrawSize(texture, def, overrideVisual, true);
				drawByAnchor(texture.resized(drawSize.x, drawSize.y), pos + iconOverrideOffset, visual.placementAnchor);
				return true;
			}
		}

		if (visual.image.isEmpty())
		{
			return false;
		}

		const FilePath unitPath = ResolveCatalogVisualPath(visual.kind, visual.image);
		if (!FileSystem::Exists(unitPath))
		{
			return false;
		}

		const Texture* texture = ResolveUnitDisplayTexture(assets, unitPath, isMoving);
		if (!texture)
		{
			return false;
		}
		const bool isBuildingVisual = (visual.kind.lowercased() == U"building");
		const SizeF drawSize = ResolveUnitTextureDrawSize(*texture, def, visual, isBuildingVisual);
		const Vec2 visualOffset = Vec2{ static_cast<double>(visual.visualOffset.x), static_cast<double>(visual.visualOffset.y) } * visual.visualScale;
		const UnitPlacementAnchor anchor = isBuildingVisual ? visual.placementAnchor : UnitPlacementAnchor::Center;
		drawByAnchor(texture->resized(drawSize.x, drawSize.y), pos + visualOffset, anchor);
		return true;
	}

	inline FilePath ResolveBuildActionIconPath(const BuildActionDef& action, const DefinitionStores& defs, const BattleRenderAssets& assets)
	{
		if (action.spawnUnit < defs.units.size())
		{
			const String unitTag = defs.units[action.spawnUnit].unit_id;
			const UnitVisualInfo visual = FindUnitVisualInfoByTag(assets, unitTag);
			if (!visual.image.isEmpty())
			{
				const FilePath iconPath = ResolveCatalogVisualPath(visual.kind, visual.image);
				if (FileSystem::Exists(iconPath))
				{
					return iconPath;
				}
			}
		}

		if (!action.icon.isEmpty())
		{
			return ResolveBuildIconPath(action.icon);
		}

		return FilePath{};
	}

	inline Array<FilePath> ResolveBuildActionIconPaths(const BuildActionDef& action, const DefinitionStores& defs, const BattleRenderAssets& assets)
	{
		Array<FilePath> iconPaths;
		if (!action.iconLayers.isEmpty())
		{
			for (const auto& icon : action.iconLayers)
			{
				if (icon.isEmpty())
				{
					continue;
				}

				iconPaths << ResolveBuildIconPath(icon);
			}

			if (!iconPaths.isEmpty())
			{
				return iconPaths;
			}
		}

		const FilePath fallbackPath = ResolveBuildActionIconPath(action, defs, assets);
		if (!fallbackPath.isEmpty())
		{
			iconPaths << fallbackPath;
		}

		return iconPaths;
	}

	inline bool DrawBuildActionIcon(const BuildActionDef& action, const DefinitionStores& defs, const BattleRenderAssets& assets, const Vec2& center, double size)
	{
		const Array<FilePath> iconPaths = ResolveBuildActionIconPaths(action, defs, assets);
		if (iconPaths.isEmpty())
		{
			return false;
		}

		bool drewAny = false;
		for (const auto& iconPath : iconPaths)
		{
			if (iconPath.isEmpty() || !FileSystem::Exists(iconPath))
			{
				continue;
			}

			if (FileSystem::Extension(iconPath).lowercased() == U"gif")
			{
				if (!assets.unitGifFrameCache.contains(iconPath))
				{
					AnimatedGIFReader reader{ iconPath };
					Array<Image> frames;
					Array<int32> frameDelaysMillisec;
					int32 durationMillisec = 0;
					if (reader && reader.read(frames, frameDelaysMillisec, durationMillisec) && !frames.isEmpty())
					{
						Array<Texture> textures;
						textures.reserve(frames.size());
						for (const auto& frame : frames)
						{
							textures << Texture{ frame };
						}
						assets.unitGifFrameCache.emplace(iconPath, std::move(textures));
						assets.unitGifFrameDelaysMillisecCache.emplace(iconPath, std::move(frameDelaysMillisec));
						assets.unitGifDurationMillisecCache.emplace(iconPath, Max(durationMillisec, 1));
					}
				}

				if (assets.unitGifFrameCache.contains(iconPath))
				{
					const Array<Texture>& frames = assets.unitGifFrameCache.at(iconPath);
					if (!frames.isEmpty())
					{
						const bool animate = RectF{ Arg::center = center, size + 6.0, size + 6.0 }.mouseOver();
						size_t frameIndex = 0;
						if (animate
							&& assets.unitGifFrameDelaysMillisecCache.contains(iconPath)
							&& assets.unitGifDurationMillisecCache.contains(iconPath))
						{
							const Array<int32>& delays = assets.unitGifFrameDelaysMillisecCache.at(iconPath);
							const int32 duration = assets.unitGifDurationMillisecCache.at(iconPath);
							if (!delays.isEmpty() && duration > 0)
							{
								frameIndex = AnimatedGIFReader::GetFrameIndex(Scene::Time(), delays, duration);
							}
						}

						frames[Min(frameIndex, frames.size() - 1)].resized(size, size).drawAt(center);
						drewAny = true;
					}
				}

				continue;
			}

			if (!assets.iconTextureCache.contains(iconPath))
			{
				assets.iconTextureCache.emplace(iconPath, Texture{ iconPath });
			}
			assets.iconTextureCache.at(iconPath).resized(size, size).drawAt(center);
			drewAny = true;
		}

		return drewAny;
	}
}
