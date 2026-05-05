# include "../stdafx.h"
# include "CameraController.hpp"

namespace app
{
    CameraController::CameraController(const Size& sceneSize, Array<CameraPreset> presets)
        : m_camera{ sceneSize, 40_deg, Vec3{ 0, 3, -16 } }
        , m_presets{ std::move(presets) }
    {
        if (m_presets.isEmpty())
        {
            m_presets = DefaultPresets();
        }

        resetCamera();
    }

    void CameraController::update(const CameraControllerUpdateArgs& args)
    {
        if (not updateWheelDragPan(args.isDragPanBlocked) && not updateWheelFocus(args))
        {
            if (not isBlocked(args.isFreeCameraBlocked))
            {
                m_camera.update(6.0);
            }
        }
    }

    CameraControllerUIEvents CameraController::drawUI()
    {
        CameraControllerUIEvents events;
        if (isUIHidden())
        {
            return events;
        }

        const RectF cameraPresetPanel = getCameraPresetPanelRect();
        ui::Panel(cameraPresetPanel);
        const double buttonGap = 8.0;
        const double innerPadding = 8.0;
        const double buttonWidth = (cameraPresetPanel.w - (innerPadding * 2.0) - (buttonGap * 3.0)) / 4.0;

        for (size_t i = 0; i < m_presets.size(); ++i)
        {
            const RectF buttonRect{
                cameraPresetPanel.x + innerPadding + (i * (buttonWidth + buttonGap)),
                cameraPresetPanel.y + 8,
                buttonWidth,
                32
            };

            if (ui::Button(ui::DefaultFont(), m_presets[i].label, buttonRect))
            {
                applyPreset(i);
            }

            if (i == m_activePresetIndex)
            {
                buttonRect.rounded(6).drawFrame(2.0, ui::GetTheme().accent);
            }
        }

        const RectF resetButtonRect{
            cameraPresetPanel.x + innerPadding + (3 * (buttonWidth + buttonGap)),
            cameraPresetPanel.y + 8,
            buttonWidth,
            32
        };
        if (ui::Button(ui::DefaultFont(), U"Reset", resetButtonRect))
        {
            resetCamera();
        }

        const RectF previewButton = getPreviewButtonRect();
        if (ui::Button(ui::DefaultFont(), U"Preview", previewButton))
        {
            m_uiHiddenUntil = Scene::Time() + m_previewHideSeconds;
            events.previewStarted = true;
        }

        return events;
    }

    const DebugCamera3D& CameraController::camera() const noexcept
    {
        return m_camera;
    }

    DebugCamera3D& CameraController::camera() noexcept
    {
        return m_camera;
    }

    bool CameraController::isUIHidden() const
    {
        return (Scene::Time() < m_uiHiddenUntil);
    }

    bool CameraController::isCursorOnUI() const
    {
        return (not isUIHidden())
            && (getCameraPresetPanelRect().mouseOver() || getPreviewButtonRect().mouseOver());
    }

    Array<CameraPreset> CameraController::DefaultPresets()
    {
        return {
            { U"Low", Vec3{ 0, 3, -16 }, Vec3{ 0, 0, 0 } },
            { U"Medium", Vec3{ 0, 10, -18 }, Vec3{ 0, 0, 0 } },
            { U"Top", Vec3{ 0, 24, -0.6 }, Vec3{ 0, 0, 0 } },
        };
    }

    bool CameraController::isBlocked(const std::function<bool()>& predicate) const
    {
        return predicate && predicate();
    }

    void CameraController::applyPreset(const size_t index)
    {
        if (index >= m_presets.size())
        {
            return;
        }

        m_activePresetIndex = index;
        const Vec3 currentFocus = m_camera.getFocusPosition();
        const Vec3 presetOffset = m_presets[index].eye - m_presets[index].focus;
        m_camera.setView(currentFocus + presetOffset, currentFocus);
    }

    void CameraController::resetCamera()
    {
        m_activePresetIndex = 0;
        m_camera.setView(m_presets[0].eye, m_presets[0].focus);
    }

    RectF CameraController::getCameraPresetPanelRect() const
    {
        return RectF{ (Scene::Width() * 0.5) - 266, Scene::Height() - 72, 532, 48 };
    }

    RectF CameraController::getPreviewButtonRect() const
    {
        const double windowBarHeight = 36.0;
        const double buttonHeight = 32.0;
        const double offset = 10.0;
        return RectF{ 12, Scene::Height() - windowBarHeight - buttonHeight - offset, 112, buttonHeight };
    }

    bool CameraController::updateWheelDragPan(const std::function<bool()>& blockPredicate)
    {
        if (not MouseM.pressed()) return false;
        if (isBlocked(blockPredicate)) return false;

        const Vec2 delta = Cursor::DeltaF();
        if (delta.isZero()) return true;

        const InfinitePlane gp{ Float3{ 0, 0, 0 }, Float3{ 0, 1, 0 } };
        const Vec2 curPos = Cursor::PosF();
        const Vec2 prevPos = curPos - delta;

        const auto curHit = m_camera.screenToRay(curPos).intersects(gp);
        const auto prevHit = m_camera.screenToRay(prevPos).intersects(gp);
        if (not curHit || not prevHit) return true;

        const Vec3 panDelta = m_camera.screenToRay(prevPos).point_at(*prevHit)
            - m_camera.screenToRay(curPos).point_at(*curHit);

        m_camera.setView(m_camera.getEyePosition() + panDelta, m_camera.getFocusPosition() + panDelta);
        return true;
    }

    bool CameraController::updateWheelFocus(const CameraControllerUpdateArgs& args)
    {
        if (isBlocked(args.isWheelZoomBlocked))
        {
            return false;
        }

        const double wheel = Mouse::Wheel();
        if (wheel == 0.0)
        {
            return false;
        }

        if (not args.getWheelZoomFocus)
        {
            return false;
        }

        const auto zoomFocus = args.getWheelZoomFocus(m_camera);
        if (not zoomFocus)
        {
            return false;
        }

        const Vec3 eye = m_camera.getEyePosition();
        const Vec3 eyeOffset = eye - *zoomFocus;
        const double distance = eyeOffset.length();
        if (distance <= 0.001)
        {
            return false;
        }

        const double zoomScale = Math::Pow(0.85, wheel);
        const double targetDistance = Clamp(distance * zoomScale, 3.0, 80.0);
        m_camera.setView(*zoomFocus + eyeOffset * (targetDistance / distance), *zoomFocus);
        return true;
    }
}
