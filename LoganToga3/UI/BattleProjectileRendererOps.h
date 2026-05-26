#pragma once
# include <Siv3D.hpp>
# include "BattleUnitRendererAssetOps.h"

namespace LT3
{
	inline bool IsDiagonalProjectileAngle(double angleRad)
	{
		const double normalized = Math::Fmod(Math::Fmod(angleRad, Math::TwoPi) + Math::TwoPi, Math::TwoPi);
		const double octant = normalized / Math::QuarterPi;
		const int32 nearest = static_cast<int32>(Math::Round(octant)) & 7;
		return (nearest % 2) == 1;
	}

	inline double ResolveProjectileScreenAngle(double angleRad)
	{
		const Vec2 worldDirection{ Cos(angleRad), Sin(angleRad) };
		const Vec2 screenDirection = ToQuarterIso(worldDirection);
		if (screenDirection.lengthSq() <= 0.0001)
		{
			return angleRad;
		}

		return Math::Atan2(screenDirection.y, screenDirection.x);
	}

	inline bool ShouldUseDiagonalProjectileImage(const SkillDef& skill, double angleRad, bool quarterViewProjected)
	{
		const double imageAngle = quarterViewProjected ? ResolveProjectileScreenAngle(angleRad) : angleRad;
		return !skill.projectileDiagonalImage.isEmpty()
			&& IsDiagonalProjectileAngle(imageAngle);
	}

	inline FilePath ResolveProjectileImagePath(const SkillDef& skill, double angleRad, bool quarterViewProjected = true)
	{
		String imageName = skill.projectileImage;
		if (ShouldUseDiagonalProjectileImage(skill, angleRad, quarterViewProjected))
		{
			imageName = skill.projectileDiagonalImage;
		}
		if (imageName.isEmpty())
		{
			return FilePath{};
		}

		const Array<FilePath> candidates = {
			ResolveBuildIconPath(imageName),
			ResolveSystemImagePath(imageName),
			ResolveUnitChipPath(imageName),
			ResolveBuildingChipPath(imageName),
		};
		for (const auto& path : candidates)
		{
			if (FileSystem::Exists(path))
			{
				return path;
			}
		}

		return FilePath{};
	}

	inline Vec2 ResolveProjectileDrawCenter(const SkillDef& skill, const SizeF& drawSize, double angleRad)
	{
		if (skill.projectileCenter == SkillProjectileCenter::Off)
		{
			return Vec2{ 0.0, -drawSize.y * 0.5 };
		}
		if (skill.projectileCenter == SkillProjectileCenter::End)
		{
			return Vec2{ Cos(angleRad), Sin(angleRad) } * (drawSize.y * 0.5);
		}
		return Vec2{ 0.0, 0.0 };
	}

	inline Optional<Vec2> ResolveSwingEndProjectileTipScreen(const SkillDef& skill, const Vec2& rootWorld, double angleRad)
	{
		if (skill.projectileMotion != SkillProjectileMotion::Swing || skill.projectileCenter != SkillProjectileCenter::End)
		{
			return none;
		}

		const double length = Max(1.0, skill.projectileHeight);
		const Vec2 direction{ Cos(angleRad), Sin(angleRad) };
		return ToQuarterScreen(rootWorld + direction * length);
	}

	inline Optional<Vec2> ResolveSwingEndProjectileTipScreenInPlane(const SkillDef& skill, const Vec2& rootPos, double angleRad)
	{
		if (skill.projectileMotion != SkillProjectileMotion::Swing || skill.projectileCenter != SkillProjectileCenter::End)
		{
			return none;
		}

		const double length = Max(1.0, skill.projectileHeight);
		const Vec2 direction{ Cos(angleRad), Sin(angleRad) };
		return rootPos + direction * length;
	}

	inline bool DrawSwingEndProjectileTexture(const BattleRenderAssets& assets, const SkillDef& skill, const Vec2& rootScreen, const Vec2& tipScreen)
	{
		const FilePath imagePath = ResolveProjectileImagePath(skill, 0.0);
		if (imagePath.isEmpty())
		{
			return false;
		}

		if (!assets.iconTextureCache.contains(imagePath))
		{
			assets.iconTextureCache.emplace(imagePath, Texture{ imagePath });
		}

		const Vec2 delta = tipScreen - rootScreen;
		const double drawLength = delta.length();
		if (drawLength <= 1.0)
		{
			return false;
		}

		const Texture& texture = assets.iconTextureCache.at(imagePath);
		const double width = Max(1.0, skill.projectileWidth);
		const TextureRegion region = texture.resized(width, drawLength);
		const double drawAngle = Math::Atan2(delta.y, delta.x) + Math::HalfPi;
		region.rotatedAt(region.region().bottomCenter(), drawAngle).drawAt(rootScreen);
		return true;
	}

