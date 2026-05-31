#pragma once
# include "SkillEditorInput.Common.h"
# include "BuildingEditorCommon.h"
# include "QuarterView.h"
# include "../BattleWorld/BattleWorld.h"

namespace LT3
{
	inline bool TryFindSkillSoundPreviewCandidateHint(const BattleWorld& world, const DefinitionStores& defs, SkillDefId skillId, String& hintText)
	{
		if (skillId < 0 || skillId >= static_cast<SkillDefId>(defs.skills.size()))
		{
			return false;
		}

		const double zoom = GetQuarterViewCameraScale();
		if (zoom < BattleSkillSoundZoomThreshold)
		{
			hintText = U"ズーム不足のため直接試聴";
			return false;
		}

		const Vec2 cursor = Cursor::PosF();
		const Vec2 center{ Scene::Width() * 0.5, Scene::Height() * 0.5 };
		Array<std::pair<UnitId, String>> nearCursor;
		Array<std::pair<UnitId, String>> nearCenter;
		nearCursor.reserve(world.units.size());
		nearCenter.reserve(world.units.size());

		for (UnitId unit = 0; unit < static_cast<UnitId>(world.units.size()); ++unit)
		{
			if (unit >= static_cast<UnitId>(world.units.alive.size()) || !world.units.alive[unit])
			{
				continue;
			}
			if (unit >= static_cast<UnitId>(world.units.defId.size()))
			{
				continue;
			}

			const UnitDefId defId = world.units.defId[unit];
			if (defId < 0 || defId >= static_cast<UnitDefId>(defs.units.size()))
			{
				continue;
			}

			const UnitDef& unitDef = defs.units[defId];
			if (!UnitDefHasSkill(unitDef, defs, skillId))
			{
				continue;
			}

			const Vec2 screenPos = ToQuarterViewportScreen(world.units.position[unit]);
			if (!RectF{ -96.0, -96.0, Scene::Width() + 192.0, Scene::Height() + 192.0 }.intersects(screenPos))
			{
				continue;
			}

			const String name = unitDef.name.isEmpty() ? unitDef.unit_id : unitDef.name;
			if (screenPos.distanceFrom(cursor) <= BattleSkillSoundCursorRadius)
			{
				nearCursor << std::pair<UnitId, String>{ unit, name };
			}
			if (screenPos.distanceFrom(center) <= BattleSkillSoundCenterRadius)
			{
				nearCenter << std::pair<UnitId, String>{ unit, name };
			}
		}

		if (!nearCursor.isEmpty())
		{
			const auto& picked = nearCursor.choice();
			hintText = U"カーソル近傍: {}"_fmt(picked.second);
			return true;
		}

		if (!nearCenter.isEmpty())
		{
			const auto& picked = nearCenter.choice();
			hintText = U"画面中央近傍: {}"_fmt(picked.second);
			return true;
		}

		hintText = U"近傍候補なしのため直接試聴";
		return false;
	}

	inline FilePath ResolveSkillSoundEffectDirectory()
	{
		const FilePath existingPath = ResolveSkillSoundEffectPath(U"__lt3_directory_probe__.ogg");
		return FileSystem::ParentPath(existingPath);
	}

	inline bool IsSkillEditorSelectableAudioFile(const FilePath& path)
	{
		const String extension = FileSystem::Extension(path).lowercased();
		return extension == U"wav" || extension == U"mp3" || extension == U"ogg" || extension == U"flac" || extension == U"aac" || extension == U"m4a";
	}

	inline bool PlaySkillEditorSoundEffectPreview(MapEditorState& editor, const BattleWorld& world, const DefinitionStores& defs, SkillDefId skillId, const SkillDef& skill)
	{
		if (skill.soundEffect.isEmpty())
		{
			editor.statusText = U"Skill SE is not set";
			editor.skillSoundPreviewUnitHint = U"";
			return true;
		}

		const FilePath soundPath = ResolveSkillSoundEffectPath(skill.soundEffect);
		if (soundPath.isEmpty() || !FileSystem::Exists(soundPath))
		{
			editor.statusText = U"Skill SE not found: {}"_fmt(skill.soundEffect);
			editor.skillSoundPreviewUnitHint = U"";
			return true;
		}

		static HashTable<FilePath, Audio> s_audioCache;
		auto cacheIt = s_audioCache.find(soundPath);
		if (cacheIt == s_audioCache.end())
		{
			cacheIt = s_audioCache.emplace(soundPath, Audio{ soundPath }).first;
		}

		if (!cacheIt->second)
		{
			editor.statusText = U"Skill SE load failed: {}"_fmt(skill.soundEffect);
			editor.skillSoundPreviewUnitHint = U"";
			return true;
		}

		cacheIt->second.playOneShot(Clamp(skill.soundEffectVolume, 0.0, 1.0));
		String hintText;
		const bool usedNearbyCandidate = TryFindSkillSoundPreviewCandidateHint(world, defs, skillId, hintText);
		editor.statusText = usedNearbyCandidate
			? U"Skill SE preview: {} ({})"_fmt(skill.soundEffect, hintText)
			: U"Skill SE preview: {}"_fmt(skill.soundEffect);
		editor.skillSoundPreviewUnitHint = hintText;
		return true;
	}

