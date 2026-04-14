# include "MainUi.hpp"
# include "MainSettings.hpp"
# include "SkyAppText.hpp"
# include "SkyAppUiInternal.hpp"
# include "SkyAppUiPanelFrameInternal.hpp"

namespace MainSupport
{
  using SkyAppText::TextId;
	using SkyAppText::Tr;
	using SkyAppText::TrFormat;

 namespace
	{
       inline constexpr double ScaleButtonStepLarge = 0.5;
		inline constexpr double ScaleButtonStepMedium = 0.1;
		inline constexpr double ScaleButtonStepSmall = 0.01;
		inline constexpr double ModelHeightDragRoundStep = 0.001;

           [[nodiscard]] StringView ToModelHeightTargetLabel(const UnitRenderModel renderModel)
		{
             return GetUnitRenderModelLabel(renderModel);
		}

            [[nodiscard]] double GetActiveModelScale(const ModelHeightSettings& modelHeightSettings, const UnitRenderModel renderModel)
		{
              return GetModelScale(modelHeightSettings, renderModel);
		}

            [[nodiscard]] double GetModelHeightWorldY(const UnitRenderModel renderModel,
				const std::array<Vec3, UnitRenderModelCount>& previewRenderPositions)
		{
             return previewRenderPositions[GetUnitRenderModelIndex(renderModel)].y;
		}

		[[nodiscard]] StringView ToTextureTargetLabel(const TireTrackTextureSegment segment)
		{
			return GetTireTrackTextureSegmentLabel(segment);
		}

		[[nodiscard]] double RoundModelHeightEditorValue(const double value, const double roundStep)
		{
			if (roundStep <= 0.0)
			{
				return value;
			}

			return (Math::Round(value / roundStep) * roundStep);
		}

		void DrawDragValueRect(const Rect& rect,
			const int32 controlId,
			const StringView label,
			double& value,
			const double minValue,
			const double maxValue,
			const double roundStep)
		{
			static Optional<int32> activeControlId;

			const bool hovered = rect.mouseOver();
			if (MouseL.down() && hovered)
			{
				activeControlId = controlId;
			}

			const bool active = (activeControlId && (*activeControlId == controlId));
			const RectF sliderTrackRect{ (rect.x + 12.0), (rect.bottomY() - 12.0), (rect.w - 24.0), 8.0 };

			if (active)
			{
				if (MouseL.pressed())
				{
					const double cursorRatio = Math::Saturate((Cursor::PosF().x - sliderTrackRect.x) / Max(1.0, sliderTrackRect.w));
					value = Clamp(RoundModelHeightEditorValue((minValue + (maxValue - minValue) * cursorRatio), roundStep), minValue, maxValue);
				}
				else
				{
					activeControlId.reset();
				}
			}
			else
			{
				value = Clamp(RoundModelHeightEditorValue(value, roundStep), minValue, maxValue);
			}

			const double ratio = Math::Saturate((value - minValue) / Max(0.0001, (maxValue - minValue)));
			rect.rounded(8).draw(active
				? ColorF{ 0.90, 0.94, 1.0, 0.92 }
				: (hovered ? ColorF{ 0.98, 0.99, 1.0, 0.84 } : ColorF{ 0.96, 0.97, 0.99, 0.78 }))
				.drawFrame(1.0, 0.0, active ? ColorF{ 0.28, 0.46, 0.74, 0.96 } : ColorF{ 0.58, 0.64, 0.72, 0.84 });

			SimpleGUI::GetFont()(label).draw((rect.x + 12), (rect.y + 6), SkyAppSupport::UiInternal::EditorTextOnCardPrimaryColor());
			SimpleGUI::GetFont()(U"{:.3f}"_fmt(value)).draw((rect.rightX() - 74), (rect.y + 6), SkyAppSupport::UiInternal::EditorTextOnCardPrimaryColor());

			if (active)
			{
				sliderTrackRect.rounded(4).draw(ColorF{ 0.12, 0.14, 0.18, 0.92 });
				RectF{ sliderTrackRect.pos, (sliderTrackRect.w * ratio), sliderTrackRect.h }.rounded(4).draw(ColorF{ 0.38, 0.70, 0.96, 0.95 });
				sliderTrackRect.rounded(4).drawFrame(1.0, ColorF{ 0.78, 0.86, 0.96, 0.72 });
				RectF knobRect{ Arg::center = Vec2{ (sliderTrackRect.x + sliderTrackRect.w * ratio), sliderTrackRect.centerY() }, 14, 22 };
				knobRect.rounded(4).draw(ColorF{ 0.94, 0.97, 1.0 }).drawFrame(1.0, 0.0, ColorF{ 0.25, 0.34, 0.50, 0.95 });
			}
			else
			{
                SimpleGUI::GetFont()(TrFormat(TextId::CommonDragToAdjustRange, U"{:.2f}, {:.2f}"_fmt(minValue, maxValue))).draw((rect.x + 12), (rect.y + 28), SkyAppSupport::UiInternal::EditorTextOnCardSecondaryColor());
			}
		}
	}

