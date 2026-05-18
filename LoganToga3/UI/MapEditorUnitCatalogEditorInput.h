#pragma once
# include <Siv3D.hpp>
# include "MapEditorUnitCatalogFileOps.h"
# include "MapEditorUnitCatalogLayoutInput.h"

namespace LT3
{
	inline bool ProcessUnitCatalogEditorInput(MapEditorState& editor, UnitCatalog& catalog)
	{
		bool consumed = false;

		// インライン名前編集（TextInput）
		if (editor.unitRenameTargetIndex)
		{
			TextInput::UpdateText(editor.unitRenameEditText);
			// TOML を破壊する文字（改行・制御文字・"・\）を除去
			editor.unitRenameEditText.remove_if([](char32 c)
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
					editor.unitRenameEditText += clip;
				}
			}
			if (KeyEnter.down())
			{
				const int32 idx = *editor.unitRenameTargetIndex;
				if (0 <= idx && idx < static_cast<int32>(catalog.entries.size()))
				{
					catalog.entries[idx].name = editor.unitRenameEditText.isEmpty()
						? catalog.entries[idx].name
						: editor.unitRenameEditText;
					SaveUnitCatalogToml(catalog, editor.statusText);
					editor.unitCatalogDirty = true;
					editor.statusText = U"Renamed unit: {}"_fmt(catalog.entries[idx].tag);
				}
				editor.unitRenameTargetIndex = none;
				editor.unitRenameEditText = U"";
			}
			else if (KeyEscape.down())
			{
				// Duplicate 由来のキャンセルのみエントリを削除
				if (editor.unitRenameIsDuplicate)
				{
					const int32 idx = *editor.unitRenameTargetIndex;
					if (0 <= idx && idx < static_cast<int32>(catalog.entries.size()))
					{
						catalog.entries.remove_at(idx);
						SaveUnitCatalogToml(catalog, editor.statusText);
						editor.unitCatalogDirty = true;
						if (editor.selectedUnitCatalogIndex >= static_cast<int32>(catalog.entries.size()))
						{
							editor.selectedUnitCatalogIndex = static_cast<int32>(catalog.entries.size()) - 1;
						}
					}
				}
				editor.unitRenameTargetIndex = none;
				editor.unitRenameEditText = U"";
				editor.unitRenameIsDuplicate = false;
			}
			return true;
		}

		// コンテキストメニュー処理
		if (editor.unitContextMenuTargetIndex)
		{
			const RectF menuRect = EditorUnitContextMenuRect(editor.unitContextMenuPos);
			const RectF dupItem = EditorUnitContextMenuItemRect(editor.unitContextMenuPos, 0);
			if (dupItem.leftClicked())
			{
				const int32 srcIdx = *editor.unitContextMenuTargetIndex;
				editor.unitContextMenuTargetIndex = none;
				if (0 <= srcIdx && srcIdx < static_cast<int32>(catalog.entries.size()))
				{
					UnitCatalogEntry newEntry = catalog.entries[srcIdx];
					String baseTag = newEntry.tag;
					String candidateTag = baseTag + U"_copy";
					int32 suffix = 2;
					while (catalog.entries.any([&](const UnitCatalogEntry& e) { return e.tag == candidateTag; }))
					{
						candidateTag = baseTag + U"_copy" + Format(suffix++);
					}
					newEntry.tag = candidateTag;
					catalog.entries << newEntry;
					const int32 newIdx = static_cast<int32>(catalog.entries.size()) - 1;
					editor.selectedUnitCatalogIndex = newIdx;
					editor.showUnitParameterEditor = true;
					const RectF viewport = EditorUnitListViewportRect();
					const double maxScroll = Max(0.0, EditorUnitListContentHeight(catalog) - viewport.h);
					editor.unitListScroll = maxScroll;
					editor.unitRenameTargetIndex = newIdx;
					editor.unitRenameEditText = newEntry.name;
					editor.unitRenameIsDuplicate = true;
				}
				return true;
			}
			const RectF renameItem = EditorUnitContextMenuItemRect(editor.unitContextMenuPos, 1);
			if (renameItem.leftClicked())
			{
				const int32 srcIdx = *editor.unitContextMenuTargetIndex;
				editor.unitContextMenuTargetIndex = none;
				if (0 <= srcIdx && srcIdx < static_cast<int32>(catalog.entries.size()))
				{
					editor.selectedUnitCatalogIndex = srcIdx;
					editor.showUnitParameterEditor = true;
					editor.unitRenameTargetIndex = srcIdx;
					editor.unitRenameEditText = catalog.entries[srcIdx].name;
					editor.unitRenameIsDuplicate = false;
				}
				return true;
			}
			else if (!menuRect.mouseOver() && (MouseL.down() || MouseR.down()))
			{
				editor.unitContextMenuTargetIndex = none;
				return false;
			}
			return true;
		}