	inline bool DrawProjectileTexture(const BattleRenderAssets& assets, const SkillDef& skill, const Vec2& drawPos, double angleRad, bool quarterViewProjected = true)
	{
		const double screenAngle = quarterViewProjected ? ResolveProjectileScreenAngle(angleRad) : angleRad;
		const bool useDiagonalImage = ShouldUseDiagonalProjectileImage(skill, angleRad, quarterViewProjected);
		const FilePath imagePath = ResolveProjectileImagePath(skill, angleRad, quarterViewProjected);
		if (imagePath.isEmpty())
		{
			return false;
		}

		if (!assets.iconTextureCache.contains(imagePath))
		{
			assets.iconTextureCache.emplace(imagePath, Texture{ imagePath });
		}

		const Texture& texture = assets.iconTextureCache.at(imagePath);
		const double width = Max(1.0, skill.projectileWidth);
		const double height = Max(1.0, skill.projectileHeight);
		const SizeF drawSize{ width, height };
		const double assetForwardOffset = useDiagonalImage ? (Math::HalfPi + Math::QuarterPi) : Math::HalfPi;
		const double drawAngle = skill.projectileD360 ? (screenAngle + assetForwardOffset) : 0.0;
		const TextureRegion region = texture.resized(drawSize.x, drawSize.y);
		if (skill.projectileCenter == SkillProjectileCenter::End)
		{
			region.rotatedAt(region.region().bottomCenter(), drawAngle).drawAt(drawPos);
			return true;
		}

		const Vec2 centerOffset = ResolveProjectileDrawCenter(skill, drawSize, drawAngle);
		region.rotated(drawAngle).drawAt(drawPos + centerOffset);
		return true;
	}

	inline bool ShouldDrawSkillRay(const SkillDef& skill)
	{
		if (skill.kind != SkillKind::Missile || skill.rayMode == SkillRayMode::None)
		{
			return false;
		}

		return skill.projectileMotion == SkillProjectileMotion::Direct
			|| skill.projectileMotion == SkillProjectileMotion::Arc
			|| skill.projectileMotion == SkillProjectileMotion::Parabola;
	}

	inline ColorF ResolveSkillRayColor(const SkillDef& skill, double alphaScale = 1.0)
	{
		const ColorF base = skill.color;
		const ColorF tinted{
			Clamp(base.r * 0.40 + 0.15, 0.0, 1.0),
			Clamp(base.g * 0.55 + 0.30, 0.0, 1.0),
			Clamp(base.b * 0.85 + 0.60, 0.0, 1.0),
			Clamp(0.36 * alphaScale, 0.0, 1.0)
		};
		return tinted;
	}

	inline void DrawSkillRayLine(const SkillDef& skill, const Vec2& drawPos, double angleRad, bool quarterViewProjected = true, const Optional<Vec2>& rayTailPos = none)
	{
		const double screenAngle = quarterViewProjected ? ResolveProjectileScreenAngle(angleRad) : angleRad;
		const Vec2 dir{ Cos(screenAngle), Sin(screenAngle) };
		const double lineLen = Max(12.0, skill.projectileHeight * 1.8 * Max(0.1, skill.rayLength));
		const double lineWidth = Max(1.2, skill.projectileWidth * 0.18);
		const Vec2 tail = rayTailPos.value_or(drawPos - dir * lineLen);
		Line{ drawPos, tail }.draw(lineWidth, ResolveSkillRayColor(skill));
	}

