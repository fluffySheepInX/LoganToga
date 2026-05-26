#pragma once
# include <Siv3D.hpp>
# include "MapEditorTypes.h"
# include "../Data/BattleAssetPaths.h"
# include "../Data/UnitCatalog.h"
# include "../Data/BuildLineIconOverrides.h"
# include "EditorMutationHelpers.h"
# include "RectUiHelpers.h"

namespace LT3
{
	inline RectF BuildingEditorPanelRect()
	{
		const RectF bar{ 600, 60, 420, 32 };
		return RectF{ bar.x, bar.y + bar.h, bar.w, 680.0 };
	}

	inline RectF BuildingEditorTabRect(const MapEditorState& editor, int32 tabIndex)
	{
		const RectF panel = BuildingEditorPanelWithPosRect(editor);
		return RectF{ panel.x + 24.0 + tabIndex * 70.0, panel.y + 14.0, 66.0, 24.0 };
	}

	inline RectF BuildingEditorPreviewRect(const MapEditorState& editor)
	{
		const RectF panel = BuildingEditorPanelWithPosRect(editor);
		return RectF{ panel.x + 24.0, panel.y + 50.0, panel.w - 48.0, 118.0 };
	}

	inline RectF BuildingEditorLineIconPathRect(const MapEditorState& editor, int32 index)
	{
		const RectF panel = BuildingEditorPanelWithPosRect(editor);
		const double yBase = panel.y + panel.h - 98.0;
		return RectF{ panel.x + 24.0, yBase + index * 22.0, panel.w - 112.0, 18.0 };
	}

	inline RectF BuildingEditorLineIconBrowseRect(const MapEditorState& editor, int32 index)
	{
		const RectF panel = BuildingEditorPanelWithPosRect(editor);
		const double yBase = panel.y + panel.h - 104.0;
		return RectF{ panel.x + panel.w - 84.0, yBase + index * 22.0, 60.0, 18.0 };
	}

	inline RectF BuildingEditorCloseRect(const MapEditorState& editor)
	{
		const RectF panel = BuildingEditorPanelWithPosRect(editor);
		return RectF{ panel.x + panel.w - 42.0, panel.y + 10.0, 28.0, 28.0 };
	}

	inline RectF BuildingEditorAxisButtonRect(const MapEditorState& editor, int32 axisIndex, int32 buttonIndex)
	{
		const RectF panel = BuildingEditorPanelWithPosRect(editor);
		return RectF{ panel.x + 56.0 + buttonIndex * 54.0, panel.y + 230.0 + axisIndex * 60.0, 48.0, 36.0 };
	}

	inline RectF BuildingEditorAnchorButtonRect(const MapEditorState& editor, int32 index)
	{
		const RectF panel = BuildingEditorPanelWithPosRect(editor);
		return RectF{ panel.x + 24.0 + index * 98.0, panel.y + 376.0, 90.0, 28.0 };
	}

	inline RectF BuildingEditorRenderSizeModeButtonRect(const MapEditorState& editor, int32 index)
	{
		const RectF panel = BuildingEditorPanelWithPosRect(editor);
		return RectF{ panel.x + 24.0 + index * 98.0, panel.y + 420.0, 90.0, 28.0 };
	}

	inline RectF BuildingEditorArtWidthRefButtonRect(const MapEditorState& editor, int32 index)
	{
		const RectF panel = BuildingEditorPanelWithPosRect(editor);
		return RectF{ panel.x + 24.0 + index * 98.0, panel.y + 464.0, 90.0, 28.0 };
	}

	inline RectF BuildingEditorSizeValueButtonRect(const MapEditorState& editor, int32 index)
	{
		const RectF panel = BuildingEditorPanelWithPosRect(editor);
		return RectF{ panel.x + 142.0 + index * 54.0, panel.y + 508.0, 48.0, 34.0 };
	}

	inline RectF BuildingEditorKeepAspectButtonRect(const MapEditorState& editor)
	{
		const RectF panel = BuildingEditorPanelWithPosRect(editor);
		return RectF{ panel.x + 24.0, panel.y + 508.0, 110.0, 34.0 };
	}

	inline RectF BuildingEditorResetRect(const MapEditorState& editor)
	{
		const RectF panel = BuildingEditorPanelWithPosRect(editor);
		return RectF{ panel.x + panel.w - 122.0, panel.y + panel.h - 46.0, 98.0, 28.0 };
	}

	inline Optional<const BuildActionDef*> FindLineBuildActionForCatalogEntry(const UnitCatalogEntry& entry, const DefinitionStores& defs)
	{
		if (entry.unit_id.isEmpty())
		{
			return none;
		}

		for (const auto& action : defs.buildActions)
		{
			if (action.placementMode != BuildPlacementMode::Line)
			{
				continue;
			}

			if (action.ownerTag == entry.unit_id || action.spawnTag == entry.unit_id || action.resultTag == entry.unit_id)
			{
				return &action;
			}
		}

		return none;
	}


