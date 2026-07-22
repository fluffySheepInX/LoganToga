# include "../stdafx.h"
# include "BuildingEditorPanel.hpp"

namespace building
{
	namespace
	{
		constexpr double PanelWidth = 380.0;
		constexpr double CollapsedIconSize = 64.0;
			constexpr double HeaderHeight = 78.0;
		constexpr double SectionGap = 10.0;
		constexpr double SectionPadding = 10.0;
		constexpr double RowHeight = 34.0;

		[[nodiscard]] RectF SectionRect(const RectF& panel, const double y, const double height)
		{
			return RectF{ panel.x + 14.0, y, panel.w - 28.0, height };
		}

		[[nodiscard]] double ContentWidth(const RectF& section)
		{
			return section.w - SectionPadding * 2.0;
		}

		void DrawSectionTitle(const Font& font, StringView title, const RectF& section)
		{
			font(title).draw(section.pos.movedBy(SectionPadding, 6), ui::GetTheme().textMuted);
		}

		void ApplyPreset(GeneratedBuilding& building, const BuildingFacadeStyle style, const BuildingRoofType roofType, const Vec2 size, const int32 floors)
		{
			building.facadeStyle = style;
			building.roofType = roofType;
			building.footprintSize = size;
			building.floors = floors;
			if (style == BuildingFacadeStyle::Warehouse)
			{
				building.materialSet.wallColor = ColorF{ 0.48, 0.50, 0.54, 1.0 };
				building.materialSet.roofColor = ColorF{ 0.22, 0.22, 0.24, 1.0 };
			}
			else if (style == BuildingFacadeStyle::Haunted)
			{
				building.materialSet.wallColor = ColorF{ 0.34, 0.36, 0.34, 1.0 };
				building.materialSet.roofColor = ColorF{ 0.12, 0.08, 0.09, 1.0 };
			}
			else
			{
				building.materialSet.wallColor = ColorF{ 0.72, 0.68, 0.60, 1.0 };
				building.materialSet.roofColor = ColorF{ 0.28, 0.12, 0.10, 1.0 };
			}
			Sanitize(building);
		}
	}

