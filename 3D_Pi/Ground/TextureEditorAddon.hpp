# pragma once
# include <Siv3D.hpp>
# include "../Editors/IEditorAddon.hpp"
# include "TextureEditor.hpp"

namespace app
{
    class TextureEditorAddon final : public IEditorAddon
    {
    public:
        explicit TextureEditorAddon(const FilePath& savePath = U"data/ground_layers.toml")
            : m_editor{ savePath } {}

        const EditorAddonDescriptor& descriptor() const noexcept override
        {
            static const EditorAddonDescriptor descriptor{
                U"TextureEditor",
                U"Texture Editor",
                Optional<Input>{ KeyT },
                0,
                0,
                0,
                0
            };
            return descriptor;
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
            return m_editor.wantsMouseWheelCapture();
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
        TextureEditor m_editor;
    };
}
