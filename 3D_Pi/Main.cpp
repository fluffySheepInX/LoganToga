# include <Siv3D.hpp>
# include "Effects/Effects.hpp"

void Main()
{
	Window::Resize(1280, 720);
	const ColorF backgroundColor = ColorF{ 0.4, 0.6, 0.8 }.removeSRGBCurve();

	const Mesh groundPlane{ MeshData::OneSidedPlane(2000, { 400, 400 }) };
	const Texture groundTexture{ U"example/texture/ground.jpg", TextureDesc::MippedSRGB };

	// モデルデータをロード
	const Model blacksmithModel{ U"example/obj/blacksmith.obj" };
	const Model millModel{ U"example/obj/mill.obj" };
	const Model treeModel{ U"example/obj/tree.obj" };
	const Model pineModel{ U"example/obj/pine.obj" };
	const Model siv3dkunModel{ U"example/obj/siv3d-kun.obj" };

	// モデルに付随するテクスチャをアセット管理に登録
	Model::RegisterDiffuseTextures(treeModel, TextureDesc::MippedSRGB);
	Model::RegisterDiffuseTextures(pineModel, TextureDesc::MippedSRGB);
	Model::RegisterDiffuseTextures(siv3dkunModel, TextureDesc::MippedSRGB);

	// 3D シーンはフル解像度で描画
	const MSRenderTexture renderTexture{ Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes };
	DebugCamera3D camera{ renderTexture.size(), 40_deg, Vec3{ 0, 3, -16 } };

	// ポストエフェクト群を構築
	const Array<pe::Effect> effects = pe::CreateDefaultEffects();
	const Array<String> effectNames = effects.map([](const pe::Effect& e) { return e.name; });
	size_t effectIndex = 0;

	while (System::Update())
	{
		camera.update(4.0);
		Graphics3D::SetCameraTransform(camera);

		// [3D シーンの描画]
		{
			const ScopedRenderTarget3D target{ renderTexture.clear(backgroundColor) };

			// [モデルの描画]
			{
				// 地面の描画
				groundPlane.draw(groundTexture);

				// 球の描画
				Sphere{ { 0, 1, 0 }, 1 }.draw(ColorF{ 0.75 }.removeSRGBCurve());

				// 鍛冶屋の描画
				blacksmithModel.draw(Vec3{ 8, 0, 4 });

				// 風車の描画
				millModel.draw(Vec3{ -8, 0, 4 });

				// 木の描画
				{
					const ScopedRenderStates3D renderStates{ BlendState::OpaqueAlphaToCoverage, RasterizerState::SolidCullNone };
					treeModel.draw(Vec3{ 16, 0, 4 });
					pineModel.draw(Vec3{ 16, 0, 0 });
				}

				// Siv3D くんの描画
				siv3dkunModel.draw(Vec3{ 2, 0, -2 }, Quaternion::RotateY(180_deg));
			}
		}

		// [RenderTexture を 2D シーンに描画 + 選択中のポストエフェクト適用]
		{
			Graphics3D::Flush();
			renderTexture.resolve();
			effects[effectIndex].apply(renderTexture);
		}

		// [UI: シェーダ選択]
		SimpleGUI::RadioButtons(effectIndex, effectNames, Vec2{ 20, 20 }, 220);

		// [UI: 選択中シェーダのパラメータ]
		if (effects[effectIndex].drawUI)
		{
			const Vec2 paramTopLeft{ 20, 40.0 + effectNames.size() * 40.0 + 12.0 };
			effects[effectIndex].drawUI(paramTopLeft);
		}
	}
}