	bool DrawTextButton(const Rect& rect, StringView label)
	{
		static const Font buttonFont{ 18, Typeface::Bold };
		const bool hovered = rect.mouseOver();
		rect.draw(hovered ? ColorF{ 0.82 } : ColorF{ 0.72 })
			.drawFrame(1, 0, ColorF{ 0.35 });
        buttonFont(label).drawAt(rect.center(), SkyAppSupport::UiInternal::EditorTextOnCardPrimaryColor());
		return hovered && MouseL.down();
	}

	bool DrawCheckBox(const Rect& rect, bool& checked, StringView label, const bool enabled)
	{
		const bool hovered = enabled && rect.mouseOver();
		const RectF rowRect{ rect };
		const RectF boxRect{ (rect.x + 8), (rect.y + (rect.h - 22) / 2.0), 22, 22 };
		const Vec2 labelPos{ (boxRect.rightX() + 8.0), (rect.y + (rect.h - 22) / 2.0 - 1.0) };

		if (hovered && MouseL.down())
		{
			checked = not checked;
		}

		rowRect.rounded(6).draw(hovered ? ColorF{ 0.90, 0.93, 0.98, 0.36 } : ColorF{ 0.78, 0.84, 0.92, 0.18 })
			.drawFrame(1.0, 0.0, hovered ? ColorF{ 0.44, 0.56, 0.74, 0.78 } : ColorF{ 0.42, 0.48, 0.56, 0.56 });
		boxRect.rounded(4).draw(hovered ? ColorF{ 0.90, 0.93, 0.98 } : ColorF{ 0.82, 0.86, 0.92 })
			.drawFrame(1.0, 0.0, ColorF{ 0.32, 0.38, 0.46 });

		if (checked)
		{
			boxRect.stretched(-4).rounded(3).draw(ColorF{ 0.33, 0.53, 0.82 });
			SimpleGUI::GetFont()(U"✓").drawAt(boxRect.center(), ColorF{ 0.98 });
		}

		SimpleGUI::GetFont()(label).draw(labelPos, enabled ? SkyAppSupport::UiInternal::EditorTextOnLightPrimaryColor() : SkyAppSupport::UiInternal::EditorTextOnLightSecondaryColor());
		return hovered && MouseL.down();
	}