	/// <summary>
	/// スキルアイコンおよび弾画像入力を処理します。
	/// </summary>
	inline bool ProcessSkillEditorAssetInput(MapEditorState& editor, const BattleWorld& world, DefinitionStores& defs, double scroll)
	{
		if (HandleRectButtonClick(SkillEditorIconBrowseRect(scroll)))
		{
			const Array<FileFilter> imageFilters = { FileFilter::PNG(), FileFilter::JPEG(), FileFilter::BMP(), FileFilter::GIF(), FileFilter::AllFiles() };
			const Array<FilePath> sourcePaths = Dialog::OpenFiles(imageFilters);
			if (!sourcePaths.isEmpty())
			{
				const FilePath targetDirectory = FileSystem::ParentPath(ResolveBuildIconPath(U"__lt3_directory_probe__.png"));
				FileSystem::CreateDirectories(targetDirectory);
				Array<String> iconLayers;
				iconLayers.reserve(sourcePaths.size());

				for (const auto& sourcePath : sourcePaths)
				{
					const String extension = FileSystem::Extension(sourcePath).lowercased();
					if (!(extension == U"png" || extension == U"jpg" || extension == U"jpeg" || extension == U"bmp" || extension == U"gif"))
					{
						editor.statusText = U"Skill icon must be image file: {}"_fmt(sourcePath);
						return true;
					}

					const String fileName = FileSystem::FileName(sourcePath);
					const FilePath targetPath = targetDirectory + fileName;
					if (FileSystem::FullPath(sourcePath) != FileSystem::FullPath(targetPath))
					{
						if (FileSystem::Exists(targetPath))
						{
							FileSystem::Remove(targetPath);
						}

						if (!FileSystem::Copy(sourcePath, targetPath))
						{
							editor.statusText = U"Skill icon copy failed: {}"_fmt(targetPath);
							return true;
						}
					}

					iconLayers << fileName;
					BuildingEditorTextureCache().erase(ResolveBuildIconPath(fileName));
				}

				const Array<String> normalizedIconLayers = NormalizeSkillIconLayerOrder(iconLayers);
				String skillTag;
				Array<String> warnings;
				ApplyEditorMutation([&]()
				{
					return MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
					{
						skillTag = selected.tag;
						selected.iconLayers = normalizedIconLayers;
						selected.icon = selected.iconLayers.isEmpty() ? U"" : selected.iconLayers.front();
						warnings = ValidateSkillIconLayers(selected);
						return true;
					});
				}, [&]()
				{
					if (warnings.isEmpty())
					{
						defs.skillIconWarningsByTag.erase(skillTag);
					}
					else
					{
						defs.skillIconWarningsByTag[skillTag] = warnings;
						editor.statusText = U"Skill icon warning: {}"_fmt(warnings.front());
					}
				});
			}
			return true;
		}

