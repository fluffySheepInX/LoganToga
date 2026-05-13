# pragma once
# include <Siv3D.hpp>
# include <algorithm>
# include "../Editors/IEditorAddon.hpp"
# include "../UI/EditorIconLayout.hpp"
# include "../UI/RectUI.hpp"

namespace app
{
    enum class CameraWorkLookAtMode
    {
        None,
        Attacker,
        Target,
        Center,
        Bone,
        CustomPoint,
    };

    enum class CameraWorkInterpolation
    {
        Constant,
        Linear,
        EaseInOut,
    };

    struct CameraWorkShake
    {
        bool enabled = false;
        double duration = 0.18;
        double strength = 0.25;
        double frequency = 25.0;
        double damping = 0.8;
    };

    struct CameraWorkKeyframe
    {
        double time = 0.0;
        Vec3 position{ 0.0, 1.5, -4.0 };
        Vec3 rotationDegrees{ 10.0, 0.0, 0.0 };
        double fieldOfView = 55.0;
        CameraWorkLookAtMode lookAtMode = CameraWorkLookAtMode::Attacker;
        CameraWorkInterpolation interpolation = CameraWorkInterpolation::Linear;
        CameraWorkShake shake;
    };

    struct CameraWorkClip
    {
        String id = U"attack_camera";
        String name = U"Attack Camera";
        String motionName = U"Attack";
        double duration = 1.2;
        Array<CameraWorkKeyframe> keyframes;
    };

    struct CameraWorkPreviewState
    {
        Vec3 eye{ 0.0, 1.5, -4.0 };
        Vec3 focus{ 0.0, 1.5, -3.0 };
        double fieldOfView = 55.0;
    };

    class CameraWorkEditorAddon final : public IEditorAddon
    {
    public:
        explicit CameraWorkEditorAddon(const FilePath& savePath = U"data/camera_work.toml")
            : m_savePath{ savePath }
        {
            resetDefaultClip();
        }

        const EditorAddonDescriptor& descriptor() const noexcept override
        {
            static const EditorAddonDescriptor descriptor{
                U"CameraWorkEditor",
                U"Camera Work Editor",
                Optional<Input>{ KeyC },
                0,
                0,
                0,
                0
            };
            return descriptor;
        }

        void update(const EditorUpdateContext& context) override
        {
            syncCollapsedIconRegistry();
            if (not m_enabled)
            {
                m_draggingPanel = false;
                m_draggingMarker.reset();
                return;
            }

            m_lastDeltaTime = context.deltaTime;

            if (m_playing)
            {
                m_currentTime += context.deltaTime * m_playbackSpeed;
                if (m_currentTime > m_clip.duration)
                {
                    if (m_loop)
                    {
                        m_currentTime = 0.0;
                    }
                    else
                    {
                        m_currentTime = m_clip.duration;
                        m_playing = false;
                    }
                }

                m_scrubTime = m_currentTime;
            }

            m_status = U"Debug: play={} dt={:.4f} current={:.3f}s scrub={:.3f}s"_fmt(
                m_playing ? U"ON" : U"OFF", m_lastDeltaTime, m_currentTime, m_scrubTime);

            if (m_playing && (not m_clip.keyframes.isEmpty()))
            {
                m_selectedKeyframeIndex = findNearestKeyframeIndex(m_currentTime);
            }

            if (KeySpace.down() && isPanelOpen())
            {
                togglePlayback();
            }
        }

        void draw3D(const EditorDraw3DContext&) override
        {
            if (not m_enabled)
            {
                return;
            }

            for (size_t i = 0; i < m_clip.keyframes.size(); ++i)
            {
                const auto& key = m_clip.keyframes[i];
                const ColorF color = (i == m_selectedKeyframeIndex) ? ColorF{ Palette::Orange } : ColorF{ 0.25, 0.65, 1.0 };
                Sphere{ key.position, 0.13 }.draw(color);

                const Vec3 forward = rotationToForward(key.rotationDegrees);
                Line3D{ key.position, key.position + forward * 0.8 }.draw(color);

                if (i + 1 < m_clip.keyframes.size())
                {
                    Line3D{ key.position, m_clip.keyframes[i + 1].position }.draw(ColorF{ 0.25, 0.65, 1.0, 0.55 });
                }
            }

            if (m_clip.keyframes.size() >= 2)
            {
                const CameraWorkKeyframe preview = sampleKeyframe(activePreviewTime());
                Sphere{ preview.position + Vec3{ 0.0, 0.22, 0.0 }, 0.09 }.draw(Palette::Yellow);
                Line3D{ preview.position + Vec3{ 0.0, 0.22, 0.0 }, preview.position + Vec3{ 0.0, 0.22, 0.0 } + rotationToForward(preview.rotationDegrees) * 1.0 }.draw(Palette::Yellow);
            }
        }

