#pragma once
# include <Siv3D.hpp>
# include "../App/AppSceneSharedData.h"

namespace LT3
{
	// Title UI エディタのホットスポット領域を返します。
	inline RectF TitleUiEditorHotspotRect()
	{
		const Point logicalSceneSize{ 1600, 900 };
		return RectF{ logicalSceneSize.x - TitleEditorHotspotSize, logicalSceneSize.y - TitleUnderBarHeight - TitleEditorHotspotSize, TitleEditorHotspotSize, TitleEditorHotspotSize };
	}

	// Title UI エディタのパネル領域を返します。
	inline RectF TitleUiEditorPanelRect()
	{
		return RectF{ TitleEditorPanelX, TitleEditorPanelY, 320, 220 };
	}

// Title UI エディタのボタンを描画し、クリックを返します。
	inline bool HandleTitleLayoutEditorButton(const RectF& rect, const String& label, const Font& font)
	{
		const bool hovered = rect.mouseOver();
		if (hovered) Cursor::RequestStyle(CursorStyle::Hand);
		rect.draw(ColorF{ 0.14, 0.18, 0.24, 0.92 }).drawFrame(2.0, hovered ? ColorF{ 1.0, 0.84, 0.0, 0.95 } : ColorF{ 1.0, 1.0, 1.0, 0.22 });
		font(label).drawAt(18, rect.center(), Palette::White);
		return rect.leftClicked();
	}

	// 指定要素に対応する編集対象矩形を返します。
	inline RectF& EditableTitleLayoutRect(AppSharedData& data, TitleUiEditableElement element)
	{
		return (element == TitleUiEditableElement::MusicEditorToggle) ? data.titleUiLayout.musicEditorToggleRect : data.titleUiLayout.skirmishButtonRect;
	}

	// 指定要素の表示名を返します。
	inline StringView EditableTitleLayoutLabel(TitleUiEditableElement element)
	{
		return (element == TitleUiEditableElement::MusicEditorToggle) ? U"Music Editor" : U"Skirmish";
	}

	// Title UI レイアウト編集入力を処理します。
	inline void UpdateTitleLayoutEditor(AppSharedData& data)
	{
		TitleUiEditorState& editor = data.titleUiEditor;
		RepairTitleUiLayout(data.titleUiLayout);
		const RectF hotspotRect = TitleUiEditorHotspotRect();
		if (hotspotRect.mouseOver()) Cursor::RequestStyle(CursorStyle::Hand);
		if (hotspotRect.leftClicked())
		{
			editor.open = !editor.open;
			editor.statusText = editor.open ? U"Title UI Editor opened" : U"Title UI Editor closed";
			return;
		}
		if (!editor.open) return;

		const RectF panelRect = TitleUiEditorPanelRect();
		const auto saveGridChange = [&](int32 delta)
		{
			const TitleUiLayout before = data.titleUiLayout;
			data.titleUiLayout.gridSize = Clamp(data.titleUiLayout.gridSize + delta, 8, 160);
			if (SaveTitleUiLayoutToml(data.titleUiLayout)) editor.statusText = U"Grid: {}"_fmt(data.titleUiLayout.gridSize);
			else { data.titleUiLayout = before; editor.statusText = U"Title UI layout save failed; grid change reverted"; }
		};
		if (HandleTitleLayoutEditorButton(RectF{ panelRect.x + 16, panelRect.y + 44, 36, 32 }, U"-", data.uiFont)) { saveGridChange(-8); return; }
		if (HandleTitleLayoutEditorButton(RectF{ panelRect.x + 188, panelRect.y + 44, 36, 32 }, U"+", data.uiFont)) { saveGridChange(8); return; }

		const Array<TitleUiEditableElement> elements = { TitleUiEditableElement::SkirmishButton, TitleUiEditableElement::MusicEditorToggle };
		for (int32 i = 0; i < static_cast<int32>(elements.size()); ++i)
		{
			if (RectF{ panelRect.x + 16, panelRect.y + 92 + i * 40.0, 208, 30 }.leftClicked())
			{
				editor.selectedElement = elements[i];
				editor.statusText = U"Selected: {}"_fmt(EditableTitleLayoutLabel(elements[i]));
				return;
			}
		}
		if (!editor.selectedElement) return;

		RectF& target = EditableTitleLayoutRect(data, *editor.selectedElement);
		const RectF resizeHandle{ target.x + target.w - 12, target.y, 12, target.h };
		if (!editor.dragOffset && !editor.resizing && MouseL.down() && resizeHandle.mouseOver())
		{
			editor.layoutBeforePointerEdit = data.titleUiLayout;
			editor.resizing = true;
			editor.resizeAnchorLeft = target.x;
			return;
		}
		if (!editor.resizing && !editor.dragOffset && MouseL.down() && target.mouseOver())
		{
			editor.layoutBeforePointerEdit = data.titleUiLayout;
			editor.dragOffset = Cursor::PosF() - target.pos;
			return;
		}
		if (editor.resizing && MouseL.pressed())
		{
			target.w = Max(TitleUiEditorMinButtonWidth, SnapTitleUiScalar(Cursor::PosF().x - editor.resizeAnchorLeft, data.titleUiLayout.gridSize));
			editor.statusText = U"Resized: {}"_fmt(EditableTitleLayoutLabel(*editor.selectedElement));
		}
		else if (editor.dragOffset && MouseL.pressed())
		{
			const Point scene{ 1600, 900 };
			const Vec2 snapped = SnapTitleUiPosition(Cursor::PosF() - *editor.dragOffset, data.titleUiLayout.gridSize);
			target.x = Clamp(snapped.x, 0.0, Max(0.0, scene.x - target.w));
			target.y = Clamp(snapped.y, 0.0, Max(0.0, scene.y - TitleUnderBarHeight - target.h));
			editor.statusText = U"Moved: {}"_fmt(EditableTitleLayoutLabel(*editor.selectedElement));
		}
		if (MouseL.up())
		{
			if (editor.dragOffset || editor.resizing)
			{
				RepairTitleUiLayout(data.titleUiLayout);
				if (SaveTitleUiLayoutToml(data.titleUiLayout)) editor.statusText = U"Title UI layout saved";
				else if (editor.layoutBeforePointerEdit) { data.titleUiLayout = *editor.layoutBeforePointerEdit; editor.statusText = U"Title UI layout save failed; changes reverted"; }
			}
			editor.dragOffset.reset();
			editor.resizing = false;
			editor.layoutBeforePointerEdit.reset();
		}
	}