	inline void DrawSkillRayImage(const BattleRenderAssets& assets, const SkillDef& skill, const Vec2& drawPos, double angleRad, bool quarterViewProjected = true)
	{
		const FilePath imagePath = ResolveProjectileImagePath(skill, angleRad, quarterViewProjected);
		if (imagePath.isEmpty())
		{
			DrawSkillRayLine(skill, drawPos, angleRad, quarterViewProjected);
			return;
		}

		if (!assets.iconTextureCache.contains(imagePath))
		{
			assets.iconTextureCache.emplace(imagePath, Texture{ imagePath });
		}

		const Texture& texture = assets.iconTextureCache.at(imagePath);
		const double width = Max(1.0, skill.projectileWidth);
		const double height = Max(1.0, skill.projectileHeight);
		const SizeF drawSize{ width, height };
		const bool useDiagonalImage = ShouldUseDiagonalProjectileImage(skill, angleRad, quarterViewProjected);
		const double screenAngle = quarterViewProjected ? ResolveProjectileScreenAngle(angleRad) : angleRad;
		const double assetForwardOffset = useDiagonalImage ? (Math::HalfPi + Math::QuarterPi) : Math::HalfPi;
		const double drawAngle = skill.projectileD360 ? (screenAngle + assetForwardOffset) : 0.0;
		const Vec2 backDir{ Cos(screenAngle), Sin(screenAngle) };

		constexpr int32 rayCopies = 3;
		const double spacing = Max(8.0, skill.projectileHeight * 0.34 * Max(0.1, skill.rayLength));
		for (int32 i = 0; i < rayCopies; ++i)
		{
			const Vec2 tailPos = drawPos - backDir * spacing * (i + 1);
			const double fade = 1.0 - i * 0.27;
			const TextureRegion region = texture.resized(drawSize.x, drawSize.y);
			if (skill.projectileCenter == SkillProjectileCenter::End)
			{
				region.rotatedAt(region.region().bottomCenter(), drawAngle).drawAt(tailPos, ResolveSkillRayColor(skill, fade));
			}
			else
			{
				const Vec2 centerOffset = ResolveProjectileDrawCenter(skill, drawSize, drawAngle);
				region.rotated(drawAngle).drawAt(tailPos + centerOffset, ResolveSkillRayColor(skill, fade));
			}
		}
	}

	inline void DrawSkillRay(const BattleRenderAssets& assets, const SkillDef& skill, const Vec2& drawPos, double angleRad, bool quarterViewProjected = true, const Optional<Vec2>& rayTailPos = none)
	{
		if (!ShouldDrawSkillRay(skill))
		{
			return;
		}

		if (skill.rayMode == SkillRayMode::Line)
		{
			DrawSkillRayLine(skill, drawPos, angleRad, quarterViewProjected, rayTailPos);
			return;
		}

		if (skill.rayMode == SkillRayMode::Image)
		{
			DrawSkillRayImage(assets, skill, drawPos, angleRad, quarterViewProjected);
		}
	}

	inline void DrawProjectiles(const BattleWorld& world, const DefinitionStores& defs, const BattleRenderAssets* assets = nullptr)
	{
		for (size_t i = 0; i < world.projectiles.position.size(); ++i)
		{
			const SkillDef& skill = defs.skills[world.projectiles.skill[i]];
			const Vec2 base = ToQuarterScreen(world.projectiles.position[i]);
			const Vec2 drawPos = base + Vec2{ 0.0, -world.projectiles.height[i] };
			if (assets)
			{
				Optional<Vec2> rayTailPos = none;
				if (skill.rayLockToCaster
					&& world.projectiles.owner[i] != InvalidUnitId
					&& world.projectiles.owner[i] < world.units.position.size()
					&& IsValidUnit(world, world.projectiles.owner[i]))
				{
					rayTailPos = ToQuarterScreen(world.units.position[world.projectiles.owner[i]]);
				}
				DrawSkillRay(*assets, skill, drawPos, world.projectiles.angleRad[i], true, rayTailPos);
			}
			if (assets)
			{
				if (const Optional<Vec2> tipScreen = ResolveSwingEndProjectileTipScreen(skill, world.projectiles.position[i], world.projectiles.angleRad[i]))
				{
					if (DrawSwingEndProjectileTexture(*assets, skill, drawPos, *tipScreen))
					{
						continue;
					}
				}
			}
			if (assets && DrawProjectileTexture(*assets, skill, drawPos, world.projectiles.angleRad[i]))
			{
				continue;
			}
			if (world.projectiles.motion[i] == SkillProjectileMotion::Parabola && world.projectiles.height[i] > 1.0)
			{
				Circle{ base, 3.5 }.draw(ColorF{ 0, 0, 0, 0.25 });
				Line{ base, drawPos }.draw(1.0, ColorF{ skill.color, 0.28 });
			}
			if (world.projectiles.motion[i] == SkillProjectileMotion::Orbit)
			{
				Circle{ drawPos, 6 }.drawFrame(2.0, skill.color);
			}
			else
			{
				Circle{ drawPos, 4 }.draw(skill.color);
			}
		}
	}
}
