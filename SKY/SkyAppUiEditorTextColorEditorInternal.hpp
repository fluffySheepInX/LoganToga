# pragma once
# include "SkyAppUiPanelFrameInternal.hpp"
# include "SkyAppUiEditorTextColorsInternal.hpp"
# include "MainUi.hpp"

namespace SkyAppSupport
{
	namespace UiInternal
	{
		struct EditorTextColorEditorState
		{
			bool isOpen = false;
			EditorTextColorSlot selectedSlot = EditorTextColorSlot::DarkPrimary;
			MainSupport::EditorTextColorSettings originalSettings;
			MainSupport::EditorTextColorSettings workingSettings;
			bool initializedForOpen = false;
			String statusMessage;
			double statusMessageUntil = 0.0;
		};

		inline void DrawEditorTextColorPreview(const RectF& rect, const MainSupport::EditorTextColorSettings& settings)
		{
			const RectF darkPreview{ rect.x, rect.y, rect.w, 72 };
			const RectF panelPreview{ rect.x, rect.y + 84, rect.w, 72 };
			const RectF cardPreview{ rect.x, rect.y + 168, rect.w, 72 };
			const RectF selectedPreview{ rect.x, rect.y + 252, rect.w, 72 };

			darkPreview.rounded(12).draw(ColorF{ 0.08, 0.13, 0.21, 0.98 });
			darkPreview.rounded(12).drawFrame(1.5, 0, ColorF{ 0.56, 0.72, 0.92, 0.72 });
			SimpleGUI::GetFont()(U"Dark surface preview").draw(darkPreview.pos.movedBy(12, 10), settings.darkPrimary);
			SimpleGUI::GetFont()(U"secondary text").draw(darkPreview.pos.movedBy(12, 34), settings.darkSecondary);
			SimpleGUI::GetFont()(U"accent / guide text").draw(darkPreview.pos.movedBy(12, 54), settings.darkAccent);

			panelPreview.rounded(12).draw(ColorF{ 0.20, 0.20, 0.22, 0.90 });
			panelPreview.rounded(12).drawFrame(1.5, 0, ColorF{ 0.86, 0.82, 0.52, 0.80 });
			SimpleGUI::GetFont()(U"Panel surface preview").draw(panelPreview.pos.movedBy(12, 10), settings.lightPrimary);
			SimpleGUI::GetFont()(U"secondary text").draw(panelPreview.pos.movedBy(12, 34), settings.lightSecondary);
			SimpleGUI::GetFont()(U"accent / guide text").draw(panelPreview.pos.movedBy(12, 54), settings.lightAccent);

			cardPreview.rounded(12).draw(ColorF{ 0.97, 0.96, 0.94, 0.98 });
			cardPreview.rounded(12).drawFrame(1.5, 0, ColorF{ 0.42, 0.46, 0.54, 0.72 });
			SimpleGUI::GetFont()(U"Card surface preview").draw(cardPreview.pos.movedBy(12, 10), settings.cardPrimary);
			SimpleGUI::GetFont()(U"secondary text").draw(cardPreview.pos.movedBy(12, 34), settings.cardSecondary);

			selectedPreview.rounded(12).draw(ColorF{ 0.33, 0.53, 0.82, 0.98 });
			selectedPreview.rounded(12).drawFrame(1.5, 0, ColorF{ 0.20, 0.32, 0.52, 0.90 });
			SimpleGUI::GetFont()(U"Selected surface preview").draw(selectedPreview.pos.movedBy(12, 10), settings.selectedPrimary);
			SimpleGUI::GetFont()(U"secondary text").draw(selectedPreview.pos.movedBy(12, 34), settings.selectedSecondary);
		}

