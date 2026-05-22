#pragma once
# include <Siv3D.hpp>
# include "BuildingEditorCommon.h"
# include "RectUiHelpers.h"

namespace LT3
{
	inline bool ProcessBuildingEditorInput(MapEditorState& editor, UnitCatalog& catalog, const DefinitionStores& defs)
	{
		if (!editor.showBuildingEditor)
		{
			return false;
		}

		const RectF panel = BuildingEditorPanelWithPosRect(editor.uiBuildingEditorPos);
		if (!panel.mouseOver())
		{
			return false;
		}

		if (!editor.uiLayoutEditEnabled && (HandleRectButtonClick(BuildingEditorCloseRect(editor)) || HandleRectButtonClick(EditorUnitBuildingCloseRect(editor))))
		{
			editor.showBuildingEditor = false;
			editor.showUnitParameterEditor = false;
			editor.statusText = U"BuildingEditor OFF";
			return true;
		}

		if (HandleIntTabButtons(editor.buildingEditorTab, 5, [&](int32 index)
			{
				return BuildingEditorTabRect(editor, index);
			}))
		{
			return true;
		}

		if (!HasSelectedCatalogEntry(editor, catalog))
		{
			return true;
		}

		const UnitCatalogEntry& entry = catalog.entries[editor.selectedUnitCatalogIndex];
		if (HandleRectButtonClick(BuildingEditorAnchorButtonRect(editor, 0)))
		{
			SetSelectedUnitPlacementAnchor(editor, catalog, UnitPlacementAnchor::Center);
			return true;
		}
		if (HandleRectButtonClick(BuildingEditorAnchorButtonRect(editor, 1)))
		{
			SetSelectedUnitPlacementAnchor(editor, catalog, UnitPlacementAnchor::BottomCenter);
			return true;
		}
		if (HandleRectButtonClick(BuildingEditorRenderSizeModeButtonRect(editor, 0)))
		{
			SetSelectedUnitRenderSizeMode(editor, catalog, UnitRenderSizeMode::Gameplay);
			return true;
		}
		if (HandleRectButtonClick(BuildingEditorRenderSizeModeButtonRect(editor, 1)))
		{
			SetSelectedUnitRenderSizeMode(editor, catalog, UnitRenderSizeMode::Art);
			return true;
		}

		const bool artMode = (entry.renderSizeMode == UnitRenderSizeMode::Art);
		if (artMode)
		{
			if (HandleRectButtonClick(BuildingEditorArtWidthRefButtonRect(editor, 0)))
			{
				SetSelectedUnitArtWidthReference(editor, catalog, UnitArtWidthReference::Cell);
				return true;
			}
			if (HandleRectButtonClick(BuildingEditorArtWidthRefButtonRect(editor, 1)))
			{
				SetSelectedUnitArtWidthReference(editor, catalog, UnitArtWidthReference::Pixel);
				return true;
			}
			if (HandleRectButtonClick(BuildingEditorKeepAspectButtonRect(editor)))
			{
				ToggleSelectedUnitArtKeepAspect(editor, catalog);
				return true;
			}
			if (const Optional<double> delta = FindClickedDeltaButton({ -0.4, -0.1, 0.1, 0.4 }, [&](int32 index)
				{
					return BuildingEditorSizeValueButtonRect(editor, index);
				}))
			{
				ChangeSelectedUnitArtWidthValueForTab(editor, catalog, editor.buildingEditorTab, *delta);
				return true;
			}
		}
		else
		{
			if (const Optional<double> delta = FindClickedDeltaButton({ -0.4, -0.1, 0.1, 0.4 }, [&](int32 index)
				{
					return BuildingEditorSizeValueButtonRect(editor, index);
				}))
			{
				ChangeSelectedUnitGameplaySizeMul(editor, catalog, *delta);
				return true;
			}
		}

		const Optional<const BuildActionDef*> lineAction = FindLineBuildActionForCatalogEntry(entry, defs);
		if (lineAction && editor.buildingEditorLineActionTag != (*lineAction)->tag)
		{
			editor.buildingEditorLineActionTag = (*lineAction)->tag;
			editor.buildingEditorIconHorizontal = (*lineAction)->lineIconHorizontal;
			editor.buildingEditorIconDiagUpRight = (*lineAction)->lineIconDiagUpRight;
			editor.buildingEditorIconDiagUpLeft = (*lineAction)->lineIconDiagUpLeft;
		}

		if (lineAction)
		{
			const Array<FileFilter> imageFilters = { FileFilter::PNG(), FileFilter::JPEG(), FileFilter::BMP(), FileFilter::AllFiles() };
			const auto pickFileName = [&](int32 index)
			{
				const Optional<FilePath> path = Dialog::OpenFile(imageFilters);
				if (!path)
				{
					return;
				}

				const String fileName = FileSystem::FileName(*path);
				if (index == 0)
				{
					editor.buildingEditorIconHorizontal = fileName;
				}
				else if (index == 1)
				{
					editor.buildingEditorIconDiagUpRight = fileName;
				}
				else
				{
					editor.buildingEditorIconDiagUpLeft = fileName;
				}

				if (SaveBuildLineIconOverride(
					editor.buildingEditorLineActionTag,
					editor.buildingEditorIconHorizontal,
					editor.buildingEditorIconDiagUpRight,
					editor.buildingEditorIconDiagUpLeft,
					editor.statusText))
				{
					editor.buildLineIconsDirty = true;
				}
			};

			if (BuildingEditorLineIconBrowseRect(editor, 0).leftClicked())
			{
				pickFileName(0);
			}
			if (BuildingEditorLineIconBrowseRect(editor, 1).leftClicked())
			{
				pickFileName(1);
			}
			if (BuildingEditorLineIconBrowseRect(editor, 2).leftClicked())
			{
				pickFileName(2);
			}
		}

		const Point offsetDeltas[2][4] = {
			{ Point{ -4, 0 }, Point{ -1, 0 }, Point{ 1, 0 }, Point{ 4, 0 } },
			{ Point{ 0, -4 }, Point{ 0, -1 }, Point{ 0, 1 }, Point{ 0, 4 } },
		};
		for (int32 axisIndex = 0; axisIndex < 2; ++axisIndex)
		{
			for (int32 buttonIndex = 0; buttonIndex < 4; ++buttonIndex)
			{
				if (HandleRectButtonClick(BuildingEditorAxisButtonRect(editor, axisIndex, buttonIndex)))
				{
					ChangeSelectedUnitEditorTabOffset(editor, catalog, offsetDeltas[axisIndex][buttonIndex]);
				}
			}
		}

		if (HandleRectButtonClick(BuildingEditorResetRect(editor)))
		{
			ResetSelectedUnitEditorTabOffset(editor, catalog);
		}

		return true;
	}
}