	inline HashTable<FilePath, Texture>& BuildingEditorTextureCache()
	{
		static HashTable<FilePath, Texture> cache;
		return cache;
	}

	inline bool HasSelectedCatalogEntry(const MapEditorState& editor, const UnitCatalog& catalog)
	{
		return 0 <= editor.selectedUnitCatalogIndex
			&& editor.selectedUnitCatalogIndex < static_cast<int32>(catalog.entries.size());
	}

	inline void ChangeSelectedUnitPointOffset(MapEditorState& editor, UnitCatalog& catalog, Point UnitCatalogEntry::* field, const Point& delta)
	{
		MutateSelectedCatalogEntry(editor, catalog, [&](UnitCatalogEntry& entry)
		{
			Point& offset = entry.*field;
			const Point next{
				Clamp(offset.x + delta.x, -128, 128),
				Clamp(offset.y + delta.y, -128, 128)
			};
			return SetFieldIfChanged(offset, next);
		});
	}

	inline void ResetSelectedUnitPointOffset(MapEditorState& editor, UnitCatalog& catalog, Point UnitCatalogEntry::* field)
	{
		MutateSelectedCatalogEntry(editor, catalog, [&](UnitCatalogEntry& entry)
		{
			return SetFieldIfChanged(entry.*field, Point{ 0, 0 });
		});
	}

	inline void ChangeSelectedUnitVisualOffset(MapEditorState& editor, UnitCatalog& catalog, const Point& delta)
	{
		ChangeSelectedUnitPointOffset(editor, catalog, &UnitCatalogEntry::visualOffset, delta);
	}

	inline void ResetSelectedUnitVisualOffset(MapEditorState& editor, UnitCatalog& catalog)
	{
		ResetSelectedUnitPointOffset(editor, catalog, &UnitCatalogEntry::visualOffset);
	}

	inline void ChangeSelectedUnitShadowOffset(MapEditorState& editor, UnitCatalog& catalog, const Point& delta)
	{
		ChangeSelectedUnitPointOffset(editor, catalog, &UnitCatalogEntry::shadowOffset, delta);
	}

	inline void ResetSelectedUnitShadowOffset(MapEditorState& editor, UnitCatalog& catalog)
	{
		ResetSelectedUnitPointOffset(editor, catalog, &UnitCatalogEntry::shadowOffset);
	}

	inline void ChangeSelectedUnitLineIconHorizontalOffset(MapEditorState& editor, UnitCatalog& catalog, const Point& delta)
	{
		ChangeSelectedUnitPointOffset(editor, catalog, &UnitCatalogEntry::lineIconHorizontalOffset, delta);
	}

	inline void ResetSelectedUnitLineIconHorizontalOffset(MapEditorState& editor, UnitCatalog& catalog)
	{
		ResetSelectedUnitPointOffset(editor, catalog, &UnitCatalogEntry::lineIconHorizontalOffset);
	}

	inline void ChangeSelectedUnitLineIconDiagUpRightOffset(MapEditorState& editor, UnitCatalog& catalog, const Point& delta)
	{
		ChangeSelectedUnitPointOffset(editor, catalog, &UnitCatalogEntry::lineIconDiagUpRightOffset, delta);
	}

	inline void ResetSelectedUnitLineIconDiagUpRightOffset(MapEditorState& editor, UnitCatalog& catalog)
	{
		ResetSelectedUnitPointOffset(editor, catalog, &UnitCatalogEntry::lineIconDiagUpRightOffset);
	}

	inline void ChangeSelectedUnitLineIconDiagUpLeftOffset(MapEditorState& editor, UnitCatalog& catalog, const Point& delta)
	{
		ChangeSelectedUnitPointOffset(editor, catalog, &UnitCatalogEntry::lineIconDiagUpLeftOffset, delta);
	}

	inline void ResetSelectedUnitLineIconDiagUpLeftOffset(MapEditorState& editor, UnitCatalog& catalog)
	{
		ResetSelectedUnitPointOffset(editor, catalog, &UnitCatalogEntry::lineIconDiagUpLeftOffset);
	}

	inline void ChangeSelectedUnitEditorTabOffset(MapEditorState& editor, UnitCatalog& catalog, const Point& delta)
	{
		if (editor.buildingEditorTab == 1)
		{
			ChangeSelectedUnitShadowOffset(editor, catalog, delta);
		}
		else if (editor.buildingEditorTab == 2)
		{
			ChangeSelectedUnitLineIconHorizontalOffset(editor, catalog, delta);
		}
		else if (editor.buildingEditorTab == 3)
		{
			ChangeSelectedUnitLineIconDiagUpRightOffset(editor, catalog, delta);
		}
		else if (editor.buildingEditorTab == 4)
		{
			ChangeSelectedUnitLineIconDiagUpLeftOffset(editor, catalog, delta);
		}
		else
		{
			ChangeSelectedUnitVisualOffset(editor, catalog, delta);
		}
	}