		inline void DrawEditorTextColorEditor(EditorTextColorEditorState& state)
		{
			if (not state.isOpen)
			{
				return;
			}

			if (not state.initializedForOpen)
			{
				state.originalSettings = MainSupport::GetEditorTextColorSettings();
				state.workingSettings = state.originalSettings;
				state.initializedForOpen = true;
			}

			Scene::Rect().draw(ColorF{ 0.0, 0.0, 0.0, 0.36 });
			const RectF panel{ Arg::center = Scene::CenterF(), 660, 572 };
           if (const auto& ninePatch = GetConfiguredPanelNinePatch(MainSupport::PanelSkinTarget::ToolModal))
			{
				ninePatch->draw(panel);
			}
			else
			{
				panel.rounded(20).draw(ColorF{ 0.08, 0.11, 0.16, 0.98 });
			}
			panel.rounded(20).drawFrame(2, 0, ColorF{ 0.74, 0.84, 0.96, 0.86 });
			SimpleGUI::GetFont()(U"Editor Text Colors").draw(panel.pos.movedBy(20, 16), Palette::White);
			SimpleGUI::GetFont()(U"editor 系 UI の文字色を共通管理します").draw(panel.pos.movedBy(22, 44), ColorF{ 0.82, 0.89, 0.98, 0.92 });

			const Array<EditorTextColorSlot> slots{
				EditorTextColorSlot::DarkPrimary,
				EditorTextColorSlot::DarkSecondary,
				EditorTextColorSlot::DarkAccent,
				EditorTextColorSlot::LightPrimary,
				EditorTextColorSlot::LightSecondary,
				EditorTextColorSlot::LightAccent,
				EditorTextColorSlot::CardPrimary,
				EditorTextColorSlot::CardSecondary,
				EditorTextColorSlot::SelectedPrimary,
				EditorTextColorSlot::SelectedSecondary,
				EditorTextColorSlot::Warning,
				EditorTextColorSlot::Error,
			};

			for (size_t i = 0; i < slots.size(); ++i)
			{
				const RectF slotButton{ panel.x + 20, panel.y + 82 + static_cast<double>(i) * 30.0, 176, 24 };
				const bool selected = (state.selectedSlot == slots[i]);
				const bool hovered = slotButton.mouseOver();
				slotButton.rounded(10).draw(selected
					? ColorF{ 0.24, 0.40, 0.66, 0.96 }
					: (hovered ? ColorF{ 0.14, 0.22, 0.34, 0.94 } : ColorF{ 0.10, 0.16, 0.26, 0.90 }));
				slotButton.rounded(10).drawFrame(1.5, 0, selected ? ColorF{ 0.90, 0.95, 1.0, 0.94 } : ColorF{ 0.46, 0.56, 0.70, 0.70 });
				SimpleGUI::GetFont()(ToLabel(slots[i])).draw(slotButton.pos.movedBy(12, 3), Palette::White);
				if (hovered && MouseL.down())
				{
					state.selectedSlot = slots[i];
				}
			}

			DrawEditorTextColorPreview(RectF{ panel.x + 216, panel.y + 82, 420, 324 }, state.workingSettings);

			ColorF& editingColor = GetEditorTextColor(state.workingSettings, state.selectedSlot);
			RectF{ panel.x + 216, panel.y + 418, 420, 46 }.rounded(12).draw(ColorF{ 0.12, 0.16, 0.22, 0.96 });
			RectF{ panel.x + 228, panel.y + 428, 52, 26 }.rounded(8).draw(editingColor).drawFrame(1, 0, Palette::White);
			SimpleGUI::GetFont()(U"Selected: {}"_fmt(ToLabel(state.selectedSlot))).draw(Vec2{ panel.x + 294, panel.y + 428 }, Palette::White);

			SimpleGUI::Slider(U"R: {:.2f}"_fmt(editingColor.r), editingColor.r, 0.0, 1.0, Vec2{ panel.x + 216, panel.y + 474 }, 70, 320);
			SimpleGUI::Slider(U"G: {:.2f}"_fmt(editingColor.g), editingColor.g, 0.0, 1.0, Vec2{ panel.x + 216, panel.y + 500 }, 70, 320);
			SimpleGUI::Slider(U"B: {:.2f}"_fmt(editingColor.b), editingColor.b, 0.0, 1.0, Vec2{ panel.x + 216, panel.y + 526 }, 70, 320);
			SimpleGUI::Slider(U"A: {:.2f}"_fmt(editingColor.a), editingColor.a, 0.0, 1.0, Vec2{ panel.x + 216, panel.y + 552 }, 70, 320);

			MainSupport::GetMutableEditorTextColorSettings() = state.workingSettings;

			const Rect saveButton{ static_cast<int32>(panel.x + 20), static_cast<int32>(panel.y + 528), 126, 30 };
			const Rect resetButton{ static_cast<int32>(panel.x + 154), static_cast<int32>(panel.y + 528), 88, 30 };
			const Rect closeButton{ static_cast<int32>(panel.x + 250), static_cast<int32>(panel.y + 528), 88, 30 };

			if (MainSupport::DrawTextButton(saveButton, U"Save TOML"))
			{
				state.statusMessage = MainSupport::SaveEditorTextColorSettings(state.workingSettings)
					? U"Saved: {}"_fmt(MainSupport::EditorTextColorSettingsPath)
					: U"Save failed";
				state.statusMessageUntil = (Scene::Time() + 2.5);
				state.originalSettings = state.workingSettings;
			}

			if (MainSupport::DrawTextButton(resetButton, U"Reset"))
			{
				state.workingSettings = MainSupport::EditorTextColorSettings{};
				MainSupport::GetMutableEditorTextColorSettings() = state.workingSettings;
			}

			if (MainSupport::DrawTextButton(closeButton, U"Close"))
			{
				MainSupport::GetMutableEditorTextColorSettings() = state.originalSettings;
				state.workingSettings = state.originalSettings;
				state.initializedForOpen = false;
				state.isOpen = false;
				return;
			}

			if (Scene::Time() < state.statusMessageUntil)
			{
				SimpleGUI::GetFont()(state.statusMessage).draw(panel.pos.movedBy(350, 18), ColorF{ 1.0, 0.94, 0.72, 0.96 });
			}

			if (KeyEscape.down())
			{
				MainSupport::GetMutableEditorTextColorSettings() = state.originalSettings;
				state.workingSettings = state.originalSettings;
				state.initializedForOpen = false;
				state.isOpen = false;
			}
		}

		[[nodiscard]] inline EditorTextColorEditorState& SharedEditorTextColorEditorState()
		{
			static EditorTextColorEditorState state;
			return state;
		}

		inline void OpenSharedEditorTextColorEditor()
		{
			SharedEditorTextColorEditorState().isOpen = true;
		}

		inline void DrawSharedEditorTextColorEditor()
		{
			DrawEditorTextColorEditor(SharedEditorTextColorEditorState());
		}
	}
}
