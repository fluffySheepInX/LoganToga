#pragma once
# include <Siv3D.hpp>
# include "MapEditorDrawBaseCommandEditorHelpers.h"

namespace LT3
{
	// Command 一覧を描画する。
	inline void DrawCommandEditorCommandList(MapEditorState& editor, const DefinitionStores& defs, const RectF& commandViewport, const Font& uiFont)
	{
		const int32 commandCount = static_cast<int32>(defs.buildActions.size());
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
			DrawCommandEditorLayeredIcon(action, iconRect);

			uiFont(action.name).draw(12, row.x + 56.0, row.y + 7.0, Palette::White);
			uiFont(U"{}:{}"_fmt(action.ownerTag, action.id)).draw(10, row.x + 56.0, row.y + 29.0, Palette::Lightgray);
			if (action.isMove)
			{
				const RectF placeBadge{ row.x + row.w - 70.0, row.y + 8.0, 62.0, 18.0 };
				placeBadge.draw(ColorF{ 0.10, 0.20, 0.30, 0.94 }).drawFrame(1, ColorF{ 0.45, 0.90, 1.0, 0.85 });
				uiFont(U"PLACE").drawAt(9, placeBadge.center(), Palette::Aqua);
			}

			if (editor.commandRenameTargetIndex == i)
			{
				const RectF renameRect = EditorCommandRenameOverlayRect(row);
				renameRect.draw(ColorF{ 0.06, 0.08, 0.12, 0.98 }).drawFrame(2, ColorF{ 1.0, 0.84, 0.0 });
				uiFont(editor.commandRenameEditText + U"|").draw(13, renameRect.x + 8.0, renameRect.y + 6.0, Palette::White);
				uiFont(U"Enter:確定  Esc:キャンセル").draw(10, renameRect.x + renameRect.w + 6.0, renameRect.y + 9.0, ColorF{ 1, 1, 1, 0.55 });
			}
		}
	}

	// Command コンテキストメニューを描画する。
	inline void DrawCommandEditorContextMenu(MapEditorState& editor, const Font& uiFont)
	{
		if (!editor.commandContextMenuTargetIndex)
		{
			return;
		}

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
		const RectF descriptionItem = EditorCommandContextMenuItemRect(editor.commandContextMenuPos, 3);
		descriptionItem.draw(descriptionItem.mouseOver() ? ColorF{ 0.16, 0.22, 0.18, 0.96 } : ColorF{ 0.0, 0.0, 0.0, 0.0 });
		uiFont(U"説明文編集").draw(13, descriptionItem.x + 8.0, descriptionItem.y + 5.0, Palette::White);
	}

	// Command モードタブを描画する。
	inline void DrawCommandEditorModeTabs(MapEditorState& editor, const Font& uiFont)
	{
		const Array<String> modeLabels = { U"Bind", U"Spawn", U"Inspect" };
		for (int32 modeIndex = 0; modeIndex < static_cast<int32>(modeLabels.size()); ++modeIndex)
		{
			const RectF modeRect = EditorCommandModeTabRect(modeIndex);
			const bool active = (editor.commandEditorMode == modeIndex);
			modeRect.draw(active ? ColorF{ 0.16, 0.18, 0.13, 0.95 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
				.drawFrame(2, modeRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : (active ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.14 }));
			uiFont(modeLabels[modeIndex]).drawAt(12, modeRect.center(), active ? Palette::White : Palette::Lightgray);
		}
	}

	// Unit/Spawn パネルを描画する。
	inline void DrawCommandEditorUnitPanel(MapEditorState& editor, const UnitCatalog& catalog, const DefinitionStores& defs, const BuildActionDef& selectedAction, const Array<String>& selectedSpawnTags, bool missingSpawnForUnitResult, bool allowMultipleSpawns, const RectF& unitViewport, const Font& uiFont)
	{
		unitViewport.draw(ColorF{ 0, 0, 0, 0.10 });

		constexpr int32 columns = 5;
		const Array<int32> sortedUnitIndices = SortedUnitCatalogEntryIndicesById(catalog);
		const int32 unitCount = static_cast<int32>(sortedUnitIndices.size());
		const int32 unitRows = (unitCount + columns - 1) / columns;
		const double unitContentHeight = unitRows * 96.0 + 8.0;
		const double unitMaxScroll = Max(0.0, unitContentHeight - unitViewport.h);
		editor.commandUnitListScroll = Clamp(editor.commandUnitListScroll, 0.0, unitMaxScroll);

		const double unitViewportBottom = unitViewport.y + unitViewport.h;
		for (int32 i = 0; i < unitCount; ++i)
		{
			const UnitCatalogEntry& entry = catalog.entries[sortedUnitIndices[i]];
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
						return CommandEditorOwnerTagEquals(entry.unit_id, spawnTag);
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
			const RectF clearSpawnRect = EditorCommandSpawnClearRect();
			const RectF spawnStatusRect = EditorCommandSpawnFooterTextRect(0);
			const RectF spawnHintRect = EditorCommandSpawnFooterTextRect(1);
			const RectF spawnGuideRect = EditorCommandSpawnFooterTextRect(2);
			clearSpawnRect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 })
				.drawFrame(2, clearSpawnRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
			uiFont(U"Clear Spawn").drawAt(12, clearSpawnRect.center(), Palette::White);
			uiFont(U"Spawn target: {}"_fmt(selectedSpawnTags.isEmpty() ? U"(none)" : selectedSpawnTags.join(U", "))).draw(12, spawnStatusRect.pos, Palette::Aqua);
			if (missingSpawnForUnitResult)
			{
				uiFont(U"WARN: Unit result だが spawn 未設定").draw(11, spawnHintRect.pos, Palette::Orange);
			}
			uiFont(allowMultipleSpawns ? U"施設owner: 複数選択可 / ×: 他ユニットをspawnしない" : U"通常owner: 単一選択 / ×: 他ユニットをspawnしない").draw(11, spawnGuideRect.pos, Palette::Lightgray);
		}
	}

	// Command Editor フッターを描画する。
	inline void DrawCommandEditorFooter(MapEditorState& editor, const RectF& panel, const Font& uiFont)
	{
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