		if (editor.uiLayoutEditEnabled)
		{
			UpdateParamEditorDrag(editor);
			UpdateBuildingEditorDrag(editor);
			if (editor.uiLayoutDraggingParamEditor || editor.uiLayoutDraggingBuildingEditor)
			{
				return true;
			}
		}

		if (editor.showUnitList && EditorUnitListPanelRect().mouseOver())
		{
			const RectF viewport = EditorUnitListViewportRect();
			const double maxScroll = Max(0.0, EditorUnitListContentHeight(catalog) - viewport.h);
			editor.unitListScroll = Clamp(editor.unitListScroll - Mouse::Wheel() * 42.0, 0.0, maxScroll);
			consumed = true;

			for (int32 i = 0; i < static_cast<int32>(catalog.entries.size()); ++i)
			{
				const RectF row = EditorUnitListRowRect(viewport, i, editor.unitListScroll);
				if (!viewport.intersects(row))
				{
					continue;
				}

				const RectF moveUpRect = EditorUnitRowMoveUpRect(row);
				const RectF moveDownRect = EditorUnitRowMoveDownRect(row);
				if ((i > 0) && moveUpRect.leftClicked())
				{
					const UnitCatalogEntry movedEntry = catalog.entries[i];
					catalog.entries[i] = catalog.entries[i - 1];
					catalog.entries[i - 1] = movedEntry;
					editor.selectedUnitCatalogIndex = i - 1;
					SaveUnitCatalogToml(catalog, editor.statusText);
					editor.unitCatalogDirty = true;
					editor.statusText = U"Moved unit up: {}"_fmt(catalog.entries[i - 1].tag);
					return true;
				}
				if ((i + 1 < static_cast<int32>(catalog.entries.size())) && moveDownRect.leftClicked())
				{
					const UnitCatalogEntry movedEntry = catalog.entries[i];
					catalog.entries[i] = catalog.entries[i + 1];
					catalog.entries[i + 1] = movedEntry;
					editor.selectedUnitCatalogIndex = i + 1;
					SaveUnitCatalogToml(catalog, editor.statusText);
					editor.unitCatalogDirty = true;
					editor.statusText = U"Moved unit down: {}"_fmt(catalog.entries[i + 1].tag);
					return true;
				}

				const RectF previewRect = EditorUnitListPreviewRect(row);
				if (previewRect.leftClicked())
				{
					editor.selectedUnitCatalogIndex = i;
					editor.showUnitParameterEditor = true;
					if (!ChangeSelectedUnitImageFromDialog(editor, catalog))
					{
						editor.statusText = U"Editing unit: {}"_fmt(catalog.entries[i].tag);
					}
				}
				else if (row.leftClicked())
				{
					editor.selectedUnitCatalogIndex = i;
					editor.showUnitParameterEditor = true;
					editor.statusText = U"Editing unit: {}"_fmt(catalog.entries[i].tag);
				}
				else if (row.rightClicked())
				{
					editor.unitContextMenuTargetIndex = i;
					editor.unitContextMenuPos = Cursor::PosF();
				}
			}
		}