	BuildingPanelActions BuildingEditorPanel::draw(BuildingDocument& document, Optional<String> selectedBuildingId, StringView status)
	{
		BuildingPanelActions actions;
		actions.selectedBuildingId = selectedBuildingId;
		syncCollapsedIconRegistry();

		if (not m_open)
		{
			const RectF iconRect = getCollapsedIconRect();
			if (MouseR.down() && iconRect.mouseOver())
			{
				m_dragging = true;
				m_dragOffset = Cursor::PosF() - iconRect.pos;
			}
			if (not MouseR.pressed())
			{
				m_dragging = false;
			}
			if (m_dragging)
			{
				updateCollapsedIconDrag(iconRect);
			}

			iconRect.draw(ColorF{ 1.0, 1.0, 1.0, 0.02 });
			iconRect.drawFrame(2.0, Palette::Black);
			ui::editor_icon::DrawToggleIcon(m_toggleIcon, iconRect);
			if (not m_toggleIcon)
			{
				m_font(U"B").drawAt(iconRect.center(), ui::GetTheme().text);
			}
			if (iconRect.mouseOver())
			{
				ui::Tooltip(m_font, U"Expand Building Builder", Cursor::PosF().movedBy(18, 18));
			}
			if (iconRect.leftClicked())
			{
				expandFromCollapsedIcon(iconRect);
			}
			return actions;
		}

		const RectF panel = getPanelRect();
		const RectF contentRect = getPanelContentRect(panel);
		const double contentHeight = getPanelContentHeight();
		const double maxScrollY = Max(0.0, contentHeight - contentRect.h);
		if (contentRect.mouseOver())
		{
			m_scrollY = Clamp(m_scrollY - Mouse::Wheel() * ui::layout::ScrollStep, 0.0, maxScrollY);
		}
		else
		{
			m_scrollY = Clamp(m_scrollY, 0.0, maxScrollY);
		}

		ui::Panel(panel);
		m_font(U"Building Builder").draw(panel.pos.movedBy(16, 12), ui::GetTheme().text);
		m_smallFont(U"B : building editor").draw(panel.pos.movedBy(16, 40), ui::GetTheme().textMuted);

		const RectF collapseButton{ panel.x + panel.w - 74, panel.y + 10, 64, 64 };
		const RectF dragHeader{ panel.x, panel.y, panel.w, 48 };
		if (MouseL.down() && dragHeader.mouseOver() && (not collapseButton.mouseOver()))
		{
			m_dragging = true;
			m_dragOffset = Cursor::PosF() - m_panelPos;
		}
		if (MouseR.down() && collapseButton.mouseOver())
		{
			m_dragging = true;
			m_dragOffset = Cursor::PosF() - m_panelPos;
		}
		if (not (MouseL.pressed() || MouseR.pressed()))
		{
			m_dragging = false;
		}
		if (m_dragging)
		{
			m_panelPos = Cursor::PosF() - m_dragOffset;
			m_panelPos.x = Clamp(m_panelPos.x, 0.0, Max(0.0, Scene::Width() - panel.w));
			m_panelPos.y = Clamp(m_panelPos.y, 0.0, Max(0.0, Scene::Height() - panel.h));
		}

		collapseButton.draw(ColorF{ 1.0, 1.0, 1.0, 0.02 });
		collapseButton.drawFrame(2.0, Palette::Black);
		ui::editor_icon::DrawToggleIcon(m_toggleIcon, collapseButton);
		if (collapseButton.leftClicked())
		{
			closeToIcon();
			return actions;
		}

		if (contentRect.mouseOver() && MouseM.pressed())
		{
			m_scrollY = Clamp(m_scrollY - Cursor::DeltaF().y * 1.6, 0.0, maxScrollY);
		}

		{
			const ScopedRenderStates2D scissor{ RasterizerState::SolidCullNoneScissor };
			const Rect previousScissor = Graphics2D::GetScissorRect();
			Graphics2D::SetScissorRect(contentRect.asRect());

			double y = contentRect.y - m_scrollY;
			y = drawBuildingListSection(panel, y, document, actions, actions.selectedBuildingId);
			GeneratedBuilding* selectedBuilding = getSelectedBuilding(document, actions.selectedBuildingId);
			y = drawTransformSection(panel, y, selectedBuilding, actions);
			y = drawShapeSection(panel, y, selectedBuilding);
			y = drawRoofSection(panel, y, selectedBuilding);
			y = drawMaterialSection(panel, y, selectedBuilding, actions);
			y = drawPresetSection(panel, y, selectedBuilding);
			y = drawSaveLoadSection(panel, y, actions, status);

			Graphics2D::SetScissorRect(previousScissor);
		}

		if (0.0 < maxScrollY)
		{
			const RectF scrollTrack{
				panel.rightX() - ui::layout::PanelPadding - ui::layout::ScrollbarWidth,
				contentRect.y,
				ui::layout::ScrollbarWidth,
				contentRect.h
			};
			const double thumbHeight = Max(ui::layout::ScrollbarMinThumbHeight, scrollTrack.h * (contentRect.h / contentHeight));
			const double thumbTravel = Max(0.0, scrollTrack.h - thumbHeight);
			const double thumbY = scrollTrack.y + (thumbTravel * (m_scrollY / maxScrollY));
			const RectF thumbRect{ scrollTrack.x, thumbY, scrollTrack.w, thumbHeight };

			if (MouseL.down() && thumbRect.mouseOver())
			{
				m_isScrollThumbDragging = true;
				m_scrollThumbGrabOffsetY = Cursor::PosF().y - thumbRect.y;
			}
			if (not MouseL.pressed())
			{
				m_isScrollThumbDragging = false;
			}
			if (m_isScrollThumbDragging)
			{
				const double thumbTop = Clamp(Cursor::PosF().y - m_scrollThumbGrabOffsetY, scrollTrack.y, scrollTrack.y + thumbTravel);
				const double t = (thumbTravel > 0.0) ? ((thumbTop - scrollTrack.y) / thumbTravel) : 0.0;
				m_scrollY = t * maxScrollY;
			}

			scrollTrack.rounded(4).draw(ColorF{ 0.82, 0.86, 0.91, 0.8 });
			thumbRect.rounded(4).draw(ui::GetTheme().accent);
		}
		else
		{
			m_isScrollThumbDragging = false;
		}

		return actions;
	}

	bool BuildingEditorPanel::wantsMouseCapture() const
	{
		return getPanelRect().mouseOver() || getCollapsedIconRect().mouseOver();
	}

	bool BuildingEditorPanel::wantsMouseWheelCapture() const
	{
		return m_open && getPanelContentRect(getPanelRect()).mouseOver();
	}

	bool BuildingEditorPanel::isOpen() const noexcept
	{
		return m_open;
	}

	void BuildingEditorPanel::toggleOpen()
	{
		m_open = not m_open;
		syncCollapsedIconRegistry();
	}

