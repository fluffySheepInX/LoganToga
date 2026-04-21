# include <Siv3D.hpp>
# include "libs/AddonGaussian.h"
# include "Effects/Effects.hpp"
# include "UI/Layout.hpp"
# include "UI/RectUI.hpp"

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

	const Font& uiFont = ui::DefaultFont();

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
	// チェイン用 ping-pong 中間 RT
	//   - 線形 (非 sRGB) にして sRGB の往復符号化を排除
	//   - シーンと同サイズ・深度なし
	const RenderTexture chainA{ Scene::Size(), TextureFormat::R8G8B8A8_Unorm };
	const RenderTexture chainB{ Scene::Size(), TextureFormat::R8G8B8A8_Unorm };
	DebugCamera3D camera{ renderTexture.size(), 40_deg, Vec3{ 0, 3, -16 } };

	// ポストエフェクト群を構築
	const Array<pe::Effect> effects = pe::CreateDefaultEffects();
	const Array<String> effectNames = effects.map([](const pe::Effect& e) { return e.name; });

	// チェイン: 各スロットが effects のインデックス。0 (なし) はパススルー
	Array<size_t> chain = { 0 };
	constexpr size_t MaxChainLength = 6;
	double panelScrollY = 0.0;

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

		// [RenderTexture を 2D シーンに描画 + 効果チェイン適用]
		{
			Graphics3D::Flush();
			renderTexture.resolve();

			// ping-pong: 入力は最初 renderTexture、以降は chainA/chainB を交互に
			const Texture* input = &renderTexture;
			const RenderTexture* targets[2] = { &chainA, &chainB };
			size_t targetIdx = 0;

			for (size_t i = 0; i < chain.size(); ++i)
			{
				const pe::Effect& e = effects[chain[i]];
				const bool isLast = (i + 1 == chain.size());

				if (isLast)
				{
					// 最終段は画面 (シーン) へ
					e.apply(*input);
				}
				else
				{
					const RenderTexture* dst = targets[targetIdx];
					// 中間 RT への描画は: ブレンド OFF + Opaque で全画素上書き
					// (clear と Scoped* の構築順序 / 既定アルファブレンドの干渉を排除)
					{
						const ScopedRenderTarget2D rt{ *dst };
						const ScopedRenderStates2D blend{ BlendState::Opaque };
						dst->clear(ColorF{ 0, 0, 0, 1 });
						e.apply(*input);
					}
					Graphics2D::Flush();
					input = dst;
					targetIdx ^= 1;
				}
			}
		}

		// [UI: 効果チェイン編集]
		{
			const double chainSectionHeight = chain.size() * (ui::RadioListHeight(effectNames.size(), ui::layout::RowHeight) + ui::layout::SectionGap)
				+ 44;
			const size_t effectUiCount = chain.count_if([&](const size_t effectIndex)
				{
					return static_cast<bool>(effects[effectIndex].drawUI);
				});
			const double paramsHeight = effectUiCount * (28 + ui::layout::ParamBlockHeight + ui::layout::SectionGap);
			const double contentHeight = chainSectionHeight + paramsHeight;
			const double desiredPanelHeight = 46 + contentHeight;
			const double maxPanelHeight = (Scene::Height() - ui::layout::PanelMargin * 2);
			const double panelHeight = Clamp(desiredPanelHeight, ui::layout::MinPanelHeight, maxPanelHeight);
			const RectF panelRect{
				ui::layout::PanelMargin,
				ui::layout::PanelMargin,
				ui::layout::PanelWidth,
			  panelHeight
			};
			const RectF contentRect{
				panelRect.x + ui::layout::PanelPadding,
				panelRect.y + ui::layout::HeaderHeight,
				panelRect.w - ui::layout::PanelPadding * 2 - ui::layout::ScrollbarWidth - 8,
				panelRect.h - ui::layout::HeaderHeight - ui::layout::PanelPadding
			};
			const double maxScrollY = Max(0.0, contentHeight - contentRect.h);
			if (contentRect.mouseOver())
			{
				panelScrollY = Clamp(panelScrollY - Mouse::Wheel() * ui::layout::ScrollStep, 0.0, maxScrollY);
			}
			else
			{
				panelScrollY = Clamp(panelScrollY, 0.0, maxScrollY);
			}

			ui::Panel(panelRect);
			uiFont(U"効果チェイン (上から順に適用)").draw(
				 panelRect.pos.movedBy(ui::layout::PanelPadding, 12), ui::GetTheme().text);

			{
				const ScopedRenderStates2D scissor{ RasterizerState::SolidCullNoneScissor };
				const Rect previousScissor = Graphics2D::GetScissorRect();
				Graphics2D::SetScissorRect(contentRect.asRect());

				Vec2 uiPos = contentRect.pos.movedBy(0, -panelScrollY);
				for (size_t i = 0; i < chain.size(); ++i)
				{
					const double sectionHeight = ui::RadioListHeight(effectNames.size(), ui::layout::RowHeight);
					const RectF sectionRect{ uiPos, contentRect.w, sectionHeight };
					ui::Section(sectionRect);

					const RectF radioRect{ sectionRect.x + 8, sectionRect.y + 8, ui::layout::RadioWidth, sectionRect.h - 16 };
					ui::RadioList(uiFont, chain[i], effectNames, radioRect, ui::layout::RowHeight);

					const RectF removeRect{ sectionRect.rightX() - 80, sectionRect.y + 8, ui::layout::ButtonSize, ui::layout::ButtonSize };
					if ((chain.size() > 1) && ui::Button(uiFont, U"×", removeRect))
					{
						chain.erase(chain.begin() + i);
						break;
					}

					if (i > 0)
					{
						const RectF upRect{ sectionRect.rightX() - 40, sectionRect.y + 8, ui::layout::ButtonSize, ui::layout::ButtonSize };
						if (ui::Button(uiFont, U"↑", upRect))
						{
							std::swap(chain[i], chain[i - 1]);
						}
					}

					uiPos.y += sectionHeight + ui::layout::SectionGap;
				}

				const RectF addRect{ uiPos, ui::layout::AddButtonWidth, ui::layout::AddButtonHeight };
				if ((chain.size() < MaxChainLength) && ui::Button(uiFont, U"+ 段を追加", addRect))
				{
					chain.push_back(0);
				}
				const RectF presetRect{ uiPos.x + ui::layout::AddButtonWidth + 8, uiPos.y, ui::layout::AddButtonWidth + 30, ui::layout::AddButtonHeight };
				if (ui::Button(uiFont, U"Cinematic プリセット", presetRect))
				{
					chain = pe::GetCinematicPresetChain(effects);
					panelScrollY = 0.0;
				}
				uiPos.y += 48;

				for (size_t i = 0; i < chain.size(); ++i)
				{
					const pe::Effect& e = effects[chain[i]];
					if (e.drawUI)
					{
						const RectF paramSectionRect{ uiPos, contentRect.w, 28 + ui::layout::ParamBlockHeight };
						ui::Section(paramSectionRect);
						uiFont(U"[{}] {}"_fmt(i, e.name)).draw(uiPos.movedBy(8, 0), ui::GetTheme().text);
						e.drawUI(uiPos.movedBy(8, 28));
						uiPos.y += paramSectionRect.h + ui::layout::SectionGap;
					}
				}

				Graphics2D::SetScissorRect(previousScissor);
			}

			if (0.0 < maxScrollY)
			{
				const RectF scrollTrack{
					panelRect.rightX() - ui::layout::PanelPadding - ui::layout::ScrollbarWidth,
					contentRect.y,
					ui::layout::ScrollbarWidth,
					contentRect.h
				};
				const double thumbHeight = Max(ui::layout::ScrollbarMinThumbHeight, scrollTrack.h * (contentRect.h / contentHeight));
				const double thumbTravel = Max(0.0, scrollTrack.h - thumbHeight);
				const double thumbY = scrollTrack.y + (thumbTravel * (panelScrollY / maxScrollY));

				scrollTrack.rounded(4).draw(ColorF{ 0.82, 0.86, 0.91, 0.8 });
				RectF{ scrollTrack.x, thumbY, scrollTrack.w, thumbHeight }.rounded(4).draw(ui::GetTheme().accent);
			}
		}

#pragma region Addon
		if (GaussianFSAddon::TriggerOrDisplayESC()) break;
		if (GaussianFSAddon::TriggerOrDisplayLang()) break;
		if (GaussianFSAddon::TriggerOrDisplaySceneSize()) break;
		if (GaussianFSAddon::IsHide()) Window::Minimize();
		if (GaussianFSAddon::IsGameEnd()) break;
		GaussianFSAddon::DragProcessWindow();
#pragma endregion
	}
}


