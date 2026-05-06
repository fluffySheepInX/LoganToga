# include "../stdafx.h"
# include "CameraController.hpp"

namespace app
{
    namespace
    {
        void ShowCopyFrameToastImpl(StringView message)
        {
# if SIV3D_PLATFORM(WINDOWS)
            if (Platform::Windows::ToastNotification::IsAvailable())
            {
                Platform::Windows::ToastNotification::Show(ToastNotificationItem{
                    .title = U"3D Pi",
                    .message = String{ message },
                    .audio = false,
                });
            }
# endif
        }
    }

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
        processPendingFrameCopy();

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
        const double buttonGap = 8.0;
        const double buttonSize = 32.0;
        const ColorF hoverOverlay{ 1.0, 0.96, 0.55, 0.22 };
        const ColorF selectedOverlay{ 1.0, 0.96, 0.55, 0.32 };

        for (size_t i = 0; i < m_presets.size(); ++i)
        {
            const RectF buttonRect{
                cameraPresetPanel.x + (i * (buttonSize + buttonGap)),
                cameraPresetPanel.y,
                buttonSize,
                buttonSize
            };

            const bool hovered = buttonRect.mouseOver();
            buttonRect.draw(ColorF{ 1.0, 1.0, 1.0, 0.02 });
            buttonRect.drawFrame(2.0, Palette::Black);

            if (buttonRect.leftClicked())
            {
                applyPreset(i);
            }

            if (const auto icon = getPresetIcon(m_presets[i].label))
            {
                const Texture& texture = icon->get();
                const double iconScale = Min(buttonRect.w / texture.width(), buttonRect.h / texture.height());
                texture.scaled(iconScale).drawAt(buttonRect.center());
            }
            else
            {
                ui::DefaultFont()(m_presets[i].label).drawAt(buttonRect.center(), ui::GetTheme().text);
            }

            if (hovered)
            {
                buttonRect.draw(hoverOverlay);
            }

            if (i == m_activePresetIndex)
            {
                buttonRect.draw(selectedOverlay);
            }
        }

        const RectF resetButtonRect{
            cameraPresetPanel.x + (3 * (buttonSize + buttonGap)),
            cameraPresetPanel.y,
            buttonSize,
            buttonSize
        };
        const bool resetHovered = resetButtonRect.mouseOver();
        resetButtonRect.draw(ColorF{ 1.0, 1.0, 1.0, 0.02 });
        resetButtonRect.drawFrame(2.0, Palette::Black);
        if (m_resetIcon)
        {
            const double resetIconScale = Min(resetButtonRect.w / m_resetIcon.width(), resetButtonRect.h / m_resetIcon.height());
            m_resetIcon.scaled(resetIconScale).drawAt(resetButtonRect.center());
        }
        else
        {
            ui::DefaultFont()(U"R").drawAt(resetButtonRect.center(), ui::GetTheme().text);
        }
        if (resetButtonRect.leftClicked())
        {
            resetCamera();
        }
        if (resetHovered)
        {
            resetButtonRect.draw(hoverOverlay);
        }

        const RectF previewButton = getPreviewButtonRect();
        const bool previewHovered = previewButton.mouseOver();
        previewButton.draw(ColorF{ 1.0, 1.0, 1.0, 0.02 });
        previewButton.drawFrame(2.0, Palette::Black);
        if (m_previewIcon)
        {
            const double previewIconScale = Min(previewButton.w / m_previewIcon.width(), previewButton.h / m_previewIcon.height());
            m_previewIcon.scaled(previewIconScale).drawAt(previewButton.center());
        }
        else
        {
            ui::DefaultFont()(U"Preview").drawAt(previewButton.center(), ui::GetTheme().text);
        }
        if (previewButton.leftClicked())
        {
            m_uiHiddenUntil = Scene::Time() + m_previewHideSeconds;
            events.previewStarted = true;
        }
        if (previewHovered)
        {
            previewButton.draw(hoverOverlay);
        }

        const RectF copyFrameButton = getCopyFrameButtonRect();
        const bool copyHovered = copyFrameButton.mouseOver();
        copyFrameButton.draw(ColorF{ 1.0, 1.0, 1.0, 0.02 });
        copyFrameButton.drawFrame(2.0, Palette::Black);
        if (m_copyIcon)
        {
            const double copyIconScale = Min(copyFrameButton.w / m_copyIcon.width(), copyFrameButton.h / m_copyIcon.height());
            m_copyIcon.scaled(copyIconScale).drawAt(copyFrameButton.center());
        }
        else
        {
            ui::DefaultFont()(U"Copy").drawAt(copyFrameButton.center(), ui::GetTheme().text);
        }
        if (copyFrameButton.leftClicked())
        {
            requestCopyCurrentFrameToClipboard();
        }
        if (copyHovered)
        {
            copyFrameButton.draw(hoverOverlay);
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
            && (getCameraPresetPanelRect().mouseOver() || getPreviewButtonRect().mouseOver() || getCopyFrameButtonRect().mouseOver());
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

    void CameraController::requestCopyCurrentFrameToClipboard()
    {
        if (m_frameCopyPending)
        {
            return;
        }

        ScreenCapture::RequestCurrentFrame();
        m_frameCopyPending = true;
    }

    void CameraController::processPendingFrameCopy()
    {
        if (not m_frameCopyPending)
        {
            return;
        }

        m_frameCopyPending = false;

        Image screenshot;
        if (ScreenCapture::GetFrame(screenshot) && screenshot)
        {
            Clipboard::SetImage(screenshot);
            showCopyFrameToast(U"Copy Frame: copied to clipboard");
        }
        else
        {
            showCopyFrameToast(U"Copy Frame: failed");
        }
    }

    void CameraController::showCopyFrameToast(StringView message)
    {
        ShowCopyFrameToastImpl(message);
    }

    Optional<std::reference_wrapper<const Texture>> CameraController::getPresetIcon(StringView label) const
    {
        if ((label == U"Low") && m_lowPresetIcon)
        {
            return std::cref(m_lowPresetIcon);
        }

        if (((label == U"Middle") || (label == U"Medium")) && m_middlePresetIcon)
        {
            return std::cref(m_middlePresetIcon);
        }

        if ((label == U"Top") && m_topPresetIcon)
        {
            return std::cref(m_topPresetIcon);
        }

        return none;
    }

    RectF CameraController::getCameraPresetPanelRect() const
    {
        constexpr double buttonSize = 32.0;
        constexpr double buttonGap = 8.0;
        constexpr double totalWidth = (buttonSize * 4.0) + (buttonGap * 3.0);
        return RectF{ (Scene::Width() * 0.5) - (totalWidth * 0.5), Scene::Height() - 56, totalWidth, buttonSize };
    }

    RectF CameraController::getPreviewButtonRect() const
    {
        const double windowBarHeight = 36.0;
        const double buttonSize = 96.0;
        const double offset = 10.0;
        return RectF{ 12, Scene::Height() - windowBarHeight - offset - buttonSize, buttonSize, buttonSize };
    }

    RectF CameraController::getCopyFrameButtonRect() const
    {
        const RectF previewButton = getPreviewButtonRect();
        const double buttonSize = 64.0;
        return RectF{ (previewButton.rightX() + 12), previewButton.y + (previewButton.h - buttonSize), buttonSize, buttonSize };
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