	inline void ResetSelectedUnitEditorTabOffset(MapEditorState& editor, UnitCatalog& catalog)
	{
		if (editor.buildingEditorTab == 1)
		{
			ResetSelectedUnitShadowOffset(editor, catalog);
		}
		else if (editor.buildingEditorTab == 2)
		{
			ResetSelectedUnitLineIconHorizontalOffset(editor, catalog);
		}
		else if (editor.buildingEditorTab == 3)
		{
			ResetSelectedUnitLineIconDiagUpRightOffset(editor, catalog);
		}
		else if (editor.buildingEditorTab == 4)
		{
			ResetSelectedUnitLineIconDiagUpLeftOffset(editor, catalog);
		}
		else
		{
			ResetSelectedUnitVisualOffset(editor, catalog);
		}
	}

	inline void SetSelectedUnitPlacementAnchor(MapEditorState& editor, UnitCatalog& catalog, UnitPlacementAnchor anchor)
	{
		MutateSelectedCatalogEntry(editor, catalog, [&](UnitCatalogEntry& entry)
		{
			return SetFieldIfChanged(entry.placementAnchor, anchor);
		});
	}

	inline void SetSelectedUnitRenderSizeMode(MapEditorState& editor, UnitCatalog& catalog, UnitRenderSizeMode mode)
	{
		MutateSelectedCatalogEntry(editor, catalog, [&](UnitCatalogEntry& entry)
		{
			return SetFieldIfChanged(entry.renderSizeMode, mode);
		});
	}

	inline void SetSelectedUnitArtWidthReference(MapEditorState& editor, UnitCatalog& catalog, UnitArtWidthReference reference)
	{
		MutateSelectedCatalogEntry(editor, catalog, [&](UnitCatalogEntry& entry)
		{
			return SetFieldIfChanged(entry.artWidthReference, reference);
		});
	}

	inline void ChangeSelectedUnitArtWidthValue(MapEditorState& editor, UnitCatalog& catalog, double delta)
	{
		MutateSelectedCatalogEntry(editor, catalog, [&](UnitCatalogEntry& entry)
		{
			return AdjustField(entry.artWidthValue, delta, [](double value)
			{
				return Clamp(value, 0.1, 8.0);
			});
		});
	}

	inline double GetSelectedUnitArtWidthValueForTab(const UnitCatalogEntry& entry, int32 tab)
	{
		if (tab == 2)
		{
			return entry.artWidthValueLineHorizontal;
		}
		if (tab == 3)
		{
			return entry.artWidthValueLineDiagUpRight;
		}
		if (tab == 4)
		{
			return entry.artWidthValueLineDiagUpLeft;
		}

		return entry.artWidthValue;
	}

	inline void ChangeSelectedUnitArtWidthValueForTab(MapEditorState& editor, UnitCatalog& catalog, int32 tab, double delta)
	{
		MutateSelectedCatalogEntry(editor, catalog, [&](UnitCatalogEntry& entry)
		{
			auto adjust = [&](double& value)
			{
				return AdjustField(value, delta, [](double raw)
				{
					return Clamp(raw, 0.1, 8.0);
				});
			};

			if (tab == 2)
			{
				return adjust(entry.artWidthValueLineHorizontal);
			}
			if (tab == 3)
			{
				return adjust(entry.artWidthValueLineDiagUpRight);
			}
			if (tab == 4)
			{
				return adjust(entry.artWidthValueLineDiagUpLeft);
			}

			return adjust(entry.artWidthValue);
		});
	}

	inline void ChangeSelectedUnitGameplaySizeMul(MapEditorState& editor, UnitCatalog& catalog, double delta)
	{
		MutateSelectedCatalogEntry(editor, catalog, [&](UnitCatalogEntry& entry)
		{
			return AdjustField(entry.gameplaySizeMul, delta, [](double value)
			{
				return Clamp(value, 0.2, 8.0);
			});
		});
	}

	inline void ToggleSelectedUnitArtKeepAspect(MapEditorState& editor, UnitCatalog& catalog)
	{
		MutateSelectedCatalogEntry(editor, catalog, [&](UnitCatalogEntry& entry)
		{
			return ToggleField(entry.artKeepAspect);
		});
	}

	inline void DrawBuildingEditorButton(const RectF& rect, StringView text, const Font& uiFont)
	{
		RectButtonStyle style;
		style.normalText = Palette::White;
		style.fontSize = 16;
		DrawRectButton(rect, text, false, uiFont, style);
	}
}
