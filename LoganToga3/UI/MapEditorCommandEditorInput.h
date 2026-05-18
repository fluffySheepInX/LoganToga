#pragma once
# include <Siv3D.hpp>
# include "MapEditorMapData.h"
# include "BuildingEditorCommon.h"
# include "../Data/Loaders/BuildActionDefLoader.h"

namespace LT3
{
		inline bool NormalizeCommandIdsContainsOwnerTag(const Array<String>& tags, StringView target)
		{
			for (const auto& tag : tags)
			{
				if (!tag.isEmpty() && !target.isEmpty() && tag.lowercased() == String{ target }.lowercased())
				{
					return true;
				}
			}

			return false;
		}

		inline Array<String> NormalizeCommandIdsOwnerTags(const Array<String>& source)
		{
			Array<String> normalized;
			for (const auto& tag : source)
			{
				if (tag.isEmpty())
				{
					continue;
				}

				if (!NormalizeCommandIdsContainsOwnerTag(normalized, tag))
				{
					normalized << tag;
				}
			}

			return normalized;
		}

	inline Array<String> NormalizeSpawnTagsForEditor(const Array<String>& source)
	{
		return NormalizeCommandIdsOwnerTags(source);
	}

	inline bool IsFacilityUnitCatalogEntry(const UnitCatalogEntry& entry)
	{
		const String kind = entry.kind.lowercased();
		const String classBuild = entry.classBuild.lowercased();
		return (kind == U"building") || (classBuild == U"home");
	}

	inline bool ActionOwnerIncludesFacilityUnit(const BuildActionDef& action, const UnitCatalog& catalog)
	{
		const Array<String> ownerTags = action.ownerTags.isEmpty()
			? Array<String>{ action.ownerTag }
			: NormalizeCommandIdsOwnerTags(action.ownerTags);

		for (const auto& ownerTag : ownerTags)
		{
			for (const auto& entry : catalog.entries)
			{
				if (EqualsIgnoreCaseOwnerTag(entry.tag, ownerTag) && IsFacilityUnitCatalogEntry(entry))
				{
					return true;
				}
			}
		}

		return false;
	}

	inline void RefreshActionSpawnSelection(BuildActionDef& action, const DefinitionStores& defs)
	{
		action.spawnTags = NormalizeSpawnTagsForEditor(action.spawnTags);
		action.spawnUnits.clear();
		for (const auto& spawnTag : action.spawnTags)
		{
			if (defs.unitByTag.contains(spawnTag))
			{
				action.spawnUnits << defs.unitByTag.at(spawnTag);
			}
		}

		action.spawnTag = action.spawnTags.isEmpty() ? U"" : action.spawnTags.front();
		action.resultTag = action.spawnTag;
		action.spawnUnit = action.spawnUnits.isEmpty() ? InvalidUnitDefId : action.spawnUnits.front();
	}

		inline bool NormalizeCommandIdsForOwnerTags(MapEditorState& editor, DefinitionStores& defs, const Array<String>& targetOwnerTags)
		{
			Array<String> normalizedOwnerTags = NormalizeCommandIdsOwnerTags(targetOwnerTags);
			if (normalizedOwnerTags.isEmpty())
			{
				return false;
			}

			int32 serial = 0;
			int32 updatedCount = 0;
			for (auto& action : defs.buildActions)
			{
				Array<String> actionOwnerTags = NormalizeCommandIdsOwnerTags(action.ownerTags);
				if (actionOwnerTags.isEmpty() && !action.ownerTag.isEmpty())
				{
					actionOwnerTags << action.ownerTag;
				}

				bool ownerMatched = false;
				for (const auto& ownerTag : actionOwnerTags)
				{
					if (NormalizeCommandIdsContainsOwnerTag(normalizedOwnerTags, ownerTag))
					{
						ownerMatched = true;
						break;
					}
				}

				if (!ownerMatched)
				{
					continue;
				}

				action.ownerTags = actionOwnerTags;
				action.ownerTag = actionOwnerTags.isEmpty() ? U"" : actionOwnerTags.front();
				action.id = U"{:03d}"_fmt(serial++);
				action.tag = U"{}:{}"_fmt(action.ownerTag, action.id);
				++updatedCount;
			}

			if (updatedCount <= 0)
			{
				return false;
			}

			editor.commandBindingsDirty = true;
			editor.statusText = U"Normalized command ids: {}"_fmt(updatedCount);
			return true;
		}

	inline bool EqualsOwnerTagIgnoreCase(StringView a, StringView b)
	{
		return !a.isEmpty() && !b.isEmpty() && String{ a }.lowercased() == String{ b }.lowercased();
	}

