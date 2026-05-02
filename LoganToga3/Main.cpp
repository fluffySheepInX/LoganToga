# include <Siv3D.hpp> // Siv3D v0.6.16
# include "AppState.h"

void Main()
{
    LT3::InitializeGaussianAddon();
	LT3::AppState app;
	LT3::InitializeApp(app);

	while (System::Update())
	{
		{
			const double scale = GaussianFSAddon::GetSCALE();
			const Vec2 offset = GaussianFSAddon::GetOFFSET();
			const Transformer2D screenScaling{ Mat3x2::Scale(scale).translated(offset), TransformCursor::Yes };

           LT3::UpdateApp(app);
			LT3::DrawApp(app);
		}

        if (!LT3::ProcessGaussianAddonFrameEnd()) break;


	}
}
