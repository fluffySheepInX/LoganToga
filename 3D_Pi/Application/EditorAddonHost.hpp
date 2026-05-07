# pragma once
# include <Siv3D.hpp>
# include <memory>
# include "../Editors/IEditorAddon.hpp"

namespace app
{
    class EditorAddonHost
    {
    public:
        void registerAddon(std::unique_ptr<IEditorAddon> addon)
        {
            if (addon)
            {
                m_addons << std::move(addon);
            }
        }

        void update(const EditorUpdateContext& context)
        {
            for (auto& addon : m_addons)
            {
                addon->update(context);
            }

            for (auto& addon : m_addons)
            {
                if (addon->isEnabled() && addon->isPanelOpen() && addon->wantsMouseCapture())
                {
                    m_activeEditorId = String{ addon->id() };
                }
            }

            if (KeyS.down())
            {
                dispatchCommand(EditorCommand::Save);
            }
            if (KeyL.down())
            {
                dispatchCommand(EditorCommand::Load);
            }
            if (KeyControl.pressed() && KeyZ.down())
            {
                dispatchCommand(EditorCommand::Undo);
            }
            if (KeyControl.pressed() && KeyD.down())
            {
                dispatchCommand(EditorCommand::Duplicate);
            }
            if (KeyDelete.down())
            {
                dispatchCommand(EditorCommand::DeleteSelection);
            }
            if (KeyEnter.down())
            {
                dispatchCommand(EditorCommand::Confirm);
            }
            if (KeyEscape.down())
            {
                dispatchCommand(EditorCommand::Cancel);
            }
            if (KeyG.down())
            {
                dispatchCommand(EditorCommand::ToggleGhost);
            }

            if (KeyR.down())
            {
                setActiveEditor(U"RoadEditor");
                dispatchCommand(EditorCommand::Toggle);
            }

            if (KeyT.down())
            {
                setActiveEditor(U"TextureEditor");
                dispatchCommand(EditorCommand::Toggle);
            }
        }

        void draw3D(const EditorDraw3DContext& context)
        {
            for (auto& addon : m_addons)
            {
                addon->draw3D(context);
            }
        }

        void drawUI(const EditorUIContext& context)
        {
            for (auto& addon : m_addons)
            {
                addon->drawUI(context);
            }
        }

        bool wantsMouseCapture() const
        {
            for (const auto& addon : m_addons)
            {
                if (addon->isEnabled() && addon->wantsMouseCapture())
                {
                    return true;
                }
            }

            return false;
        }

        bool wantsMouseWheelCapture() const
        {
            for (const auto& addon : m_addons)
            {
                if (addon->isEnabled() && addon->wantsMouseWheelCapture())
                {
                    return true;
                }
            }

            return false;
        }

        Optional<String> activeEditorId() const
        {
            return m_activeEditorId;
        }

        void setActiveEditor(StringView id)
        {
            for (const auto& addon : m_addons)
            {
                if (addon->id() == id)
                {
                    m_activeEditorId = String{ id };
                    return;
                }
            }
        }

        bool dispatchCommand(const EditorCommand command)
        {
            if (m_activeEditorId)
            {
                for (auto& addon : m_addons)
                {
                    if (addon->id() == *m_activeEditorId)
                    {
                        if (addon->handleCommand(command))
                        {
                            return true;
                        }

                        break;
                    }
                }
            }

            for (auto& addon : m_addons)
            {
                if (addon->handleCommand(command))
                {
                    return true;
                }
            }

            return false;
        }

    private:
        Array<std::unique_ptr<IEditorAddon>> m_addons;
        Optional<String> m_activeEditorId;
    };
}
