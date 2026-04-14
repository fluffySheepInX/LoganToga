# include "MapEditorPanelInternal.hpp"
# include "SkyAppUiInternal.hpp"

using namespace MapEditorDetail;

namespace MapEditorDetail
{
	void DrawMapEditorSelectionDetailSection(MapEditorState& state, MapData& mapData, const Rect& panelRect, const Font& font)
	{
		const Rect detailSectionRect{ (panelRect.x + 12), (panelRect.y + 456), (panelRect.w - 24), 156 };
		const int32 detailY = (detailSectionRect.y + 36);
		const int32 detailButtonX = (detailSectionRect.rightX() - 100);
		DrawMapEditorPanelSection(detailSectionRect);
		font(U"Selection / Detail").draw((detailSectionRect.x + 10), (detailSectionRect.y + 8), SkyAppSupport::UiInternal::EditorTextOnLightPrimaryColor());
		if ((not state.selectionMode) && IsTerrainPaintTool(state.selectedTool))
		{
			font(U"Terrain Paint: {}"_fmt(ToLabel(state.selectedTool))).draw((detailSectionRect.x + 10), detailY, SkyAppSupport::UiInternal::EditorTextOnPanelPrimaryColor());
			const Array<Color>& palette = TerrainColorPalette();
			for (size_t i = 0; i < palette.size(); ++i)
			{
				const Rect colorButton{
					(detailSectionRect.x + 10 + (static_cast<int32>(i) * 48)),
					(detailY + 24),
					36,
					28,
				};
				const bool selectedColor = (state.selectedTerrainColor == palette[i]);
				colorButton.draw(ColorF{ palette[i] }).drawFrame(selectedColor ? 3 : 1, 0, selectedColor ? ColorF{ 0.10, 0.12, 0.14 } : ColorF{ 0.40 });
				if (colorButton.leftClicked())
				{
					state.selectedTerrainColor = palette[i];
				}
			}
			font(U"Paint Color: {}, {}, {}"_fmt(state.selectedTerrainColor.r, state.selectedTerrainColor.g, state.selectedTerrainColor.b)).draw((detailSectionRect.x + 10), (detailY + 60), SkyAppSupport::UiInternal::EditorTextOnPanelPrimaryColor());
		}
		else if (IsValidNavPointIndex(mapData, state.selectedNavPointIndex))
		{
			NavPoint& navPoint = mapData.navPoints[*state.selectedNavPointIndex];
			const Rect deleteButton{ detailButtonX, (detailY - 4), 88, 28 };
			const Rect radiusDownButton{ detailButtonX, (detailY + 30), 28, 28 };
			const Rect radiusUpButton{ (detailButtonX + 60), (detailY + 30), 28, 28 };
			font(U"Selected: NavPoint {} ({:.1f}, {:.1f}, {:.1f})"_fmt(*state.selectedNavPointIndex, navPoint.position.x, navPoint.position.y, navPoint.position.z)).draw((detailSectionRect.x + 10), detailY, SkyAppSupport::UiInternal::EditorTextOnPanelPrimaryColor());
			font(U"Radius: {:.1f} / Links: {}"_fmt(navPoint.radius, CountNavLinksForPoint(mapData, *state.selectedNavPointIndex))).draw((detailSectionRect.x + 10), (detailY + 24), SkyAppSupport::UiInternal::EditorTextOnPanelPrimaryColor());

			if (DrawEditorButton(deleteButton, U"削除"))
			{
				RemoveNavPointAt(mapData, *state.selectedNavPointIndex);
				state.selectedNavPointIndex.reset();
				state.pendingNavLinkStartIndex.reset();
				SetStatusMessage(state, U"選択中 NavPoint を削除");
			}

			if (DrawEditorButton(radiusDownButton, U"-"))
			{
				navPoint.radius = Clamp((navPoint.radius - 0.1), 0.5, 8.0);
			}

			if (DrawEditorButton(radiusUpButton, U"+"))
			{
				navPoint.radius = Clamp((navPoint.radius + 0.1), 0.5, 8.0);
			}
		}
		else if (IsValidPlacedModelIndex(mapData, state.selectedPlacedModelIndex))
		{
			PlacedModel& placedModel = mapData.placedModels[*state.selectedPlacedModelIndex];
			const Rect deleteButton{ detailButtonX, (detailY - 4), 88, 28 };
			font(U"Selected: {} ({:.1f}, {:.1f}, {:.1f})"_fmt(ToString(placedModel.type), placedModel.position.x, placedModel.position.y, placedModel.position.z)).draw((detailSectionRect.x + 10), detailY, SkyAppSupport::UiInternal::EditorTextOnPanelPrimaryColor());

			if (DrawEditorButton(deleteButton, U"削除"))
			{
				mapData.placedModels.erase(mapData.placedModels.begin() + *state.selectedPlacedModelIndex);
				state.selectedPlacedModelIndex.reset();
				state.roadResizeDrag.reset();
				state.roadRotateDrag.reset();
				SetStatusMessage(state, U"選択中モデルを削除");
			}

			if (placedModel.type == PlaceableModelType::Wall)
			{
				const Rect lengthDownButton{ detailButtonX, (detailY + 24), 28, 28 };
				const Rect lengthUpButton{ (detailButtonX + 60), (detailY + 24), 28, 28 };
				const Rect yawDownButton{ detailButtonX, (detailY + 56), 28, 28 };
				const Rect yawUpButton{ (detailButtonX + 60), (detailY + 56), 28, 28 };
				font(U"Length: {:.1f} / Yaw: {:.0f}°"_fmt(placedModel.wallLength, Math::ToDegrees(placedModel.yaw))).draw((detailSectionRect.x + 10), (detailY + 24), SkyAppSupport::UiInternal::EditorTextOnPanelPrimaryColor());

				if (DrawEditorButton(lengthDownButton, U"-"))
				{
					placedModel.wallLength = Clamp((placedModel.wallLength - 1.0), 2.0, 80.0);
				}

				if (DrawEditorButton(lengthUpButton, U"+"))
				{
					placedModel.wallLength = Clamp((placedModel.wallLength + 1.0), 2.0, 80.0);
				}

				if (DrawEditorButton(yawDownButton, U"↺"))
				{
					placedModel.yaw -= 15_deg;
				}

				if (DrawEditorButton(yawUpButton, U"↻"))
				{
					placedModel.yaw += 15_deg;
				}
			}
			else if (placedModel.type == PlaceableModelType::Road)
			{
				const Rect lengthDownButton{ detailButtonX, (detailY + 24), 28, 28 };
				const Rect lengthUpButton{ (detailButtonX + 60), (detailY + 24), 28, 28 };
				const Rect widthDownButton{ detailButtonX, (detailY + 56), 28, 28 };
				const Rect widthUpButton{ (detailButtonX + 60), (detailY + 56), 28, 28 };
				const Rect yawDownButton{ detailButtonX, (detailY + 88), 28, 28 };
				const Rect yawUpButton{ (detailButtonX + 60), (detailY + 88), 28, 28 };
				font(U"Length: {:.1f} / Width: {:.1f} / Yaw: {:.0f}°"_fmt(placedModel.roadLength, placedModel.roadWidth, Math::ToDegrees(placedModel.yaw))).draw((detailSectionRect.x + 10), (detailY + 24), SkyAppSupport::UiInternal::EditorTextOnPanelPrimaryColor());

				if (DrawEditorButton(lengthDownButton, U"-"))
				{
					placedModel.roadLength = Clamp((placedModel.roadLength - 1.0), 2.0, 80.0);
				}

				if (DrawEditorButton(lengthUpButton, U"+"))
				{
					placedModel.roadLength = Clamp((placedModel.roadLength + 1.0), 2.0, 80.0);
				}

				if (DrawEditorButton(widthDownButton, U"-"))
				{
					placedModel.roadWidth = Clamp((placedModel.roadWidth - 1.0), 2.0, 80.0);
				}

				if (DrawEditorButton(widthUpButton, U"+"))
				{
					placedModel.roadWidth = Clamp((placedModel.roadWidth + 1.0), 2.0, 80.0);
				}

				if (DrawEditorButton(yawDownButton, U"↺"))
				{
					placedModel.yaw -= 15_deg;
				}

				if (DrawEditorButton(yawUpButton, U"↻"))
				{
					placedModel.yaw += 15_deg;
				}
			}
           else if (placedModel.type == PlaceableModelType::TireTrackDecal)
			{
				const Rect lengthDownButton{ detailButtonX, (detailY + 24), 28, 28 };
				const Rect lengthUpButton{ (detailButtonX + 60), (detailY + 24), 28, 28 };
				const Rect widthDownButton{ detailButtonX, (detailY + 56), 28, 28 };
				const Rect widthUpButton{ (detailButtonX + 60), (detailY + 56), 28, 28 };
				const Rect yawDownButton{ detailButtonX, (detailY + 88), 28, 28 };
				const Rect yawUpButton{ (detailButtonX + 60), (detailY + 88), 28, 28 };
				font(U"Length: {:.1f} / Width: {:.1f} / Yaw: {:.0f}°"_fmt(placedModel.roadLength, placedModel.roadWidth, Math::ToDegrees(placedModel.yaw))).draw((detailSectionRect.x + 10), (detailY + 24), SkyAppSupport::UiInternal::EditorTextOnPanelPrimaryColor());

				if (DrawEditorButton(lengthDownButton, U"-"))
				{
					placedModel.roadLength = Clamp((placedModel.roadLength - 1.0), 2.0, 80.0);
				}

				if (DrawEditorButton(lengthUpButton, U"+"))
				{
					placedModel.roadLength = Clamp((placedModel.roadLength + 1.0), 2.0, 80.0);
				}

				if (DrawEditorButton(widthDownButton, U"-"))
				{
					placedModel.roadWidth = Clamp((placedModel.roadWidth - 1.0), 2.0, 80.0);
				}

				if (DrawEditorButton(widthUpButton, U"+"))
				{
					placedModel.roadWidth = Clamp((placedModel.roadWidth + 1.0), 2.0, 80.0);
				}

				if (DrawEditorButton(yawDownButton, U"↺"))
				{
					placedModel.yaw -= 15_deg;
				}

				if (DrawEditorButton(yawUpButton, U"↻"))
				{
					placedModel.yaw += 15_deg;
				}
			}
		}
		else if (IsValidResourceAreaIndex(mapData, state.selectedResourceAreaIndex))
		{
			const ResourceArea& resourceArea = mapData.resourceAreas[*state.selectedResourceAreaIndex];
			const Rect deleteButton{ detailButtonX, (detailY - 4), 88, 28 };
			font(U"Selected: {} ({:.1f}, {:.1f}, {:.1f}) r={:.1f}"_fmt(ToString(resourceArea.type), resourceArea.position.x, resourceArea.position.y, resourceArea.position.z, resourceArea.radius)).draw((detailSectionRect.x + 10), detailY, SkyAppSupport::UiInternal::EditorTextOnPanelPrimaryColor());

			if (DrawEditorButton(deleteButton, U"削除"))
			{
				mapData.resourceAreas.erase(mapData.resourceAreas.begin() + *state.selectedResourceAreaIndex);
				state.selectedResourceAreaIndex.reset();
				SetStatusMessage(state, U"選択中資源エリアを削除");
			}
		}
		else
		{
			font(U"Selected: none").draw((detailSectionRect.x + 10), detailY, SkyAppSupport::UiInternal::EditorTextOnPanelPrimaryColor());
		}
	}
}