	inline bool ContainsOwnerTag(const Array<String>& tags, StringView target)
	{
		for (const auto& tag : tags)
		{
			if (EqualsOwnerTagIgnoreCase(tag, target))
			{
				return true;
			}
		}

		return false;
	}

	inline Array<String> NormalizeOwnerTagsForEditor(const Array<String>& source)
	{
		Array<String> normalized;
		for (const auto& tag : source)
		{
			if (tag.isEmpty())
			{
				continue;
			}

			if (!ContainsOwnerTag(normalized, tag))
			{
				normalized << tag;
			}
		}

		return normalized;
	}

	inline bool IsCommandSelectableImageFile(const FilePath& path)
	{
		const String extension = FileSystem::Extension(path).lowercased();
		return extension == U"png" || extension == U"gif";
	}

	inline bool ChangeSelectedCommandImageFromDialog(MapEditorState& editor, DefinitionStores& defs, int32 actionIndex)
	{
		if (actionIndex < 0 || static_cast<int32>(defs.buildActions.size()) <= actionIndex)
		{
			return false;
		}

		const Array<FileFilter> imageFilters = { FileFilter::PNG(), FileFilter::GIF(), FileFilter::AllFiles() };
		const Optional<FilePath> sourcePath = Dialog::OpenFile(imageFilters);
		if (!sourcePath)
		{
			return false;
		}

		if (!IsCommandSelectableImageFile(*sourcePath))
		{
			editor.statusText = U"Command image must be png or gif: {}"_fmt(*sourcePath);
			return true;
		}

		BuildActionDef& action = defs.buildActions[actionIndex];
		const FilePath targetDirectory = FileSystem::ParentPath(ResolveBuildIconPath(U"__lt3_directory_probe__.png"));
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
				editor.statusText = U"Command image copy failed: {}"_fmt(targetPath);
				return true;
			}
		}

		action.icon = fileName;
		BuildingEditorTextureCache().erase(ResolveBuildIconPath(fileName));
		editor.commandBindingsDirty = true;
		editor.statusText = U"Command image changed: {} -> {}"_fmt(action.id, fileName);
		return true;
	}

	inline bool ProcessCommandEditorInput(MapEditorState& editor, UnitCatalog& catalog, DefinitionStores& defs)
	{
		if (!editor.showCommandEditor)
		{
			return false;
		}

		if (editor.commandRenameTargetIndex)
		{
			TextInput::UpdateText(editor.commandRenameEditText);
			editor.commandRenameEditText.remove_if([](char32 c)
			{
				return c == U'\n' || c == U'\r' || c == U'\t'
					|| c == U'"' || c == U'\\' || c < U' ';
			});
			if ((KeyControl | KeyCommand).pressed() && KeyV.down())
			{
				String clip;
				if (Clipboard::GetText(clip) && !clip.isEmpty())
				{
					clip.remove_if([](char32 c)
					{
						return c == U'\n' || c == U'\r' || c == U'\t'
							|| c == U'"' || c == U'\\' || c < U' ';
					});
					editor.commandRenameEditText += clip;
				}
			}

			if (KeyEnter.down())
			{
				const int32 idx = *editor.commandRenameTargetIndex;
				if (0 <= idx && idx < static_cast<int32>(defs.buildActions.size()))
				{
					defs.buildActions[idx].name = editor.commandRenameEditText.isEmpty()
						? defs.buildActions[idx].name
						: editor.commandRenameEditText;
					editor.commandBindingsDirty = true;
					editor.statusText = U"Renamed command: {}"_fmt(defs.buildActions[idx].id);
				}
				editor.commandRenameTargetIndex = none;
				editor.commandRenameEditText = U"";
				editor.commandRenameIsDuplicate = false;
			}
			else if (KeyEscape.down())
			{
				if (editor.commandRenameIsDuplicate)
				{
					const int32 idx = *editor.commandRenameTargetIndex;
					if (0 <= idx && idx < static_cast<int32>(defs.buildActions.size()))
					{
						defs.buildActions.remove_at(idx);
						if (editor.selectedCommandActionIndex >= static_cast<int32>(defs.buildActions.size()))
						{
							editor.selectedCommandActionIndex = static_cast<int32>(defs.buildActions.size()) - 1;
						}
						editor.commandBindingsDirty = true;
					}
				}
				editor.commandRenameTargetIndex = none;
				editor.commandRenameEditText = U"";
				editor.commandRenameIsDuplicate = false;
			}
			return true;
		}

		if (editor.commandContextMenuTargetIndex)
		{
			const RectF menuRect = EditorCommandContextMenuRect(editor.commandContextMenuPos);
			const RectF dupItem = EditorCommandContextMenuItemRect(editor.commandContextMenuPos, 0);
			if (dupItem.leftClicked())
			{
				const int32 srcIdx = *editor.commandContextMenuTargetIndex;
				editor.commandContextMenuTargetIndex = none;
				if (0 <= srcIdx && srcIdx < static_cast<int32>(defs.buildActions.size()))
				{
					BuildActionDef duplicated = defs.buildActions[srcIdx];
					duplicated.id = duplicated.id + U"_copy";
					duplicated.tag = U"{}:{}"_fmt(duplicated.ownerTag, duplicated.id);
					duplicated.name += U" copy";
					defs.buildActions << duplicated;

					const int32 newIdx = static_cast<int32>(defs.buildActions.size()) - 1;
					editor.selectedCommandActionIndex = newIdx;
					const double contentHeight = defs.buildActions.size() * 66.0;
					const double maxScroll = Max(0.0, contentHeight - EditorCommandListViewportRect().h);
					editor.commandListScroll = maxScroll;

					editor.commandRenameTargetIndex = newIdx;
					editor.commandRenameEditText = duplicated.name;
					editor.commandRenameIsDuplicate = true;
					editor.commandBindingsDirty = true;
				}
				return true;
			}

			const RectF renameItem = EditorCommandContextMenuItemRect(editor.commandContextMenuPos, 1);
			if (renameItem.leftClicked())
			{
				const int32 srcIdx = *editor.commandContextMenuTargetIndex;
				editor.commandContextMenuTargetIndex = none;
				if (0 <= srcIdx && srcIdx < static_cast<int32>(defs.buildActions.size()))
				{
					editor.selectedCommandActionIndex = srcIdx;
					editor.commandRenameTargetIndex = srcIdx;
					editor.commandRenameEditText = defs.buildActions[srcIdx].name;
					editor.commandRenameIsDuplicate = false;
				}
				return true;
			}
			const RectF imageItem = EditorCommandContextMenuItemRect(editor.commandContextMenuPos, 2);
			if (imageItem.leftClicked())
			{
				const int32 srcIdx = *editor.commandContextMenuTargetIndex;
				editor.commandContextMenuTargetIndex = none;
				if (0 <= srcIdx && srcIdx < static_cast<int32>(defs.buildActions.size()))
				{
					editor.selectedCommandActionIndex = srcIdx;
					return ChangeSelectedCommandImageFromDialog(editor, defs, srcIdx);
				}
				return true;
			}
			else if (!menuRect.mouseOver() && (MouseL.down() || MouseR.down()))
			{
				editor.commandContextMenuTargetIndex = none;
				return false;
			}
			return true;
		}

		bool consumed = false;
		const RectF panel = EditorCommandPanelRect();
		const RectF commandViewport = EditorCommandListViewportRect();
		const RectF unitViewport = EditorCommandUnitViewportRect();
		const Array<RectF> modeRects = { EditorCommandModeTabRect(0), EditorCommandModeTabRect(1), EditorCommandModeTabRect(2) };
		const RectF normalizeIdsRect = EditorCommandNormalizeIdsRect();
		const RectF saveRect = EditorCommandSaveRect();
		const RectF closeRect = EditorCommandCloseRect();

		const int32 commandCount = static_cast<int32>(defs.buildActions.size());
		if (commandCount > 0)
		{
			editor.selectedCommandActionIndex = Clamp(editor.selectedCommandActionIndex, 0, commandCount - 1);
		}
		else
		{
			editor.selectedCommandActionIndex = -1;
		}

		for (int32 modeIndex = 0; modeIndex < static_cast<int32>(modeRects.size()); ++modeIndex)
		{
			if (modeRects[modeIndex].leftClicked())
			{
				editor.commandEditorMode = modeIndex;
				consumed = true;
			}
		}

		if (normalizeIdsRect.leftClicked())
		{
			if (0 <= editor.selectedCommandActionIndex && editor.selectedCommandActionIndex < commandCount)
			{
				const BuildActionDef& selectedAction = defs.buildActions[editor.selectedCommandActionIndex];
				if (NormalizeCommandIdsForOwnerTags(editor, defs, selectedAction.ownerTags.isEmpty()
					? Array<String>{ selectedAction.ownerTag }
					: selectedAction.ownerTags))
				{
					consumed = true;
				}
			}
			else
			{
				editor.statusText = U"Select a command before normalizing ids";
				consumed = true;
			}
		}

		if (saveRect.leftClicked())
		{
			SaveBuildActionDefinitions(defs, editor.statusText);
			editor.commandBindingsDirty = false;
			consumed = true;
		}
		if (closeRect.leftClicked())
		{
			editor.showCommandEditor = false;
			consumed = true;
		}

		if (commandViewport.mouseOver())
		{
			const double contentHeight = Max(0.0, commandCount * 66.0);
			const double maxScroll = Max(0.0, contentHeight - commandViewport.h);
			editor.commandListScroll = Clamp(editor.commandListScroll - Mouse::Wheel() * 42.0, 0.0, maxScroll);
			consumed = true;
		}

		const double commandViewportBottom = commandViewport.y + commandViewport.h;
		for (int32 i = 0; i < commandCount; ++i)
		{
			const RectF row = EditorCommandRowRect(commandViewport, i, editor.commandListScroll);
			if (!((commandViewport.y <= row.y) && ((row.y + row.h) <= commandViewportBottom)))
			{
				continue;
			}

			if (row.leftClicked())
			{
				editor.selectedCommandActionIndex = i;
				consumed = true;
				break;
			}
			if (row.rightClicked())
			{
				editor.commandContextMenuTargetIndex = i;
				editor.commandContextMenuPos = Cursor::PosF();
				consumed = true;
				break;
			}
		}

		if (editor.selectedCommandActionIndex >= 0 && editor.selectedCommandActionIndex < commandCount)
		{
			constexpr int32 columns = 5;
			const int32 unitCount = static_cast<int32>(catalog.entries.size());
			const int32 unitRows = (unitCount + columns - 1) / columns;
			const double unitContentHeight = unitRows * 96.0 + 8.0;
			const double unitMaxScroll = Max(0.0, unitContentHeight - unitViewport.h);
			if (editor.commandEditorMode != 2 && unitViewport.mouseOver())
			{
				editor.commandUnitListScroll = Clamp(editor.commandUnitListScroll - Mouse::Wheel() * 42.0, 0.0, unitMaxScroll);
				consumed = true;
			}

			BuildActionDef& action = defs.buildActions[editor.selectedCommandActionIndex];
			const double unitViewportBottom = unitViewport.y + unitViewport.h;
			if (editor.commandEditorMode != 2)
			{
				for (int32 i = 0; i < unitCount; ++i)
				{
					const UnitCatalogEntry& entry = catalog.entries[i];
					const RectF cell = EditorCommandUnitCellRect(unitViewport, i, columns, editor.commandUnitListScroll);
					if ((cell.y + cell.h) < unitViewport.y || unitViewportBottom < cell.y)
					{
						continue;
					}

					if (cell.leftClicked())
					{
						if (editor.commandEditorMode == 0)
						{
							Array<String>& ownerTags = action.ownerTags;
							const bool hasTag = ContainsOwnerTag(ownerTags, entry.tag);

							if (hasTag)
							{
								Array<String> filtered;
								for (const auto& value : ownerTags)
								{
									if (!EqualsOwnerTagIgnoreCase(value, entry.tag))
									{
										filtered << value;
									}
								}
								ownerTags = std::move(filtered);
							}
							else
							{
								ownerTags << entry.tag;
							}

							ownerTags = NormalizeOwnerTagsForEditor(ownerTags);
							action.ownerTag = ownerTags.isEmpty() ? U"" : ownerTags.front();
							action.tag = U"{}:{}"_fmt(action.ownerTag, action.id);
							editor.statusText = U"Command binding updated: {}"_fmt(action.name);
						}
						else if (editor.commandEditorMode == 1)
						{
							action.resultType = BuildActionResultType::Unit;
							Array<String> spawnTags = action.spawnTags;
							if (spawnTags.isEmpty() && !action.spawnTag.isEmpty())
							{
								spawnTags << action.spawnTag;
							}

							const bool allowMultipleSpawns = ActionOwnerIncludesFacilityUnit(action, catalog);
							const bool alreadySelected = ContainsOwnerTag(spawnTags, entry.tag);
							if (allowMultipleSpawns)
							{
								if (alreadySelected)
								{
									Array<String> filtered;
									for (const auto& value : spawnTags)
									{
										if (!EqualsOwnerTagIgnoreCase(value, entry.tag))
										{
											filtered << value;
										}
									}
									spawnTags = std::move(filtered);
								}
								else
								{
									spawnTags << entry.tag;
								}
							}
							else
							{
								spawnTags = alreadySelected ? spawnTags : Array<String>{ entry.tag };
							}

							action.spawnTags = std::move(spawnTags);
							RefreshActionSpawnSelection(action, defs);
							editor.statusText = U"Command spawn updated: {} -> {}"_fmt(
								action.name,
								action.spawnTags.isEmpty() ? U"(none)" : action.spawnTags.join(U", "));
						}

						editor.commandBindingsDirty = true;
						consumed = true;
						break;
					}
				}
			}
		}

		if (panel.mouseOver())
		{
			consumed = true;
		}

		return consumed;
	}
}