 void DrawAnimationClipSelector(UnitModel& model, StringView title, const int32 x, const int32 y, const int32 width)
	{
		if (not model.hasAnimations())
		{
			return;
		}

		const auto& clips = model.animations();
       SimpleGUI::GetFont()(U"{} ({})"_fmt(title, clips.size())).draw(x, y, SkyAppSupport::UiInternal::EditorTextOnPanelPrimaryColor());

		for (size_t i = 0; i < clips.size(); ++i)
		{
			const int32 buttonY = (y + 28 + static_cast<int32>(i) * 34);
			const bool isCurrent = (i == model.currentClipIndex());
			const Rect clipButton{ x, buttonY, width, 30 };
			const String label = U"[{}] {}"_fmt(i, clips[i].name);
			const bool hovered = clipButton.mouseOver();
			clipButton.draw(isCurrent ? ColorF{ 0.55, 0.75, 0.95 } : (hovered ? ColorF{ 0.82 } : ColorF{ 0.72 }))
				.drawFrame(1, 0, isCurrent ? ColorF{ 0.2, 0.4, 0.7 } : ColorF{ 0.35 });
           SimpleGUI::GetFont()(label).draw((clipButton.x + 8), (clipButton.y + 4), isCurrent ? SkyAppSupport::UiInternal::EditorTextOnSelectedPrimaryColor() : SkyAppSupport::UiInternal::EditorTextOnCardPrimaryColor());

			if (hovered && MouseL.down())
			{
				model.setClipIndex(i);
			}
		}
	}