		if (HandleRectButtonClick(SkillEditorSoundEffectClearRect(scroll)))
		{
			MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
			{
				const String empty;
				return SetFieldIfChanged(selected.soundEffect, empty);
			});
			editor.skillSoundPreviewUnitHint = U"";
			editor.statusText = U"Skill SE cleared";
			return true;
		}

		if (HandleRectButtonClick(SkillEditorSoundEffectBrowseRect(scroll)))
		{
			const Array<FileFilter> audioFilters = { FileFilter::AllAudioFiles(), FileFilter::AllFiles() };
			const Optional<FilePath> sourcePath = Dialog::OpenFile(audioFilters);
			if (sourcePath)
			{
				if (!IsSkillEditorSelectableAudioFile(*sourcePath))
				{
					editor.statusText = U"Skill SE must be audio file: {}"_fmt(*sourcePath);
					return true;
				}

				const FilePath targetDirectory = ResolveSkillSoundEffectDirectory();
				FileSystem::CreateDirectories(targetDirectory);
				const String fileName = FileSystem::FileName(*sourcePath);
				const FilePath targetPath = targetDirectory + fileName;
				if (FileSystem::FullPath(*sourcePath) != FileSystem::FullPath(targetPath))
				{
					if (FileSystem::Exists(targetPath))
					{
						FileSystem::Remove(targetPath);
					}

					if (!FileSystem::Copy(*sourcePath, targetPath))
					{
						editor.statusText = U"Skill SE copy failed: {}"_fmt(targetPath);
						return true;
					}
				}

				MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
				{
					return SetFieldIfChanged(selected.soundEffect, fileName);
				});
				editor.skillSoundPreviewUnitHint = U"";
				editor.statusText = U"Skill SE changed: {}"_fmt(fileName);
			}
			return true;
		}

		if (HandleRectButtonClick(SkillEditorSoundEffectPlayRect(scroll)))
		{
			return PlaySkillEditorSoundEffectPreview(editor, world, defs, editor.selectedSkillIndex, defs.skills[editor.selectedSkillIndex]);
		}

		switch (DetectRectNumberStepperInput(SkillEditorSoundEffectVolumeStepperRects(scroll)))
		{
		case RectNumberStepperInputAction::Decrement:
			MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
			{
				const double next = Clamp(selected.soundEffectVolume - 0.05, 0.0, 1.0);
				return SetFieldIfChanged(selected.soundEffectVolume, next);
			});
			editor.statusText = U"Skill SE volume: {:.2f}"_fmt(defs.skills[editor.selectedSkillIndex].soundEffectVolume);
			return true;
		case RectNumberStepperInputAction::Increment:
			MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
			{
				const double next = Clamp(selected.soundEffectVolume + 0.05, 0.0, 1.0);
				return SetFieldIfChanged(selected.soundEffectVolume, next);
			});
			editor.statusText = U"Skill SE volume: {:.2f}"_fmt(defs.skills[editor.selectedSkillIndex].soundEffectVolume);
			return true;
		default:
			break;
		}

		for (int32 imageIndex = 0; imageIndex < 2; ++imageIndex)
		{
			if (HandleRectButtonClick(SkillEditorProjectileImageClearRect(imageIndex, scroll)))
			{
				MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
				{
					const String empty;
					if (imageIndex == 0)
					{
						return SetFieldIfChanged(selected.projectileImage, empty);
					}

					return SetFieldIfChanged(selected.projectileDiagonalImage, empty);
				});
				editor.statusText = (imageIndex == 0) ? U"Skill projectile image cleared" : U"Skill projectile diagonal image cleared";
				return true;
			}

			if (HandleRectButtonClick(SkillEditorProjectileImageBrowseRect(imageIndex, scroll)))
			{
				const Array<FileFilter> imageFilters = { FileFilter::PNG(), FileFilter::JPEG(), FileFilter::BMP(), FileFilter::GIF(), FileFilter::AllFiles() };
				const Optional<FilePath> sourcePath = Dialog::OpenFile(imageFilters);
				if (sourcePath)
				{
					String fileName;
					if (!CopySkillEditorImageToBuildIcons(*sourcePath, fileName, editor.statusText))
					{
						return true;
					}

					MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
					{
						if (imageIndex == 0)
						{
							return SetFieldIfChanged(selected.projectileImage, fileName);
						}

						return SetFieldIfChanged(selected.projectileDiagonalImage, fileName);
					});
				}
				return true;
			}
		}

		if (HandleRectButtonClick(SkillEditorBomImageClearRect(scroll)))
		{
			MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
			{
				const String empty;
				return SetFieldIfChanged(selected.bomImage, empty);
			});
			editor.statusText = U"Skill bom image cleared";
			return true;
		}

		if (HandleRectButtonClick(SkillEditorBomImageBrowseRect(scroll)))
		{
			const Array<FileFilter> imageFilters = { FileFilter::PNG(), FileFilter::JPEG(), FileFilter::BMP(), FileFilter::GIF(), FileFilter::AllFiles() };
			const Optional<FilePath> sourcePath = Dialog::OpenFile(imageFilters);
			if (sourcePath)
			{
				String fileName;
				if (!CopySkillEditorImageToBuildIcons(*sourcePath, fileName, editor.statusText))
				{
					return true;
				}

				MutateSelectedSkillDefinition(editor, defs, [&](SkillDef& selected)
				{
					return SetFieldIfChanged(selected.bomImage, fileName);
				});
			}
			return true;
		}

		return false;
	}
}
