#pragma once
# include <Siv3D.hpp>
# include "MapEditorDrawBaseUnitEditorHelpers.h"

namespace LT3
{
	inline bool CommandEditorOwnerTagEquals(StringView a, StringView b)
	{
		return !a.isEmpty() && !b.isEmpty() && String{ a }.lowercased() == String{ b }.lowercased();
	}

	inline Array<String> CommandEditorSpawnTags(const BuildActionDef& action)
	{
		Array<String> spawnTags = action.spawnTags;
		if (spawnTags.isEmpty() && !action.spawnTag.isEmpty())
		{
			spawnTags << action.spawnTag;
		}

		Array<String> normalized;
		for (const auto& spawnTag : spawnTags)
		{
			if (spawnTag.isEmpty())
			{
				continue;
			}

			bool exists = false;
			for (const auto& existing : normalized)
			{
				if (CommandEditorOwnerTagEquals(existing, spawnTag))
				{
					exists = true;
					break;
				}
			}

			if (!exists)
			{
				normalized << spawnTag;
			}
		}

		return normalized;
	}

	inline bool IsCommandEditorFacilityUnit(const UnitCatalogEntry& entry)
	{
		const String kind = entry.kind.lowercased();
		const String classBuild = entry.classBuild.lowercased();
		return (kind == U"building") || (classBuild == U"home");
	}

	inline bool CommandEditorActionOwnedByEntry(const BuildActionDef& action, const UnitCatalogEntry& entry)
	{
		if (CommandEditorOwnerTagEquals(action.ownerTag, entry.tag))
		{
			return true;
		}

		for (const auto& ownerTag : action.ownerTags)
		{
			if (CommandEditorOwnerTagEquals(ownerTag, entry.tag))
			{
				return true;
			}
		}

		return false;
	}

	inline bool CommandEditorEntrySpawnsOtherUnits(const UnitCatalogEntry& entry, const DefinitionStores& defs)
	{
		for (const auto& action : defs.buildActions)
		{
			if (!CommandEditorActionOwnedByEntry(action, entry))
			{
				continue;
			}

			if (action.resultType != BuildActionResultType::Unit)
			{
				continue;
			}

			if (!CommandEditorSpawnTags(action).isEmpty())
			{
				return true;
			}
		}

		return false;
	}

	inline void DrawCommandEditor(MapEditorState& editor, const UnitCatalog& catalog, const DefinitionStores& defs, const Font& uiFont)
	{
		if (!editor.showCommandEditor)
		{
			return;
		}

		const RectF panel = EditorCommandPanelRect();
		const RectF commandViewport = EditorCommandListViewportRect();
		const RectF unitViewport = EditorCommandUnitViewportRect();
		panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"Command Editor").draw(18, panel.x + 20.0, panel.y + 18.0, Palette::White);
		uiFont(U"左:Command  右:実行可能Unit").draw(12, panel.x + 220.0, panel.y + 22.0, Palette::Lightgray);

		const int32 commandCount = static_cast<int32>(defs.buildActions.size());
		if (commandCount <= 0)
		{
			uiFont(U"No commands").draw(14, panel.x + 20.0, panel.y + 52.0, Palette::Orange);
			return;
		}

		editor.selectedCommandActionIndex = Clamp(editor.selectedCommandActionIndex, 0, commandCount - 1);

		const double commandContentHeight = commandCount * 66.0;
		const double commandMaxScroll = Max(0.0, commandContentHeight - commandViewport.h);
		editor.commandListScroll = Clamp(editor.commandListScroll, 0.0, commandMaxScroll);