        void drawUI(const EditorUIContext& context) override
        {
            if (context.uiHidden)
            {
                syncCollapsedIconRegistry();
                return;
            }

            syncCollapsedIconRegistry();
            const RectF panel = getPanelRect();

            if (m_collapsed)
            {
                drawCollapsedIcon();
                return;
            }

            ui::Panel(panel);
            drawHeader(panel);
            drawToolbar(panel);
            drawKeyframeList(panel);
            drawProperties(panel);
            drawTimeline(panel);
            m_smallFont(m_status).draw(panel.pos.movedBy(16, panel.h - 34), ui::GetTheme().textMuted);
        }

        bool wantsMouseCapture() const override
        {
            syncCollapsedIconRegistry();
            if (m_collapsed)
            {
                return false;
            }

            return getPanelRect().mouseOver();
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
            return (not m_collapsed);
        }

        bool handleCommand(EditorCommand command) override
        {
            switch (command)
            {
            case EditorCommand::Toggle:
                m_enabled = (not m_enabled);
                m_draggingPanel = false;
                m_draggingMarker.reset();
                m_status = m_enabled ? U"Camera Work Editor: ON" : U"Camera Work Editor: OFF";
                syncCollapsedIconRegistry();
                return true;
            case EditorCommand::Save:
                if (not m_enabled)
                {
                    return false;
                }
                save();
                return true;
            case EditorCommand::Load:
                if (not m_enabled)
                {
                    return false;
                }
                load();
                return true;
            case EditorCommand::Duplicate:
                if (not m_enabled)
                {
                    return false;
                }
                duplicateSelectedKeyframe();
                return true;
            case EditorCommand::DeleteSelection:
                if (not m_enabled)
                {
                    return false;
                }
                deleteSelectedKeyframe();
                return true;
            default:
                return false;
            }
        }

        [[nodiscard]] bool isPlaying() const noexcept
        {
            return m_playing;
        }

        [[nodiscard]] CameraWorkPreviewState previewState() const
        {
            const CameraWorkKeyframe key = sampleKeyframe(activePreviewTime());
            const Vec3 forward = rotationToForward(key.rotationDegrees);
            return CameraWorkPreviewState{
                .eye = key.position,
                .focus = key.position + forward,
                .fieldOfView = key.fieldOfView,
            };
        }

    private:
        static constexpr double PanelWidth = 620.0;
        static constexpr double PanelHeight = 800.0;
        static constexpr double RowHeight = 30.0;

        FilePath m_savePath;
        Font m_font{ 18 };
        Font m_smallFont{ 12 };
        Texture m_toggleIcon{ U"texture/camera.png" };
        CameraWorkClip m_clip;
        bool m_enabled = false;
        bool m_collapsed = true;
        Vec2 m_panelPos{ 84, 20 };
        bool m_draggingPanel = false;
        Vec2 m_dragOffset{ 0, 0 };
        Optional<size_t> m_draggingMarker;
        size_t m_selectedKeyframeIndex = 0;
        double m_currentTime = 0.0;
        double m_scrubTime = 0.0;
        bool m_playing = false;
        bool m_loop = true;
        double m_playbackSpeed = 1.0;
        double m_lastDeltaTime = 0.0;
        String m_status = U"Ready";

        [[nodiscard]] RectF getPanelRect() const
        {
            if (m_collapsed)
            {
                return getCollapsedIconRect();
            }

            const double panelHeight = Min(PanelHeight, Scene::Height() - 40.0);
            const Vec2 clampedPos{
                Clamp(m_panelPos.x, 0.0, Max(0.0, Scene::Width() - PanelWidth)),
                Clamp(m_panelPos.y, 0.0, Max(0.0, Scene::Height() - panelHeight))
            };
            return RectF{ clampedPos, PanelWidth, panelHeight };
        }

        [[nodiscard]] RectF getCollapsedIconRect() const
        {
            const Vec2 resolvedPos = ui::editor_icon::ResolveCollapsedIconPosition(U"CameraWorkEditor", m_panelPos);
            return RectF{ resolvedPos, ui::editor_icon::CollapsedIconSize, ui::editor_icon::CollapsedIconSize };
        }

        void updateCollapsedIconDrag(const RectF& dragRect)
        {
            const Vec2 desired = Cursor::PosF() - m_dragOffset;
            m_panelPos = ui::editor_icon::ResolveCollapsedIconPosition(U"CameraWorkEditor", desired, dragRect.size);
            syncCollapsedIconRegistry();
        }

        void expandFromCollapsedIcon(const RectF& button)
        {
            const double panelHeight = Min(PanelHeight, Scene::Height() - 40.0);
            m_enabled = true;
            m_collapsed = false;
            m_panelPos = ui::editor_icon::GetAnchoredTopRightPosition(button, SizeF{ PanelWidth, panelHeight });
            syncCollapsedIconRegistry();
        }

