# pragma once
# include <Siv3D.hpp>
# include "../Addons/Pi3D/UI/RectUI.hpp"
# include "../Addons/Pi3D/UI/Layout.hpp"
# include "../Addons/Pi3D/UI/EditorIconLayout.hpp"
# include "BuildingDocument.hpp"

namespace building
{
	struct BuildingPanelActions
	{
		bool beginPlacement = false;
		bool duplicateSelected = false;
		bool deleteSelected = false;
		bool save = false;
		bool load = false;
		bool cancel = false;
		bool rotateLeft = false;
		bool rotateRight = false;
		bool rerollSeed = false;
		Optional<String> selectedBuildingId;
	};

	class BuildingEditorPanel
	{
	public:
		BuildingPanelActions draw(BuildingDocument& document, Optional<String> selectedBuildingId, StringView status);

		[[nodiscard]] bool wantsMouseCapture() const;
		[[nodiscard]] bool wantsMouseWheelCapture() const;
		[[nodiscard]] bool isOpen() const noexcept;
		void toggleOpen();
		void closeToIcon();
		void syncCollapsedIconRegistry() const;

	private:
		[[nodiscard]] RectF getPanelRect() const;
		[[nodiscard]] RectF getPanelContentRect(const RectF& panelRect) const;
		[[nodiscard]] RectF getCollapsedIconRect() const;
		void updateCollapsedIconDrag(const RectF& dragRect);
		void expandFromCollapsedIcon(const RectF& iconRect);
		[[nodiscard]] double getPanelHeight() const;
		[[nodiscard]] double getPanelContentHeight() const;
		[[nodiscard]] GeneratedBuilding* getSelectedBuilding(BuildingDocument& document, const Optional<String>& selectedBuildingId) const;
		[[nodiscard]] double drawBuildingListSection(const RectF& panel, double y, BuildingDocument& document, BuildingPanelActions& actions, Optional<String>& selectedBuildingId);
		[[nodiscard]] double drawTransformSection(const RectF& panel, double y, GeneratedBuilding* selectedBuilding, BuildingPanelActions& actions);
		[[nodiscard]] double drawShapeSection(const RectF& panel, double y, GeneratedBuilding* selectedBuilding);
		[[nodiscard]] double drawRoofSection(const RectF& panel, double y, GeneratedBuilding* selectedBuilding);
		[[nodiscard]] double drawMaterialSection(const RectF& panel, double y, GeneratedBuilding* selectedBuilding, BuildingPanelActions& actions);
		[[nodiscard]] double drawPresetSection(const RectF& panel, double y, GeneratedBuilding* selectedBuilding);
		[[nodiscard]] double drawSaveLoadSection(const RectF& panel, double y, BuildingPanelActions& actions, StringView status);

		Font m_font{ FontMethod::MSDF, 20, Typeface::Medium };
		Font m_smallFont{ FontMethod::MSDF, 17, Typeface::Medium };
		Vec2 m_panelPos{ ui::editor_icon::GetDockedStackPosition(4) };
		Vec2 m_dragOffset{ 0, 0 };
		double m_scrollY = 0.0;
		double m_scrollThumbGrabOffsetY = 0.0;
		bool m_open = false;
		bool m_dragging = false;
		bool m_isScrollThumbDragging = false;
		Texture m_toggleIcon{ U"texture/proIcon.png" };
	};
}
