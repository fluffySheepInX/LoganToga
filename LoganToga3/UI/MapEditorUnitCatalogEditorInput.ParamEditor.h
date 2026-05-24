#pragma once
# include "MapEditorUnitCatalogEditorInput.ListInput.h"

namespace LT3
{
	inline bool ProcessUnitCatalogParamEditorInput(MapEditorState& editor, UnitCatalog& catalog, bool& consumed)
	{
		if (!(editor.showUnitParameterEditor && EditorUnitParameterPanelRect(editor).mouseOver()))
		{
			return false;
		}

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

		return false;
	}
}