        void syncCollapsedIconRegistry() const
        {
            ui::editor_icon::RegisterCollapsedIcon(U"CameraWorkEditor", m_collapsed ? Optional<RectF>{ getCollapsedIconRect() } : none);
        }

        void drawCollapsedIcon()
        {
            const RectF button = getCollapsedIconRect();

            if (MouseR.down() && button.mouseOver())
            {
                m_draggingPanel = true;
                m_dragOffset = Cursor::PosF() - button.pos;
            }
            if (not MouseR.pressed())
            {
                m_draggingPanel = false;
            }
            if (m_draggingPanel)
            {
                updateCollapsedIconDrag(button);
            }

            button.draw(ColorF{ 1.0, 1.0, 1.0, 0.02 });
            button.drawFrame(2.0, Palette::Black);
            ui::editor_icon::DrawToggleIcon(m_toggleIcon, button, 64.0);
            if (not m_toggleIcon)
            {
                m_font(U"C").drawAt(button.center(), ui::GetTheme().text);
            }

            if (button.leftClicked())
            {
                expandFromCollapsedIcon(button);
            }
        }

        void drawHeader(const RectF& panel)
        {
            m_font(U"Camera Work Editor").draw(panel.pos.movedBy(16, 12), ui::GetTheme().text);
            m_smallFont(U"C : toggle / S : save / L : load / Space : play").draw(panel.pos.movedBy(16, 40), ui::GetTheme().textMuted);

            const RectF collapseButton{ panel.x + panel.w - 74, panel.y + 10, 64, 64 };
            const RectF dragHeader{ panel.x, panel.y, panel.w, 42 };
            if (MouseL.down() && dragHeader.mouseOver() && (not collapseButton.mouseOver()))
            {
                m_draggingPanel = true;
                m_dragOffset = Cursor::PosF() - m_panelPos;
            }
            if (MouseR.down() && collapseButton.mouseOver())
            {
                m_draggingPanel = true;
                m_dragOffset = Cursor::PosF() - m_panelPos;
            }
            if (not (MouseL.pressed() || MouseR.pressed()))
            {
                m_draggingPanel = false;
            }
            if (m_draggingPanel)
            {
                m_panelPos = Cursor::PosF() - m_dragOffset;
                m_panelPos.x = Clamp(m_panelPos.x, 0.0, Max(0.0, Scene::Width() - panel.w));
                m_panelPos.y = Clamp(m_panelPos.y, 0.0, Max(0.0, Scene::Height() - panel.h));
            }

            collapseButton.draw(ColorF{ 1.0, 1.0, 1.0, 0.02 });
            collapseButton.drawFrame(2.0, Palette::Black);
            ui::editor_icon::DrawToggleIcon(m_toggleIcon, collapseButton, 64.0);
            if (not m_toggleIcon)
            {
                m_font(U"C").drawAt(collapseButton.center(), ui::GetTheme().text);
            }
            if (collapseButton.leftClicked())
            {
                const Vec2 desiredCollapsedPos = ui::editor_icon::GetAnchoredTopRightPosition(
                    collapseButton, SizeF{ ui::editor_icon::CollapsedIconSize, ui::editor_icon::CollapsedIconSize });
                m_panelPos = ui::editor_icon::ResolveCollapsedIconPosition(U"CameraWorkEditor", desiredCollapsedPos);
                m_collapsed = true;
                syncCollapsedIconRegistry();
            }
        }

        void drawToolbar(const RectF& panel)
        {
            const RectF section{ panel.x + 14, panel.y + 80, panel.w - 28, 76 };
            ui::Section(section);
            m_smallFont(U"Clip: {} / Motion: {}"_fmt(m_clip.name, m_clip.motionName)).draw(section.pos.movedBy(12, 8), ui::GetTheme().textMuted);

            const double w = (section.w - 84.0) / 7.0;
            const double y = section.y + 34;
            const Array<String> labels{ U"+Key", U"Dup", U"Delete", U"Save", U"Load", U"Reset", m_playing ? U"Pause" : U"Play" };
            for (size_t i = 0; i < labels.size(); ++i)
            {
                const RectF button{ section.x + 12 + i * (w + 10), y, w, 30 };
                if (ui::Button(m_smallFont, labels[i], button))
                {
                    if (i == 0) addKeyframeAtCurrentTime();
                    else if (i == 1) duplicateSelectedKeyframe();
                    else if (i == 2) deleteSelectedKeyframe();
                    else if (i == 3) save();
                    else if (i == 4) load();
                    else if (i == 5) resetDefaultClip();
                    else if (i == 6) togglePlayback();
                }
            }
        }