		if (editor.showUnitParameterEditor && EditorUnitParameterPanelRect(editor).mouseOver())
		{
			consumed = true;
			if (EditorUnitParameterCloseRect(editor).leftClicked() || EditorUnitBuildingCloseRect(editor).leftClicked())
			{
				editor.showUnitParameterEditor = false;
				editor.showBuildingEditor = false;
			}

			for (int32 tabIndex = 0; tabIndex < 4; ++tabIndex)
			{
				if (EditorUnitParamInnerTabRect(editor, tabIndex).leftClicked())
				{
					editor.unitParamEditorTab = tabIndex;
					return true;
				}
			}

			if (!(0 <= editor.selectedUnitCatalogIndex && editor.selectedUnitCatalogIndex < static_cast<int32>(catalog.entries.size())))
			{
				return true;
			}

			UnitCatalogEntry& entry = catalog.entries[editor.selectedUnitCatalogIndex];
			auto commit = [&]()
				{
					SaveUnitCatalogToml(catalog, editor.statusText);
					editor.unitCatalogDirty = true;
				};

			enum class UnitParamRowKind
			{
				Hp,
				BuildingHp,
				Mp,
				Attack,
				Defense,
				Speed,
				Magic,
				MagicDefense,
				Move,
				Vision,
				VisualScale,
				GoldCost,
				TrustCost,
				FoodCost,
				MaintainRange,
			};

			struct UnitParamRowSpec
			{
				UnitParamRowKind kind;
				String helpText;
			};

			Array<UnitParamRowSpec> rows;
			switch (editor.unitParamEditorTab)
			{
			case 0:
				rows = {
					{ UnitParamRowKind::Hp, U"HP を増減します" },
					{ UnitParamRowKind::BuildingHp, U"Building HP を増減します" },
					{ UnitParamRowKind::Mp, U"MP を増減します" }
				};
				break;
			case 1:
				rows = {
					{ UnitParamRowKind::Attack, U"攻撃力を増減します" },
					{ UnitParamRowKind::Defense, U"防御力を増減します" },
					{ UnitParamRowKind::Speed, U"速度を増減します" },
					{ UnitParamRowKind::Magic, U"魔力を増減します" },
					{ UnitParamRowKind::MagicDefense, U"魔法防御を増減します" }
				};
				break;
			case 2:
				rows = {
					{ UnitParamRowKind::Move, U"移動力。Uで move=0（Use SPD）" },
					{ UnitParamRowKind::Vision, U"視界半径（セル）" },
					{ UnitParamRowKind::VisualScale, U"見た目サイズ" }
				};
				break;
			default:
				rows = {
					{ UnitParamRowKind::GoldCost, U"Gold 資源コストを増減します" },
					{ UnitParamRowKind::TrustCost, U"Trust 資源コストを増減します" },
					{ UnitParamRowKind::FoodCost, U"Food 資源コストを増減します" },
					{ UnitParamRowKind::MaintainRange, U"maintain_range を増減します" }
				};
				break;
			}

			auto adjustInt = [&](int32 UnitCatalogEntry::* field, int32 delta, int32 minValue, int32 maxValue)
				{
					entry.*field = Clamp((entry.*field) + delta, minValue, maxValue);
					commit();
				};

			auto resetInt = [&](int32 UnitCatalogEntry::* field, int32 value)
				{
					entry.*field = value;
					commit();
				};

			auto adjustScale = [&](double delta)
				{
					entry.visualScale = Math::Round(Clamp(entry.visualScale + delta, 0.25, 3.0) * 100.0) / 100.0;
					commit();
				};

			const RectF viewport = EditorUnitParamListViewportRect(editor);
			for (int32 rowIndex = 0; rowIndex < static_cast<int32>(rows.size()); ++rowIndex)
			{
				const RectF row = EditorUnitParamRowRect(viewport, rowIndex);
				if (!viewport.intersects(row))
				{
					continue;
				}

				for (int32 buttonIndex = 0; buttonIndex < 5; ++buttonIndex)
				{
					const RectF buttonRect = EditorUnitParamRowButtonRect(row, buttonIndex);
					if (!buttonRect.leftClicked())
					{
						continue;
					}

					if (buttonIndex == 4)
					{
						editor.statusText = rows[rowIndex].helpText;
						return true;
					}

					switch (rows[rowIndex].kind)
					{
					case UnitParamRowKind::Hp:
						if (buttonIndex == 0) adjustInt(&UnitCatalogEntry::hp, -10, 0, 99999);
						else if (buttonIndex == 1) adjustInt(&UnitCatalogEntry::hp, 10, 0, 99999);
						else if (buttonIndex == 2) resetInt(&UnitCatalogEntry::hp, 0);
						break;
					case UnitParamRowKind::BuildingHp:
						if (buttonIndex == 0) adjustInt(&UnitCatalogEntry::buildingHp, -10, 0, 99999);
						else if (buttonIndex == 1) adjustInt(&UnitCatalogEntry::buildingHp, 10, 0, 99999);
						else if (buttonIndex == 2) resetInt(&UnitCatalogEntry::buildingHp, 0);
						break;
					case UnitParamRowKind::Mp:
						if (buttonIndex == 0) adjustInt(&UnitCatalogEntry::mp, -10, 0, 99999);
						else if (buttonIndex == 1) adjustInt(&UnitCatalogEntry::mp, 10, 0, 99999);
						else if (buttonIndex == 2) resetInt(&UnitCatalogEntry::mp, 0);
						break;
					case UnitParamRowKind::Attack:
						if (buttonIndex == 0) adjustInt(&UnitCatalogEntry::attack, -5, 0, 99999);
						else if (buttonIndex == 1) adjustInt(&UnitCatalogEntry::attack, 5, 0, 99999);
						else if (buttonIndex == 2) resetInt(&UnitCatalogEntry::attack, 0);
						break;
					case UnitParamRowKind::Defense:
						if (buttonIndex == 0) adjustInt(&UnitCatalogEntry::defense, -5, 0, 99999);
						else if (buttonIndex == 1) adjustInt(&UnitCatalogEntry::defense, 5, 0, 99999);
						else if (buttonIndex == 2) resetInt(&UnitCatalogEntry::defense, 0);
						break;
					case UnitParamRowKind::Speed:
						if (buttonIndex == 0) adjustInt(&UnitCatalogEntry::speed, -5, 0, 99999);
						else if (buttonIndex == 1) adjustInt(&UnitCatalogEntry::speed, 5, 0, 99999);
						else if (buttonIndex == 2) resetInt(&UnitCatalogEntry::speed, 0);
						break;
					case UnitParamRowKind::Magic:
						if (buttonIndex == 0) adjustInt(&UnitCatalogEntry::magic, -5, 0, 99999);
						else if (buttonIndex == 1) adjustInt(&UnitCatalogEntry::magic, 5, 0, 99999);
						else if (buttonIndex == 2) resetInt(&UnitCatalogEntry::magic, 0);
						break;
					case UnitParamRowKind::MagicDefense:
						if (buttonIndex == 0) adjustInt(&UnitCatalogEntry::magicDefense, -5, 0, 99999);
						else if (buttonIndex == 1) adjustInt(&UnitCatalogEntry::magicDefense, 5, 0, 99999);
						else if (buttonIndex == 2) resetInt(&UnitCatalogEntry::magicDefense, 0);
						break;
					case UnitParamRowKind::Move:
						if (buttonIndex == 0) adjustInt(&UnitCatalogEntry::move, -25, 0, 2000);
						else if (buttonIndex == 1) adjustInt(&UnitCatalogEntry::move, 25, 0, 2000);
						else if (buttonIndex == 2) resetInt(&UnitCatalogEntry::move, 0);
						else if (buttonIndex == 3)
						{
							entry.move = 0;
							commit();
							editor.statusText = U"MOVE: Use SPD (move=0)";
						}
						break;
					case UnitParamRowKind::Vision:
						if (buttonIndex == 0) adjustInt(&UnitCatalogEntry::visionRadius, -1, 0, 40);
						else if (buttonIndex == 1) adjustInt(&UnitCatalogEntry::visionRadius, 1, 0, 40);
						else if (buttonIndex == 2) resetInt(&UnitCatalogEntry::visionRadius, 6);
						break;
					case UnitParamRowKind::VisualScale:
						if (buttonIndex == 0) adjustScale(-0.05);
						else if (buttonIndex == 1) adjustScale(0.05);
						else if (buttonIndex == 2)
						{
							entry.visualScale = 1.0;
							commit();
						}
						break;
					case UnitParamRowKind::GoldCost:
						if (buttonIndex == 0) adjustInt(&UnitCatalogEntry::goldCost, -10, 0, 99999);
						else if (buttonIndex == 1) adjustInt(&UnitCatalogEntry::goldCost, 10, 0, 99999);
						else if (buttonIndex == 2) resetInt(&UnitCatalogEntry::goldCost, 0);
						break;
					case UnitParamRowKind::TrustCost:
						if (buttonIndex == 0) adjustInt(&UnitCatalogEntry::trustCost, -10, 0, 99999);
						else if (buttonIndex == 1) adjustInt(&UnitCatalogEntry::trustCost, 10, 0, 99999);
						else if (buttonIndex == 2) resetInt(&UnitCatalogEntry::trustCost, 0);
						break;
					case UnitParamRowKind::FoodCost:
						if (buttonIndex == 0) adjustInt(&UnitCatalogEntry::foodCost, -10, 0, 99999);
						else if (buttonIndex == 1) adjustInt(&UnitCatalogEntry::foodCost, 10, 0, 99999);
						else if (buttonIndex == 2) resetInt(&UnitCatalogEntry::foodCost, 0);
						break;
					case UnitParamRowKind::MaintainRange:
						if (buttonIndex == 0) adjustInt(&UnitCatalogEntry::maintainRange, -1, 0, 99999);
						else if (buttonIndex == 1) adjustInt(&UnitCatalogEntry::maintainRange, 1, 0, 99999);
						else if (buttonIndex == 2) resetInt(&UnitCatalogEntry::maintainRange, 0);
						break;
					}

					if (buttonIndex == 3 && rows[rowIndex].kind != UnitParamRowKind::Move)
					{
						editor.statusText = U"U action はこの項目では未割当です";
					}

					return true;
				}
			}
		}

		return consumed;
	}
}
