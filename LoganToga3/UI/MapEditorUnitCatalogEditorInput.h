#pragma once
# include "MapEditorUnitCatalogEditorInput.ParamEditor.h"
# include "MapEditorUnitCatalogEditorInput.UniqueEditor.h"

namespace LT3
{
	inline bool ProcessUnitCatalogEditorInput(MapEditorState& editor, UnitCatalog& catalog)
	{
		bool consumed = false;

		if (ProcessUnitCatalogRenameInput(editor, catalog))
		{
			return true;
		}

		if (ProcessUnitCatalogContextMenuInput(editor, catalog))
		{
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

		if (ProcessUnitCatalogListInput(editor, catalog, consumed))
		{
			return true;
		}

		if (ProcessUnitCatalogParamEditorInput(editor, catalog, consumed))
		{
			return true;
		}

		if (ProcessUnitCatalogUniqueEditorInput(editor, catalog, consumed))
		{
			return true;
		}

		return consumed;
	}
}