        void drawKeyframeList(const RectF& panel)
        {
            const RectF section{ panel.x + 14, panel.y + 166, 176, 408 };
            ui::Section(section);
            m_smallFont(U"Keyframes").draw(section.pos.movedBy(12, 8), ui::GetTheme().textMuted);

            for (size_t i = 0; i < m_clip.keyframes.size(); ++i)
            {
                const RectF row{ section.x + 10, section.y + 30 + i * RowHeight, section.w - 20, RowHeight - 4 };
                if (row.y + row.h > section.y + section.h - 8)
                {
                    break;
                }

                const bool selected = (i == m_selectedKeyframeIndex);
                row.rounded(5).draw(selected ? ColorF{ 0.88, 0.94, 1.0 } : (row.mouseOver() ? ui::GetTheme().itemHovered : ui::GetTheme().item));
                row.rounded(5).drawFrame(selected ? 2.0 : 1.0, selected ? ui::GetTheme().accent : ui::GetTheme().panelBorder);
                m_smallFont(U"#{:02d}  {:.2f}s"_fmt(i + 1, m_clip.keyframes[i].time)).draw(row.pos.movedBy(8, 6), ui::GetTheme().text);

                if (row.leftClicked())
                {
                    m_selectedKeyframeIndex = i;
                    seekToTime(m_clip.keyframes[i].time);
                }
            }
        }

        void drawProperties(const RectF& panel)
        {
            const RectF section{ panel.x + 200, panel.y + 166, panel.w - 214, 408 };
            ui::Section(section);
            m_smallFont(U"Selected Keyframe Parameters").draw(section.pos.movedBy(12, 8), ui::GetTheme().textMuted);

            if (m_clip.keyframes.isEmpty())
            {
                m_smallFont(U"No keyframes").draw(section.pos.movedBy(12, 36), ui::GetTheme().textMuted);
                return;
            }

            m_selectedKeyframeIndex = Min(m_selectedKeyframeIndex, m_clip.keyframes.size() - 1);
            auto& key = m_clip.keyframes[m_selectedKeyframeIndex];
            const Vec2 base{ section.x + 12, section.y + 34 };
            const double labelWidth = 96.0;
            const double sliderWidth = section.w - 34.0;

            bool sortNeeded = false;
            sortNeeded |= ui::SliderH(U"Time", key.time, 0.0, m_clip.duration, base + Vec2{ 0, 0 }, labelWidth, sliderWidth - labelWidth);
            ui::SliderH(U"Pos X", key.position.x, -12.0, 12.0, base + Vec2{ 0, 34 }, labelWidth, sliderWidth - labelWidth);
            ui::SliderH(U"Pos Y", key.position.y, 0.0, 8.0, base + Vec2{ 0, 68 }, labelWidth, sliderWidth - labelWidth);
            ui::SliderH(U"Pos Z", key.position.z, -12.0, 12.0, base + Vec2{ 0, 102 }, labelWidth, sliderWidth - labelWidth);
            ui::SliderH(U"Rot X", key.rotationDegrees.x, -90.0, 90.0, base + Vec2{ 0, 136 }, labelWidth, sliderWidth - labelWidth);
            ui::SliderH(U"Rot Y", key.rotationDegrees.y, -180.0, 180.0, base + Vec2{ 0, 170 }, labelWidth, sliderWidth - labelWidth);
            ui::SliderH(U"Rot Z", key.rotationDegrees.z, -45.0, 45.0, base + Vec2{ 0, 204 }, labelWidth, sliderWidth - labelWidth);
            ui::SliderH(U"FOV", key.fieldOfView, 25.0, 80.0, base + Vec2{ 0, 238 }, labelWidth, sliderWidth - labelWidth);

            const RectF lookAtRow{ base.x, base.y + 280, sliderWidth, 30 };
            drawEnumButtons(lookAtRow, { U"None", U"Atk", U"Tgt", U"Center", U"Bone", U"Custom" }, static_cast<size_t>(key.lookAtMode), [&](size_t index)
            {
                key.lookAtMode = static_cast<CameraWorkLookAtMode>(index);
            });

            const RectF interpolationRow{ base.x, base.y + 316, sliderWidth, 30 };
            drawEnumButtons(interpolationRow, { U"Const", U"Linear", U"Ease" }, static_cast<size_t>(key.interpolation), [&](size_t index)
            {
                key.interpolation = static_cast<CameraWorkInterpolation>(index);
            });

            const RectF shakeButton{ base.x, base.y + 352, 116, 30 };
            if (ui::Button(m_smallFont, key.shake.enabled ? U"Shake: ON" : U"Shake: OFF", shakeButton))
            {
                key.shake.enabled = (not key.shake.enabled);
            }
            ui::SliderH(U"Strength", key.shake.strength, 0.0, 1.0, base + Vec2{ 124, 350 }, 70.0, sliderWidth - 194.0);

            if (sortNeeded)
            {
                sortKeyframes();
            }
        }

