#pragma once
# include "SkillEditorDraw.Common.h"
# include "SkillEditorAssets.h"
# include "MapEditorAssetUtils.h"
# include "BattleUnitRendererAssetOps.h"
# include "BattleProjectileRendererOps.h"

namespace LT3
{
	inline bool DrawSkillEditorSandboxBomVisual(BattleRenderAssets& assets, const MapEditorState& editor)
	{
		if (editor.skillSandboxLastBomDisplaySec <= 0.0)
		{
			return false;
		}

		const FilePath imagePath = ResolveSkillIconPath(editor.skillSandboxLastBomImage);
		if (imagePath.isEmpty())
		{
			return false;
		}

		const double duration = Max(0.05, editor.skillSandboxLastBomVisualDurationSec);
		const double alpha = Clamp(editor.skillSandboxLastBomDisplaySec / duration, 0.0, 1.0);
		const double drawSize = Max(12.0, editor.skillSandboxLastBomRadius * 2.0 * Max(0.1, editor.skillSandboxLastBomVisualScale));
		const ColorF tint{ 1.0, 1.0, 1.0, 0.32 + 0.68 * alpha };

		if (FileSystem::Extension(imagePath).lowercased() == U"gif")
		{
			if (!assets.unitGifDurationMillisecCache.contains(imagePath))
			{
				Array<Texture> frames;
				Array<int32> delaysMillisec;
				int32 durationMillisec = 0;
				AnimatedGIFReader reader{ imagePath };
				Array<Image> images;
				if (reader && reader.read(images, delaysMillisec, durationMillisec) && !images.isEmpty())
				{
					frames.reserve(images.size());
					for (auto image : images)
					{
						PremultiplyImageAlpha(image);
						frames << Texture{ image };
					}
				}

				assets.unitGifFrameCache.emplace(imagePath, std::move(frames));
				assets.unitGifFrameDelaysMillisecCache.emplace(imagePath, std::move(delaysMillisec));
				assets.unitGifDurationMillisecCache.emplace(imagePath, Max(durationMillisec, 1));
			}

			if (assets.unitGifFrameCache.contains(imagePath))
			{
				const Array<Texture>& frames = assets.unitGifFrameCache.at(imagePath);
				const Array<int32>& delaysMillisec = assets.unitGifFrameDelaysMillisecCache.at(imagePath);
				const int32 durationMillisec = assets.unitGifDurationMillisecCache.at(imagePath);
				if (!frames.isEmpty() && !delaysMillisec.isEmpty() && durationMillisec > 0)
				{
					const size_t frameIndex = AnimatedGIFReader::GetFrameIndex(Scene::Time(), delaysMillisec, durationMillisec);
					frames[Min(frameIndex, frames.size() - 1)].resized(drawSize).drawAt(editor.skillSandboxLastBomCenter, tint);
					return true;
				}
			}
		}

		if (!assets.iconTextureCache.contains(imagePath))
		{
			assets.iconTextureCache.emplace(imagePath, Texture{ imagePath });
		}

		if (assets.iconTextureCache.contains(imagePath))
		{
			assets.iconTextureCache.at(imagePath).resized(drawSize).drawAt(editor.skillSandboxLastBomCenter, tint);
			return true;
		}

		return false;
	}

