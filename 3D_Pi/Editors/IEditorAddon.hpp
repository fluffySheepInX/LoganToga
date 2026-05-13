# pragma once
# include <Siv3D.hpp>

namespace app
{
    struct EditorAddonDescriptor
    {
        String id;
        String displayName;
        Optional<Input> toggleShortcut;
        int32 updateOrder = 0;
        int32 draw3DOrder = 0;
        int32 drawUIOrder = 0;
        int32 inputPriority = 0;
    };

    enum class EditorCommand
    {
        Toggle,
        Save,
        Load,
        Undo,
        Duplicate,
        DeleteSelection,
        Confirm,
        Cancel,
        ToggleGhost,
        CancelTransientTool,
    };

    struct EditorUpdateContext
    {
        const BasicCamera3D& camera;
        bool cursorBlockedByUI = false;
        bool uiHidden = false;
        double deltaTime = 0.0;
        Size sceneSize{ 0, 0 };
    };

    struct EditorDraw3DContext
    {
        const BasicCamera3D& camera;
        bool uiHidden = false;
    };

    struct EditorUIContext
    {
        bool uiHidden = false;
        Size sceneSize{ 0, 0 };
        Optional<String> activeEditorId;
        Optional<String> focusedEditorId;
        Optional<String> hoveredEditorId;
    };

    class IEditorAddon
    {
    public:
        virtual ~IEditorAddon() = default;

        virtual const EditorAddonDescriptor& descriptor() const noexcept = 0;

        StringView id() const noexcept
        {
            return descriptor().id;
        }

        StringView displayName() const noexcept
        {
            return descriptor().displayName;
        }

        virtual void update(const EditorUpdateContext& context) = 0;
        virtual void draw3D(const EditorDraw3DContext& context) = 0;
        virtual void drawUI(const EditorUIContext& context) = 0;

        virtual bool wantsMouseCapture() const = 0;
        virtual bool wantsMouseWheelCapture() const = 0;
        virtual bool isEnabled() const = 0;
        virtual bool isPanelOpen() const = 0;

        virtual bool handleCommand(EditorCommand command) = 0;
    };
}
