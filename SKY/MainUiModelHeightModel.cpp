# include "MainUiModelHeightInternal.hpp"
# include "MainSettings.hpp"
# include "SkyAppText.hpp"

namespace MainSupport::ModelHeightEditorDetail
{
    using SkyAppText::Tr;
    using SkyAppText::TrFormat;

    namespace
    {
        [[nodiscard]] String BuildSavedModelHeightKeyPrefix(FilePathView modelPath)
        {
            if (const auto renderModel = TryGetDefaultModelRenderModel(modelPath))
            {
                return String{ GetUnitRenderModelLabel(*renderModel) };
            }

            return U"\"Model:{}:"_fmt(MakeModelHeightFileKey(modelPath));
        }

        [[nodiscard]] String FinalizeSavedModelHeightKey(const String& keyPrefix, StringView suffix)
        {
            if (keyPrefix.starts_with(U"\"Model:"))
            {
                return (keyPrefix + suffix + U"\"");
            }

            return (keyPrefix + suffix);
        }

        [[nodiscard]] String BuildModelSaveDiagnosticsMessage(const ModelHeightSettings& settings, FilePathView modelPath)
        {
            const String keyPrefix = BuildSavedModelHeightKeyPrefix(modelPath);
            const String scaleKey = FinalizeSavedModelHeightKey(keyPrefix, U"Scale");
            const String offsetKey = FinalizeSavedModelHeightKey(keyPrefix, U"OffsetY");
            const String idleKey = FinalizeSavedModelHeightKey(keyPrefix, U"IdleAnimationClip");
            const String moveKey = FinalizeSavedModelHeightKey(keyPrefix, U"MoveAnimationClip");
            const String attackKey = FinalizeSavedModelHeightKey(keyPrefix, U"AttackAnimationClip");
            return U"{} = {:.3f}\n{} = {:.3f}\n{} = {}\n{} = {}\n{} = {}"_fmt(
                scaleKey,
                GetModelScale(settings, modelPath),
                offsetKey,
                GetModelHeightOffset(settings, modelPath),
                idleKey,
                GetModelAnimationClipIndex(settings, modelPath, UnitModelAnimationRole::Idle),
                moveKey,
                GetModelAnimationClipIndex(settings, modelPath, UnitModelAnimationRole::Move),
                attackKey,
                GetModelAnimationClipIndex(settings, modelPath, UnitModelAnimationRole::Attack));
        }
    }

