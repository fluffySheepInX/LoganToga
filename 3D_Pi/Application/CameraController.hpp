# pragma once
# include <Siv3D.hpp>
# include <functional>
# include "../UI/RectUI.hpp"

namespace app
{
    struct CameraPreset
    {
        String label;
        Vec3 eye;
        Vec3 focus;
    };

    struct CameraControllerUpdateArgs
    {
        std::function<bool()> isDragPanBlocked;
        std::function<bool()> isWheelZoomBlocked;
        std::function<bool()> isFreeCameraBlocked;
        std::function<Optional<Vec3>(const DebugCamera3D&)> getWheelZoomFocus;
    };

    struct CameraControllerUIEvents
    {
        bool previewStarted = false;
    };

    class CameraController
    {
    public:
        explicit CameraController(const Size& sceneSize, Array<CameraPreset> presets = DefaultPresets());

        void update(const CameraControllerUpdateArgs& args);
        [[nodiscard]] CameraControllerUIEvents drawUI();

        [[nodiscard]] const DebugCamera3D& camera() const noexcept;
        [[nodiscard]] DebugCamera3D& camera() noexcept;
        [[nodiscard]] bool isUIHidden() const;
        [[nodiscard]] bool isCursorOnUI() const;

        [[nodiscard]] static Array<CameraPreset> DefaultPresets();

    private:
        [[nodiscard]] bool isBlocked(const std::function<bool()>& predicate) const;
        void applyPreset(const size_t index);
        void resetCamera();
        void requestCopyCurrentFrameToClipboard();
        void processPendingFrameCopy();
        static void showCopyFrameToast(StringView message);
        [[nodiscard]] Optional<std::reference_wrapper<const Texture>> getPresetIcon(StringView label) const;
        [[nodiscard]] RectF getCameraPresetPanelRect() const;
        [[nodiscard]] RectF getPreviewButtonRect() const;
        [[nodiscard]] RectF getCopyFrameButtonRect() const;
        [[nodiscard]] bool updateWheelDragPan(const std::function<bool()>& isBlocked);
        [[nodiscard]] bool updateWheelFocus(const CameraControllerUpdateArgs& args);

        DebugCamera3D m_camera;
        Array<CameraPreset> m_presets;
        size_t m_activePresetIndex = 0;
        double m_uiHiddenUntil = 0.0;
        double m_previewHideSeconds = 5.0;
        bool m_frameCopyPending = false;
        Texture m_lowPresetIcon{ U"texture/Low.png" };
        Texture m_middlePresetIcon{ U"texture/middle.png" };
        Texture m_topPresetIcon{ U"texture/top.png" };
        Texture m_resetIcon{ U"texture/reset.png" };
        Texture m_previewIcon{ U"texture/preview.png" };
        Texture m_copyIcon{ U"texture/copy.png" };
    };
}
