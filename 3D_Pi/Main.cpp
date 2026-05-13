# include <Siv3D.hpp>
# include "libs/AddonGaussian.h"
# include "Addons/Pi3D/Pi3D.hpp"
# include "Application/CameraController.hpp"
# include "Application/SceneAssets.hpp"
# include "Application/EditorAddonHost.hpp"
# include "Application/SceneAssetsEditorAddon.hpp"
# include "Application/CameraWorkEditorAddon.hpp"
# include "Application/RenderPipeline.hpp"
# include "Road/RoadEditorAddon.hpp"
# include "Ground/TextureEditorAddon.hpp"
# include "Procedural/ProceduralEditorAddon.hpp"

void Main()
{
#pragma region Addon
    Addon::Register<GaussianFSAddon>(U"GaussianFSAddon");
    GaussianFSAddon::Condition({ 1600,900 });
    GaussianFSAddon::SetLangSet({
        { U"Japan",     U"日本語" },
        { U"English",   U"English" },
        { U"Deutsch",   U"Deutsch" },
        { U"Test",      U"TestLang" },
        });
    GaussianFSAddon::SetLang(U"Japan");
    GaussianFSAddon::SetSceneSet({
        { U"1600*900", U"1600",U"900"},
        { U"1200*600", U"1200",U"600"},
        });
    GaussianFSAddon::SetScene(U"1600*900");
#pragma endregion

    Pi3D::RegisterAddon();

    app::SceneAssets sceneAssets;
    app::CameraController cameraController{ Scene::Size() };
    app::EditorAddonHost editorHost;
    app::RenderPipeline renderPipeline{ sceneAssets, editorHost };

    editorHost.registerAddon(std::make_unique<app::RoadEditorAddon>());
    editorHost.registerAddon(std::make_unique<app::TextureEditorAddon>());
    editorHost.registerAddon(std::make_unique<app::ProceduralEditorAddon>());
    editorHost.registerAddon(std::make_unique<app::SceneAssetsEditorAddon>(sceneAssets));
    auto cameraWorkEditor = std::make_unique<app::CameraWorkEditorAddon>();
    auto* cameraWorkEditorPtr = cameraWorkEditor.get();
    editorHost.registerAddon(std::move(cameraWorkEditor));

    const auto getWheelZoomFocusPosition = [&]() -> Optional<Vec3>
    {
        const Ray centerRay = cameraController.camera().screenToRay(Scene::CenterF());
        const InfinitePlane groundPlane{ Float3{ 0, 0, 0 }, Float3{ 0, 1, 0 } };

        if (const auto distance = centerRay.intersects(groundPlane))
        {
            return centerRay.point_at(*distance);
        }

        return none;
    };

    const auto drawScene = [&]()
    {
        renderPipeline.draw(cameraController.camera(), cameraController.isUIHidden());
    };

    std::chrono::steady_clock::time_point lastEditorUpdateClock = std::chrono::steady_clock::now();
    bool hasLastEditorUpdateClock = false;

    while (System::Update())
    {
        const auto editorUpdateNow = std::chrono::steady_clock::now();
        double editorDeltaTime = (1.0 / 60.0);
        if (hasLastEditorUpdateClock)
        {
            editorDeltaTime = std::chrono::duration<double>(editorUpdateNow - lastEditorUpdateClock).count();
        }
        lastEditorUpdateClock = editorUpdateNow;
        hasLastEditorUpdateClock = true;

        const bool cursorOnMainUI = cameraController.isCursorOnUI() || editorHost.wantsMouseCapture();

        cameraController.update({
            .isDragPanBlocked = [&]() { return cursorOnMainUI; },
            .isWheelZoomBlocked = [&]() { return editorHost.wantsMouseWheelCapture() || Pi3D::WantsMouseWheelCapture() || cursorOnMainUI; },
            .isFreeCameraBlocked = [&]() { return cursorOnMainUI; },
            .getWheelZoomFocus = [&](const DebugCamera3D&) { return getWheelZoomFocusPosition(); }
        });

        editorHost.update({
            .camera = cameraController.camera(),
            .cursorBlockedByUI = cursorOnMainUI,
            .uiHidden = cameraController.isUIHidden(),
            .deltaTime = editorDeltaTime,
            .sceneSize = Scene::Size(),
        });

        if (cameraWorkEditorPtr && cameraWorkEditorPtr->isPlaying())
        {
            const auto preview = cameraWorkEditorPtr->previewState();
            cameraController.camera().setView(preview.eye, preview.focus);
        }

        Graphics3D::SetCameraTransform(cameraController.camera());

#pragma region Addon
        Pi3D::Update(cameraController.camera().getEyePosition(), cameraController.camera().getFocusPosition());
        Pi3D::Begin3D(drawScene);
        Pi3D::End3D(drawScene);
        if (not cameraController.isUIHidden())
        {
            Pi3D::DrawUI();
            const auto cameraEvents = cameraController.drawUI();
            if (cameraEvents.previewStarted)
            {
                editorHost.broadcastCommand(app::EditorCommand::CancelTransientTool);
            }

            editorHost.drawUI({
                .uiHidden = cameraController.isUIHidden(),
                .sceneSize = Scene::Size(),
                .activeEditorId = editorHost.activeEditorId(),
                .focusedEditorId = editorHost.focusedEditorId(),
                .hoveredEditorId = editorHost.hoveredEditorId(),
            });
        }

        if (GaussianFSAddon::TriggerOrDisplayESC()) break;
        if (GaussianFSAddon::TriggerOrDisplayLang()) break;
        if (GaussianFSAddon::TriggerOrDisplaySceneSize()) break;
        if (GaussianFSAddon::IsHide()) Window::Minimize();
        if (GaussianFSAddon::IsGameEnd()) break;
        GaussianFSAddon::DragProcessWindow();
#pragma endregion
    }
}


