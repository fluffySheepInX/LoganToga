# pragma once
# include <Siv3D.hpp>
# include "PiGroundMode.hpp"
# include "PiRain.hpp"
# include "PiUnderwaterParticles.hpp"
# include "../PiSettings.hpp"
# include "../Shader/PiShaderLoader.hpp"
# include "../../../UI/RectUI.hpp"
# include "../../../UI/Layout.hpp"

namespace Pi3D
{
    class PiEnvironment
    {
    public:
        static constexpr double HeaderHeight = 86.0;

        // 環境パネル本体の高さを返す
        [[nodiscard]] double getUIBodyHeight() const;

        // 環境アニメーションを更新
        void update(const double deltaTime);

        // 地面を描画
        void drawGround(const Mesh& groundPlane, const Texture& groundTexture) const;

        // 3D 環境要素を描画
        void draw3D() const;

        // シーン深度が必要かを返す
        [[nodiscard]] bool needsSceneDepth() const;

        // 大気合成パスが必要かを返す
        [[nodiscard]] bool hasAtmospherePass() const;

        // 水中後処理が必要かを返す
        [[nodiscard]] bool hasUnderwaterPostProcess() const;

        // 状態に応じて通常フォグまたは水中フォグを適用
        void applyAtmosphere(const Texture& source, const Texture& depthTexture) const;

        // 通常フォグを適用
        void applyFog(const Texture& source, const Texture& depthTexture) const;

        // 水中フォグを適用
        void applyUnderwaterFog(const Texture& source, const Texture& depthTexture) const;

        // 水中の屈折を適用
        void applyUnderwaterDistortion(const Texture& source, const Texture& depthTexture) const;

        // 水中浮遊物を 2D オーバーレイ描画
        void drawUnderwaterParticles() const;

        // 環境 UI を描画
        void drawUI(const Font& font, Vec2& uiPos, const double contentWidth);

        // 現在の環境設定を取得
        [[nodiscard]] Pi3D::EnvironmentSettings getSettings() const;

        // 保存済み環境設定を適用
        void applySettings(const Pi3D::EnvironmentSettings& settings);

    private:
        // 地面モードを保存用文字列へ変換
        [[nodiscard]] static String groundModeToString(const PiGroundMode mode);

        // 保存文字列を地面モードへ変換
        [[nodiscard]] static PiGroundMode stringToGroundMode(const String& mode);

        // 現在の地面モード表示名を返す
        [[nodiscard]] StringView getGroundModeLabel() const;

        // 地面モードを順送りで切り替え
        void cycleGroundMode();

        // 通常フォグプリセットを適用
        void applyFogPreset(const ColorF& color, const double startDistance, const double endDistance, const double density);

        // 水中プリセットを適用
        void applyUnderwaterPreset(const ColorF& color, const double startDistance, const double endDistance, const double density,
            const double distortionStrength, const double distortionSpeed, const double distortionScale, const double particleAmount);

        PiGroundMode m_groundMode = PiGroundMode::Texture;
        PiRain m_rain;
        PiUnderwaterParticles m_underwaterParticles;
        bool m_fogEnabled = false;
        bool m_underwaterEnabled = false;
        bool m_environmentPanelOpened = true;
        Texture m_environmentHelpIcon{ U"texture/hatena.png" };
        Texture m_environmentOpenCloseIcon{ U"texture/kaihei.png" };
        Texture m_groundModeIcon{ U"texture/zimenTexture.png" };
        Texture m_rainToggleIcon{ U"texture/ame.png" };
        Texture m_rainAmountIcon{ U"texture/ryou.png" };
        double m_fogStartDistance = 22.0;
        double m_fogEndDistance = 90.0;
        double m_fogDensity = 0.65;
        ColorF m_fogColor{ 0.58, 0.66, 0.76, 1.0 };
        double m_underwaterFogStartDistance = 4.0;
        double m_underwaterFogEndDistance = 70.0;
        double m_underwaterFogDensity = 0.82;
        ColorF m_underwaterFogColor{ 0.08, 0.32, 0.43, 1.0 };
        double m_underwaterDistortionStrength = 0.006;
        double m_underwaterDistortionSpeed = 0.65;
        double m_underwaterDistortionScale = 18.0;
        double m_underwaterParticleAmount = 0.75;
        const Array<ConstantBufferBinding> m_fogPSBindings = {
            { U"PSConstants2D", 0 },
            { U"PSEffectParams", 1 },
            { U"PSFogColor", 2 },
        };
        const PixelShader m_fogPS{ HLSL{ PiShaderLoader::HLSL(U"scene_fog"), U"PS" } | GLSL{ PiShaderLoader::GLSLFragment(U"scene_fog"), m_fogPSBindings } };
        const Array<ConstantBufferBinding> m_underwaterDistortionPSBindings = {
            { U"PSConstants2D", 0 },
            { U"PSEffectParams", 1 },
        };
        const PixelShader m_underwaterFogPS{ HLSL{ PiShaderLoader::HLSL(U"scene_underwater_fog"), U"PS" } | GLSL{ PiShaderLoader::GLSLFragment(U"scene_underwater_fog"), m_fogPSBindings } };
        const PixelShader m_underwaterDistortionPS{ HLSL{ PiShaderLoader::HLSL(U"scene_underwater_distort"), U"PS" } | GLSL{ PiShaderLoader::GLSLFragment(U"scene_underwater_distort"), m_underwaterDistortionPSBindings } };
        mutable ConstantBuffer<Float4> m_fogParams{ Float4{ 22.0f, 90.0f, 0.65f, 0.0f } };
        mutable ConstantBuffer<Float4> m_fogColorBuffer{ Float4{ 0.58f, 0.66f, 0.76f, 1.0f } };
        mutable ConstantBuffer<Float4> m_underwaterFogParams{ Float4{ 4.0f, 70.0f, 0.82f, 0.0f } };
        mutable ConstantBuffer<Float4> m_underwaterFogColorBuffer{ Float4{ 0.08f, 0.32f, 0.43f, 1.0f } };
        mutable ConstantBuffer<Float4> m_underwaterDistortionParams{ Float4{ 0.006f, 0.0f, 0.65f, 18.0f } };
    };
}

# include "PiEnvironment.Runtime.ipp"
# include "PiEnvironment.UI.ipp"
