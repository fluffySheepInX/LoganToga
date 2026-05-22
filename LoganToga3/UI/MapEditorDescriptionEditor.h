#pragma once
# include <Siv3D.hpp>
# include "MapEditorTypes.h"
# include "RectUiHelpers.h"
# include "../Data/Loaders/BuildActionDefLoader.h"
# include "SkillEditorCommon.h"

namespace LT3
{
	inline bool IsDescriptionEditorOpen(const MapEditorState& editor)
	{
		return editor.descriptionEditorTargetKind != DescriptionEditorTargetKind::None;
	}

	inline void CloseDescriptionEditor(MapEditorState& editor)
	{
		editor.descriptionEditorTargetKind = DescriptionEditorTargetKind::None;
		editor.descriptionEditorTargetIndex = -1;
		editor.descriptionEditorTitle.clear();
		editor.descriptionEditorText.clear();
	}

	inline void OpenDescriptionEditor(MapEditorState& editor, DescriptionEditorTargetKind kind, int32 index, StringView title, StringView text)
	{
		editor.descriptionEditorTargetKind = kind;
		editor.descriptionEditorTargetIndex = index;
		editor.descriptionEditorTitle = title;
		editor.descriptionEditorText = text;
	}

	inline bool CommitDescriptionEditor(MapEditorState& editor, UnitCatalog& catalog, DefinitionStores& defs)
	{
		const int32 index = editor.descriptionEditorTargetIndex;
		switch (editor.descriptionEditorTargetKind)
		{
		case DescriptionEditorTargetKind::Command:
			if (0 <= index && index < static_cast<int32>(defs.buildActions.size()))
			{
				defs.buildActions[index].description = editor.descriptionEditorText;
				SaveBuildActionDefinitions(defs, editor.statusText);
				editor.commandBindingsDirty = false;
				CloseDescriptionEditor(editor);
				return true;
			}
			break;
		case DescriptionEditorTargetKind::Unit:
			if (0 <= index && index < static_cast<int32>(catalog.entries.size()))
			{
				catalog.entries[index].description = editor.descriptionEditorText;
				SaveUnitCatalogToml(catalog, editor.statusText);
				editor.unitCatalogDirty = true;
				CloseDescriptionEditor(editor);
				return true;
			}
			break;
		case DescriptionEditorTargetKind::Skill:
			if (0 <= index && index < static_cast<int32>(defs.skills.size()))
			{
				defs.skills[index].description = editor.descriptionEditorText;
				SaveSkillEditorDefinitions(editor, defs);
				CloseDescriptionEditor(editor);
				return true;
			}
			break;
		default:
			break;
		}

		CloseDescriptionEditor(editor);
		return true;
	}

	inline bool ProcessDescriptionEditorInput(MapEditorState& editor, UnitCatalog& catalog, DefinitionStores& defs)
	{
		if (!IsDescriptionEditorOpen(editor))
		{
			return false;
		}

		TextInput::UpdateText(editor.descriptionEditorText);
		if (KeyEnter.down())
		{
			editor.descriptionEditorText += U"\n";
		}

		if ((KeyControl | KeyCommand).pressed() && KeyV.down())
		{
			String clip;
			if (Clipboard::GetText(clip) && !clip.isEmpty())
			{
				editor.descriptionEditorText += clip;
			}
		}

		if (HandleRectButtonClick(EditorDescriptionCopyRect()))
		{
			Clipboard::SetText(editor.descriptionEditorText);
			editor.statusText = U"Description copied";
			return true;
		}

		if (HandleRectButtonClick(EditorDescriptionPasteRect()))
		{
			String clip;
			if (Clipboard::GetText(clip))
			{
				editor.descriptionEditorText += clip;
				editor.statusText = U"Description pasted";
			}
			return true;
		}

		if (HandleRectButtonClick(EditorDescriptionSaveRect()))
		{
			return CommitDescriptionEditor(editor, catalog, defs);
		}

		if (HandleRectButtonClick(EditorDescriptionCancelRect()) || KeyEscape.down())
		{
			CloseDescriptionEditor(editor);
			return true;
		}

		return true;
	}

	inline void DrawDescriptionEditor(const MapEditorState& editor, const Font& uiFont)
	{
		if (!IsDescriptionEditorOpen(editor))
		{
			return;
		}

		RectF{ 0.0, 0.0, static_cast<double>(Scene::Width()), static_cast<double>(Scene::Height()) }.draw(ColorF{ 0.0, 0.0, 0.0, 0.32 });
		const RectF panel = EditorDescriptionPanelRect();
		const RectF textRect = EditorDescriptionTextRect();
		panel.draw(ColorF{ 0.02, 0.03, 0.045, 0.98 }).drawFrame(2, ColorF{ 1.0, 0.84, 0.0, 0.80 });
		uiFont(U"説明文編集").draw(16, panel.x + 18.0, panel.y + 14.0, Palette::White);
		uiFont(editor.descriptionEditorTitle).draw(11, panel.x + 18.0, panel.y + 40.0, Palette::Lightgray);
		textRect.draw(ColorF{ 0.05, 0.06, 0.08, 0.96 }).drawFrame(1, ColorF{ 1, 1, 1, 0.16 });
		uiFont(editor.descriptionEditorText + U"|").draw(13, textRect.x + 8.0, textRect.y + 8.0, Palette::White);
		uiFont(U"Enter:改行  Save:保存  Esc/Cancel:破棄").draw(10, textRect.x, textRect.y + textRect.h + 8.0, ColorF{ 1, 1, 1, 0.55 });
		DrawRectButton(EditorDescriptionCopyRect(), U"既存をコピー", false, uiFont, RectButtonStyle{ .fontSize = 11 });
		DrawRectButton(EditorDescriptionPasteRect(), U"貼り付け", false, uiFont, RectButtonStyle{ .fontSize = 11 });
		DrawRectButton(EditorDescriptionSaveRect(), U"Save", false, uiFont, RectButtonStyle{ .fontSize = 11 });
		DrawRectButton(EditorDescriptionCancelRect(), U"Cancel", false, uiFont, RectButtonStyle{ .fontSize = 11 });
	}
}