        void drawTimeline(const RectF& panel)
        {
            const RectF section{ panel.x + 14, panel.y + 586, panel.w - 28, 156 };
            ui::Section(section);
            m_smallFont(U"Timeline").draw(section.pos.movedBy(12, 8), ui::GetTheme().textMuted);
            m_smallFont(U"Debug play={} dt={:.4f}"_fmt(m_playing ? U"ON" : U"OFF", m_lastDeltaTime)).draw(section.pos.movedBy(section.w - 236, 8), Palette::Orange);
            m_smallFont(U"current={:.3f}s scrub={:.3f}s"_fmt(m_currentTime, m_scrubTime)).draw(section.pos.movedBy(section.w - 236, 26), Palette::Orange);

            ui::SliderH(U"Duration", m_clip.duration, 0.2, 6.0, section.pos.movedBy(12, 32), 94.0, section.w - 130.0);
            seekToTime(activePreviewTime());
            if (ui::SliderH(U"Time", m_scrubTime, 0.0, m_clip.duration, section.pos.movedBy(12, 66), 94.0, section.w - 130.0))
            {
                seekToTime(m_scrubTime);
            }

            const RectF loopButton{ section.x + 12, section.y + 104, 96, 30 };
            const RectF speedButton{ section.x + 118, section.y + 104, 96, 30 };
            if (ui::Button(m_smallFont, m_loop ? U"Loop: ON" : U"Loop: OFF", loopButton))
            {
                m_loop = (not m_loop);
            }
            if (ui::Button(m_smallFont, U"Speed x{:.2f}"_fmt(m_playbackSpeed), speedButton))
            {
                m_playbackSpeed = (m_playbackSpeed >= 2.0) ? 0.25 : (m_playbackSpeed * 2.0);
            }

            const RectF track{ section.x + 230, section.y + 110, section.w - 250, 12 };
            track.rounded(6).draw(ColorF{ 0.78, 0.82, 0.88, 1.0 });
            if (const auto activeSegmentIndex = findActiveSegmentStartIndex(activePreviewTime()))
            {
                const auto& a = m_clip.keyframes[*activeSegmentIndex];
                const auto& b = m_clip.keyframes[*activeSegmentIndex + 1];
                const double activeX = track.x + track.w * (a.time / Max(0.001, m_clip.duration));
                const double activeW = track.w * ((b.time - a.time) / Max(0.001, m_clip.duration));
                const double segmentDuration = Max(0.001, b.time - a.time);
                const double segmentProgress = Clamp((activePreviewTime() - a.time) / segmentDuration, 0.0, 1.0);
                const RectF activeSegment{ activeX, track.y, activeW, track.h };
                activeSegment.rounded(6).draw(ColorF{ 1.0, 0.84, 0.45, 0.28 });
                RectF{ activeX, track.y, activeW * segmentProgress, track.h }.rounded(6).draw(ColorF{ 1.0, 0.72, 0.22, 0.82 });
                activeSegment.rounded(6).drawFrame(1.0, ColorF{ 0.95, 0.62, 0.12, 0.9 });
            }
            const double cursorX = track.x + track.w * (activePreviewTime() / Max(0.001, m_clip.duration));
            Line{ Vec2{ cursorX, track.y - 10 }, Vec2{ cursorX, track.y + 22 } }.draw(2.0, Palette::Orange);

            if (MouseL.down() && track.stretched(8, 16).mouseOver())
            {
                seekToTime(Clamp((Cursor::PosF().x - track.x) / track.w, 0.0, 1.0) * m_clip.duration);
            }

            for (size_t i = 0; i < m_clip.keyframes.size(); ++i)
            {
                const double x = track.x + track.w * (m_clip.keyframes[i].time / Max(0.001, m_clip.duration));
                const RectF marker{ x - 5, track.y - 6, 10, 24 };
                const bool selected = (i == m_selectedKeyframeIndex);
                marker.rounded(3).draw(selected ? ColorF{ Palette::Orange } : ui::GetTheme().accent);
                if (selected)
                {
                    marker.rounded(3).drawFrame(2.0, Palette::White);
                }
                if (marker.leftClicked())
                {
                    m_selectedKeyframeIndex = i;
                    seekToTime(m_clip.keyframes[i].time);
                    m_draggingMarker = i;
                }
            }

            if (MouseL.pressed() && m_draggingMarker)
            {
                const size_t index = *m_draggingMarker;
                if (index < m_clip.keyframes.size())
                {
                    m_clip.keyframes[index].time = Clamp((Cursor::PosF().x - track.x) / track.w, 0.0, 1.0) * m_clip.duration;
                    seekToTime(m_clip.keyframes[index].time);
                    m_selectedKeyframeIndex = index;
                }
            }
            else if (m_draggingMarker)
            {
                m_draggingMarker.reset();
                sortKeyframes();
            }
        }