	inline void DrawSkillEditorSandboxPreview(const MapEditorState& editor, const DefinitionStores& defs, const UnitCatalog& catalog, const Font& uiFont)
	{
		if (!editor.showSkillSandboxPreview)
		{
			return;
		}

		static BattleRenderAssets sandboxAssets;

		const RectF preview = SkillEditorSandboxPreviewRect();
		preview.draw(ColorF{ 0.015, 0.022, 0.032, 0.96 }).drawFrame(2, ColorF{ 0.25, 0.70, 1.0, 0.55 });
		uiFont(U"Skill Sandbox Preview").draw(18, preview.x + 18.0, preview.y + 14.0, Palette::White);

		const bool isUnitMode = (editor.skillSandboxMode == SkillSandboxMode::Unit);
		const bool hasSkill = HasSelectedSkill(editor, defs);
		const SkillDef* sandboxSkill = nullptr;
		if (isUnitMode)
		{
			if (editor.skillSandboxActiveSkillId != InvalidSkillDefId
				&& editor.skillSandboxActiveSkillId < static_cast<SkillDefId>(defs.skills.size()))
			{
				sandboxSkill = &defs.skills[editor.skillSandboxActiveSkillId];
			}
		}
		else if (hasSkill)
		{
			sandboxSkill = &defs.skills[editor.selectedSkillIndex];
		}

		if (isUnitMode)
		{
			const int32 uci = editor.skillSandboxUnitCatalogIndex;
			const String unitName = (uci >= 0 && uci < static_cast<int32>(catalog.entries.size()))
				? (catalog.entries[uci].name.isEmpty() ? catalog.entries[uci].unit_id : catalog.entries[uci].name)
				: U"<none>";
			const String activeTag = (sandboxSkill && !sandboxSkill->tag.isEmpty()) ? sandboxSkill->tag : U"-";
			uiFont(U"[Unit Mode] {} | active: {}"_fmt(unitName, activeTag))
				.draw(12, preview.x + 20.0, preview.y + 44.0, ColorF{ 0.75, 1.0, 0.85 });
		}
		else
		{
			const String skillName = hasSkill ? defs.skills[editor.selectedSkillIndex].name : U"<none>";
			uiFont(U"selected: {}"_fmt(skillName.isEmpty() && hasSkill ? defs.skills[editor.selectedSkillIndex].tag : skillName))
				.draw(12, preview.x + 20.0, preview.y + 44.0, Palette::Lightgray);
		}
		DrawRectButton(SkillEditorSandboxButtonRect(0), U"Play", false, uiFont, RectButtonStyle{ .fontSize = 11 });
		DrawRectButton(SkillEditorSandboxButtonRect(1), editor.skillSandboxAutoFire ? U"Auto ON" : U"Auto OFF", editor.skillSandboxAutoFire, uiFont, RectButtonStyle{ .fontSize = 11 });
		DrawRectButton(SkillEditorSandboxButtonRect(2), U"Reset", false, uiFont, RectButtonStyle{ .fontSize = 11 });
		DrawRectButton(SkillEditorSandboxButtonRect(3), isUnitMode ? U"[Unit]" : U"[Skill]", isUnitMode, uiFont, RectButtonStyle{ .fontSize = 11 });

		const RectF arena = SkillEditorSandboxArenaRect();
		arena.draw(ColorF{ 0.04, 0.05, 0.065, 0.92 }).drawFrame(1, ColorF{ 1, 1, 1, 0.14 });
		if (isUnitMode)
		{
			// Unit Mode: 全スキルの射程リングを色分け描画
			const int32 uci = editor.skillSandboxUnitCatalogIndex;
			if (uci >= 0 && uci < static_cast<int32>(catalog.entries.size()))
			{
				const Array<ColorF> ringColors = {
					ColorF{ 0.22, 0.72, 1.0, 0.70 }, ColorF{ 1.0, 0.82, 0.18, 0.70 },
					ColorF{ 0.55, 1.0, 0.55, 0.70 }, ColorF{ 1.0, 0.42, 0.72, 0.70 }
				};
				int32 colorIdx = 0;
				for (const String& tag : catalog.entries[uci].skills)
				{
					const auto it = defs.skillByTag.find(tag);
					if (it == defs.skillByTag.end()) continue;
					const SkillDefId sid = it->second;
					if (sid < 0 || sid >= static_cast<SkillDefId>(defs.skills.size())) continue;
					const SkillDef& sk = defs.skills[sid];
					const bool isActive = (sid == editor.skillSandboxActiveSkillId);
					const ColorF col = ringColors[colorIdx % static_cast<int32>(ringColors.size())];
					const double frameW = isActive ? 3.0 : 1.2;
					Circle{ editor.skillSandboxCasterPos, Max(0.0, sk.range) }.drawFrame(frameW, col);
					if (sk.rangeMin > 0.0)
					{
						Circle{ editor.skillSandboxCasterPos, sk.rangeMin }.drawFrame(frameW * 0.7, ColorF{ col.r, col.g, col.b, col.a * 0.75 });
					}
					++colorIdx;
				}
			}
		}
		else if (sandboxSkill)
		{
			Circle{ editor.skillSandboxCasterPos, Max(0.0, sandboxSkill->range) }.drawFrame(1.6, ColorF{ 0.22, 0.72, 1.0, 0.58 });
			if (sandboxSkill->rangeMin > 0.0)
			{
				Circle{ editor.skillSandboxCasterPos, sandboxSkill->rangeMin }.drawFrame(1.6, ColorF{ 1.0, 0.72, 0.18, 0.62 });
			}
		}
		if (sandboxSkill && editor.skillSandboxLastBomDisplaySec > 0.0 && editor.skillSandboxLastBomRadius > 0.0)
		{
			const double alpha = Clamp(editor.skillSandboxLastBomDisplaySec / Max(0.05, editor.skillSandboxLastBomVisualDurationSec), 0.0, 1.0);
			ColorF fillColor{ 1.0, 0.45, 0.12, 0.10 + 0.14 * alpha };
			ColorF frameColor{ 1.0, 0.72, 0.24, 0.34 + 0.34 * alpha };
			if (editor.skillSandboxLastBomKind == SkillKind::Heal)
			{
				fillColor = ColorF{ 0.22, 1.0, 0.42, 0.10 + 0.16 * alpha };
				frameColor = ColorF{ 0.56, 1.0, 0.66, 0.38 + 0.34 * alpha };
			}
			else if (editor.skillSandboxLastBomFriendlyFire)
			{
				fillColor = ColorF{ 1.0, 0.18, 0.18, 0.12 + 0.16 * alpha };
				frameColor = ColorF{ 1.0, 0.36, 0.36, 0.40 + 0.36 * alpha };
			}
			const bool drewImage = (editor.skillSandboxLastBomVisual == SkillBomVisual::Image)
				&& DrawSkillEditorSandboxBomVisual(sandboxAssets, editor);
			if (!drewImage)
			{
				Circle{ editor.skillSandboxLastBomCenter, editor.skillSandboxLastBomRadius }.draw(fillColor).drawFrame(2.0, frameColor);
			}
		}
		Line{ arena.x + 28.0, editor.skillSandboxCasterPos.y, arena.x + arena.w - 28.0, editor.skillSandboxCasterPos.y }.draw(1.5, ColorF{ 0.35, 0.55, 0.80, 0.30 });
		Circle{ editor.skillSandboxCasterPos, 24.0 }.draw(ColorF{ 0.20, 0.55, 1.0, 0.85 }).drawFrame(2, ColorF{ 0.0, 1.0, 1.0, 0.75 });
		Circle{ editor.skillSandboxAllyPos, 28.0 }.draw(ColorF{ 0.22, 0.88, 0.42, 0.82 }).drawFrame(2, editor.skillSandboxDraggingAlly ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 0.72, 1.0, 0.72, 0.76 });
		for (int32 i = 0; i < static_cast<int32>(editor.skillSandboxExtraAllies.size()); ++i)
		{
			const auto& ally = editor.skillSandboxExtraAllies[i];
			const bool dragging = editor.skillSandboxDraggingExtraAllyIndex && *editor.skillSandboxDraggingExtraAllyIndex == i;
			const ColorF fill = (ally.hp > 0) ? ColorF{ 0.18, 0.72, 0.32, 0.74 } : ColorF{ 0.24, 0.24, 0.24, 0.48 };
			const ColorF frame = dragging ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 0.62, 1.0, 0.56, 0.74 };
			Circle{ ally.pos, 24.0 }.draw(fill).drawFrame(2, frame);
			uiFont(U"Ally{}"_fmt(i + 2)).drawAt(11, ally.pos + Vec2{ 0.0, 36.0 }, Palette::Lightgray);
			const double allyHpRate = ally.maxHp > 0 ? Clamp(static_cast<double>(ally.hp) / ally.maxHp, 0.0, 1.0) : 0.0;
			const RectF allyHpBack{ Arg::center = ally.pos + Vec2{ 0.0, -34.0 }, 66.0, 7.0 };
			allyHpBack.draw(ColorF{ 0.03, 0.03, 0.03, 0.85 });
			RectF{ allyHpBack.pos, allyHpBack.w * allyHpRate, allyHpBack.h }.draw(ColorF{ 0.28, 0.92, 0.42 });
		}
		Circle{ editor.skillSandboxTargetPos, 30.0 }.draw(ColorF{ 0.85, 0.18, 0.13, 0.85 }).drawFrame(2, editor.skillSandboxDraggingTarget ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1.0, 0.45, 0.35, 0.75 });
		for (int32 i = 0; i < static_cast<int32>(editor.skillSandboxExtraTargets.size()); ++i)
		{
			const auto& dummy = editor.skillSandboxExtraTargets[i];
			const bool dragging = editor.skillSandboxDraggingExtraTargetIndex && *editor.skillSandboxDraggingExtraTargetIndex == i;
			const ColorF fill = (dummy.hp > 0) ? ColorF{ 0.82, 0.36, 0.16, 0.72 } : ColorF{ 0.24, 0.24, 0.24, 0.48 };
			const ColorF frame = dragging ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1.0, 0.62, 0.26, 0.72 };
			Circle{ dummy.pos, 24.0 }.draw(fill).drawFrame(2, frame);
			uiFont(U"Dummy{}"_fmt(i + 2)).drawAt(11, dummy.pos + Vec2{ 0.0, 36.0 }, Palette::Lightgray);
			const double dummyHpRate = dummy.maxHp > 0 ? Clamp(static_cast<double>(dummy.hp) / dummy.maxHp, 0.0, 1.0) : 0.0;
			const RectF dummyHpBack{ Arg::center = dummy.pos + Vec2{ 0.0, -34.0 }, 66.0, 7.0 };
			dummyHpBack.draw(ColorF{ 0.03, 0.03, 0.03, 0.85 });
			RectF{ dummyHpBack.pos, dummyHpBack.w * dummyHpRate, dummyHpBack.h }.draw(ColorF{ 0.95, 0.58, 0.22 });
		}
		uiFont(U"Caster").drawAt(12, editor.skillSandboxCasterPos + Vec2{ 0.0, 44.0 }, Palette::Lightgray);
		uiFont(U"Ally").drawAt(12, editor.skillSandboxAllyPos + Vec2{ 0.0, 44.0 }, Palette::Lightgray);
		uiFont(U"Sandbag").drawAt(12, editor.skillSandboxTargetPos + Vec2{ 0.0, 50.0 }, Palette::Lightgray);

		const double casterHpRate = editor.skillSandboxCasterMaxHp > 0 ? Clamp(static_cast<double>(editor.skillSandboxCasterHp) / editor.skillSandboxCasterMaxHp, 0.0, 1.0) : 0.0;
		const RectF casterHpBack{ Arg::center = editor.skillSandboxCasterPos + Vec2{ 0.0, -40.0 }, 76.0, 7.0 };
		casterHpBack.draw(ColorF{ 0.03, 0.03, 0.03, 0.85 });
		RectF{ casterHpBack.pos, casterHpBack.w * casterHpRate, casterHpBack.h }.draw(ColorF{ 0.25, 0.80, 1.0 });
		uiFont(U"HP {}/{}"_fmt(editor.skillSandboxCasterHp, editor.skillSandboxCasterMaxHp)).drawAt(10, casterHpBack.center().movedBy(0.0, -13.0), Palette::White);

		const double allyHpRate = editor.skillSandboxAllyMaxHp > 0 ? Clamp(static_cast<double>(editor.skillSandboxAllyHp) / editor.skillSandboxAllyMaxHp, 0.0, 1.0) : 0.0;
		const RectF allyHpBack{ Arg::center = editor.skillSandboxAllyPos + Vec2{ 0.0, -40.0 }, 76.0, 7.0 };
		allyHpBack.draw(ColorF{ 0.03, 0.03, 0.03, 0.85 });
		RectF{ allyHpBack.pos, allyHpBack.w * allyHpRate, allyHpBack.h }.draw(ColorF{ 0.28, 0.92, 0.42 });
		uiFont(U"HP {}/{}"_fmt(editor.skillSandboxAllyHp, editor.skillSandboxAllyMaxHp)).drawAt(10, allyHpBack.center().movedBy(0.0, -13.0), Palette::White);

		const double hpRate = editor.skillSandboxTargetMaxHp > 0 ? Clamp(static_cast<double>(editor.skillSandboxTargetHp) / editor.skillSandboxTargetMaxHp, 0.0, 1.0) : 0.0;
		const RectF hpBack{ Arg::center = editor.skillSandboxTargetPos + Vec2{ 0.0, -46.0 }, 88.0, 8.0 };
		hpBack.draw(ColorF{ 0.03, 0.03, 0.03, 0.85 });
		RectF{ hpBack.pos, hpBack.w * hpRate, hpBack.h }.draw(ColorF{ 0.25, 0.95, 0.25 });
		uiFont(U"HP {}/{}"_fmt(editor.skillSandboxTargetHp, editor.skillSandboxTargetMaxHp)).drawAt(10, hpBack.center().movedBy(0.0, -14.0), Palette::White);

		if (!hasSkill && !isUnitMode)
		{
			uiFont(U"スキルを選択してください").drawAt(14, arena.center(), ColorF{ 1, 1, 1, 0.70 });
			return;
		}

		for (const auto& projectile : editor.skillSandboxProjectiles)
		{
			// 弾ごとのskillIdでSkillDefを解決 (Unit Mode) / fallback to selected skill
			const SkillDef* projSkill = nullptr;
			if (projectile.skillId != InvalidSkillDefId && projectile.skillId < static_cast<SkillDefId>(defs.skills.size()))
			{
				projSkill = &defs.skills[projectile.skillId];
			}
			else if (hasSkill)
			{
				projSkill = &defs.skills[editor.selectedSkillIndex];
			}
			if (!projSkill) continue;
			const SkillDef& skill = *projSkill;
			const Vec2 drawPos = projectile.position + Vec2{ 0.0, -projectile.height };
			const Optional<Vec2> rayTailPos = skill.rayLockToCaster ? Optional<Vec2>{ editor.skillSandboxCasterPos } : none;
			DrawSkillRay(sandboxAssets, skill, drawPos, projectile.angleRad, false, rayTailPos);
			if (const Optional<Vec2> tipScreen = ResolveSwingEndProjectileTipScreenInPlane(skill, projectile.position, projectile.angleRad))
			{
				if (DrawSwingEndProjectileTexture(sandboxAssets, skill, drawPos, *tipScreen))
				{
					continue;
				}
			}
			if (!DrawProjectileTexture(sandboxAssets, skill, drawPos, projectile.angleRad, false))
			{
				if (projectile.motion == SkillProjectileMotion::Parabola && projectile.height > 1.0)
				{
					Circle{ projectile.position, 3.5 }.draw(ColorF{ 0, 0, 0, 0.25 });
					Line{ projectile.position, drawPos }.draw(1.0, ColorF{ skill.color, 0.28 });
				}
				if (projectile.motion == SkillProjectileMotion::Orbit)
				{
					Circle{ drawPos, 6 }.drawFrame(2.0, skill.color);
				}
				else
				{
					Circle{ drawPos, 4 }.draw(skill.color);
				}
			}
		}

		uiFont(U"cool {:.2f}s  shots {}  {}"_fmt(
			editor.skillSandboxCooldownLeftSec,
			editor.skillSandboxProjectiles.size(),
			sandboxSkill ? (sandboxSkill->burstFireMode == SkillBurstFireMode::Staggered ? U"stagger" : U"simul") : U""))
			.draw(11, arena.x + 12.0, arena.y + 10.0, ColorF{ 1, 1, 1, 0.62 });
		if (sandboxSkill)
		{
			const SkillDef& skill = *sandboxSkill;
			uiFont(U"order {}"_fmt(skill.burstOrderMode == SkillBurstOrderMode::Random ? U"random" : U"seq"))
				.draw(11, arena.x + 12.0, arena.y + 26.0, ColorF{ 1, 1, 1, 0.52 });
			uiFont(U"range {:.1f} / min {:.1f}"_fmt(skill.range, skill.rangeMin))
				.draw(11, arena.x + 12.0, arena.y + 42.0, ColorF{ 1, 1, 1, 0.56 });
			uiFont(U"bom {}  r {:.1f}  ff {}  self x{:.2f}"_fmt(
				skill.bom ? U"on" : U"off",
				skill.bomRadius,
				skill.bomFriendlyFire ? U"on" : U"off",
				skill.bomSelfDamageScale))
				.draw(11, arena.x + 12.0, arena.y + 58.0, ColorF{ 1, 1, 1, 0.56 });
		}
		uiFont(U"Allies / Sandbag / Dummy はドラッグで移動できます").draw(11, arena.x + 12.0, arena.y + arena.h - 24.0, ColorF{ 1, 1, 1, 0.56 });
	}
}