	void BuildingEditorPanel::closeToIcon()
	{
		const RectF panel = getPanelRect();
		const RectF collapseButton{ panel.x + panel.w - 74, panel.y + 10, 64, 64 };
		const Vec2 desiredCollapsedPos = ui::editor_icon::GetAnchoredTopRightPosition(collapseButton, SizeF{ CollapsedIconSize, CollapsedIconSize });
		m_panelPos = ui::editor_icon::ResolveCollapsedIconPosition(U"BuildingEditor", desiredCollapsedPos);
		m_open = false;
		syncCollapsedIconRegistry();
	}

	void BuildingEditorPanel::syncCollapsedIconRegistry() const
	{
		if (not m_open)
		{
			ui::editor_icon::RegisterCollapsedIcon(U"BuildingEditor", getCollapsedIconRect());
		}
	}

	RectF BuildingEditorPanel::getPanelRect() const
	{
		const double height = getPanelHeight();
		return RectF{ m_panelPos.x, m_panelPos.y, PanelWidth, height };
	}

	RectF BuildingEditorPanel::getPanelContentRect(const RectF& panelRect) const
	{
		return RectF{
			panelRect.x + ui::layout::PanelPadding,
			panelRect.y + HeaderHeight,
			panelRect.w - ui::layout::PanelPadding * 2.0 - ui::layout::ScrollbarWidth - 8.0,
			panelRect.h - HeaderHeight - ui::layout::PanelPadding
		};
	}

	RectF BuildingEditorPanel::getCollapsedIconRect() const
	{
		return RectF{ m_panelPos, CollapsedIconSize, CollapsedIconSize };
	}

	void BuildingEditorPanel::updateCollapsedIconDrag(const RectF& dragRect)
	{
		const Vec2 desiredPos = Cursor::PosF() - m_dragOffset;
		m_panelPos.x = Clamp(desiredPos.x, 0.0, Max(0.0, Scene::Width() - dragRect.w));
		m_panelPos.y = Clamp(desiredPos.y, 0.0, Max(0.0, Scene::Height() - dragRect.h));
	}

	void BuildingEditorPanel::expandFromCollapsedIcon(const RectF& iconRect)
	{
		m_open = true;
		m_panelPos = ui::editor_icon::GetAnchoredTopRightPosition(iconRect, SizeF{ PanelWidth, getPanelHeight() });
		syncCollapsedIconRegistry();
	}

	double BuildingEditorPanel::getPanelHeight() const
	{
		return Min(820.0, Max(520.0, Scene::Height() - 40.0));
	}

	double BuildingEditorPanel::getPanelContentHeight() const
	{
		double y = 0.0;
		y += 178.0 + SectionGap;
		y += 152.0 + SectionGap;
		y += 188.0 + SectionGap;
		y += 116.0 + SectionGap;
		y += 190.0 + SectionGap;
		y += 96.0 + SectionGap;
		y += 104.0 + SectionGap;
		return y;
	}

	GeneratedBuilding* BuildingEditorPanel::getSelectedBuilding(BuildingDocument& document, const Optional<String>& selectedBuildingId) const
	{
		if (selectedBuildingId)
		{
			return document.findById(*selectedBuildingId);
		}
		return nullptr;
	}

	double BuildingEditorPanel::drawBuildingListSection(const RectF& panel, const double y, BuildingDocument& document, BuildingPanelActions& actions, Optional<String>& selectedBuildingId)
	{
		const RectF section = SectionRect(panel, y, 178.0);
		ui::Section(section);
		DrawSectionTitle(m_smallFont, U"Building List", section);
		const RectF newButton{ section.x + 10, section.y + 32, section.w - 20, RowHeight };
		if (ui::Button(m_smallFont, U"New Building", newButton))
		{
			actions.beginPlacement = true;
		}

		const double listY = newButton.bottomY() + 8.0;
		const size_t visibleCount = Min<size_t>(3, document.buildings.size());
		for (size_t i = 0; i < visibleCount; ++i)
		{
			const auto& building = document.buildings[i];
			const RectF row{ section.x + 10, listY + i * 30.0, section.w - 20, 28.0 };
			if (selectedBuildingId && (*selectedBuildingId == building.id))
			{
				row.rounded(6).draw(ColorF{ 0.86, 0.92, 1.0, 1.0 });
			}
			else
			{
				row.rounded(6).draw(ui::GetTheme().item);
			}
			row.rounded(6).drawFrame(1.0, ui::GetTheme().panelBorder);
			m_smallFont(building.name).draw(row.pos.movedBy(8, 4), ui::GetTheme().text);
			if (row.leftClicked())
			{
				selectedBuildingId = building.id;
				actions.selectedBuildingId = selectedBuildingId;
			}
		}

		const double commandY = section.bottomY() - RowHeight - 10.0;
		const double buttonW = (section.w - 30.0) * 0.5;
		if (ui::Button(m_smallFont, U"Duplicate", RectF{ section.x + 10, commandY, buttonW, RowHeight }))
		{
			actions.duplicateSelected = true;
		}
		if (ui::Button(m_smallFont, U"Delete", RectF{ section.x + 20 + buttonW, commandY, buttonW, RowHeight }))
		{
			actions.deleteSelected = true;
		}
		return section.bottomY() + SectionGap;
	}

