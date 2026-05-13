# pragma once
# include <Siv3D.hpp>
# include "SceneAssets.hpp"
# include "EditorAddonHost.hpp"
# include "../Addons/Pi3D/Pi3D.hpp"

namespace app
{
    class RenderPipeline
    {
    public:
        RenderPipeline(SceneAssets& sceneAssets, EditorAddonHost& editorHost)
            : m_sceneAssets{ sceneAssets }
            , m_editorHost{ editorHost } {}

        void draw(const BasicCamera3D& camera, const bool uiHidden)
        {
            Pi3D::EnvironmentRef().drawGround(m_sceneAssets.groundPlane(), m_sceneAssets.groundTexture());

            m_editorHost.draw3D({
                .camera = camera,
                .uiHidden = uiHidden,
            });

            const auto& kicker = Pi3D::LightingRef().getKickerRuntime();
            m_sceneAssets.drawStaticScene(Pi3D::LightingRef().getEffectiveSunDirection(), KickerLightDrawParams{
                .enabled = kicker.enabled,
                .direction = kicker.direction,
                .colorLinear = kicker.colorLinear,
                .intensity = kicker.intensity,
            });
            Pi3D::EnvironmentRef().draw3D();
        }

    private:
        SceneAssets& m_sceneAssets;
        EditorAddonHost& m_editorHost;
    };
}
