# include <Siv3D.hpp>
# include "libs/AddonGaussian.h"
# include "Effects/Effects.hpp"
# include "Lighting/Lighting.hpp"
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

	constexpr TextureFormat HDRTextureFormat = TextureFormat::R16G16B16A16_Float;

	const Mesh groundPlane{ MeshData::OneSidedPlane(2000, { 400, 400 }) };
	const Texture groundTexture{ U"example/texture/ground.jpg", TextureDesc::MippedSRGB };

	// モデルデータをロード
	const Model blacksmithModel{ U"example/obj/blacksmith.obj" };
	const Model millModel{ U"example/obj/mill.obj" };
	const Model treeModel{ U"example/obj/tree.obj" };
	const Model pineModel{ U"example/obj/pine.obj" };
	const Model siv3dkunModel{ U"example/obj/siv3d-kun.obj" };
	const Array<ConstantBufferBinding> dofDepthVSBindings = {
		{ U"VSPerView", 1 },
		{ U"VSPerObject", 2 },
		{ U"VSPerMaterial", 3 },
	};
	const Array<ConstantBufferBinding> dofDepthPSBindings = {
		{ U"PSPerView", 1 },
		{ U"PSPerMaterial", 3 },
	};
	const VertexShader dofDepthVS =
		HLSL{ U"example/shader/hlsl/dof_depth.hlsl", U"VS" }
	  | GLSL{ U"example/shader/glsl/dof_depth.vert", dofDepthVSBindings };
	const PixelShader dofDepthPS =
		HLSL{ U"example/shader/hlsl/dof_depth.hlsl", U"PS" }
	  | GLSL{ U"example/shader/glsl/dof_depth.frag", dofDepthPSBindings };

	// モデルに付随するテクスチャをアセット管理に登録
	Model::RegisterDiffuseTextures(treeModel, TextureDesc::MippedSRGB);
	Model::RegisterDiffuseTextures(pineModel, TextureDesc::MippedSRGB);
	Model::RegisterDiffuseTextures(siv3dkunModel, TextureDesc::MippedSRGB);

  // 3D シーンはフル解像度の HDR (half-float) で描画
	//   - Bloom / Tonemap 前の高輝度を保持する
	//   - sRGB RT ではなく linear float RT を使う
	const MSRenderTexture renderTexture{ Scene::Size(), HDRTextureFormat, HasDepth::Yes };
	// チェイン用 ping-pong 中間 RT
   //   - HDR のまま段間を受け渡すため half-float を使用
	//   - シーンと同サイズ・深度なし
	const RenderTexture chainA{ Scene::Size(), HDRTextureFormat };
	const RenderTexture chainB{ Scene::Size(), HDRTextureFormat };
    const RenderTexture sceneDepthTexture{ Scene::Size(), TextureFormat::R32_Float, HasDepth::Yes };
	DebugCamera3D camera{ renderTexture.size(), 40_deg, Vec3{ 0, 3, -16 } };
	const auto drawScene = [&]()
	{
		groundPlane.draw(groundTexture);
		Sphere{ { 0, 1, 0 }, 1 }.draw(ColorF{ 0.75 }.removeSRGBCurve());
		blacksmithModel.draw(Vec3{ 8, 0, 4 });
		millModel.draw(Vec3{ -8, 0, 4 });

		{
			const ScopedRenderStates3D renderStates{ BlendState::OpaqueAlphaToCoverage, RasterizerState::SolidCullNone };
			treeModel.draw(Vec3{ 16, 0, 4 });
			pineModel.draw(Vec3{ 16, 0, 0 });
		}

		siv3dkunModel.draw(Vec3{ 2, 0, -2 }, Quaternion::RotateY(180_deg));
	};

	// ポストエフェクト群を構築
	const Array<pe::Effect> effects = pe::CreateDefaultEffects();
	const Array<String> effectNames = effects.map([](const pe::Effect& e) { return e.name; });

	// チェイン: 各スロットが effects のインデックス。0 (なし) はパススルー
	Array<size_t> chain = { 0 };
	constexpr size_t MaxChainLength = 6;
	double panelScrollY = 0.0;

	// ライティングプリセット (Phase 1)
	size_t lightingPresetIndex = 1; // 既定: 昼
	size_t prevLightingPresetIndex = lightingPresetIndex;
	const Array<lighting::Preset>& lightingPresets = lighting::GetPresets();
	const Array<String> lightingPresetNames = lightingPresets.map([](const lighting::Preset& p) { return p.name; });

	// ライティング微調整 (Phase 1 後段: スライダ + 方位)
	double sunIntensityScale = 1.0;
	double ambientIntensityScale = 1.0;
	Optional<size_t> sunDirectionOverride;

	// 効果インデックスを名前で取得 (見つからなければ 0 = なし)
	const auto findEffectIndex = [&](StringView name) -> size_t
	{
		for (size_t i = 0; i < effects.size(); ++i)
		{
			if (effects[i].name == name) { return i; }
		}
		return 0;
	};
	const size_t acesEffectIndex = findEffectIndex(U"Tonemap (ACES)");

	while (System::Update())
	{
		camera.update(4.0);
		Graphics3D::SetCameraTransform(camera);

		// [ライティングプリセット適用]
		lighting::Overrides lightingOverrides;
		lightingOverrides.sunIntensityScale = sunIntensityScale;
		lightingOverrides.ambientIntensityScale = ambientIntensityScale;
		lightingOverrides.sunDirectionIndex = sunDirectionOverride;
		const ColorF currentBackground = lighting::Apply(lightingPresets[lightingPresetIndex], lightingOverrides);

		// [プリセット切替検出: マジックアワー → ACES tonemap 自動挿入]
		if (lightingPresetIndex != prevLightingPresetIndex)
		{
			if ((lightingPresets[lightingPresetIndex].name == U"マジックアワー")
				&& (acesEffectIndex != 0)
				&& (not chain.contains(acesEffectIndex)))
			{
				if ((chain.size() == 1) && (chain[0] == 0))
				{
					chain[0] = acesEffectIndex;
				}
				else if (chain.size() < MaxChainLength)
				{
					chain.push_back(acesEffectIndex);
				}
				panelScrollY = 0.0;
			}
			prevLightingPresetIndex = lightingPresetIndex;
		}

		// [3D シーンの描画]
		{
			const ScopedRenderTarget3D target{ renderTexture.clear(currentBackground) };
			drawScene();
		}

     // [DoF 用深度パス]
		{
			const ScopedRenderTarget3D target{ sceneDepthTexture.clear(ColorF{ 100000.0, 0.0, 0.0, 1.0 }) };
			const ScopedCustomShader3D shader{ dofDepthVS, dofDepthPS };
			drawScene();
		}

		// [RenderTexture を 2D シーンに描画 + 効果チェイン適用]
		{
			Graphics3D::Flush();
			renderTexture.resolve();
			pe::SetSceneDepthTexture(sceneDepthTexture);

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
         const Array<size_t> cinematicPresetChain = pe::GetCinematicPresetChain(effects);
			const Array<size_t> dustyPresetChain = pe::GetDustyPresetChain(effects);

			const auto getParamRows = [&](const pe::Effect& e)
			{
                if (not e.drawUI)
				{
                    return 0;
				}

				if ((e.name == U"Bloom") || (e.name == U"CRT") || (e.name == U"Warm Grade"))
				{
					return 4;
				}
				if (e.name == U"DoF")
				{
					return 5;
				}
				if ((e.name == U"アウトライン") || (e.name == U"Vignette"))
				{
					return 3;
				}
				if ((e.name == U"トゥーン") || (e.name == U"Tonemap (ACES)") || (e.name == U"Film Grain"))
				{
					return 2;
				}

               return 1;
			};

			const auto getPresetName = [&]() -> String
			{
				if (chain == dustyPresetChain)
				{
					return U"Dusty (古い洋ゲー風)";
				}
				if (chain == cinematicPresetChain)
				{
					return U"Cinematic";
				}
				if ((chain.size() == 1) && (chain[0] == 0))
				{
					return U"なし";
				}

				return U"Custom";
			};

			const auto getPresetDescription = [&]() -> String
			{
				if (chain == dustyPresetChain)
				{
					return U"黄土色寄り、乾いた空気感、古い洋ゲー風の画作り";
				}
				if (chain == cinematicPresetChain)
				{
					return U"Bloom と ACES を軸にした映画風のチェイン";
				}
				if ((chain.size() == 1) && (chain[0] == 0))
				{
					return U"ポストエフェクトなし";
				}

				return U"手動編集されたチェイン";
			};

			const auto getParamBlockHeight = [&](const pe::Effect& e)
			{
				const int32 rows = getParamRows(e);
				if (rows <= 0)
				{
					return 0.0;
				}

				return (34.0 + (rows - 1) * 40.0);
			};

			const double chainControlHeight = ui::layout::AddButtonHeight + ui::layout::SectionGap;
		  const double presetSectionBodyHeight = 206.0;
			const double presetSectionHeight = presetSectionBodyHeight + ui::layout::SectionGap;
			const double lightingSectionBodyHeight = 76.0 + lightingPresets.size() * ui::layout::RowHeight + 8.0
				+ 38.0  // Sun 強度スライダ
				+ 42.0  // Ambient 強度スライダ
				+ 26.0  // "Sun 方向" ラベル
				+ 98.0  // 3x3 方位グリッド (30 * 3 + 4 * 2)
				+ 8.0
				+ 30.0  // "プリセット方向に戻す" ボタン
				+ 8.0;
			const double lightingSectionHeight = lightingSectionBodyHeight + ui::layout::SectionGap;

			const double chainSectionHeight = chain.size() * (ui::RadioListHeight(effectNames.size(), ui::layout::RowHeight) + ui::layout::SectionGap)
			   + chainControlHeight + presetSectionHeight + lightingSectionHeight;
			double paramsHeight = 0.0;
			for (const size_t effectIndex : chain)
			{
				const pe::Effect& e = effects[effectIndex];
              const double paramBlockHeight = getParamBlockHeight(e);
				if (0.0 < paramBlockHeight)
				{
                  paramsHeight += (28.0 + paramBlockHeight + ui::layout::SectionGap);
				}
			}
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

				// [ライティングセクション (Phase 1)]
				{
					const RectF lightingSectionRect{ uiPos, contentRect.w, lightingSectionBodyHeight };
					ui::Section(lightingSectionRect);
					const lighting::Preset& lp = lightingPresets[lightingPresetIndex];
					uiFont(U"ライティング (時間帯)").draw(uiPos.movedBy(8, 0), ui::GetTheme().text);
					uiFont(U"現在: {}"_fmt(lp.name)).draw(uiPos.movedBy(8, 26), Palette::Dimgray);
					uiFont(lp.description).draw(uiPos.movedBy(8, 50), Palette::Gray);

					const RectF radioRect{
						lightingSectionRect.x + 8,
						lightingSectionRect.y + 76,
						lightingSectionRect.w - 16,
						lightingPresets.size() * ui::layout::RowHeight
					};
					ui::RadioList(uiFont, lightingPresetIndex, lightingPresetNames, radioRect, ui::layout::RowHeight);

					double cy = 76.0 + lightingPresets.size() * ui::layout::RowHeight + 8.0;

					// Sun 強度スライダ
					const double sliderLabelW = 110.0;
					const double sliderTotalW = lightingSectionRect.w - 16;
					ui::SliderH(U"Sun 強度", sunIntensityScale, 0.0, 5.0,
						uiPos.movedBy(8, cy), sliderLabelW, sliderTotalW - sliderLabelW);
					cy += 38.0;

					// Ambient 強度スライダ
					ui::SliderH(U"Ambient 強度", ambientIntensityScale, 0.0, 3.0,
						uiPos.movedBy(8, cy), sliderLabelW, sliderTotalW - sliderLabelW);
					cy += 42.0;

					// Sun 方向ラベル
					uiFont(U"Sun 方向 (水平方位)").draw(uiPos.movedBy(8, cy), ui::GetTheme().text);
					cy += 26.0;

					// 3x3 方位グリッド (中央セルは空)
					//   row 0: NW(7), N(0), NE(1)
					//   row 1: W(6),  -,    E(2)
					//   row 2: SW(5), S(4), SE(3)
					constexpr int gridLayout[3][3] = {
						{ 7, 0, 1 },
						{ 6, -1, 2 },
						{ 5, 4, 3 },
					};
					constexpr double cellW = 60.0, cellH = 30.0, gap = 4.0;
					const Vec2 gridOrigin = uiPos.movedBy(8, cy);
					for (int r = 0; r < 3; ++r)
					{
						for (int c = 0; c < 3; ++c)
						{
							const int idx = gridLayout[r][c];
							if (idx < 0) { continue; }
							const RectF btn{
								gridOrigin.x + c * (cellW + gap),
								gridOrigin.y + r * (cellH + gap),
								cellW, cellH
							};
							const bool selected = sunDirectionOverride && (*sunDirectionOverride == static_cast<size_t>(idx));
							if (ui::Button(uiFont, lighting::DirectionLabels[idx], btn))
							{
								sunDirectionOverride = static_cast<size_t>(idx);
							}
							if (selected)
							{
								btn.rounded(6).drawFrame(2.5, ui::GetTheme().accent);
							}
						}
					}
					cy += 3 * cellH + 2 * gap + 8.0;

					// プリセット方向に戻す
					const RectF resetBtn{ uiPos.x + 8, uiPos.y + cy, lightingSectionRect.w - 16, 30.0 };
					if (ui::Button(uiFont, U"プリセット方向に戻す", resetBtn))
					{
						sunDirectionOverride.reset();
					}

					uiPos.y += lightingSectionRect.h + ui::layout::SectionGap;
				}

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
              uiPos.y += ui::layout::AddButtonHeight + ui::layout::SectionGap;

             const RectF presetSectionRect{ uiPos, contentRect.w, presetSectionBodyHeight };
				ui::Section(presetSectionRect);
				uiFont(U"プリセット").draw(uiPos.movedBy(8, 0), ui::GetTheme().text);
				uiFont(U"現在: {}"_fmt(getPresetName())).draw(uiPos.movedBy(8, 28), Palette::Dimgray);
				uiFont(getPresetDescription()).draw(uiPos.movedBy(8, 54), Palette::Gray);

				const RectF cinematicRect{ uiPos.x + 8, uiPos.y + 88, contentRect.w - 16, ui::layout::AddButtonHeight };
				if (ui::Button(uiFont, U"Cinematic", cinematicRect))
				{
					chain = cinematicPresetChain;
					panelScrollY = 0.0;
				}

				const RectF dustyRect{ uiPos.x + 8, uiPos.y + 126, contentRect.w - 16, ui::layout::AddButtonHeight };
				if (ui::Button(uiFont, U"Dusty (古い洋ゲー風)", dustyRect))
				{
					chain = dustyPresetChain;
					panelScrollY = 0.0;
				}

				const RectF noneRect{ uiPos.x + 8, uiPos.y + 164, contentRect.w - 16, ui::layout::AddButtonHeight };
				if (ui::Button(uiFont, U"なしに戻す", noneRect))
				{
					chain = { 0 };
					panelScrollY = 0.0;
				}

				uiPos.y += presetSectionRect.h + ui::layout::SectionGap;

				for (size_t i = 0; i < chain.size(); ++i)
				{
					const pe::Effect& e = effects[chain[i]];
					if (e.drawUI)
					{
                     const double paramBlockHeight = getParamBlockHeight(e);
						const RectF paramSectionRect{ uiPos, contentRect.w, 28 + paramBlockHeight };
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