	double BuildingEditorPanel::drawTransformSection(const RectF& panel, const double y, GeneratedBuilding* selectedBuilding, BuildingPanelActions& actions)
	{
		const RectF section = SectionRect(panel, y, 152.0);
		ui::Section(section);
		DrawSectionTitle(m_smallFont, U"Transform", section);
		if (not selectedBuilding)
		{
			m_smallFont(U"No building selected").draw(section.pos.movedBy(10, 38), ui::GetTheme().textMuted);
			return section.bottomY() + SectionGap;
		}

		ui::SliderH(U"X: {:.1f}"_fmt(selectedBuilding->origin.x), selectedBuilding->origin.x, -80.0, 80.0, section.pos.movedBy(10, 34), 92.0, ContentWidth(section) - 92.0);
		ui::SliderH(U"Z: {:.1f}"_fmt(selectedBuilding->origin.z), selectedBuilding->origin.z, -80.0, 80.0, section.pos.movedBy(10, 70), 92.0, ContentWidth(section) - 92.0);
		ui::SliderH(U"Rot: {:.2f}"_fmt(selectedBuilding->rotation01), selectedBuilding->rotation01, 0.0, 1.0, section.pos.movedBy(10, 106), 92.0, ContentWidth(section) - 92.0);
		const double buttonW = 44.0;
		if (ui::Button(m_smallFont, U"↶", RectF{ section.rightX() - 102.0, section.y + 106.0, buttonW, 28.0 }))
		{
			actions.rotateLeft = true;
		}
		if (ui::Button(m_smallFont, U"↷", RectF{ section.rightX() - 52.0, section.y + 106.0, buttonW, 28.0 }))
		{
			actions.rotateRight = true;
		}
		Sanitize(*selectedBuilding);
		return section.bottomY() + SectionGap;
	}

	double BuildingEditorPanel::drawShapeSection(const RectF& panel, const double y, GeneratedBuilding* selectedBuilding)
	{
		const RectF section = SectionRect(panel, y, 188.0);
		ui::Section(section);
		DrawSectionTitle(m_smallFont, U"Shape", section);
		if (selectedBuilding)
		{
			double floors = static_cast<double>(selectedBuilding->floors);
			ui::SliderH(U"Width: {:.1f}"_fmt(selectedBuilding->footprintSize.x), selectedBuilding->footprintSize.x, 1.0, 40.0, section.pos.movedBy(10, 34), 112.0, ContentWidth(section) - 112.0);
			ui::SliderH(U"Depth: {:.1f}"_fmt(selectedBuilding->footprintSize.y), selectedBuilding->footprintSize.y, 1.0, 40.0, section.pos.movedBy(10, 70), 112.0, ContentWidth(section) - 112.0);
			if (ui::SliderH(U"Floors: {:.0f}"_fmt(floors), floors, 1.0, 8.0, section.pos.movedBy(10, 106), 112.0, ContentWidth(section) - 112.0))
			{
				selectedBuilding->floors = static_cast<int32>(Math::Round(floors));
			}
			ui::SliderH(U"Floor H: {:.1f}"_fmt(selectedBuilding->floorHeight), selectedBuilding->floorHeight, 1.5, 6.0, section.pos.movedBy(10, 142), 112.0, ContentWidth(section) - 112.0);
			Sanitize(*selectedBuilding);
		}
		return section.bottomY() + SectionGap;
	}

	double BuildingEditorPanel::drawRoofSection(const RectF& panel, const double y, GeneratedBuilding* selectedBuilding)
	{
		const RectF section = SectionRect(panel, y, 116.0);
		ui::Section(section);
		DrawSectionTitle(m_smallFont, U"Roof", section);
		if (selectedBuilding)
		{
			const double buttonW = (section.w - 40.0) / 3.0;
			const double buttonY = section.y + 34.0;
			if (ui::Button(m_smallFont, U"Flat", RectF{ section.x + 10, buttonY, buttonW, RowHeight })) selectedBuilding->roofType = BuildingRoofType::Flat;
			if (ui::Button(m_smallFont, U"Gable", RectF{ section.x + 15 + buttonW, buttonY, buttonW, RowHeight })) selectedBuilding->roofType = BuildingRoofType::Gable;
			if (ui::Button(m_smallFont, U"Hip", RectF{ section.x + 20 + buttonW * 2, buttonY, buttonW, RowHeight })) selectedBuilding->roofType = BuildingRoofType::Hip;
			ui::SliderH(U"Height: {:.1f}"_fmt(selectedBuilding->roofHeight), selectedBuilding->roofHeight, 0.0, 5.0, section.pos.movedBy(10, 74), 112.0, ContentWidth(section) - 112.0);
			Sanitize(*selectedBuilding);
		}
		return section.bottomY() + SectionGap;
	}