        void drawEnumButtons(const RectF& row, const Array<String>& labels, const size_t selectedIndex, const std::function<void(size_t)>& onSelect)
        {
            const double gap = 5.0;
            const double width = (row.w - gap * (labels.size() - 1)) / labels.size();
            for (size_t i = 0; i < labels.size(); ++i)
            {
                const RectF button{ row.x + i * (width + gap), row.y, width, row.h };
                if (ui::Button(m_smallFont, labels[i], button))
                {
                    onSelect(i);
                }
                if (i == selectedIndex)
                {
                    button.rounded(6).drawFrame(2.0, ui::GetTheme().accent);
                }
            }
        }

        void addKeyframeAtCurrentTime()
        {
            CameraWorkKeyframe key = sampleKeyframe(activePreviewTime());
            key.time = activePreviewTime();
            m_clip.keyframes << key;
            sortKeyframes();
            m_status = U"Added keyframe at {:.2f}s"_fmt(activePreviewTime());
        }

        void duplicateSelectedKeyframe()
        {
            if (m_clip.keyframes.isEmpty())
            {
                return;
            }

            m_selectedKeyframeIndex = Min(m_selectedKeyframeIndex, m_clip.keyframes.size() - 1);
            CameraWorkKeyframe key = m_clip.keyframes[m_selectedKeyframeIndex];
            key.time = Clamp(key.time + 0.1, 0.0, m_clip.duration);
            m_clip.keyframes << key;
            sortKeyframes();
            m_status = U"Duplicated keyframe";
        }

        void deleteSelectedKeyframe()
        {
            if (m_clip.keyframes.size() <= 1)
            {
                m_status = U"At least one keyframe is required";
                return;
            }

            m_selectedKeyframeIndex = Min(m_selectedKeyframeIndex, m_clip.keyframes.size() - 1);
            m_clip.keyframes.erase(m_clip.keyframes.begin() + static_cast<Array<CameraWorkKeyframe>::difference_type>(m_selectedKeyframeIndex));
            m_selectedKeyframeIndex = Min(m_selectedKeyframeIndex, m_clip.keyframes.size() - 1);
            seekToTime(m_clip.keyframes[m_selectedKeyframeIndex].time);
            m_status = U"Deleted keyframe";
        }

        [[nodiscard]] double activePreviewTime() const noexcept
        {
            return m_playing ? m_currentTime : m_scrubTime;
        }

        void seekToTime(const double time)
        {
            const double clamped = Clamp(time, 0.0, m_clip.duration);
            m_currentTime = clamped;
            m_scrubTime = clamped;
        }

        void togglePlayback()
        {
            if (m_playing)
            {
                m_playing = false;
                m_scrubTime = m_currentTime;
                return;
            }

            m_enabled = true;
            seekToTime(m_scrubTime);
            m_playing = true;
        }

        void sortKeyframes()
        {
            const double selectedTime = m_clip.keyframes.isEmpty() ? 0.0 : m_clip.keyframes[Min(m_selectedKeyframeIndex, m_clip.keyframes.size() - 1)].time;
            std::stable_sort(m_clip.keyframes.begin(), m_clip.keyframes.end(), [](const CameraWorkKeyframe& a, const CameraWorkKeyframe& b)
            {
                return a.time < b.time;
            });

            for (size_t i = 0; i < m_clip.keyframes.size(); ++i)
            {
                if (Math::Abs(m_clip.keyframes[i].time - selectedTime) < 0.0001)
                {
                    m_selectedKeyframeIndex = i;
                    break;
                }
            }
        }

        [[nodiscard]] size_t findNearestKeyframeIndex(const double time) const
        {
            if (m_clip.keyframes.isEmpty())
            {
                return 0;
            }

            size_t nearestIndex = 0;
            double nearestDistance = Math::Abs(m_clip.keyframes.front().time - time);
            for (size_t i = 1; i < m_clip.keyframes.size(); ++i)
            {
                const double distance = Math::Abs(m_clip.keyframes[i].time - time);
                if (distance < nearestDistance)
                {
                    nearestDistance = distance;
                    nearestIndex = i;
                }
            }

            return nearestIndex;
        }

        [[nodiscard]] Optional<size_t> findActiveSegmentStartIndex(const double time) const
        {
            if (m_clip.keyframes.size() < 2)
            {
                return none;
            }

            for (size_t i = 0; i + 1 < m_clip.keyframes.size(); ++i)
            {
                if ((m_clip.keyframes[i].time <= time) && (time <= m_clip.keyframes[i + 1].time))
                {
                    return i;
                }
            }

            return none;
        }

