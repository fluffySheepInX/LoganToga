#pragma once
# include <Siv3D.hpp>
# include "SkillEditorCommon.h"

namespace LT3
{
	inline void DrawSkillEditorInfoIcon(const RectF& rect, const FilePath& iconPath, StringView fallbackText, const Font& uiFont)
	{
		rect.draw(ColorF{ 0.05, 0.06, 0.08, 0.84 }).drawFrame(1, rect.mouseOver() ? ColorF{ 1.0, 0.84, 0.0 } : ColorF{ 1, 1, 1, 0.16 });
		if (!iconPath.isEmpty() && FileSystem::Exists(iconPath))
		{
			auto& cache = BuildingEditorTextureCache();
			if (!cache.contains(iconPath))
			{
				cache.emplace(iconPath, Texture{ iconPath });
			}

			cache.at(iconPath).resized(static_cast<int32>(rect.w - 2.0), static_cast<int32>(rect.h - 2.0)).drawAt(rect.center());
		}
		else
		{
			uiFont(fallbackText).drawAt(12, rect.center(), Palette::White);
		}
	}

	inline void DrawSkillEditorTooltip(const Font& uiFont, StringView title, const Array<String>& lines, Optional<double> explicitX = none)
	{
		if (lines.isEmpty())
		{
			return;
		}

		const double width = 360.0;
		const double height = 34.0 + lines.size() * 18.0;
		const Vec2 defaultPos = Cursor::PosF() + Vec2{ 16.0, 14.0 };
		const Vec2 pos = explicitX ? Vec2{ *explicitX, defaultPos.y } : defaultPos;
		const RectF tip{ pos, width, height };
		tip.draw(ColorF{ 0.02, 0.03, 0.045, 0.97 }).drawFrame(1, ColorF{ 1, 1, 1, 0.22 });
		uiFont(title).draw(12, tip.x + 8.0, tip.y + 7.0, Palette::Aqua);
		for (int32 i = 0; i < static_cast<int32>(lines.size()); ++i)
		{
			uiFont(lines[i]).draw(10, tip.x + 8.0, tip.y + 28.0 + i * 18.0, Palette::White);
		}
	}

	inline void DrawSkillEditorValueRow(const Font& uiFont, const MapEditorState& editor, const SkillDef& skill, int32 row, StringView label, StringView value, double scroll, String& hoverHelpText, Optional<double>& hoverHelpTooltipX, String& hoverNoteText, Optional<double>& hoverNoteTooltipX)
	{
		const RectF rowRect = SkillEditorValueRowRect(row, scroll);
		const RectF viewport = SkillEditorDetailViewportRect();
		if (rowRect.y + rowRect.h < viewport.y || viewport.y + viewport.h < rowRect.y)
		{
			return;
		}
		const bool locked = IsSkillEditorValueRowLocked(skill, row);
		uiFont(label).draw(11, rowRect.x + 8.0, rowRect.y + 4.0, locked ? ColorF{ 0.62, 0.66, 0.72 } : ColorF{ Palette::Aqua });
		const String shownValue = (editor.skillValueEditingRow == row) ? editor.skillValueEditingText : String{ value };
		const String stepText = U"x{}"_fmt(SkillEditorValueStep(editor, row));
		DrawRectNumberStepper(SkillEditorValueStepperRects(row, scroll), shownValue, stepText, editor.skillValueEditingRow == row, editor.skillValueStepMenuRow && *editor.skillValueStepMenuRow == row, !locked, uiFont);
		const RectF helpRect = SkillEditorValueHelpIconRect(row, scroll);
		DrawSkillEditorInfoIcon(helpRect, SkillEditorHelpIconPath(), U"?", uiFont);
		if (helpRect.mouseOver() && row < static_cast<int32>(SkillEditorValueHelpTexts().size()))
		{
			hoverHelpText = SkillEditorValueHelpTexts()[row];
			hoverHelpTooltipX = helpRect.x - 360.0;
		}
		const bool showNoteIcon = locked || (row == 20 && skill.projectileMotion == SkillProjectileMotion::Swing);
		if (showNoteIcon)
		{
			const RectF noteRect = SkillEditorValueNoteIconRect(row, scroll);
			DrawSkillEditorInfoIcon(noteRect, SkillEditorHelpIconPath(), U"?", uiFont);
			if (noteRect.mouseOver())
			{
				hoverNoteText = SkillEditorValueRowLockNote(skill, row);
				hoverNoteTooltipX = noteRect.x - 360.0;
			}
		}
	}
}