	double BuildingEditorPanel::drawMaterialSection(const RectF& panel, const double y, GeneratedBuilding* selectedBuilding, BuildingPanelActions& actions)
	{
		const RectF section = SectionRect(panel, y, 190.0);
		ui::Section(section);
		DrawSectionTitle(m_smallFont, U"Material", section);
		if (selectedBuilding)
		{
			ui::SliderH(U"Wall R: {:.2f}"_fmt(selectedBuilding->materialSet.wallColor.r), selectedBuilding->materialSet.wallColor.r, 0.0, 1.0, section.pos.movedBy(10, 34), 116.0, ContentWidth(section) - 116.0);
			ui::SliderH(U"Wall G: {:.2f}"_fmt(selectedBuilding->materialSet.wallColor.g), selectedBuilding->materialSet.wallColor.g, 0.0, 1.0, section.pos.movedBy(10, 70), 116.0, ContentWidth(section) - 116.0);
			ui::SliderH(U"Wall B: {:.2f}"_fmt(selectedBuilding->materialSet.wallColor.b), selectedBuilding->materialSet.wallColor.b, 0.0, 1.0, section.pos.movedBy(10, 106), 116.0, ContentWidth(section) - 116.0);
			const double buttonW = (section.w - 30.0) * 0.5;
			if (ui::Button(m_smallFont, U"Style", RectF{ section.x + 10, section.y + 146, buttonW, RowHeight }))
			{
				const int32 next = (static_cast<int32>(selectedBuilding->facadeStyle) + 1) % 4;
				selectedBuilding->facadeStyle = static_cast<BuildingFacadeStyle>(next);
			}
			if (ui::Button(m_smallFont, U"Reroll", RectF{ section.x + 20 + buttonW, section.y + 146, buttonW, RowHeight }))
			{
				actions.rerollSeed = true;
			}
			Sanitize(*selectedBuilding);
		}
		return section.bottomY() + SectionGap;
	}

	double BuildingEditorPanel::drawPresetSection(const RectF& panel, const double y, GeneratedBuilding* selectedBuilding)
	{
		const RectF section = SectionRect(panel, y, 96.0);
		ui::Section(section);
		DrawSectionTitle(m_smallFont, U"Preset", section);
		if (selectedBuilding)
		{
			const double buttonW = (section.w - 30.0) * 0.5;
			if (ui::Button(m_smallFont, U"Small House", RectF{ section.x + 10, section.y + 34, buttonW, RowHeight })) ApplyPreset(*selectedBuilding, BuildingFacadeStyle::OldTown, BuildingRoofType::Gable, Vec2{ 5.0, 6.0 }, 2);
			if (ui::Button(m_smallFont, U"Warehouse", RectF{ section.x + 20 + buttonW, section.y + 34, buttonW, RowHeight })) ApplyPreset(*selectedBuilding, BuildingFacadeStyle::Warehouse, BuildingRoofType::Flat, Vec2{ 11.0, 8.0 }, 1);
		}
		return section.bottomY() + SectionGap;
	}

	double BuildingEditorPanel::drawSaveLoadSection(const RectF& panel, const double y, BuildingPanelActions& actions, StringView status)
	{
		const RectF section = SectionRect(panel, y, 104.0);
		ui::Section(section);
		DrawSectionTitle(m_smallFont, U"Save / Load", section);
		const double buttonW = (section.w - 30.0) * 0.5;
		if (ui::Button(m_smallFont, U"Save", RectF{ section.x + 10, section.y + 34, buttonW, RowHeight }))
		{
			actions.save = true;
		}
		if (ui::Button(m_smallFont, U"Load", RectF{ section.x + 20 + buttonW, section.y + 34, buttonW, RowHeight }))
		{
			actions.load = true;
		}
		m_smallFont(status).draw(section.pos.movedBy(10, 74), ui::GetTheme().textMuted);
		return section.bottomY() + SectionGap;
	}
}