        [[nodiscard]] CameraWorkKeyframe sampleKeyframe(const double time) const
        {
            if (m_clip.keyframes.isEmpty())
            {
                return CameraWorkKeyframe{};
            }
            if (m_clip.keyframes.size() == 1 || time <= m_clip.keyframes.front().time)
            {
                return m_clip.keyframes.front();
            }
            if (time >= m_clip.keyframes.back().time)
            {
                return m_clip.keyframes.back();
            }

            for (size_t i = 0; i + 1 < m_clip.keyframes.size(); ++i)
            {
                const auto& a = m_clip.keyframes[i];
                const auto& b = m_clip.keyframes[i + 1];
                if (time < a.time || time > b.time)
                {
                    continue;
                }

                double t = (time - a.time) / Max(0.001, b.time - a.time);
                if (a.interpolation == CameraWorkInterpolation::Constant)
                {
                    t = 0.0;
                }
                else if (a.interpolation == CameraWorkInterpolation::EaseInOut)
                {
                    t = t * t * (3.0 - 2.0 * t);
                }

                CameraWorkKeyframe result = a;
                result.time = time;
                result.position = a.position.lerp(b.position, t);
                result.rotationDegrees = a.rotationDegrees.lerp(b.rotationDegrees, t);
                result.fieldOfView = Math::Lerp(a.fieldOfView, b.fieldOfView, t);
                return result;
            }

            return m_clip.keyframes.back();
        }

        [[nodiscard]] static Vec3 rotationToForward(const Vec3& degrees)
        {
            const double pitch = Math::ToRadians(degrees.x);
            const double yaw = Math::ToRadians(degrees.y);
            return Vec3{
                Math::Sin(yaw) * Math::Cos(pitch),
                -Math::Sin(pitch),
                Math::Cos(yaw) * Math::Cos(pitch)
            }.normalized();
        }

        [[nodiscard]] static String ToString(const CameraWorkLookAtMode mode)
        {
            switch (mode)
            {
            case CameraWorkLookAtMode::Attacker: return U"Attacker";
            case CameraWorkLookAtMode::Target: return U"Target";
            case CameraWorkLookAtMode::Center: return U"Center";
            case CameraWorkLookAtMode::Bone: return U"Bone";
            case CameraWorkLookAtMode::CustomPoint: return U"CustomPoint";
            default: return U"None";
            }
        }

        [[nodiscard]] static CameraWorkLookAtMode ToLookAtMode(const String& text)
        {
            if (text == U"Attacker") return CameraWorkLookAtMode::Attacker;
            if (text == U"Target") return CameraWorkLookAtMode::Target;
            if (text == U"Center") return CameraWorkLookAtMode::Center;
            if (text == U"Bone") return CameraWorkLookAtMode::Bone;
            if (text == U"CustomPoint") return CameraWorkLookAtMode::CustomPoint;
            return CameraWorkLookAtMode::None;
        }

        [[nodiscard]] static String ToString(const CameraWorkInterpolation interpolation)
        {
            switch (interpolation)
            {
            case CameraWorkInterpolation::Constant: return U"Constant";
            case CameraWorkInterpolation::EaseInOut: return U"EaseInOut";
            default: return U"Linear";
            }
        }

        [[nodiscard]] static CameraWorkInterpolation ToInterpolation(const String& text)
        {
            if (text == U"Constant") return CameraWorkInterpolation::Constant;
            if (text == U"EaseInOut") return CameraWorkInterpolation::EaseInOut;
            return CameraWorkInterpolation::Linear;
        }

        void resetDefaultClip()
        {
            m_clip = CameraWorkClip{};
            m_clip.keyframes = {
                CameraWorkKeyframe{ .time = 0.0, .position = Vec3{ 0.0, 1.5, -4.0 }, .rotationDegrees = Vec3{ 10.0, 0.0, 0.0 }, .fieldOfView = 55.0 },
                CameraWorkKeyframe{ .time = 0.35, .position = Vec3{ 1.2, 1.1, -2.2 }, .rotationDegrees = Vec3{ 4.0, -24.0, 0.0 }, .fieldOfView = 70.0, .lookAtMode = CameraWorkLookAtMode::Target },
                CameraWorkKeyframe{ .time = 0.70, .position = Vec3{ -0.6, 1.8, -4.8 }, .rotationDegrees = Vec3{ 12.0, 8.0, 0.0 }, .fieldOfView = 52.0, .lookAtMode = CameraWorkLookAtMode::Center },
            };
            m_selectedKeyframeIndex = 0;
            m_playing = false;
            seekToTime(0.0);
            m_status = U"Default clip loaded";
        }