		commandViewport.draw(ColorF{ 0, 0, 0, 0.10 });
		const double commandViewportBottom = commandViewport.y + commandViewport.h;
		for (int32 i = 0; i < commandCount; ++i)
		{
			const BuildActionDef& action = defs.buildActions[i];
			const RectF row = EditorCommandRowRect(commandViewport, i, editor.commandListScroll);
			if (!((commandViewport.y <= row.y) && ((row.y + row.h) <= commandViewportBottom)))
			{
				continue;
			}

			const bool selected = (editor.selectedCommandActionIndex == i);
			row.draw(selected ? ColorF{ 0.16, 0.18, 0.13, 0.95 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
				.drawFrame(2, row.mouseOver() ? ColorF{ 0.0, 0.75, 1.0 } : (selected ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.14 }));

			const RectF iconRect{ row.x + 8.0, row.y + 9.0, 40.0, 40.0 };
			iconRect.draw(ColorF{ 0.05, 0.06, 0.08, 0.96 }).drawFrame(1, ColorF{ 1, 1, 1, 0.10 });
			const FilePath iconPath = ResolveBuildIconPath(action.icon);
			if (FileSystem::Exists(iconPath))
			{
				auto& cache = BuildingEditorTextureCache();
				if (!cache.contains(iconPath))
				{
					cache.emplace(iconPath, Texture{ iconPath });
				}
				const Texture& iconTexture = cache.at(iconPath);
				const double fitScale = Min((iconRect.w - 4.0) / Max(1.0, static_cast<double>(iconTexture.width())), (iconRect.h - 4.0) / Max(1.0, static_cast<double>(iconTexture.height())));
				iconTexture.scaled(Min(1.0, fitScale)).drawAt(iconRect.center());
			}

			uiFont(action.name).draw(12, row.x + 56.0, row.y + 7.0, Palette::White);
			uiFont(U"{}:{}"_fmt(action.ownerTag, action.id)).draw(10, row.x + 56.0, row.y + 29.0, Palette::Lightgray);

			if (editor.commandRenameTargetIndex == i)
			{
				const RectF renameRect = EditorCommandRenameOverlayRect(row);
				renameRect.draw(ColorF{ 0.06, 0.08, 0.12, 0.98 }).drawFrame(2, ColorF{ 1.0, 0.84, 0.0 });
				uiFont(editor.commandRenameEditText + U"|").draw(13, renameRect.x + 8.0, renameRect.y + 6.0, Palette::White);
				uiFont(U"Enter:確定  Esc:キャンセル").draw(10, renameRect.x + renameRect.w + 6.0, renameRect.y + 9.0, ColorF{ 1, 1, 1, 0.55 });
			}
		}

		if (editor.commandContextMenuTargetIndex)
		{
			const RectF menuRect = EditorCommandContextMenuRect(editor.commandContextMenuPos);
			menuRect.draw(ColorF{ 0.06, 0.08, 0.14, 0.97 }).drawFrame(1, ColorF{ 1, 1, 1, 0.30 });
			const RectF dupItem = EditorCommandContextMenuItemRect(editor.commandContextMenuPos, 0);
			dupItem.draw(dupItem.mouseOver() ? ColorF{ 0.16, 0.22, 0.18, 0.96 } : ColorF{ 0.0, 0.0, 0.0, 0.0 });
			uiFont(U"Duplicate").draw(13, dupItem.x + 8.0, dupItem.y + 5.0, Palette::White);
			const RectF renameItem = EditorCommandContextMenuItemRect(editor.commandContextMenuPos, 1);
			renameItem.draw(renameItem.mouseOver() ? ColorF{ 0.16, 0.22, 0.18, 0.96 } : ColorF{ 0.0, 0.0, 0.0, 0.0 });
			uiFont(U"名前編集").draw(13, renameItem.x + 8.0, renameItem.y + 5.0, Palette::White);
			const RectF imageItem = EditorCommandContextMenuItemRect(editor.commandContextMenuPos, 2);
			imageItem.draw(imageItem.mouseOver() ? ColorF{ 0.16, 0.22, 0.18, 0.96 } : ColorF{ 0.0, 0.0, 0.0, 0.0 });
			uiFont(U"イメージ変更").draw(13, imageItem.x + 8.0, imageItem.y + 5.0, Palette::White);
		}

		const BuildActionDef& selectedAction = defs.buildActions[editor.selectedCommandActionIndex];
		const Array<String> selectedSpawnTags = CommandEditorSpawnTags(selectedAction);
		const bool missingSpawnForUnitResult = (selectedAction.resultType == BuildActionResultType::Unit) && selectedSpawnTags.isEmpty();
		const bool allowMultipleSpawns = [&]() -> bool
		{
			for (const auto& entry : catalog.entries)
			{
				if (CommandEditorActionOwnedByEntry(selectedAction, entry) && IsCommandEditorFacilityUnit(entry))
				{
					return true;
				}
			}
			return false;
		}();
		unitViewport.draw(ColorF{ 0, 0, 0, 0.10 });

		const Array<String> modeLabels = { U"Bind", U"Spawn", U"Inspect" };
		for (int32 modeIndex = 0; modeIndex < static_cast<int32>(modeLabels.size()); ++modeIndex)
		{
			const RectF modeRect = EditorCommandModeTabRect(modeIndex);
			const bool active = (editor.commandEditorMode == modeIndex);
			modeRect.draw(active ? ColorF{ 0.16, 0.18, 0.13, 0.95 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
				.drawFrame(2, modeRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : (active ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.14 }));
			uiFont(modeLabels[modeIndex]).drawAt(12, modeRect.center(), active ? Palette::White : Palette::Lightgray);
		}

		constexpr int32 columns = 5;
		const int32 unitCount = static_cast<int32>(catalog.entries.size());
		const int32 unitRows = (unitCount + columns - 1) / columns;
		const double unitContentHeight = unitRows * 96.0 + 8.0;
		const double unitMaxScroll = Max(0.0, unitContentHeight - unitViewport.h);
		editor.commandUnitListScroll = Clamp(editor.commandUnitListScroll, 0.0, unitMaxScroll);

		if (editor.commandEditorMode == 2)
		{
			uiFont(U"Owner").draw(12, unitViewport.x + 12.0, unitViewport.y + 12.0, Palette::Lightgray);
			uiFont(selectedAction.ownerTag).draw(12, unitViewport.x + 120.0, unitViewport.y + 12.0, Palette::White);
			uiFont(U"ID").draw(12, unitViewport.x + 12.0, unitViewport.y + 40.0, Palette::Lightgray);
			uiFont(selectedAction.id).draw(12, unitViewport.x + 120.0, unitViewport.y + 40.0, Palette::White);
			uiFont(U"Result Type").draw(12, unitViewport.x + 12.0, unitViewport.y + 68.0, Palette::Lightgray);
			uiFont(BuildActionResultTypeToTomlValue(selectedAction.resultType)).draw(12, unitViewport.x + 120.0, unitViewport.y + 68.0, Palette::White);
			uiFont(U"Spawn").draw(12, unitViewport.x + 12.0, unitViewport.y + 96.0, Palette::Lightgray);
			uiFont(selectedSpawnTags.isEmpty() ? U"(none)" : selectedSpawnTags.join(U", ")).draw(12, unitViewport.x + 120.0, unitViewport.y + 96.0, Palette::White);
			if (missingSpawnForUnitResult)
			{
				uiFont(U"WARN: Unit result requires spawn target").draw(12, unitViewport.x + 12.0, unitViewport.y + 124.0, Palette::Orange);
			}
			uiFont(U"Mode: Inspect は閲覧専用").draw(12, unitViewport.x + 12.0, unitViewport.y + 140.0, Palette::Aqua);
		}
		else
		{

			const double unitViewportBottom = unitViewport.y + unitViewport.h;
			for (int32 i = 0; i < unitCount; ++i)
			{
				const UnitCatalogEntry& entry = catalog.entries[i];
				const RectF cell = EditorCommandUnitCellRect(unitViewport, i, columns, editor.commandUnitListScroll);
				if ((cell.y + cell.h) < unitViewport.y || unitViewportBottom < cell.y)
				{
					continue;
				}

				const bool selectedByMode = (editor.commandEditorMode == 0)
					? IsCatalogEntryBoundToAction(entry, selectedAction)
					: ((selectedAction.resultType == BuildActionResultType::Unit)
						&& selectedSpawnTags.any([&](const String& spawnTag)
						{
							return CommandEditorOwnerTagEquals(entry.tag, spawnTag);
						}));
				cell.draw(selectedByMode ? ColorF{ 0.15, 0.21, 0.15, 0.96 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
					.drawFrame(2, cell.mouseOver() ? ColorF{ 0.0, 0.75, 1.0 } : (selectedByMode ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.14 }));

				const RectF iconRect{ cell.x + 18.0, cell.y + 10.0, 52.0, 52.0 };
				iconRect.draw(ColorF{ 0.05, 0.06, 0.08, 0.96 }).drawFrame(1, ColorF{ 1, 1, 1, 0.08 });
				const FilePath imagePath = ResolveCatalogVisualPath(entry.kind, entry.image);
				if (!imagePath.isEmpty() && FileSystem::Exists(imagePath))
				{
					auto& cache = BuildingEditorTextureCache();
					if (!cache.contains(imagePath))
					{
						cache.emplace(imagePath, Texture{ imagePath });
					}
					const Texture& texture = cache.at(imagePath);
					const double fitScale = Min((iconRect.w - 4.0) / Max(1.0, static_cast<double>(texture.width())), (iconRect.h - 4.0) / Max(1.0, static_cast<double>(texture.height())));
					texture.scaled(Min(1.0, fitScale)).drawAt(iconRect.center());
				}

				uiFont(entry.name).draw(10, cell.x + 8.0, cell.y + 66.0, Palette::White);

				if (editor.commandEditorMode == 1 && !CommandEditorEntrySpawnsOtherUnits(entry, defs))
				{
					const RectF markRect{ cell.x + cell.w - 24.0, cell.y + 6.0, 18.0, 18.0 };
					markRect.draw(ColorF{ 0.40, 0.08, 0.08, 0.94 }).drawFrame(1, ColorF{ 1.0, 0.82, 0.82, 0.75 });
					uiFont(U"×").drawAt(11, markRect.center(), Palette::White);
				}
			}

			if (editor.commandEditorMode == 1)
			{
				uiFont(U"Spawn target: {}"_fmt(selectedSpawnTags.isEmpty() ? U"(none)" : selectedSpawnTags.join(U", "))).draw(12, unitViewport.x + 12.0, unitViewport.y + unitViewport.h - 40.0, Palette::Aqua);
				if (missingSpawnForUnitResult)
				{
					uiFont(U"WARN: spawn未設定のため保存・表示不可").draw(11, unitViewport.x + 12.0, unitViewport.y + unitViewport.h - 60.0, Palette::Orange);
				}
				uiFont(allowMultipleSpawns ? U"施設owner: 複数選択可 / ×: 他ユニットをspawnしない" : U"通常owner: 単一選択 / ×: 他ユニットをspawnしない").draw(11, unitViewport.x + 12.0, unitViewport.y + unitViewport.h - 20.0, Palette::Lightgray);
			}
		}

		if (editor.commandBindingsDirty)
		{
			uiFont(U"* bindings modified").draw(12, panel.x + 20.0, panel.y + panel.h - 46.0, Palette::Orange);
		}

		const RectF normalizeIdsRect = EditorCommandNormalizeIdsRect();
		normalizeIdsRect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 })
			.drawFrame(2, normalizeIdsRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"Normalize IDs").drawAt(13, normalizeIdsRect.center(), Palette::White);

		const RectF saveRect = EditorCommandSaveRect();
		saveRect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 })
			.drawFrame(2, saveRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"Save").drawAt(14, saveRect.center(), Palette::White);

		const RectF closeRect = EditorCommandCloseRect();
		closeRect.draw(ColorF{ 0.12, 0.05, 0.05, 0.95 })
			.drawFrame(2, closeRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		uiFont(U"Close").drawAt(14, closeRect.center(), Palette::White);
	}
}
