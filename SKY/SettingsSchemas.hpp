# pragma once
# include "TomlSchema.hpp"
# include "MainContextTypes.hpp"
# include "AppContextTypes.hpp"

namespace MainSupport::SettingsSchemas
{
    // --- EditorTextColorSettings ---------------------------------------
    // Field order MUST match the previous hand-written Load/Save pairs in
    // MainSettingsEditorTextColors.cpp to keep TOML output byte-compatible.

    template <class V, class P>
    void VisitEditorTextColors(V&& v, P&& p)
    {
        constexpr TomlSchema::ColorFCodec c{};

        v(U"darkPrimary",         p.darkPrimary,         c);
        v(U"darkSecondary",       p.darkSecondary,       c);
        v(U"darkAccent",          p.darkAccent,          c);
        v(U"lightPrimary",        p.lightPrimary,        c);
        v(U"lightSecondary",      p.lightSecondary,      c);
        v(U"lightAccent",         p.lightAccent,         c);
        v(U"cardPrimary",         p.cardPrimary,         c);
        v(U"cardSecondary",       p.cardSecondary,       c);
        v(U"selectedPrimary",     p.selectedPrimary,     c);
        v(U"selectedSecondary",   p.selectedSecondary,   c);
        v(U"warning",             p.warning,             c);
        v(U"error",               p.error,               c);
    }

    // --- UiLayoutSettings (Point fields only) --------------------------
    // Order / keys must match MainSettingsUiLayout.cpp's existing TOML
    // output. battleCommandIconSize and post-load Clamp/Rect logic remain
    // procedural in the cpp file.

    // --- CameraSettings ------------------------------------------------

    template <class V, class P>
    void VisitCameraSettings(V&& v, P&& p)
    {
        constexpr TomlSchema::Vec3Codec vc{};

        v(U"eye",   p.eye,   vc);
        v(U"focus", p.focus, vc);
    }

    // --- ResourceStock (initial player resources) ----------------------

    template <class V, class P>
    void VisitInitialPlayerResources(V&& v, P&& p)
    {
        constexpr TomlSchema::DoubleCodec dc{};

        v(U"budget",    p.budget,    dc);
        v(U"gunpowder", p.gunpowder, dc);
        v(U"mana",      p.mana,      dc);
    }

    template <class V, class P>
    void VisitUiLayoutPositions(V&& v, P&& p)
    {
        constexpr TomlSchema::PointCodec pt{};

        v(U"miniMap",                p.miniMapPosition,                pt);
        v(U"miniMapSize",            p.miniMapSize,                    pt);
        v(U"resourcePanel",          p.resourcePanelPosition,          pt);
        v(U"resourcePanelSize",      p.resourcePanelSize,              pt);
        v(U"modelHeight",            p.modelHeightPosition,            pt);
        v(U"terrainVisualSettings",  p.terrainVisualSettingsPosition,  pt);
        v(U"fogSettings",            p.fogSettingsPosition,            pt);
        v(U"unitEditor",             p.unitEditorPosition,             pt);
        v(U"unitEditorList",         p.unitEditorListPosition,         pt);
    }
}