        void save()
        {
            const FilePath dir = FileSystem::ParentPath(m_savePath);
            if (not dir.isEmpty())
            {
                FileSystem::CreateDirectories(dir);
            }

            TextWriter writer{ m_savePath };
            if (not writer)
            {
                m_status = U"Failed to save {}"_fmt(m_savePath);
                return;
            }

            writer.writeln(U"id = \"{}\""_fmt(m_clip.id));
            writer.writeln(U"name = \"{}\""_fmt(m_clip.name));
            writer.writeln(U"motionName = \"{}\""_fmt(m_clip.motionName));
            writer.writeln(U"duration = {:.3f}"_fmt(m_clip.duration));
            writer.writeln(U"");

            for (const auto& key : m_clip.keyframes)
            {
                writer.writeln(U"[[keyframes]]");
                writer.writeln(U"time = {:.3f}"_fmt(key.time));
                writer.writeln(U"positionX = {:.3f}"_fmt(key.position.x));
                writer.writeln(U"positionY = {:.3f}"_fmt(key.position.y));
                writer.writeln(U"positionZ = {:.3f}"_fmt(key.position.z));
                writer.writeln(U"rotationX = {:.3f}"_fmt(key.rotationDegrees.x));
                writer.writeln(U"rotationY = {:.3f}"_fmt(key.rotationDegrees.y));
                writer.writeln(U"rotationZ = {:.3f}"_fmt(key.rotationDegrees.z));
                writer.writeln(U"fov = {:.3f}"_fmt(key.fieldOfView));
                writer.writeln(U"lookAtMode = \"{}\""_fmt(ToString(key.lookAtMode)));
                writer.writeln(U"interpolation = \"{}\""_fmt(ToString(key.interpolation)));
                writer.writeln(U"shakeEnabled = {}"_fmt(key.shake.enabled ? U"true" : U"false"));
                writer.writeln(U"shakeDuration = {:.3f}"_fmt(key.shake.duration));
                writer.writeln(U"shakeStrength = {:.3f}"_fmt(key.shake.strength));
                writer.writeln(U"shakeFrequency = {:.3f}"_fmt(key.shake.frequency));
                writer.writeln(U"shakeDamping = {:.3f}"_fmt(key.shake.damping));
                writer.writeln(U"");
            }

            m_status = U"Saved {}"_fmt(m_savePath);
        }

        void load()
        {
            if (not FileSystem::Exists(m_savePath))
            {
                m_status = U"File not found: {}"_fmt(m_savePath);
                return;
            }

            const TOMLReader toml{ m_savePath };
            if (not toml)
            {
                m_status = U"Failed to load {}"_fmt(m_savePath);
                return;
            }

            CameraWorkClip loaded;
            loaded.id = toml[U"id"].getOr<String>(U"attack_camera");
            loaded.name = toml[U"name"].getOr<String>(U"Attack Camera");
            loaded.motionName = toml[U"motionName"].getOr<String>(U"Attack");
            loaded.duration = toml[U"duration"].getOr<double>(1.2);

            for (const auto& v : toml[U"keyframes"].tableArrayView())
            {
                CameraWorkKeyframe key;
                key.time = v[U"time"].getOr<double>(0.0);
                key.position.x = v[U"positionX"].getOr<double>(0.0);
                key.position.y = v[U"positionY"].getOr<double>(1.5);
                key.position.z = v[U"positionZ"].getOr<double>(-4.0);
                key.rotationDegrees.x = v[U"rotationX"].getOr<double>(10.0);
                key.rotationDegrees.y = v[U"rotationY"].getOr<double>(0.0);
                key.rotationDegrees.z = v[U"rotationZ"].getOr<double>(0.0);
                key.fieldOfView = v[U"fov"].getOr<double>(55.0);
                key.lookAtMode = ToLookAtMode(v[U"lookAtMode"].getOr<String>(U"Attacker"));
                key.interpolation = ToInterpolation(v[U"interpolation"].getOr<String>(U"Linear"));
                key.shake.enabled = v[U"shakeEnabled"].getOr<bool>(false);
                key.shake.duration = v[U"shakeDuration"].getOr<double>(0.18);
                key.shake.strength = v[U"shakeStrength"].getOr<double>(0.25);
                key.shake.frequency = v[U"shakeFrequency"].getOr<double>(25.0);
                key.shake.damping = v[U"shakeDamping"].getOr<double>(0.8);
                loaded.keyframes << key;
            }

            if (loaded.keyframes.isEmpty())
            {
                loaded.keyframes << CameraWorkKeyframe{};
            }

            m_clip = loaded;
            m_selectedKeyframeIndex = 0;
            m_playing = false;
            seekToTime(m_clip.keyframes.front().time);
            sortKeyframes();
            m_status = U"Loaded {} keyframes"_fmt(m_clip.keyframes.size());
        }
    };
}