	void DrawModelHeightEditor(ModelHeightSettings& modelHeightSettings,
        UnitRenderModel& activeRenderModel,
     bool& textureMode,
		TireTrackTextureSegment& activeTextureSegment,
		String& modelHeightMessage,
		double& modelHeightMessageUntil,
		const Rect& panelRect,
         const std::array<Vec3, UnitRenderModelCount>& previewRenderPositions)
	{
        const Rect listPanel{ panelRect.x, panelRect.y, 156, panelRect.h };
		const Rect detailPanel{ (panelRect.x + 164), panelRect.y, (panelRect.w - 164), panelRect.h };
         double& activeOffset = GetModelHeightOffset(modelHeightSettings, activeRenderModel);
		 double& activeScale = GetModelScale(modelHeightSettings, activeRenderModel);
     double& activeTextureYOffset = GetTireTrackYOffset(modelHeightSettings, activeTextureSegment);
     double& activeTextureOpacity = GetTireTrackOpacity(modelHeightSettings, activeTextureSegment);
		double& activeTextureSoftness = GetTireTrackSoftness(modelHeightSettings, activeTextureSegment);
		double& activeTextureWarmth = GetTireTrackWarmth(modelHeightSettings, activeTextureSegment);
		activeOffset = Clamp(activeOffset, ModelHeightOffsetMin, ModelHeightOffsetMax);
		activeScale = Clamp(activeScale, ModelScaleMin, ModelScaleMax);
		activeTextureYOffset = Clamp(activeTextureYOffset, TireTrackYOffsetMin, TireTrackYOffsetMax);
		activeTextureOpacity = Clamp(activeTextureOpacity, TireTrackOpacityMin, TireTrackOpacityMax);
		activeTextureSoftness = Clamp(activeTextureSoftness, TireTrackSoftnessMin, TireTrackSoftnessMax);
		activeTextureWarmth = Clamp(activeTextureWarmth, TireTrackWarmthMin, TireTrackWarmthMax);

      SkyAppSupport::UiInternal::DrawNinePatchPanelFrame(panelRect, Tr(TextId::ModelHeightPanelTitle), ColorF{ 1.0, 0.92 });
        Rect{ (listPanel.rightX() + 3), (panelRect.y + 8), 1, (panelRect.h - 16) }.draw(ColorF{ 0.72, 0.72, 0.74 });
        const Rect modelToggleRect{ (listPanel.x + 12), (listPanel.y + 36), 62, 26 };
		const Rect textureToggleRect{ (listPanel.x + 82), (listPanel.y + 36), 62, 26 };
		modelToggleRect.rounded(6).draw(textureMode ? ColorF{ 0.96, 0.97, 0.99, 0.82 } : ColorF{ 0.33, 0.53, 0.82 })
			.drawFrame(1.0, 0.0, textureMode ? ColorF{ 0.58, 0.64, 0.72, 0.84 } : ColorF{ 0.20, 0.32, 0.52 });
		textureToggleRect.rounded(6).draw(textureMode ? ColorF{ 0.33, 0.53, 0.82 } : ColorF{ 0.96, 0.97, 0.99, 0.82 })
			.drawFrame(1.0, 0.0, textureMode ? ColorF{ 0.20, 0.32, 0.52 } : ColorF{ 0.58, 0.64, 0.72, 0.84 });
		SimpleGUI::GetFont()(U"model").drawAt(modelToggleRect.center(), textureMode ? ColorF{ 0.14 } : ColorF{ 0.98 });
		SimpleGUI::GetFont()(U"texture").drawAt(textureToggleRect.center(), textureMode ? ColorF{ 0.98 } : ColorF{ 0.14 });

		if (modelToggleRect.mouseOver() && MouseL.down())
		{
			textureMode = false;
		}

		if (textureToggleRect.mouseOver() && MouseL.down())
		{
			textureMode = true;
		}

		  SimpleGUI::GetFont()(textureMode ? U"Textures" : Tr(TextId::ModelHeightTargets)).draw((listPanel.x + 16), (listPanel.y + 70), ColorF{ 0.18 });

        int32 targetIndex = 0;
		if (textureMode)
		{
			for (const TireTrackTextureSegment segment : GetTireTrackTextureSegments())
			{
				const Rect buttonRect{ (listPanel.x + 12), (listPanel.y + 96 + targetIndex * 58), 132, 48 };
				const bool selected = (activeTextureSegment == segment);
				const bool hovered = buttonRect.mouseOver();
				buttonRect.draw(selected ? ColorF{ 0.33, 0.53, 0.82 } : (hovered ? ColorF{ 0.94, 0.95, 0.98 } : ColorF{ 0.98, 0.97, 0.95 }))
					.drawFrame(1, 0, selected ? ColorF{ 0.20, 0.32, 0.52 } : ColorF{ 0.58, 0.56, 0.52 });
				SimpleGUI::GetFont()(ToTextureTargetLabel(segment)).draw((buttonRect.x + 10), (buttonRect.y + 6), selected ? ColorF{ 0.98 } : ColorF{ 0.14 });
                SimpleGUI::GetFont()(U"Y {0} / A {1}"_fmt(U"{:.3f}"_fmt(GetTireTrackYOffset(modelHeightSettings, segment)), U"{:.2f}"_fmt(GetTireTrackOpacity(modelHeightSettings, segment)))).draw((buttonRect.x + 10), (buttonRect.y + 26), selected ? ColorF{ 0.96 } : ColorF{ 0.28 });

				if (hovered && MouseL.down())
				{
					activeTextureSegment = segment;
				}

				++targetIndex;
			}

			SimpleGUI::GetFont()(U"Texture: {0}"_fmt(ToTextureTargetLabel(activeTextureSegment))).draw((detailPanel.x + 16), (detailPanel.y + 38), ColorF{ 0.14 });
			DrawDragValueRect(Rect{ detailPanel.x + 16, detailPanel.y + 70, 376, 46 }, 10, U"texture yOffset", activeTextureYOffset, TireTrackYOffsetMin, TireTrackYOffsetMax, ModelHeightDragRoundStep);
			DrawDragValueRect(Rect{ detailPanel.x + 16, detailPanel.y + 116, 376, 46 }, 11, U"opacity", activeTextureOpacity, TireTrackOpacityMin, TireTrackOpacityMax, 0.01);
			DrawDragValueRect(Rect{ detailPanel.x + 16, detailPanel.y + 162, 376, 46 }, 12, U"softness", activeTextureSoftness, TireTrackSoftnessMin, TireTrackSoftnessMax, 0.01);
			DrawDragValueRect(Rect{ detailPanel.x + 16, detailPanel.y + 208, 376, 46 }, 13, U"warmth", activeTextureWarmth, TireTrackWarmthMin, TireTrackWarmthMax, 0.01);

          if (DrawTextButton(Rect{ detailPanel.x + 16, detailPanel.y + 254, 56, 28 }, U"-0.01"))
			{
				activeTextureYOffset = Max(TireTrackYOffsetMin, (activeTextureYOffset - 0.01));
			}

         if (DrawTextButton(Rect{ detailPanel.x + 80, detailPanel.y + 254, 56, 28 }, U"-0.001"))
			{
				activeTextureYOffset = Max(TireTrackYOffsetMin, (activeTextureYOffset - 0.001));
			}

            if (DrawTextButton(Rect{ detailPanel.x + 144, detailPanel.y + 254, 56, 28 }, U"+0.001"))
			{
				activeTextureYOffset = Min(TireTrackYOffsetMax, (activeTextureYOffset + 0.001));
			}

         if (DrawTextButton(Rect{ detailPanel.x + 208, detailPanel.y + 254, 56, 28 }, U"+0.01"))
			{
				activeTextureYOffset = Min(TireTrackYOffsetMax, (activeTextureYOffset + 0.01));
			}

           if (DrawTextButton(Rect{ detailPanel.x + 16, detailPanel.y + 294, 118, 30 }, Tr(TextId::ModelHeightResetTarget)))
			{
				ModelHeightSettings defaultSettings;
				activeTextureYOffset = GetTireTrackYOffset(defaultSettings, activeTextureSegment);
               activeTextureOpacity = GetTireTrackOpacity(defaultSettings, activeTextureSegment);
				activeTextureSoftness = GetTireTrackSoftness(defaultSettings, activeTextureSegment);
				activeTextureWarmth = GetTireTrackWarmth(defaultSettings, activeTextureSegment);
			}

           SimpleGUI::GetFont()(U"texture yOffset: {0}"_fmt(U"{:.3f}"_fmt(activeTextureYOffset))).draw((detailPanel.x + 16), (detailPanel.y + 332), ColorF{ 0.12 });
			SimpleGUI::GetFont()(U"opacity {0} / softness {1} / warmth {2}"_fmt(U"{:.2f}"_fmt(activeTextureOpacity), U"{:.2f}"_fmt(activeTextureSoftness), U"{:.2f}"_fmt(activeTextureWarmth))).draw((detailPanel.x + 16), (detailPanel.y + 356), ColorF{ 0.12 });

            if (DrawTextButton(Rect{ detailPanel.x + 16, detailPanel.y + 384, 92, 28 }, Tr(TextId::CommonSave)))
			{
				modelHeightMessage = SaveModelHeightSettings(modelHeightSettings)
			 ? TrFormat(TextId::ModelHeightSavedWithPath, ModelHeightSettingsPath)
					: Tr(TextId::ModelHeightSaveFailed);
				modelHeightMessageUntil = (Scene::Time() + 2.0);
			}

         if (DrawTextButton(Rect{ detailPanel.x + 116, detailPanel.y + 384, 92, 28 }, Tr(TextId::CommonReload)))
			{
				modelHeightSettings = LoadModelHeightSettings();
			modelHeightMessage = Tr(TextId::ModelHeightReloaded);
				modelHeightMessageUntil = (Scene::Time() + 2.0);
			}

           if (DrawTextButton(Rect{ detailPanel.x + 216, detailPanel.y + 384, 92, 28 }, Tr(TextId::CommonResetAll)))
			{
				modelHeightSettings = {};
		   modelHeightMessage = Tr(TextId::ModelHeightOffsetsScalesReset);
				modelHeightMessageUntil = (Scene::Time() + 2.0);
			}
		}
		else
		{
			for (const UnitRenderModel renderModel : GetUnitRenderModels())
			{
				const Rect buttonRect{ (listPanel.x + 12), (listPanel.y + 96 + targetIndex * 58), 132, 48 };
				const bool selected = (activeRenderModel == renderModel);
				const bool hovered = buttonRect.mouseOver();
				buttonRect.draw(selected ? ColorF{ 0.33, 0.53, 0.82 } : (hovered ? ColorF{ 0.94, 0.95, 0.98 } : ColorF{ 0.98, 0.97, 0.95 }))
					.drawFrame(1, 0, selected ? ColorF{ 0.20, 0.32, 0.52 } : ColorF{ 0.58, 0.56, 0.52 });
				SimpleGUI::GetFont()(ToModelHeightTargetLabel(renderModel)).draw((buttonRect.x + 10), (buttonRect.y + 6), selected ? ColorF{ 0.98 } : ColorF{ 0.14 });
				SimpleGUI::GetFont()(TrFormat(TextId::ModelHeightItemYOffset, U"{:.3f}"_fmt(GetModelHeightOffset(modelHeightSettings, renderModel)))).draw((buttonRect.x + 10), (buttonRect.y + 26), selected ? ColorF{ 0.96 } : ColorF{ 0.28 });

				if (hovered && MouseL.down())
				{
					activeRenderModel = renderModel;
				}

				++targetIndex;
			}

			SimpleGUI::GetFont()(TrFormat(TextId::ModelHeightTargetCurrent, ToModelHeightTargetLabel(activeRenderModel))).draw((detailPanel.x + 16), (detailPanel.y + 38), ColorF{ 0.14 });
			SimpleGUI::Slider(TrFormat(TextId::ModelHeightOffsetY, U"{:.3f}"_fmt(activeOffset)), activeOffset, ModelHeightOffsetMin, ModelHeightOffsetMax, Vec2{ detailPanel.x + 16.0, detailPanel.y + 70.0 }, 180, 260);

         if (DrawTextButton(Rect{ detailPanel.x + 16, detailPanel.y + 112, 56, 28 }, U"-1.0"))
			{
				activeOffset = Max(ModelHeightOffsetMin, (activeOffset - 1.0));
			}

          if (DrawTextButton(Rect{ detailPanel.x + 80, detailPanel.y + 112, 56, 28 }, U"-0.1"))
			{
				activeOffset = Max(ModelHeightOffsetMin, (activeOffset - 0.1));
			}

         if (DrawTextButton(Rect{ detailPanel.x + 144, detailPanel.y + 112, 56, 28 }, U"-0.01"))
			{
				activeOffset = Max(ModelHeightOffsetMin, (activeOffset - 0.01));
			}

         if (DrawTextButton(Rect{ detailPanel.x + 208, detailPanel.y + 112, 56, 28 }, U"+0.01"))
			{
				activeOffset = Min(ModelHeightOffsetMax, (activeOffset + 0.01));
			}

            if (DrawTextButton(Rect{ detailPanel.x + 272, detailPanel.y + 112, 56, 28 }, U"+0.1"))
			{
				activeOffset = Min(ModelHeightOffsetMax, (activeOffset + 0.1));
			}

         if (DrawTextButton(Rect{ detailPanel.x + 336, detailPanel.y + 112, 56, 28 }, U"+1.0"))
			{
				activeOffset = Min(ModelHeightOffsetMax, (activeOffset + 1.0));
			}

          DrawDragValueRect(Rect{ detailPanel.x + 16, detailPanel.y + 154, 376, 46 }, 0, Tr(TextId::ModelHeightScaleLabel), activeScale, ModelScaleMin, ModelScaleMax, ModelHeightDragRoundStep);

           if (DrawTextButton(Rect{ detailPanel.x + 16, detailPanel.y + 200, 56, 28 }, U"-0.5"))
			{
				activeScale = Max(ModelScaleMin, (activeScale - ScaleButtonStepLarge));
			}

           if (DrawTextButton(Rect{ detailPanel.x + 80, detailPanel.y + 200, 56, 28 }, U"-0.1"))
			{
				activeScale = Max(ModelScaleMin, (activeScale - ScaleButtonStepMedium));
			}

         if (DrawTextButton(Rect{ detailPanel.x + 144, detailPanel.y + 200, 56, 28 }, U"-0.01"))
			{
				activeScale = Max(ModelScaleMin, (activeScale - ScaleButtonStepSmall));
			}

         if (DrawTextButton(Rect{ detailPanel.x + 208, detailPanel.y + 200, 56, 28 }, U"+0.01"))
			{
				activeScale = Min(ModelScaleMax, (activeScale + ScaleButtonStepSmall));
			}

          if (DrawTextButton(Rect{ detailPanel.x + 272, detailPanel.y + 200, 56, 28 }, U"+0.1"))
			{
				activeScale = Min(ModelScaleMax, (activeScale + ScaleButtonStepMedium));
			}

          if (DrawTextButton(Rect{ detailPanel.x + 336, detailPanel.y + 200, 56, 28 }, U"+0.5"))
			{
				activeScale = Min(ModelScaleMax, (activeScale + ScaleButtonStepLarge));
			}

         if (DrawTextButton(Rect{ detailPanel.x + 16, detailPanel.y + 242, 118, 30 }, Tr(TextId::ModelHeightResetTarget)))
			{
				activeOffset = 0.0;
				activeScale = 1.0;
			}

         SimpleGUI::GetFont()(TrFormat(TextId::ModelHeightWorldY, U"{:.3f}"_fmt(GetModelHeightWorldY(activeRenderModel, previewRenderPositions)))).draw((detailPanel.x + 16), (detailPanel.y + 286), ColorF{ 0.12 });
			SimpleGUI::GetFont()(TrFormat(TextId::ModelHeightCurrentScale, U"{:.3f}"_fmt(GetActiveModelScale(modelHeightSettings, activeRenderModel)))).draw((detailPanel.x + 16), (detailPanel.y + 310), ColorF{ 0.12 });
			SimpleGUI::GetFont()(TrFormat(TextId::ModelHeightRangeSummary, U"{:.1f}"_fmt(ModelHeightOffsetMin), U"{:.1f}"_fmt(ModelHeightOffsetMax), U"{:.2f}"_fmt(ModelScaleMin), U"{:.2f}"_fmt(ModelScaleMax))).draw((detailPanel.x + 16), (detailPanel.y + 334), ColorF{ 0.12 });

            if (DrawTextButton(Rect{ detailPanel.x + 16, detailPanel.y + 364, 92, 30 }, Tr(TextId::CommonSave)))
			{
				modelHeightMessage = SaveModelHeightSettings(modelHeightSettings)
			 ? TrFormat(TextId::ModelHeightSavedWithPath, ModelHeightSettingsPath)
					: Tr(TextId::ModelHeightSaveFailed);
				modelHeightMessageUntil = (Scene::Time() + 2.0);
			}

         if (DrawTextButton(Rect{ detailPanel.x + 116, detailPanel.y + 364, 92, 30 }, Tr(TextId::CommonReload)))
			{
				modelHeightSettings = LoadModelHeightSettings();
			modelHeightMessage = Tr(TextId::ModelHeightReloaded);
				modelHeightMessageUntil = (Scene::Time() + 2.0);
			}

           if (DrawTextButton(Rect{ detailPanel.x + 216, detailPanel.y + 364, 92, 30 }, Tr(TextId::CommonResetAll)))
			{
				modelHeightSettings = {};
		   modelHeightMessage = Tr(TextId::ModelHeightOffsetsScalesReset);
				modelHeightMessageUntil = (Scene::Time() + 2.0);
			}
		}

		if (Scene::Time() < modelHeightMessageUntil)
		{
            SimpleGUI::GetFont()(modelHeightMessage).draw((detailPanel.x + 16), (detailPanel.y + 394), ColorF{ 0.12 });
		}
	}
}
