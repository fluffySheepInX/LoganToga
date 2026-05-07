# pragma once
# include <Siv3D.hpp>
# include "../Editors/IEditorAddon.hpp"
# include "RoadEditor.hpp"

namespace app
{
    class RoadEditorAddon final : public IEditorAddon
    {
    public:
        explicit RoadEditorAddon(const FilePath& texturePath = U"texture/road.jpg",
            const FilePath& savePath = U"data/roads.toml",
            const FilePath& presetsDir = U"data/road_presets/")
            : m_editor{ texturePath, savePath, presetsDir } {}

        StringView id() const noexcept override
        {
            return U"RoadEditor";
        }

        StringView displayName() const noexcept override
        {
            return U"Road Editor";
        }

        void update(const EditorUpdateContext& context) override
        {
            m_editor.update(context.camera);
        }

        void draw3D(const EditorDraw3DContext&) override
        {
            m_editor.draw3D();
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
        RoadEditor m_editor;
    };
}
