#pragma once
# include <Siv3D.hpp>
# include "MapEditorInput.h"

namespace LT3
{
	inline const Texture* GetMapEditorAssetDisplayTexture(const MapEditorAsset& asset, double timeSec);

	inline Vec2 DecalShadowDirectionVector(DecalShadowDirection direction)
	{
		switch (direction)
		{
		case DecalShadowDirection::North: return Vec2{ 0.0, -1.0 };
		case DecalShadowDirection::NorthEast: return Vec2{ 1.0, -1.0 }.normalized();
		case DecalShadowDirection::East: return Vec2{ 1.0, 0.0 };
		case DecalShadowDirection::SouthEast: return Vec2{ 1.0, 1.0 }.normalized();
		case DecalShadowDirection::South: return Vec2{ 0.0, 1.0 };
		case DecalShadowDirection::SouthWest: return Vec2{ -1.0, 1.0 }.normalized();
		case DecalShadowDirection::West: return Vec2{ -1.0, 0.0 };
		case DecalShadowDirection::NorthWest: return Vec2{ -1.0, -1.0 }.normalized();
		default: return Vec2{ 1.0, 1.0 }.normalized();
		}
	}

	inline void DrawDecalShadow(const MapEditorAsset& asset, const Vec2& bottomCenter, double scale, double opacity, double animationTimeSec)
	{
		if (!asset.useDecalShadow)
		{
			return;
		}

		const Vec2 direction = DecalShadowDirectionVector(asset.decalShadowDirection);
		const double clampedScale = Clamp(scale, 0.1, 4.0);
		const double shadowOpacity = Clamp(asset.decalShadowOpacity * opacity, 0.0, 1.0);
		const double shadowLength = Max(0.0, asset.decalShadowLength) * clampedScale;
		const double softness = Max(0.0, asset.decalShadowBlur);
		const Vec2 shadowOffset = direction * shadowLength;

		if (asset.decalShadowMode == DecalShadowMode::Circle)
		{
			const double radiusX = 18.0 * clampedScale + shadowLength * 0.35;
			const double radiusY = 9.0 * clampedScale + shadowLength * 0.18;
			Ellipse{ bottomCenter + shadowOffset.movedBy(0.0, 8.0 * clampedScale), radiusX, radiusY }
				.draw(ColorF{ 0.0, 0.0, 0.0, shadowOpacity * 0.65 });
			if (softness > 0.0)
			{
				Ellipse{ bottomCenter + shadowOffset.movedBy(0.0, 8.0 * clampedScale), radiusX + softness, radiusY + softness * 0.5 }
					.draw(ColorF{ 0.0, 0.0, 0.0, shadowOpacity * 0.20 });
			}
			return;
		}

		if (const Texture* texture = GetMapEditorAssetDisplayTexture(asset, animationTimeSec))
		{
			const ScopedRenderStates2D blend{ BlendState::Premultiplied };
			texture->scaled(clampedScale).draw(Arg::bottomCenter = bottomCenter + shadowOffset, ColorF{ 0.0, 0.0, 0.0, shadowOpacity * 0.75 });
			if (softness > 0.0)
			{
				texture->scaled(clampedScale * (1.0 + softness * 0.01)).draw(Arg::bottomCenter = bottomCenter + shadowOffset.movedBy(softness * 0.3, softness * 0.25), ColorF{ 0.0, 0.0, 0.0, shadowOpacity * 0.20 });
			}
		}
	}

	inline void DrawAssetPreview(const MapEditorAsset& asset, const Vec2& center, const SizeF& size)
	{
		const auto getDisplayTexture = [&](double timeSec) -> const Texture*
		{
			if (asset.isAnimatedGif && !asset.animationFrames.isEmpty() && !asset.animationFrameDelaysMillisec.isEmpty())
			{
				const size_t frameIndex = AnimatedGIFReader::GetFrameIndex(timeSec, asset.animationFrameDelaysMillisec, Max(asset.animationDurationMillisec, 1));
				return &asset.animationFrames[Min(frameIndex, asset.animationFrames.size() - 1)];
			}

			return asset.texture ? &asset.texture : nullptr;
		};

		if (const Texture* texture = getDisplayTexture(Scene::Time()))
		{
			texture->resized(size).drawAt(center);
		}
		else
		{
			RectF{ Arg::center = center, size }.draw(ColorF{ 0.18, 0.18, 0.20 });
		}
	}

	inline const Texture* GetMapEditorAssetDisplayTexture(const MapEditorAsset& asset, double timeSec)
	{
		if (asset.isAnimatedGif && !asset.animationFrames.isEmpty() && !asset.animationFrameDelaysMillisec.isEmpty())
		{
			const size_t frameIndex = AnimatedGIFReader::GetFrameIndex(timeSec, asset.animationFrameDelaysMillisec, Max(asset.animationDurationMillisec, 1));
			return &asset.animationFrames[Min(frameIndex, asset.animationFrames.size() - 1)];
		}

		return asset.texture ? &asset.texture : nullptr;
	}

	inline double ComputeMapEditorGifPhaseOffsetSec(const MapEditorAsset& asset, int32 cellX, int32 cellY)
	{
		if (!asset.isAnimatedGif || (asset.animationDurationMillisec <= 0))
		{
			return 0.0;
		}

		uint64 seed = 0x9E3779B97F4A7C15ULL;
		for (const char32 ch : asset.fileName)
		{
			seed ^= static_cast<uint64>(ch) + 0x9E3779B97F4A7C15ULL + (seed << 6) + (seed >> 2);
		}

		seed ^= static_cast<uint64>(static_cast<uint32>(cellX) * 0x85EBCA6BU);
		seed ^= static_cast<uint64>(static_cast<uint32>(cellY) * 0xC2B2AE35U);
		seed ^= (seed >> 30);
		seed *= 0xBF58476D1CE4E5B9ULL;
		seed ^= (seed >> 27);
		seed *= 0x94D049BB133111EBULL;
		seed ^= (seed >> 31);

		constexpr double invUint64Max = 1.0 / static_cast<double>(Largest<uint64>);
		const double unitPhase = static_cast<double>(seed) * invUint64Max;
		return (asset.animationDurationMillisec / 1000.0) * unitPhase;
	}

	inline void DrawPlacedMapAsset(const MapEditorAsset& asset, const Vec2& bottomCenter, double scale, double opacity, double animationTimeSec)
	{
		DrawDecalShadow(asset, bottomCenter, scale, opacity, animationTimeSec);
		if (const Texture* texture = GetMapEditorAssetDisplayTexture(asset, animationTimeSec))
		{
			const ScopedRenderStates2D blend{ BlendState::Premultiplied };
				const double a = Clamp(opacity, 0.0, 1.0);
				texture->scaled(Clamp(scale, 0.1, 4.0)).draw(Arg::bottomCenter = bottomCenter, ColorF{ a, a });
		}
		else
		{
			RectF{ Arg::bottomCenter = bottomCenter, 96 * Clamp(scale, 0.1, 4.0), 48 * Clamp(scale, 0.1, 4.0) }.draw(ColorF{ 0.18, 0.18, 0.20, Clamp(opacity, 0.0, 1.0) });
		}
	}

	inline void DrawPlacedMapAsset(const MapEditorAsset& asset, const Vec2& bottomCenter, double scale, double opacity)
	{
		DrawPlacedMapAsset(asset, bottomCenter, scale, opacity, Scene::Time());
	}

	inline void DrawPlacedMapAsset(const MapEditorAsset& asset, const Vec2& bottomCenter)
	{
		DrawPlacedMapAsset(asset, bottomCenter, 1.0, 1.0);
	}
}
