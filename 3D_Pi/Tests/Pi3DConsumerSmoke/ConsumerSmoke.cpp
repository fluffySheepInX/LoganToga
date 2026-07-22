# include <Pi3D/Pi3D.hpp>

namespace Pi3DConsumerSmoke
{
	// Consumer から Pi3D の公開 API 一式をコンパイルできることを検証する。
	void CompilePublicApi()
	{
		Pi3D::Config config;
		config.editorUIEnabled = true;
		config.persistenceEnabled = false;
		static_cast<void>(Pi3D::RegisterAddon(config));

		const auto drawScene = []() {};
		Pi3D::Update();
		Pi3D::Begin3D(drawScene);
		Pi3D::End3D(drawScene);
		Pi3D::DrawUI();
		static_cast<void>(Pi3D::WantsMouseWheelCapture());
		static_cast<void>(Pi3D::EnvironmentRef());
		static_cast<void>(Pi3D::Effects());
		static_cast<void>(Pi3D::LightingRef());
	}
}
