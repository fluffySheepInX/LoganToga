# include "SkyAppUiUnitEditorInternal.hpp"
# include "MainSettings.hpp"
# include "MainUi.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
    namespace
    {
        [[nodiscard]] String ToModelReferenceLabel(const UnitEditorSettings& unitEditorSettings, const UnitTeam team, const SapperUnitType unitType)
        {
            const FilePath& modelPath = GetUnitModelPath(unitEditorSettings, team, unitType);

            if (modelPath.isEmpty())
            {
                return U"Default: {}"_fmt(GetUnitRenderModelLabel(GetUnitRenderModel(team, unitType)));
            }

            return FileSystem::FileName(modelPath);
        }

        struct ModelDiagnosticCache
        {
            FilePath path;
            String report;
        };

        [[nodiscard]] ModelDiagnosticCache& GetModelDiagnosticCache()
        {
            static ModelDiagnosticCache cache;
            return cache;
        }

        [[nodiscard]] FilePath ResolveUnitModelDiagnosticPath(const UnitEditorSettings& unitEditorSettings, const UnitTeam team, const SapperUnitType unitType)
        {
            const FilePath& modelPath = GetUnitModelPath(unitEditorSettings, team, unitType);
            if (not modelPath.isEmpty())
            {
                return modelPath;
            }

            switch (GetUnitRenderModel(team, unitType))
            {
            case UnitRenderModel::Ashigaru:
                return FilePath{ AshigaruModelPath };

            case UnitRenderModel::SugoiCar:
                return FilePath{ SugoiCarModelPath };

            case UnitRenderModel::Bird:
            default:
                return FilePath{ BirdModelPath };
            }
        }

        [[nodiscard]] String BuildUnitModelDiagnosticReport(const FilePath& path, const UnitRenderModel renderModel)
        {
            const UnitModel model{ path, BirdDisplayHeight, GetUnitRenderModelProceduralAnimationType(renderModel) };
            return U"[Model Path]\n{}\n\n{}"_fmt(FileSystem::FullPath(path), model.diagnosticReport());
        }
    }

    namespace UnitEditorDetail
    {
        void DrawUnitEditorSetupSection(const Rect& detailPanel,
            UnitEditorSettings& unitEditorSettings,
            const UnitTeam team,
            const SapperUnitType unitType,
            const UnitEditorPage activePage,
            UnitParameters& parameters,
            String& hoveredDescription,
            Optional<Rect>& hoveredRect,
            TimedMessage& unitEditorMessage)
        {
            using namespace UiParameterEditorDetail;

            if (activePage == UnitEditorPage::Basic)
            {
                SimpleGUI::GetFont()(U"Setup").draw((detailPanel.x + 16), (detailPanel.y + 118), UiInternal::EditorTextOnLightSecondaryColor());
                Rect{ (detailPanel.x + 86), (detailPanel.y + 130), (detailPanel.w - 102), 1 }.draw(ColorF{ 0.78, 0.82, 0.88 });
                DrawMovementTypeSelector(detailPanel, (detailPanel.y + 138), parameters.movementType);
                const Rect typeLabelRect{ (detailPanel.x + 16), (detailPanel.y + 134), 88, 28 };
                const Rect infantryButton{ (detailPanel.x + detailPanel.w - 146), (detailPanel.y + 136), 64, 24 };
                const Rect tankButton{ (detailPanel.x + detailPanel.w - 74), (detailPanel.y + 136), 58, 24 };
                if (typeLabelRect.mouseOver())
                {
                    hoveredDescription = U"移動タイプです。Infantry は素直な歩兵挙動、Tank は車両向けの重い移動カーブになります。";
                    hoveredRect = typeLabelRect;
                }
                else if (infantryButton.mouseOver())
                {
                    hoveredDescription = U"歩兵向けの移動タイプです。小回りが利きやすく、細かい進路変更に向いています。";
                    hoveredRect = infantryButton;
                }
                else if (tankButton.mouseOver())
                {
                    hoveredDescription = U"車両向けの移動タイプです。動き出しと停止がやや重く、車らしい進行になります。";
                    hoveredRect = tankButton;
                }

                if (team == UnitTeam::Enemy)
                {
                    DrawUnitAiRoleSelector(detailPanel, (detailPanel.y + 174), parameters.aiRole);
                    const Rect aiRoleLabelRect{ (detailPanel.x + 16), (detailPanel.y + 170), 96, 28 };
                    const Rect secureButton{ (detailPanel.x + detailPanel.w - 236), (detailPanel.y + 172), 72, 24 };
                    const Rect assaultButton{ (detailPanel.x + detailPanel.w - 156), (detailPanel.y + 172), 72, 24 };
                    const Rect supportButton{ (detailPanel.x + detailPanel.w - 76), (detailPanel.y + 172), 64, 24 };

                    if (aiRoleLabelRect.mouseOver())
                    {
                        hoveredDescription = U"敵 AI の役割です。資源確保を優先するか、拠点突撃を優先するか、味方支援に回るかを決めます。";
                        hoveredRect = aiRoleLabelRect;
                    }
                    else if (secureButton.mouseOver())
                    {
                        hoveredDescription = U"資源エリア確保を優先する役割です。未確保やプレイヤー支配中の資源地点へ向かいやすくなります。";
                        hoveredRect = secureButton;
                    }
                    else if (assaultButton.mouseOver())
                    {
                        hoveredDescription = U"敵拠点への攻勢を優先する役割です。資源よりもプレイヤー本拠地へ進軍しやすくなります。";
                        hoveredRect = assaultButton;
                    }
                    else if (supportButton.mouseOver())
                    {
                        hoveredDescription = U"味方主力の近くで行動する支援役です。近い非 Support 味方を追従し、いなければ前線寄りの集結地点へ向かいます。";
                        hoveredRect = supportButton;
                    }
                }

                const int32 modelRowY = ((team == UnitTeam::Enemy) ? (detailPanel.y + 206) : (detailPanel.y + 172));
                const Rect modelLabelRect{ (detailPanel.x + 16), modelRowY, 90, 28 };
                const Rect modelPathRect{ (detailPanel.x + 108), modelRowY, Max(96, detailPanel.w - 260), 28 };
                const Rect browseButton{ (detailPanel.rightX() - 138), modelRowY, 58, 28 };
                const Rect clearButton{ (detailPanel.rightX() - 72), modelRowY, 56, 28 };
                const Rect diagnosticsLabelRect{ (detailPanel.x + 16), (modelRowY + 34), 90, 24 };
                const Rect diagnosticsCheckButton{ (detailPanel.rightX() - 138), (modelRowY + 34), 58, 24 };
                const Rect diagnosticsCopyButton{ (detailPanel.rightX() - 72), (modelRowY + 34), 56, 24 };
                FilePath& currentModelPath = GetUnitModelPath(unitEditorSettings, team, unitType);

                modelPathRect.rounded(6).draw(ColorF{ 0.99, 0.99, 1.0, 0.92 });
                modelPathRect.rounded(6).drawFrame(1, 0, ColorF{ 0.78, 0.80, 0.84 });
                SimpleGUI::GetFont()(U"Model").draw((modelLabelRect.x), (modelLabelRect.y + 4), UiInternal::EditorTextOnLightSecondaryColor());
                SimpleGUI::GetFont()(ToModelReferenceLabel(unitEditorSettings, team, unitType)).draw((modelPathRect.x + 10), (modelPathRect.y + 4), UiInternal::EditorTextOnCardPrimaryColor());
                SimpleGUI::GetFont()(U"Diag").draw((diagnosticsLabelRect.x), (diagnosticsLabelRect.y + 2), UiInternal::EditorTextOnLightSecondaryColor());

                if (modelLabelRect.mouseOver())
                {
                    hoveredDescription = U"このユニット種別の描画に使うモデル参照です。Browse でモデルファイルを選ぶと、そのモデルを優先して描画します。";
                    hoveredRect = modelPathRect;
                }
                else if (modelPathRect.mouseOver())
                {
                    hoveredDescription = currentModelPath.isEmpty()
                        ? U"現在は既定モデルを使用しています。"
                        : currentModelPath;
                    hoveredRect = modelPathRect;
                }
                else if (browseButton.mouseOver())
                {
                    hoveredDescription = U"ファイルダイアログを開いて、このユニット種別の描画モデルを選択します。";
                    hoveredRect = browseButton;
                }
                else if (clearButton.mouseOver())
                {
                    hoveredDescription = U"カスタムモデル指定を解除して、既定モデルへ戻します。";
                    hoveredRect = clearButton;
                }
                else if (diagnosticsLabelRect.mouseOver())
                {
                    hoveredDescription = U"現在のモデル参照に対して、読み込み状態とボーン変形の診断結果を調べます。";
                    hoveredRect = diagnosticsLabelRect;
                }
                else if (diagnosticsCheckButton.mouseOver())
                {
                    hoveredDescription = U"現在のモデル参照に対して診断レポートを更新します。";
                    hoveredRect = diagnosticsCheckButton;
                }
                else if (diagnosticsCopyButton.mouseOver())
                {
                    hoveredDescription = U"現在のモデル参照に対する診断レポートをクリップボードへコピーします。貼り付け用です。";
                    hoveredRect = diagnosticsCopyButton;
                }

                if (DrawTextButton(browseButton, U"Browse"))
                {
                    if (const auto selectedPath = Dialog::OpenFile({ FileFilter::AllFiles() }))
                    {
                        const UnitRenderModel renderModel = GetUnitRenderModel(team, unitType);
                        const UnitModel previewModel{ *selectedPath, BirdDisplayHeight, GetUnitRenderModelProceduralAnimationType(renderModel) };

                        if (previewModel.isLoaded())
                        {
                            currentModelPath = *selectedPath;
                            unitEditorMessage.show(U"モデル参照を更新しました", 3.0);
                        }
                        else
                        {
                            unitEditorMessage.show(U"モデル読み込み失敗", 3.0);
                        }
                    }
                }

                if (DrawTextButton(clearButton, U"Clear"))
                {
                    currentModelPath.clear();
                    unitEditorMessage.show(U"既定モデルに戻しました", 3.0);
                }

                if (DrawTextButton(diagnosticsCheckButton, U"Check"))
                {
                    const UnitRenderModel renderModel = GetUnitRenderModel(team, unitType);
                    const FilePath diagnosticPath = ResolveUnitModelDiagnosticPath(unitEditorSettings, team, unitType);
                    ModelDiagnosticCache& cache = GetModelDiagnosticCache();
                    cache.path = diagnosticPath;
                    cache.report = BuildUnitModelDiagnosticReport(diagnosticPath, renderModel);
                    unitEditorMessage.show(U"モデル診断を更新しました", 3.0);
                }

                if (DrawTextButton(diagnosticsCopyButton, U"Copy"))
                {
                    const UnitRenderModel renderModel = GetUnitRenderModel(team, unitType);
                    const FilePath diagnosticPath = ResolveUnitModelDiagnosticPath(unitEditorSettings, team, unitType);
                    ModelDiagnosticCache& cache = GetModelDiagnosticCache();
                    if ((cache.path != diagnosticPath) || cache.report.isEmpty())
                    {
                        cache.path = diagnosticPath;
                        cache.report = BuildUnitModelDiagnosticReport(diagnosticPath, renderModel);
                    }
                    Clipboard::SetText(cache.report);
                    unitEditorMessage.show(U"モデル診断をコピーしました", 3.0);
                }

                return;
            }

            if (activePage == UnitEditorPage::Footprint)
            {
                SimpleGUI::GetFont()(U"Setup").draw((detailPanel.x + 16), (detailPanel.y + 118), UiInternal::EditorTextOnLightSecondaryColor());
                Rect{ (detailPanel.x + 86), (detailPanel.y + 130), (detailPanel.w - 102), 1 }.draw(ColorF{ 0.78, 0.82, 0.88 });
                DrawFootprintTypeSelector(detailPanel, (detailPanel.y + 138), parameters);
                const Rect footprintLabelRect{ (detailPanel.x + 16), (detailPanel.y + 134), 120, 28 };
                const Rect circleButton{ (detailPanel.x + detailPanel.w - 152), (detailPanel.y + 136), 64, 24 };
                const Rect capsuleButton{ (detailPanel.x + detailPanel.w - 80), (detailPanel.y + 136), 64, 24 };
                if (footprintLabelRect.mouseOver())
                {
                    hoveredDescription = U"接触判定の形です。Circle は丸、Capsule は前後に長い形で、車や横幅のある機体に向きます。";
                    hoveredRect = footprintLabelRect;
                }
                else if (circleButton.mouseOver())
                {
                    hoveredDescription = U"円形フットプリントです。歩兵などの小型ユニット向けで、扱いが単純です。";
                    hoveredRect = circleButton;
                }
                else if (capsuleButton.mouseOver())
                {
                    hoveredDescription = U"カプセル型フットプリントです。前後長を持つため、車両や縦長ユニットの接触判定を自然にできます。";
                    hoveredRect = capsuleButton;
                }
            }
        }

        void DrawUnitEditorFooter(const Rect& detailPanel,
            UnitEditorSettings& unitEditorSettings,
            const UnitTeam team,
            const SapperUnitType unitType,
            const UnitEditorPage activePage,
            UnitParameters& parameters,
            ExplosionSkillParameters& explosionSkillParameters,
            BuildMillSkillParameters& buildMillSkillParameters,
            HealSkillParameters& healSkillParameters,
            ScoutSkillParameters& scoutSkillParameters,
            Array<SpawnedSapper>& spawnedSappers,
            Array<SpawnedSapper>& enemySappers,
            String& hoveredDescription,
            Optional<Rect>& hoveredRect,
            TimedMessage& unitEditorMessage)
        {
            using namespace UiParameterEditorDetail;

            const Rect resetButton{ (detailPanel.x + 16), (detailPanel.y + detailPanel.h - 36), 92, 28 };
            const Rect applyButton{ (detailPanel.x + 116), (detailPanel.y + detailPanel.h - 36), 92, 28 };
            const Rect editorTextColorsButton{ (detailPanel.rightX() - 40), (detailPanel.y + detailPanel.h - 34), 24, 24 };
            const Rect saveButton{ (editorTextColorsButton.x - 100), (detailPanel.y + detailPanel.h - 36), 92, 28 };
            if (resetButton.mouseOver())
            {
                hoveredDescription = (activePage == UnitEditorPage::Skill)
                    ? U"この選択中の陣営 / ユニット種別のスキル設定を既定値へ戻します。爆破と固有スキルの両方が対象です。"
                    : U"この選択ユニット種別の値を既定値へ戻します。保存前でも editor 上の値は即時変更されます。";
                hoveredRect = resetButton;
            }
            else if (applyButton.mouseOver())
            {
                hoveredDescription = (activePage == UnitEditorPage::Skill)
                    ? U"スキル設定は即時反映です。次回の使用時から、爆破と固有スキルのコスト・威力・効果時間・範囲へ反映されます。"
                    : U"今いる同種ユニットへ設定を反映します。交戦距離や停止距離も次フレーム以降に更新されます。";
                hoveredRect = applyButton;
            }
            else if (saveButton.mouseOver())
            {
                hoveredDescription = U"現在の unit editor 設定を TOML に保存します。次回起動時もこの値を読み込みます。";
                hoveredRect = saveButton;
            }
            else if (editorTextColorsButton.mouseOver())
            {
                hoveredDescription = U"editor 系 UI の共通文字色設定を開きます。保存すると各 editor の文字色に反映されます。";
                hoveredRect = editorTextColorsButton;
            }

            if (DrawTextButton(resetButton, U"Reset"))
            {
                if (activePage == UnitEditorPage::Skill)
                {
                    explosionSkillParameters = MakeDefaultExplosionSkillParameters(team, unitType);
                    buildMillSkillParameters = MakeDefaultBuildMillSkillParameters(team, unitType);
                    healSkillParameters = MakeDefaultHealSkillParameters(team, unitType);
                    scoutSkillParameters = MakeDefaultScoutSkillParameters(team, unitType);
                    unitEditorMessage.show(U"スキル設定を既定値に戻しました", 3.0);
                }
                else
                {
                    parameters = MakeDefaultUnitParameters(team, unitType);
                    GetUnitModelPath(unitEditorSettings, team, unitType).clear();
                    unitEditorMessage.show(U"ユニット設定を既定値に戻しました", 3.0);
                }
            }

            if (DrawTextButton(applyButton, U"Apply"))
            {
                if (activePage == UnitEditorPage::Skill)
                {
                    unitEditorMessage.show(U"スキル設定は即時反映です", 3.0);
                }
                else
                {
                    ApplyUnitParametersToSpawned((team == UnitTeam::Player) ? spawnedSappers : enemySappers, team, unitType, parameters);
                    unitEditorMessage.show(U"出撃中ユニットへ反映しました", 3.0);
                }
            }

            if (DrawTextButton(saveButton, U"Save TOML"))
            {
                unitEditorMessage.show(SaveUnitEditorSettings(unitEditorSettings) ? U"Unit 設定を保存" : U"Unit 設定保存失敗", 3.0);
            }

            if (UiInternal::DrawEditorIconButton(editorTextColorsButton, U"色"))
            {
                UiInternal::OpenSharedEditorTextColorEditor();
            }

            if (unitEditorMessage.isVisible())
            {
                SimpleGUI::GetFont()(unitEditorMessage.text).draw((detailPanel.x + 16), (detailPanel.y + detailPanel.h - 66), UiInternal::EditorTextOnLightPrimaryColor());
            }
        }
    }
}
