# include "SkyAppUiMenusInternal.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
    namespace
    {
        struct EscMenuItem
        {
            StringView label;
            EscMenuAction action = EscMenuAction::None;
        };
    }

    EscMenuAction DrawEscMenu(const Rect& panelRect)
    {
        const Font& font = SimpleGUI::GetFont();

        static constexpr EscMenuItem Items[]
        {
            { U"Restart", EscMenuAction::Restart },
            { U"Title", EscMenuAction::Title },
            { U"1280 x 720", EscMenuAction::Resize1280x720 },
            { U"1600 x 900", EscMenuAction::Resize1600x900 },
            { U"1920 x 1080", EscMenuAction::Resize1920x1080 },
        };

        Scene::Rect().draw(ColorF{ 0.0, 0.0, 0.0, 0.35 });
        UiInternal::DrawPanelFrame(panelRect, U"ESC Menu", ColorF{ 0.98, 0.96, 0.94, 0.98 }, UiInternal::DefaultPanelFrameColor, UiInternal::DefaultPanelTitleColor, MainSupport::PanelSkinTarget::Hud);

        for (size_t i = 0; i < std::size(Items); ++i)
        {
            const Rect buttonRect = SkyAppUiLayout::MenuWideButton(panelRect, (48 + static_cast<int32>(i) * 36));

            if (DrawTextButton(buttonRect, Items[i].label))
            {
                return Items[i].action;
            }
        }

        font(U"Window: {} x {}"_fmt(Scene::Width(), Scene::Height())).draw((panelRect.x + 16), (panelRect.y + 236), ColorF{ 0.18 });
        font(U"Press ESC to close").draw((panelRect.x + 16), (panelRect.y + 258), ColorF{ 0.18 });
        return EscMenuAction::None;
    }
}
