# pragma once
# include <Siv3D.hpp>
# include "../Editors/IEditorAddon.hpp"
# include "StairGenerator.hpp"

namespace app
{
    class ProceduralEditorAddon final : public IEditorAddon
    {
    public:
        StringView id() const noexcept override
        {
            return U"ProceduralEditor";
        }

        StringView displayName() const noexcept override
        {
            return U"Procedural Editor";
        }

        void update(const EditorUpdateContext& context) override
        {
            m_editor.setUIHidden(context.uiHidden);
            m_editor.update(context.camera, context.cursorBlockedByUI);
        }

        void draw3D(const EditorDraw3DContext& context) override
        {
            m_editor.draw3D(not context.uiHidden);
        }

        void cancelTargetSelection()
        {
            m_editor.cancelTargetSelection();
        }

        void drawUI(const EditorUIContext& context) override
        {
            if (context.uiHidden)
            {
                return;
            }

            m_editor.drawUI();
        }

        bool wantsMouseCapture() const override
        {
            return m_editor.wantsMouseCapture();
        }

        bool wantsMouseWheelCapture() const override
        {
            return false;
        }

        bool isEnabled() const override
        {
            return m_editor.isEnabled();
        }

        bool isPanelOpen() const override
        {
            return m_editor.isPanelOpen();
        }

        bool handleCommand(EditorCommand command) override
        {
            return m_editor.handleCommand(command);
        }

    private:
        procedural::StairGenerator m_editor;
    };
}
