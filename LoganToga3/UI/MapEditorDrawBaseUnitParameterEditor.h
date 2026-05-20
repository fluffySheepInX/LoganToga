#pragma once
# include <Siv3D.hpp>
# include "MapEditorDrawBaseUnitEditorHelpers.h"

namespace LT3
{
	inline void DrawUnitBuildingEditorTabBar(const MapEditorState& editor, const Font& uiFont)
	{
		const bool show = editor.showUnitParameterEditor || editor.showBuildingEditor;
		if (!show)
		{
			return;
		}

		const RectF bar = EditorUnitBuildingTabBarRect(editor);
		bar.draw(ColorF{ 0.04, 0.05, 0.07, 0.96 }).drawFrame(1, ColorF{ 1, 1, 1, 0.18 });

		const Array<String> tabLabels = { U"Unit Param", U"Building Edit" };
		for (int32 i = 0; i < 2; ++i)
		{
			const RectF tab = EditorUnitBuildingTabRect(editor, i);
			const bool active = (i == 0) ? editor.showUnitParameterEditor : editor.showBuildingEditor;
			tab.draw(active ? ColorF{ 0.12, 0.22, 0.18, 0.96 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
				.drawFrame(2, tab.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
			uiFont(tabLabels[i]).drawAt(11, tab.center(), active ? Palette::White : Palette::Lightgray);
		}

		if (editor.uiLayoutEditEnabled)
		{
			const RectF dragHandle = EditorUnitParameterDragHandleRect(editor);
			const RectF topAnchorToggle = UiLayoutTopAnchorToggleRect(dragHandle);
			dragHandle.draw(editor.uiLayoutDraggingParamEditor ? ColorF{ 1.0, 0.84, 0.0, 0.9 } : ColorF{ 1.0, 0.84, 0.0, 0.4 })
				.drawFrame(1, ColorF{ 1, 1, 1, 0.2 });
			uiFont(U"↕").drawAt(11, dragHandle.center(), Palette::White);
			topAnchorToggle.draw(editor.uiParamEditorTopAnchor ? ColorF{ 0.16, 0.24, 0.18, 0.95 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
				.drawFrame(2, topAnchorToggle.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
			uiFont(U"↑").drawAt(11, topAnchorToggle.center(), editor.uiParamEditorTopAnchor ? Palette::White : Palette::Lightgray);
		}
		else
		{
			const RectF closeBtn = EditorUnitBuildingCloseRect(editor);
			closeBtn.draw(ColorF{ 0.12, 0.05, 0.05, 0.95 })
				.drawFrame(1, closeBtn.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
			uiFont(U"×").drawAt(14, closeBtn.center(), Palette::White);
		}
	}

	inline void DrawUnitParameterEditor(const MapEditorState& editor, const UnitCatalog& catalog, const Font& uiFont)
	{
		if (!editor.showUnitParameterEditor || editor.selectedUnitCatalogIndex < 0 || static_cast<int32>(catalog.entries.size()) <= editor.selectedUnitCatalogIndex)
		{
			return;
		}

		const UnitCatalogEntry& entry = catalog.entries[editor.selectedUnitCatalogIndex];
		const RectF panel = EditorUnitParameterPanelRect(editor);
		panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.94 }).drawFrame(1, ColorF{ 1, 1, 1, 0.18 });
		EditorUnitParamHeaderDividerRect(editor).draw(ColorF{ 1, 1, 1, 0.22 });

		const Array<String> tabLabels = { U"Basic", U"Combat", U"MoveVis", U"Economy" };
		for (int32 i = 0; i < static_cast<int32>(tabLabels.size()); ++i)
		{
			const RectF tabRect = EditorUnitParamInnerTabRect(editor, i);
			const bool active = (editor.unitParamEditorTab == i);
			tabRect.draw(active ? ColorF{ 0.12, 0.22, 0.18, 0.96 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
				.drawFrame(2, tabRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
			uiFont(tabLabels[i]).drawAt(11, tabRect.center(), active ? Palette::White : Palette::Lightgray);
		}

		uiFont(entry.name).draw(14, panel.x + 18.0, panel.y + 52.0, Palette::White);
		uiFont(U"unit_id:{}  image:{}"_fmt(entry.unit_id, entry.image)).draw(11, panel.x + 18.0, panel.y + 72.0, Palette::Lightgray);

		const RectF listHeader = EditorUnitParamListHeaderRect(editor);
		listHeader.draw(ColorF{ 0.05, 0.06, 0.08, 0.96 }).drawFrame(1, ColorF{ 1, 1, 1, 0.14 });
		uiFont(U"name").draw(12, listHeader.x + 10.0, listHeader.y + 8.0, Palette::White);
		uiFont(U"value").draw(12, listHeader.x + 122.0, listHeader.y + 8.0, Palette::White);

		const RectF viewport = EditorUnitParamListViewportRect(editor);
		viewport.draw(ColorF{ 0, 0, 0, 0.10 });

		struct UiParamRow
		{
			String label;
			String valueText;
			String helpText;
			bool useIcon = false;
		};

		Array<UiParamRow> rows;
		switch (editor.unitParamEditorTab)
		{
		case 0:
			rows = {
				{ U"HP", U"{}"_fmt(entry.hp), U"HP を増減します" },
				{ U"BHP", U"{}"_fmt(entry.buildingHp), U"Building HP を増減します" },
				{ U"MP", U"{}"_fmt(entry.mp), U"MP を増減します" }
			};
			break;
		case 1:
			rows = {
				{ U"ATK", U"{}"_fmt(entry.attack), U"攻撃力を増減します" },
				{ U"DEF", U"{}"_fmt(entry.defense), U"防御力を増減します" },
				{ U"SPD", U"{}"_fmt(entry.speed), U"速度を増減します" },
				{ U"MAG", U"{}"_fmt(entry.magic), U"魔力を増減します" },
				{ U"MDEF", U"{}"_fmt(entry.magicDefense), U"魔法防御を増減します" }
			};
			break;
		case 2:
			rows = {
				{ U"MOVE", U"{}"_fmt(entry.move), U"移動力。Uで move=0(Use SPD)", true },
				{ U"VISION", U"{}"_fmt(entry.visionRadius), U"視界半径（セル）" },
				{ U"SCALE", U"{:.2f}"_fmt(entry.visualScale), U"見た目サイズ" }
			};
			break;
		default:
			rows = {
				{ U"GOLD", U"{}"_fmt(entry.goldCost), U"Gold 資源コストを増減します" },
				{ U"TRUST", U"{}"_fmt(entry.trustCost), U"Trust 資源コストを増減します" },
				{ U"FOOD", U"{}"_fmt(entry.foodCost), U"Food 資源コストを増減します" },
				{ U"RANGE", U"{}"_fmt(entry.maintainRange), U"maintain_range を増減します" }
			};
			break;
		}

		String hoverHelp;
		static bool hatenaLoaded = false;
		static Texture hatenaTexture;
		if (!hatenaLoaded)
		{
			hatenaLoaded = true;
			const FilePath hatenaPath = ResolveSystemImagePath(U"hatena.png");
			if (FileSystem::Exists(hatenaPath))
			{
				hatenaTexture = Texture{ hatenaPath };
			}
		}

		for (int32 i = 0; i < static_cast<int32>(rows.size()); ++i)
		{
			const RectF row = EditorUnitParamRowRect(viewport, i);
			const RectF nameRect = EditorUnitParamRowNameRect(row);
			const RectF valueRect = EditorUnitParamRowValueRect(row);
			row.draw(ColorF{ 0.07, 0.08, 0.10, 0.94 }).drawFrame(1, row.mouseOver() ? ColorF{ 0.0, 0.75, 1.0 } : ColorF{ 1, 1, 1, 0.10 });
			uiFont(rows[i].label).draw(12, nameRect.x + 6.0, nameRect.y + 7.0, Palette::White);
			uiFont(rows[i].valueText).draw(12, valueRect.x + 6.0, valueRect.y + 7.0, Palette::Gold);

			for (int32 buttonIndex = 0; buttonIndex < 5; ++buttonIndex)
			{
				const RectF buttonRect = EditorUnitParamRowButtonRect(row, buttonIndex);
				buttonRect.draw(ColorF{ 0.08, 0.09, 0.11, 0.92 }).drawFrame(2, buttonRect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
				if (buttonIndex == 0)
				{
					uiFont(U"-").drawAt(16, buttonRect.center(), Palette::White);
				}
				else if (buttonIndex == 1)
				{
					uiFont(U"+").drawAt(16, buttonRect.center(), Palette::White);
				}
				else if (buttonIndex == 2)
				{
					uiFont(U"R").drawAt(16, buttonRect.center(), Palette::White);
				}
				else if (buttonIndex == 3)
				{
					uiFont(U"U").drawAt(16, buttonRect.center(), rows[i].useIcon ? Palette::White : Palette::Gray);
				}
				else
				{
					if (hatenaTexture)
					{
						hatenaTexture.resized(18, 18).drawAt(buttonRect.center());
					}
					else
					{
						uiFont(U"?").drawAt(16, buttonRect.center(), Palette::White);
					}
				}

				if (buttonIndex == 4 && buttonRect.mouseOver())
				{
					hoverHelp = rows[i].helpText;
				}
			}
		}

		if (!hoverHelp.isEmpty())
		{
			uiFont(hoverHelp).draw(11, panel.x + 18.0, panel.y + panel.h - 16.0, Palette::Aqua);
		}

		if (editor.uiLayoutEditEnabled)
		{
			const RectF dragHandle = EditorUnitParameterDragHandleRect(editor);
			const RectF topAnchorToggle = UiLayoutTopAnchorToggleRect(dragHandle);
			dragHandle.draw(editor.uiLayoutDraggingParamEditor ? ColorF{ 1.0, 0.84, 0.0, 0.9 } : ColorF{ 1.0, 0.84, 0.0, 0.4 })
				.drawFrame(1, ColorF{ 1, 1, 1, 0.2 });
			uiFont(U"↕").drawAt(11, dragHandle.center(), Palette::White);
			topAnchorToggle.draw(editor.uiParamEditorTopAnchor ? ColorF{ 0.16, 0.24, 0.18, 0.95 } : ColorF{ 0.08, 0.09, 0.11, 0.92 })
				.drawFrame(2, topAnchorToggle.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
			uiFont(U"↑").drawAt(11, topAnchorToggle.center(), editor.uiParamEditorTopAnchor ? Palette::White : Palette::Lightgray);
		}
	}
}