	// Title UI レイアウト編集UIを描画します。
	inline void DrawTitleLayoutEditor(const AppSharedData& data)
	{
		const RectF hotspot = TitleUiEditorHotspotRect();
		if (hotspot.mouseOver()) hotspot.rounded(10).draw(ColorF{ 0.25, 0.55, 0.90, 0.18 }).drawFrame(2.0, ColorF{ 0.70, 0.90, 1.0, 0.65 });
		if (hotspot.mouseOver()) data.uiFont(U"UI").drawAt(18, hotspot.center(), Palette::White);
		if (!data.titleUiEditor.open) return;
		const RectF panel = TitleUiEditorPanelRect();
		panel.rounded(8).draw(ColorF{ 0.10, 0.12, 0.16, 0.96 }).drawFrame(2.0, ColorF{ 1.0, 1.0, 1.0, 0.18 });
		data.uiFont(U"Title UI Editor").draw(panel.x + 16, panel.y + 12, Palette::White);
		data.uiFont(U"Grid").draw(panel.x + 64, panel.y + 50, Palette::Lightgray);
		data.uiFont(U"{} px"_fmt(data.titleUiLayout.gridSize)).draw(panel.x + 112, panel.y + 50, Palette::Orange);
		const Array<TitleUiEditableElement> elements = { TitleUiEditableElement::SkirmishButton, TitleUiEditableElement::MusicEditorToggle };
		for (int32 i = 0; i < static_cast<int32>(elements.size()); ++i)
		{
			const RectF row{ panel.x + 16, panel.y + 92 + i * 40.0, 208, 30 };
			const bool selected = data.titleUiEditor.selectedElement && *data.titleUiEditor.selectedElement == elements[i];
			row.draw(selected ? ColorF{ 0.20, 0.28, 0.42, 0.96 } : ColorF{ 0.12, 0.14, 0.20, 0.88 }).drawFrame(2.0, selected ? ColorF{ 1.0, 0.84, 0.0, 0.95 } : ColorF{ 1.0, 1.0, 1.0, 0.16 });
			data.uiFont(EditableTitleLayoutLabel(elements[i])).draw(row.x + 10, row.y + 5, Palette::White);
			const RectF& target = (elements[i] == TitleUiEditableElement::MusicEditorToggle) ? data.titleUiLayout.musicEditorToggleRect : data.titleUiLayout.skirmishButtonRect;
			target.drawFrame(2.0, selected ? ColorF{ 1.0, 0.84, 0.0, 0.95 } : ColorF{ 0.60, 0.80, 1.0, 0.55 });
			RectF{ target.x + target.w - 12, target.y, 12, target.h }.draw(selected ? ColorF{ 1.0, 0.84, 0.0, 0.60 } : ColorF{ 0.60, 0.80, 1.0, 0.30 });
		}
		data.uiFont(U"Drag button body to move").draw(panel.x + 16, panel.y + 180, Palette::Skyblue);
		data.uiFont(U"Drag right edge to resize width").draw(panel.x + 16, panel.y + 200, Palette::Skyblue);
		data.uiFont(data.titleUiEditor.statusText).draw(panel.x + 16, panel.y + 168, Palette::White);
	}
}
