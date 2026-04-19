# include "MainUiModelHeightInternal.hpp"
# include "SkyAppUiInternal.hpp"

namespace MainSupport::ModelHeightEditorDetail
{
    bool& ModelListCollapsed()
    {
        static bool collapsed = false;
        return collapsed;
    }

    bool& TextureInfoCollapsed()
    {
        static bool collapsed = false;
        return collapsed;
    }

    bool& TextureSegmentCollapsed()
    {
        static bool collapsed = false;
        return collapsed;
    }

    StringView ToModelHeightTargetLabel(const size_t previewModelIndex, const Array<String>& previewModelLabels)
    {
        return previewModelLabels[previewModelIndex];
    }

    StringView ToAnimationRoleLabel(const UnitModelAnimationRole role)
    {
        switch (role)
        {
        case UnitModelAnimationRole::Move:
            return U"Move";

        case UnitModelAnimationRole::Attack:
            return U"Attack";

        case UnitModelAnimationRole::Idle:
        default:
            return U"Idle";
        }
    }

    double GetActiveModelScale(const ModelHeightSettings& modelHeightSettings, FilePathView modelPath)
    {
        return GetModelScale(modelHeightSettings, modelPath);
    }

    double GetModelHeightWorldY(const size_t previewModelIndex, const Array<Vec3>& previewRenderPositions)
    {
        return previewRenderPositions[previewModelIndex].y;
    }

    StringView ToTextureTargetLabel(const TireTrackTextureSegment segment)
    {
        return GetTireTrackTextureSegmentLabel(segment);
    }

    double RoundModelHeightEditorValue(const double value, const double roundStep)
    {
        if (roundStep <= 0.0)
        {
            return value;
        }

        return (Math::Round(value / roundStep) * roundStep);
    }

    String ToCompactTextureName(const StringView label)
    {
        constexpr size_t MaxVisibleCharacters = 7;
        return ToCompactLabel(label, MaxVisibleCharacters);
    }

    String ToCompactLabel(const StringView label, const size_t maxVisibleCharacters)
    {
        if (label.size() <= maxVisibleCharacters)
        {
            return String{ label };
        }

        return (label.substr(0, maxVisibleCharacters) + U"...");
    }

    void DrawEditorSectionLabel(const Rect& panelRect, const StringView label)
    {
        static const Font sectionFont{ 16, Typeface::Bold };
        sectionFont(label).draw((panelRect.x + 4), panelRect.y, SkyAppSupport::UiInternal::EditorTextOnPanelPrimaryColor());
    }

    bool DrawMiniHandleButton(const Rect& rect, const StringView label)
    {
        const bool hovered = rect.mouseOver();
        rect.rounded(4).draw(hovered ? ColorF{ 0.96, 0.97, 0.99, 0.92 } : ColorF{ 0.88, 0.90, 0.94, 0.86 })
            .drawFrame(1.0, 0.0, ColorF{ 0.42, 0.48, 0.58, 0.86 });
        SimpleGUI::GetFont()(label).drawAt(rect.center(), SkyAppSupport::UiInternal::EditorTextOnCardPrimaryColor());
        return hovered && MouseL.down();
    }

    void DrawCompactSelectionCard(const Rect& rect, const StringView title, const bool selected)
    {
        const bool hovered = rect.mouseOver();
        rect.rounded(8).draw(selected
            ? ColorF{ 0.33, 0.53, 0.82, 0.96 }
            : (hovered ? ColorF{ 0.96, 0.97, 0.99, 0.90 } : ColorF{ 0.98, 0.97, 0.95, 0.82 }))
            .drawFrame(1.0, 0.0, selected ? ColorF{ 0.20, 0.32, 0.52 } : ColorF{ 0.58, 0.56, 0.52, 0.84 });
        SimpleGUI::GetFont()(title).draw((rect.x + 12), (rect.y + 16), selected ? ColorF{ 0.98 } : SkyAppSupport::UiInternal::EditorTextOnCardPrimaryColor());
    }

    void CopyTextureSegmentSettings(ModelHeightSettings& destination,
        const ModelHeightSettings& source,
        const TireTrackTextureSegment segment)
    {
        GetTireTrackYOffset(destination, segment) = GetTireTrackYOffset(source, segment);
        GetTireTrackOpacity(destination, segment) = GetTireTrackOpacity(source, segment);
        GetTireTrackSoftness(destination, segment) = GetTireTrackSoftness(source, segment);
        GetTireTrackWarmth(destination, segment) = GetTireTrackWarmth(source, segment);
    }

    void DrawCompactTextureParameterRow(const Rect& rect,
        const int32 controlId,
        const StringView label,
        double& value,
        const double minValue,
        const double maxValue,
        const double roundStep,
        const int32 decimals)
    {
        static Optional<int32> activeControlId;

        const RectF sliderTrackRect{ (rect.x + 12.0), (rect.bottomY() - 10.0), (rect.w - 24.0), 6.0 };
        const bool hovered = rect.mouseOver() || sliderTrackRect.stretched(0.0, 8.0).mouseOver();

        if (MouseL.down() && hovered)
        {
            activeControlId = controlId;
        }

        const bool active = (activeControlId && (*activeControlId == controlId));
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

        SimpleGUI::GetFont()(label).draw((rect.x + 12), (rect.y + 8), SkyAppSupport::UiInternal::EditorTextOnCardPrimaryColor());

        String valueText;
        switch (decimals)
        {
        case 1:
            valueText = U"{:.1f}"_fmt(value);
            break;

        case 2:
            valueText = U"{:.2f}"_fmt(value);
            break;

        case 3:
        default:
            valueText = U"{:.3f}"_fmt(value);
            break;
        }

        SimpleGUI::GetFont()(valueText).draw((rect.rightX() - 82), (rect.y + 8), SkyAppSupport::UiInternal::EditorTextOnCardPrimaryColor());
        sliderTrackRect.rounded(4).draw(ColorF{ 0.14, 0.16, 0.20, active ? 0.94 : 0.72 });
        RectF{ sliderTrackRect.pos, (sliderTrackRect.w * ratio), sliderTrackRect.h }.rounded(4).draw(ColorF{ 0.38, 0.70, 0.96, active ? 0.98 : 0.88 });

        if (active || hovered)
        {
            RectF knobRect{ Arg::center = Vec2{ (sliderTrackRect.x + sliderTrackRect.w * ratio), sliderTrackRect.centerY() }, 10, 16 };
            knobRect.rounded(4).draw(ColorF{ 0.94, 0.97, 1.0 }).drawFrame(1.0, 0.0, ColorF{ 0.25, 0.34, 0.50, 0.95 });
        }
    }
}