    void DrawModelModeEditor(DrawModelHeightEditorContext& context, ModelHeightEditorBindings bindings, const Rect& listPanel, const Rect& detailPanel)
    {
        const int32 detailX = (detailPanel.x + 10);
        const int32 detailWidth = Max(180, (detailPanel.w - 20));
        const int32 compactRowHeight = 48;
        const int32 compactRowGap = 12;
        const int32 sectionTop = (detailPanel.y + 52);
        static int32 modelListScrollRow = 0;

        if (not ModelListCollapsed())
        {
            Rect{ (listPanel.rightX() + 3), (context.panelRect.y + 8), 1, (context.panelRect.h - 16) }.draw(ColorF{ 0.72, 0.72, 0.74 });
            const int32 visibleRowCount = Max(1, ((listPanel.h - 112) / 58));
            const int32 maxScrollRow = Max(0, (static_cast<int32>(context.previewModelPaths.size()) - visibleRowCount));
            if (listPanel.mouseOver())
            {
                modelListScrollRow = Clamp((modelListScrollRow - static_cast<int32>(Mouse::Wheel())), 0, maxScrollRow);
            }

            for (int32 visibleIndex = 0; visibleIndex < visibleRowCount; ++visibleIndex)
            {
                const size_t modelIndex = static_cast<size_t>(modelListScrollRow + visibleIndex);
                if (context.previewModelPaths.size() <= modelIndex)
                {
                    break;
                }

                const Rect buttonRect{ (listPanel.x + 12), (listPanel.y + 96 + visibleIndex * 58), 132, 48 };
                const bool selected = (context.activePreviewModelIndex == modelIndex);
                DrawCompactSelectionCard(buttonRect, ToCompactLabel(ToModelHeightTargetLabel(modelIndex, context.previewModelLabels), 8), selected);

                if (buttonRect.mouseOver() && MouseL.down())
                {
                    context.activePreviewModelIndex = modelIndex;
                }
            }

            if (DrawMiniHandleButton(Rect{ (listPanel.rightX() - TextureColumnHandleSize - 6), (listPanel.bottomY() - TextureColumnHandleSize - 8), TextureColumnHandleSize, TextureColumnHandleSize }, U"<"))
            {
                ModelListCollapsed() = true;
            }
        }
        else if (DrawMiniHandleButton(Rect{ listPanel.x, (listPanel.bottomY() - TextureColumnHandleSize - 8), TextureColumnHandleSize, TextureColumnHandleSize }, U">"))
        {
            ModelListCollapsed() = false;
        }

        DrawEditorSectionLabel(detailPanel, U"Parameters");
        SimpleGUI::GetFont()(ToModelHeightTargetLabel(context.activePreviewModelIndex, context.previewModelLabels)).draw((detailX + 2), (detailPanel.y + 28), ColorF{ 0.14 });
        DrawCompactTextureParameterRow(Rect{ detailX, sectionTop, detailWidth, compactRowHeight }, 0, U"offset", bindings.activeOffset, ModelHeightOffsetMin, ModelHeightOffsetMax, ModelHeightDragRoundStep, 3);
        DrawCompactTextureParameterRow(Rect{ detailX, (sectionTop + compactRowHeight + compactRowGap), detailWidth, compactRowHeight }, 1, U"scale", bindings.activeScale, ModelScaleMin, ModelScaleMax, ModelHeightDragRoundStep, 3);

        int32 summaryY = (sectionTop + (compactRowHeight + compactRowGap) * 2 + 10);

        if (context.activeModel.hasAnimations())
        {
            const auto& clips = context.activeModel.animations();
            for (const UnitModelAnimationRole role : { UnitModelAnimationRole::Idle, UnitModelAnimationRole::Move, UnitModelAnimationRole::Attack })
            {
                int32& clipIndex = GetModelAnimationClipIndex(context.modelHeightSettings, bindings.activePreviewModelPath, role);
                clipIndex = Clamp(clipIndex, -1, static_cast<int32>(clips.size() - 1));

                const Rect cardRect{ detailX, summaryY, detailWidth, 44 };
                const int32 buttonGap = 6;
                const int32 buttonWidth = Max(44, ((detailWidth - 24 - buttonGap * 3) / 4));
                const int32 buttonY = (cardRect.y + 20);
                const Rect prevClipButton{ (cardRect.x + 12), buttonY, buttonWidth, 18 };
                const Rect nextClipButton{ (prevClipButton.rightX() + buttonGap), buttonY, buttonWidth, 22 };
                const Rect clearClipButton{ (nextClipButton.rightX() + buttonGap), buttonY, buttonWidth, 22 };
                const Rect previewButton{ (clearClipButton.rightX() + buttonGap), buttonY, buttonWidth, 22 };

                const String clipSummary = (0 <= clipIndex)
                    ? U"{}/{}"_fmt((clipIndex + 1), clips.size())
                    : U"None";
                cardRect.rounded(8).draw(ColorF{ 0.96, 0.97, 0.99, 0.78 }).drawFrame(1.0, 0.0, ColorF{ 0.58, 0.64, 0.72, 0.84 });
                SimpleGUI::GetFont()(ToAnimationRoleLabel(role)).draw((cardRect.x + 12), (cardRect.y + 4), ColorF{ 0.12 });
                SimpleGUI::GetFont()(ToCompactLabel((0 <= clipIndex) ? clips[clipIndex].name : U"None", 12)).draw((cardRect.x + 76), (cardRect.y + 4), ColorF{ 0.12 });
                SimpleGUI::GetFont()(clipSummary).draw((cardRect.rightX() - 48), (cardRect.y + 4), ColorF{ 0.12 });

                if (DrawTextButton(prevClipButton, U"Prev"))
                {
                    clipIndex = (clipIndex < 0)
                        ? (static_cast<int32>(clips.size()) - 1)
                        : (clipIndex - 1);
                    context.previewAnimationRole = role;
                }

                if (DrawTextButton(nextClipButton, U"Next"))
                {
                    clipIndex = (clipIndex + 1);
                    if (static_cast<int32>(clips.size()) <= clipIndex)
                    {
                        clipIndex = -1;
                    }
                    context.previewAnimationRole = role;
                }

                if (DrawTextButton(clearClipButton, U"Clear"))
                {
                    clipIndex = -1;
                    context.previewAnimationRole = role;
                }

                if (DrawTextButton(previewButton, ((context.previewAnimationRole == role) ? U"Preview*" : U"Preview")))
                {
                    context.previewAnimationRole = role;
                }

                summaryY += 50;
            }
        }

        const int32 actionButtonWidth = Max(88, ((detailWidth - 8) / 2));
        if (DrawTextButton(Rect{ detailX, summaryY, actionButtonWidth, 30 }, Tr(U"ModelHeightResetTarget")))
        {
            bindings.activeOffset = 0.0;
            bindings.activeScale = 1.0;
            GetModelAnimationClipIndex(context.modelHeightSettings, bindings.activePreviewModelPath, UnitModelAnimationRole::Idle) = -1;
            GetModelAnimationClipIndex(context.modelHeightSettings, bindings.activePreviewModelPath, UnitModelAnimationRole::Move) = -1;
            GetModelAnimationClipIndex(context.modelHeightSettings, bindings.activePreviewModelPath, UnitModelAnimationRole::Attack) = -1;
            context.previewAnimationRole = UnitModelAnimationRole::Idle;
        }

        if (DrawTextButton(Rect{ detailX, (summaryY + 38), actionButtonWidth, 30 }, U"Apply Now"))
        {
            context.modelHeightMessage = U"Applied to current scene";
            context.modelHeightMessageUntil = (Scene::Time() + 2.0);
        }

        if (DrawTextButton(Rect{ (detailX + actionButtonWidth + 8), (summaryY + 38), actionButtonWidth, 30 }, Tr(U"CommonSave")))
        {
           if (SaveModelHeightSettings(context.modelHeightSettings))
            {
                context.modelHeightMessage = BuildModelSaveDiagnosticsMessage(context.modelHeightSettings, bindings.activePreviewModelPath);
                context.modelHeightMessageUntil = (Scene::Time() + 8.0);
            }
            else
            {
                context.modelHeightMessage = Tr(U"ModelHeightSaveFailed");
                context.modelHeightMessageUntil = (Scene::Time() + 2.0);
            }
        }

        if (DrawTextButton(Rect{ detailX, (summaryY + 76), detailWidth, 30 }, U"Reload File"))
        {
            context.modelHeightSettings = LoadModelHeightSettings();
            context.modelHeightMessage = Tr(U"ModelHeightReloaded");
            context.modelHeightMessageUntil = (Scene::Time() + 2.0);
        }

        if (DrawTextButton(Rect{ (detailX + actionButtonWidth + 8), summaryY, actionButtonWidth, 30 }, Tr(U"CommonResetAll")))
        {
            context.modelHeightSettings = {};
            context.modelHeightMessage = Tr(U"ModelHeightOffsetsScalesReset");
            context.modelHeightMessageUntil = (Scene::Time() + 2.0);
        }

     const bool showDetailedStatus = ((Scene::Time() < context.modelHeightMessageUntil) && context.modelHeightMessage.includes(U'\n'));
        const int32 statusHeight = (showDetailedStatus ? 84 : 24);
        const Rect statusRect{ detailX, Min((detailPanel.bottomY() - statusHeight - 4), (summaryY + 114)), detailWidth, statusHeight };
        statusRect.rounded(6).draw(ColorF{ 0.96, 0.97, 0.99, 0.78 }).drawFrame(1.0, 0.0, ColorF{ 0.58, 0.64, 0.72, 0.84 });
     const String statusText = ((Scene::Time() < context.modelHeightMessageUntil) && (not showDetailedStatus))
            ? ToCompactLabel(context.modelHeightMessage, 26)
            : ToCompactLabel(U"worldY {:.3f} / scale {:.3f}"_fmt(GetModelHeightWorldY(context.activePreviewModelIndex, context.previewRenderPositions), GetActiveModelScale(context.modelHeightSettings, bindings.activePreviewModelPath)), 26);
       if (showDetailedStatus)
        {
            static const Font diagnosticsFont{ 11 };
            const Array<String> lines = context.modelHeightMessage.split(U'\n');
            for (size_t i = 0; i < lines.size(); ++i)
            {
                diagnosticsFont(lines[i]).draw((statusRect.x + 8), (statusRect.y + 2 + static_cast<int32>(i) * 15), ColorF{ 0.12 });
            }
        }
        else
        {
            SimpleGUI::GetFont()(statusText).draw((statusRect.x + 10), statusRect.y, ColorF{ 0.12 });
        }
    }
}
