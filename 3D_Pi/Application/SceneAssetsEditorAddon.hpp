# pragma once
# include <Siv3D.hpp>
# include "../Editors/IEditorAddon.hpp"
# include "SceneAssets.hpp"

namespace app
{
    class SceneAssetsEditorAddon final : public IEditorAddon
    {
    public:
        explicit SceneAssetsEditorAddon(SceneAssets& sceneAssets)
            : m_sceneAssets{ sceneAssets } {}

        StringView id() const noexcept override
        {
            return U"SceneAssetsEditor";
        }

        StringView displayName() const noexcept override
        {
            return U"Scene Assets Editor";
        }

        void update(const EditorUpdateContext&) override
        {
            m_sceneAssets.updateEditor();
        }

        void draw3D(const EditorDraw3DContext&) override
        {
        }

        void drawUI(const EditorUIContext& context) override
        {
            if (context.uiHidden)
            {
                return;
            }

            m_sceneAssets.drawEditorUI();
        }

        bool wantsMouseCapture() const override
        {
            return m_sceneAssets.wantsMouseCapture();
        }

        bool wantsMouseWheelCapture() const override
        {
            return false;
        }

        bool isEnabled() const override
        {
            return true;
        }

        bool isPanelOpen() const override
        {
            return true;
        }

        bool handleCommand(EditorCommand) override
        {
            return false;
        }

    private:
        SceneAssets& m_sceneAssets;
    };
}
