#pragma once
# include <Siv3D.hpp>
# include "BuildingEditorCommon.h"

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

		if (!editor.uiLayoutEditEnabled && (BuildingEditorCloseRect(editor).leftClicked() || EditorUnitBuildingCloseRect(editor).leftClicked()))
		{
			editor.showBuildingEditor = false;
			editor.showUnitParameterEditor = false;
			editor.statusText = U"BuildingEditor OFF";
			return true;
		}

		if (BuildingEditorTabRect(editor, 0).leftClicked())
		{
			editor.buildingEditorTab = 0;
			return true;
		}
		if (BuildingEditorTabRect(editor, 1).leftClicked())
		{
			editor.buildingEditorTab = 1;
			return true;
		}
		if (BuildingEditorTabRect(editor, 2).leftClicked())
		{
			editor.buildingEditorTab = 2;
			return true;
		}
		if (BuildingEditorTabRect(editor, 3).leftClicked())
		{
			editor.buildingEditorTab = 3;
			return true;
		}
		if (BuildingEditorTabRect(editor, 4).leftClicked())
		{
			editor.buildingEditorTab = 4;
			return true;
		}

		if (!HasSelectedCatalogEntry(editor, catalog))
		{
			return true;
		}

		const UnitCatalogEntry& entry = catalog.entries[editor.selectedUnitCatalogIndex];
		if (BuildingEditorAnchorButtonRect(editor, 0).leftClicked())
		{
			SetSelectedUnitPlacementAnchor(editor, catalog, UnitPlacementAnchor::Center);
			return true;
		}
		if (BuildingEditorAnchorButtonRect(editor, 1).leftClicked())
		{
			SetSelectedUnitPlacementAnchor(editor, catalog, UnitPlacementAnchor::BottomCenter);
			return true;
		}
		if (BuildingEditorRenderSizeModeButtonRect(editor, 0).leftClicked())
		{
			SetSelectedUnitRenderSizeMode(editor, catalog, UnitRenderSizeMode::Gameplay);
			return true;
		}
		if (BuildingEditorRenderSizeModeButtonRect(editor, 1).leftClicked())
		{
			SetSelectedUnitRenderSizeMode(editor, catalog, UnitRenderSizeMode::Art);
			return true;
		}

		const bool artMode = (entry.renderSizeMode == UnitRenderSizeMode::Art);
		if (artMode)
		{
			if (BuildingEditorArtWidthRefButtonRect(editor, 0).leftClicked())
			{
				SetSelectedUnitArtWidthReference(editor, catalog, UnitArtWidthReference::Cell);
				return true;
			}
			if (BuildingEditorArtWidthRefButtonRect(editor, 1).leftClicked())
			{
				SetSelectedUnitArtWidthReference(editor, catalog, UnitArtWidthReference::Pixel);
				return true;
			}
			if (BuildingEditorKeepAspectButtonRect(editor).leftClicked())
			{
				ToggleSelectedUnitArtKeepAspect(editor, catalog);
				return true;
			}
			if (BuildingEditorSizeValueButtonRect(editor, 0).leftClicked())
			{
				ChangeSelectedUnitArtWidthValueForTab(editor, catalog, editor.buildingEditorTab, -0.4);
				return true;
			}
			if (BuildingEditorSizeValueButtonRect(editor, 1).leftClicked())
			{
				ChangeSelectedUnitArtWidthValueForTab(editor, catalog, editor.buildingEditorTab, -0.1);
				return true;
			}
			if (BuildingEditorSizeValueButtonRect(editor, 2).leftClicked())
			{
				ChangeSelectedUnitArtWidthValueForTab(editor, catalog, editor.buildingEditorTab, 0.1);
				return true;
			}
			if (BuildingEditorSizeValueButtonRect(editor, 3).leftClicked())
			{
				ChangeSelectedUnitArtWidthValueForTab(editor, catalog, editor.buildingEditorTab, 0.4);
				return true;
			}
		}
		else
		{
			if (BuildingEditorSizeValueButtonRect(editor, 0).leftClicked())
			{
				ChangeSelectedUnitGameplaySizeMul(editor, catalog, -0.4);
				return true;
			}
			if (BuildingEditorSizeValueButtonRect(editor, 1).leftClicked())
			{
				ChangeSelectedUnitGameplaySizeMul(editor, catalog, -0.1);
				return true;
			}
			if (BuildingEditorSizeValueButtonRect(editor, 2).leftClicked())
			{
				ChangeSelectedUnitGameplaySizeMul(editor, catalog, 0.1);
				return true;
			}
			if (BuildingEditorSizeValueButtonRect(editor, 3).leftClicked())
			{
				ChangeSelectedUnitGameplaySizeMul(editor, catalog, 0.4);
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

		const bool shadowTab = (editor.buildingEditorTab == 1);
		const bool lineHorizontalTab = (editor.buildingEditorTab == 2);
		const bool lineRightTab = (editor.buildingEditorTab == 3);
		const bool lineLeftTab = (editor.buildingEditorTab == 4);

		if (BuildingEditorAxisButtonRect(editor, 0, 0).leftClicked())
		{
			if (shadowTab)
			{
				ChangeSelectedUnitShadowOffset(editor, catalog, Point{ -4, 0 });
			}
			else if (lineHorizontalTab)
			{
				ChangeSelectedUnitLineIconHorizontalOffset(editor, catalog, Point{ -4, 0 });
			}
			else if (lineRightTab)
			{
				ChangeSelectedUnitLineIconDiagUpRightOffset(editor, catalog, Point{ -4, 0 });
			}
			else if (lineLeftTab)
			{
				ChangeSelectedUnitLineIconDiagUpLeftOffset(editor, catalog, Point{ -4, 0 });
			}
			else
			{
				ChangeSelectedUnitVisualOffset(editor, catalog, Point{ -4, 0 });
			}
		}
		if (BuildingEditorAxisButtonRect(editor, 0, 1).leftClicked())
		{
			if (shadowTab)
			{
				ChangeSelectedUnitShadowOffset(editor, catalog, Point{ -1, 0 });
			}
			else if (lineHorizontalTab)
			{
				ChangeSelectedUnitLineIconHorizontalOffset(editor, catalog, Point{ -1, 0 });
			}
			else if (lineRightTab)
			{
				ChangeSelectedUnitLineIconDiagUpRightOffset(editor, catalog, Point{ -1, 0 });
			}
			else if (lineLeftTab)
			{
				ChangeSelectedUnitLineIconDiagUpLeftOffset(editor, catalog, Point{ -1, 0 });
			}
			else
			{
				ChangeSelectedUnitVisualOffset(editor, catalog, Point{ -1, 0 });
			}
		}
		if (BuildingEditorAxisButtonRect(editor, 0, 2).leftClicked())
		{
			if (shadowTab)
			{
				ChangeSelectedUnitShadowOffset(editor, catalog, Point{ 1, 0 });
			}
			else if (lineHorizontalTab)
			{
				ChangeSelectedUnitLineIconHorizontalOffset(editor, catalog, Point{ 1, 0 });
			}
			else if (lineRightTab)
			{
				ChangeSelectedUnitLineIconDiagUpRightOffset(editor, catalog, Point{ 1, 0 });
			}
			else if (lineLeftTab)
			{
				ChangeSelectedUnitLineIconDiagUpLeftOffset(editor, catalog, Point{ 1, 0 });
			}
			else
			{
				ChangeSelectedUnitVisualOffset(editor, catalog, Point{ 1, 0 });
			}
		}
		if (BuildingEditorAxisButtonRect(editor, 0, 3).leftClicked())
		{
			if (shadowTab)
			{
				ChangeSelectedUnitShadowOffset(editor, catalog, Point{ 4, 0 });
			}
			else if (lineHorizontalTab)
			{
				ChangeSelectedUnitLineIconHorizontalOffset(editor, catalog, Point{ 4, 0 });
			}
			else if (lineRightTab)
			{
				ChangeSelectedUnitLineIconDiagUpRightOffset(editor, catalog, Point{ 4, 0 });
			}
			else if (lineLeftTab)
			{
				ChangeSelectedUnitLineIconDiagUpLeftOffset(editor, catalog, Point{ 4, 0 });
			}
			else
			{
				ChangeSelectedUnitVisualOffset(editor, catalog, Point{ 4, 0 });
			}
		}

		if (BuildingEditorAxisButtonRect(editor, 1, 0).leftClicked())
		{
			if (shadowTab)
			{
				ChangeSelectedUnitShadowOffset(editor, catalog, Point{ 0, -4 });
			}
			else if (lineHorizontalTab)
			{
				ChangeSelectedUnitLineIconHorizontalOffset(editor, catalog, Point{ 0, -4 });
			}
			else if (lineRightTab)
			{
				ChangeSelectedUnitLineIconDiagUpRightOffset(editor, catalog, Point{ 0, -4 });
			}
			else if (lineLeftTab)
			{
				ChangeSelectedUnitLineIconDiagUpLeftOffset(editor, catalog, Point{ 0, -4 });
			}
			else
			{
				ChangeSelectedUnitVisualOffset(editor, catalog, Point{ 0, -4 });
			}
		}
		if (BuildingEditorAxisButtonRect(editor, 1, 1).leftClicked())
		{
			if (shadowTab)
			{
				ChangeSelectedUnitShadowOffset(editor, catalog, Point{ 0, -1 });
			}
			else if (lineHorizontalTab)
			{
				ChangeSelectedUnitLineIconHorizontalOffset(editor, catalog, Point{ 0, -1 });
			}
			else if (lineRightTab)
			{
				ChangeSelectedUnitLineIconDiagUpRightOffset(editor, catalog, Point{ 0, -1 });
			}
			else if (lineLeftTab)
			{
				ChangeSelectedUnitLineIconDiagUpLeftOffset(editor, catalog, Point{ 0, -1 });
			}
			else
			{
				ChangeSelectedUnitVisualOffset(editor, catalog, Point{ 0, -1 });
			}
		}
		if (BuildingEditorAxisButtonRect(editor, 1, 2).leftClicked())
		{
			if (shadowTab)
			{
				ChangeSelectedUnitShadowOffset(editor, catalog, Point{ 0, 1 });
			}
			else if (lineHorizontalTab)
			{
				ChangeSelectedUnitLineIconHorizontalOffset(editor, catalog, Point{ 0, 1 });
			}
			else if (lineRightTab)
			{
				ChangeSelectedUnitLineIconDiagUpRightOffset(editor, catalog, Point{ 0, 1 });
			}
			else if (lineLeftTab)
			{
				ChangeSelectedUnitLineIconDiagUpLeftOffset(editor, catalog, Point{ 0, 1 });
			}
			else
			{
				ChangeSelectedUnitVisualOffset(editor, catalog, Point{ 0, 1 });
			}
		}
		if (BuildingEditorAxisButtonRect(editor, 1, 3).leftClicked())
		{
			if (shadowTab)
			{
				ChangeSelectedUnitShadowOffset(editor, catalog, Point{ 0, 4 });
			}
			else if (lineHorizontalTab)
			{
				ChangeSelectedUnitLineIconHorizontalOffset(editor, catalog, Point{ 0, 4 });
			}
			else if (lineRightTab)
			{
				ChangeSelectedUnitLineIconDiagUpRightOffset(editor, catalog, Point{ 0, 4 });
			}
			else if (lineLeftTab)
			{
				ChangeSelectedUnitLineIconDiagUpLeftOffset(editor, catalog, Point{ 0, 4 });
			}
			else
			{
				ChangeSelectedUnitVisualOffset(editor, catalog, Point{ 0, 4 });
			}
		}

		if (BuildingEditorResetRect(editor).leftClicked())
		{
			if (shadowTab)
			{
				ResetSelectedUnitShadowOffset(editor, catalog);
			}
			else if (lineHorizontalTab)
			{
				ResetSelectedUnitLineIconHorizontalOffset(editor, catalog);
			}
			else if (lineRightTab)
			{
				ResetSelectedUnitLineIconDiagUpRightOffset(editor, catalog);
			}
			else if (lineLeftTab)
			{
				ResetSelectedUnitLineIconDiagUpLeftOffset(editor, catalog);
			}
			else
			{
				ResetSelectedUnitVisualOffset(editor, catalog);
			}
		}

		return true;
	}
}
