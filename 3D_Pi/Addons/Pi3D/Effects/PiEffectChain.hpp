# pragma once
# include <Siv3D.hpp>
# include "../PiSettings.hpp"
# include "../../../Effects/Effects.hpp"
# include "../../../UI/Layout.hpp"
# include "../../../UI/RectUI.hpp"

namespace Pi3D
{
	class PiEffectChain
	{
	public:
		static constexpr size_t MaxChainLength = 6;

		PiEffectChain()
			: m_effects{ pe::CreateDefaultEffects() }
			, m_effectNames{ m_effects.map([](const pe::Effect& e) { return e.name; }) }
			, m_presets{ loadPresetsFromToml() }
		{
			if (m_presets.isEmpty())
			{
				m_presets = defaultPresets();
			}
		}

		bool onLightingPresetChanged(StringView presetName, const size_t presetIndex)
		{
			if (not m_hasPrevLightingPreset)
			{
				m_prevLightingPresetIndex = presetIndex;
				m_hasPrevLightingPreset = true;
				return false;
			}

			if (presetIndex == m_prevLightingPresetIndex)
			{
				return false;
			}

			bool inserted = false;
			const size_t acesEffectIndex = findEffectIndex(U"Tonemap (ACES)");
			if ((presetName == U"マジックアワー")
				&& (acesEffectIndex != 0)
				&& (not m_chain.contains(acesEffectIndex)))
			{
				if ((m_chain.size() == 1) && (m_chain[0] == 0))
				{
					m_chain[0] = acesEffectIndex;
					inserted = true;
				}
				else if (m_chain.size() < MaxChainLength)
				{
					m_chain.push_back(acesEffectIndex);
					inserted = true;
				}
			}

			m_prevLightingPresetIndex = presetIndex;
			return inserted;
		}

      void apply(const Texture& renderTexture, const RenderTexture& chainA, const RenderTexture& chainB, const Texture& sceneDepthTexture) const
		{
			Graphics3D::Flush();
			pe::SetSceneDepthTexture(sceneDepthTexture);

			Array<size_t> activeChain;
			for (size_t i = 0; i < m_chain.size(); ++i)
			{
				if ((i < m_chainEnabled.size()) && m_chainEnabled[i])
				{
					activeChain << m_chain[i];
				}
			}
			if (activeChain.isEmpty())
			{
				renderTexture.draw();
				return;
			}

			const Texture* input = &renderTexture;
			const RenderTexture* targets[2] = { &chainA, &chainB };
			size_t targetIdx = 0;

         for (size_t i = 0; i < activeChain.size(); ++i)
			{
                const pe::Effect& e = m_effects[activeChain[i]];
				const bool isLast = (i + 1 == activeChain.size());

				if (isLast)
				{
					e.apply(*input);
				}
				else
				{
					const RenderTexture* dst = targets[targetIdx];
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

		[[nodiscard]] double getControlSectionsHeight() const
		{
			const double chainControlHeight = ui::layout::AddButtonHeight + ui::layout::SectionGap;
         const double presetSectionBodyHeight = isPresetSectionCollapsed() ? CollapsedSectionHeight : getPresetSectionBodyHeight();
			const double presetSectionHeight = presetSectionBodyHeight + ui::layout::SectionGap;
         double chainListHeight = 0.0;
			for (size_t i = 0; i < m_chain.size(); ++i)
			{
				chainListHeight += getChainSectionHeight(i) + ui::layout::SectionGap;
			}
			return chainListHeight + chainControlHeight + presetSectionHeight;
		}

		[[nodiscard]] double getChainListHeight() const
		{
         double height = ui::layout::AddButtonHeight + ui::layout::SectionGap;
			for (size_t i = 0; i < m_chain.size(); ++i)
			{
				height += getChainSectionHeight(i) + ui::layout::SectionGap;
			}
			return height;
		}

		[[nodiscard]] double getPresetHeight() const
		{
			return (isPresetSectionCollapsed() ? CollapsedSectionHeight : getPresetSectionBodyHeight()) + ui::layout::SectionGap;
		}

		[[nodiscard]] bool isPresetSectionCollapsed() const
		{
			return m_presetSectionCollapsed;
		}

        [[nodiscard]] Pi3D::EffectChainSettings getSettings() const
		{
            Pi3D::EffectChainSettings settings;
			settings.chainEffectNames.clear();
            settings.chainEnabled.clear();
			for (const size_t effectIndex : m_chain)
			{
				settings.chainEffectNames << m_effects[effectIndex].name;
			}
            settings.chainEnabled = m_chainEnabled;
			if (settings.chainEffectNames.isEmpty())
			{
				settings.chainEffectNames = { U"なし" };
               settings.chainEnabled = { true };
			}
			return settings;
		}

        void applySettings(const Pi3D::EffectChainSettings& settings)
		{
			Array<size_t> resolvedChain;
			for (const auto& effectName : settings.chainEffectNames)
			{
				const size_t effectIndex = findEffectIndex(effectName);
				if ((effectIndex != 0) || (effectName == m_effects[0].name))
				{
					resolvedChain << effectIndex;
				}
			}
			if (resolvedChain.isEmpty())
			{
				resolvedChain = { 0 };
			}
			m_chain = resolvedChain;
           m_chainEnabled = settings.chainEnabled;
			if (m_chainEnabled.size() < m_chain.size())
			{
				m_chainEnabled.resize(m_chain.size(), true);
			}
			else if (m_chain.size() < m_chainEnabled.size())
			{
				m_chainEnabled.resize(m_chain.size());
			}
		}

		[[nodiscard]] double getParamsHeight() const
		{
			double paramsHeight = 0.0;
			for (const size_t effectIndex : m_chain)
			{
				const pe::Effect& e = m_effects[effectIndex];
				const double paramBlockHeight = getParamBlockHeight(e);
				if (0.0 < paramBlockHeight)
				{
					paramsHeight += (28.0 + paramBlockHeight + ui::layout::SectionGap);
				}
			}
			return paramsHeight;
		}

		void drawUI(const Font& uiFont, Vec2& uiPos, const double contentWidth, double& panelScrollY)
		{
         drawChainListUI(uiFont, uiPos, contentWidth);
			drawPresetUI(uiFont, uiPos, contentWidth, panelScrollY);
			drawParamsUI(uiFont, uiPos, contentWidth);
		}

		void drawChainListUI(const Font& uiFont, Vec2& uiPos, const double contentWidth)
		{
			for (size_t i = 0; i < m_chain.size(); ++i)
			{
                const double sectionHeight = getChainSectionHeight(i);
				const RectF sectionRect{ uiPos, contentWidth, sectionHeight };
				ui::Section(sectionRect);

              uiFont(U"[{}]"_fmt(i + 1)).draw(sectionRect.pos.movedBy(10, 8), ui::GetTheme().text);

				const RectF selectRect{ sectionRect.x + 8, sectionRect.y + 36, contentWidth - 144, ui::layout::ButtonSize };
				if (ui::Button(uiFont, U"{} ▾"_fmt(m_effectNames[m_chain[i]]), selectRect))
				{
					if (m_openEffectSelectIndex && (*m_openEffectSelectIndex == i))
					{
						m_openEffectSelectIndex.reset();
					}
					else
					{
						m_openEffectSelectIndex = i;
					}
				}
				if (selectRect.mouseOver())
				{
					ui::Tooltip(uiFont, getEffectTooltip(m_effectNames[m_chain[i]]), Cursor::PosF().movedBy(18, 20));
				}

                const RectF enableRect{ sectionRect.rightX() - 88, sectionRect.y + 8, 80, ui::layout::ButtonSize };
				if (ui::Button(uiFont, m_chainEnabled[i] ? U"ON" : U"OFF", enableRect))
				{
					m_chainEnabled[i] = (not m_chainEnabled[i]);
				}
				if (m_chainEnabled[i])
				{
					enableRect.rounded(6).drawFrame(2.5, ui::GetTheme().accent);
				}

                const RectF removeRect{ sectionRect.rightX() - 80, sectionRect.y + 40, ui::layout::ButtonSize, ui::layout::ButtonSize };
				if ((m_chain.size() > 1) && ui::Button(uiFont, U"×", removeRect))
				{
					m_chain.erase(m_chain.begin() + i);
                  m_chainEnabled.erase(m_chainEnabled.begin() + i);
                  m_openEffectSelectIndex.reset();
					break;
				}

				if (i > 0)
				{
                   const RectF upRect{ sectionRect.rightX() - 160, sectionRect.y + 40, ui::layout::ButtonSize, ui::layout::ButtonSize };
					if (ui::Button(uiFont, U"↑", upRect))
					{
						std::swap(m_chain[i], m_chain[i - 1]);
                       std::swap(m_chainEnabled[i], m_chainEnabled[i - 1]);
					}
				}

				if ((i + 1) < m_chain.size())
				{
                  const RectF downRect{ sectionRect.rightX() - 120, sectionRect.y + 40, ui::layout::ButtonSize, ui::layout::ButtonSize };
					if (ui::Button(uiFont, U"↓", downRect))
					{
						std::swap(m_chain[i], m_chain[i + 1]);
						std::swap(m_chainEnabled[i], m_chainEnabled[i + 1]);
					}
				}

				if (m_chain.size() < MaxChainLength)
				{
                    const RectF duplicateRect{ sectionRect.rightX() - 40, sectionRect.y + 40, ui::layout::ButtonSize, ui::layout::ButtonSize };
					if (ui::Button(uiFont, U"+", duplicateRect))
					{
						m_chain.insert(m_chain.begin() + i + 1, m_chain[i]);
						m_chainEnabled.insert(m_chainEnabled.begin() + i + 1, m_chainEnabled[i]);
                      m_openEffectSelectIndex.reset();
						break;
					}
				}

				if (m_openEffectSelectIndex && (*m_openEffectSelectIndex == i))
				{
					const RectF listRect{ sectionRect.x + 8, sectionRect.y + ChainSectionBaseHeight, contentWidth - 16, m_effectNames.size() * EffectSelectRowHeight };
					for (size_t effectIndex = 0; effectIndex < m_effectNames.size(); ++effectIndex)
					{
						const RectF rowRect{ listRect.x, listRect.y + effectIndex * EffectSelectRowHeight, listRect.w, EffectSelectRowHeight - 2 };
						const bool selected = (m_chain[i] == effectIndex);
						const bool hovered = rowRect.mouseOver();
						const ColorF fill = selected ? ColorF{ 0.88, 0.94, 1.0, 1.0 } : (hovered ? ui::GetTheme().itemHovered : ui::GetTheme().item);
						rowRect.rounded(5).draw(fill);
						rowRect.rounded(5).drawFrame(1.0, ui::GetTheme().panelBorder);
						uiFont(m_effectNames[effectIndex]).draw(rowRect.x + 10, rowRect.y + 3, ui::GetTheme().text);
                      if (hovered)
						{
							ui::Tooltip(uiFont, getEffectTooltip(m_effectNames[effectIndex]), Cursor::PosF().movedBy(18, 20));
						}
						if (rowRect.leftClicked())
						{
							m_chain[i] = effectIndex;
							m_openEffectSelectIndex.reset();
						}
					}
				}

				uiPos.y += sectionHeight + ui::layout::SectionGap;
			}

			const RectF addRect{ uiPos, ui::layout::AddButtonWidth, ui::layout::AddButtonHeight };
			if ((m_chain.size() < MaxChainLength) && ui::Button(uiFont, U"+ 段を追加", addRect))
			{
				m_chain.push_back(0);
               m_chainEnabled.push_back(true);
			}
			uiPos.y += ui::layout::AddButtonHeight + ui::layout::SectionGap;
		}

     void drawPresetUI(const Font& uiFont, Vec2& uiPos, const double contentWidth, double& panelScrollY)
		{
			const double presetSectionBodyHeight = isPresetSectionCollapsed() ? CollapsedSectionHeight : getPresetSectionBodyHeight();
			const RectF presetSectionRect{ uiPos, contentWidth, presetSectionBodyHeight };
			ui::Section(presetSectionRect);
			uiFont(U"プリセット").draw(uiPos.movedBy(8, 0), ui::GetTheme().text);
          if (m_presetSectionCollapsed)
			{
				uiPos.y += presetSectionRect.h + ui::layout::SectionGap;
				return;
			}
			uiFont(U"現在: {}"_fmt(getCurrentPresetDisplayName())).draw(uiPos.movedBy(8, 28), Palette::Dimgray);
			uiFont(getCurrentPresetDescription()).draw(uiPos.movedBy(8, 54), Palette::Gray);

           double presetY = 88.0;
			for (const auto& preset : m_presets)
			{
               const RectF presetRect{ uiPos.x + 8, uiPos.y + presetY, contentWidth - 16, ui::layout::AddButtonHeight };
				if (ui::Button(uiFont, preset.displayName, presetRect))
				{
                   m_chain = preset.chain;
                 m_chainEnabled.assign(m_chain.size(), true);
					panelScrollY = 0.0;
				}
               presetY += (ui::layout::AddButtonHeight + 4.0);
			}

         const RectF noneRect{ uiPos.x + 8, uiPos.y + presetY, contentWidth - 16, ui::layout::AddButtonHeight };
			if (ui::Button(uiFont, U"なしに戻す", noneRect))
			{
				m_chain = { 0 };
             m_chainEnabled = { true };
				panelScrollY = 0.0;
			}

			uiPos.y += presetSectionRect.h + ui::layout::SectionGap;
		}

     void drawParamsUI(const Font& uiFont, Vec2& uiPos, const double contentWidth)
		{
			for (size_t i = 0; i < m_chain.size(); ++i)
			{
				const pe::Effect& e = m_effects[m_chain[i]];
				if (e.drawUI)
				{
					const double paramBlockHeight = getParamBlockHeight(e);
					const RectF paramSectionRect{ uiPos, contentWidth, 28 + paramBlockHeight };
					ui::Section(paramSectionRect);
                 uiFont(U"[{}] {}{}"_fmt(i + 1, e.name, m_chainEnabled[i] ? U"" : U" (OFF)")).draw(uiPos.movedBy(8, 0), ui::GetTheme().text);
					if (e.reset)
					{
						const RectF resetRect{ paramSectionRect.rightX() - 78, paramSectionRect.y + 4, 70, 24 };
						if (ui::Button(uiFont, U"Reset", resetRect))
						{
							e.reset();
						}
					}
					e.drawUI(uiPos.movedBy(8, 28));
					uiPos.y += paramSectionRect.h + ui::layout::SectionGap;
				}
			}
		}

	private:
      static constexpr double CollapsedSectionHeight = 42.0;
		static constexpr double ChainSectionBaseHeight = 82.0;
		static constexpr double EffectSelectRowHeight = 30.0;

		struct PresetEntry
		{
			String key;
			String displayName;
			String description;
			Array<size_t> chain;
		};

		[[nodiscard]] static FilePath resolveTomlPath()
		{
			const Array<FilePath> candidates = {
				U"Addons/Pi3D/Resources/toml/effect_presets.toml",
				U"../Addons/Pi3D/Resources/toml/effect_presets.toml",
				U"3D_Pi/Addons/Pi3D/Resources/toml/effect_presets.toml",
			};
			for (const auto& p : candidates)
			{
				if (FileSystem::Exists(p))
				{
					return p;
				}
			}
			return candidates.front();
		}

		[[nodiscard]] Array<PresetEntry> defaultPresets() const
		{
			return {
				PresetEntry{ U"cinematic", U"Cinematic", U"Bloom と ACES を軸にした映画風のチェイン", pe::GetCinematicPresetChain(m_effects) },
				PresetEntry{ U"dusty", U"Dusty (古い洋ゲー風)", U"黄土色寄り、乾いた空気感、古い洋ゲー風の画作り", pe::GetDustyPresetChain(m_effects) },
			};
		}

		[[nodiscard]] Array<PresetEntry> loadPresetsFromToml() const
		{
			Array<PresetEntry> results;
			const TOMLReader toml{ resolveTomlPath() };
			if (not toml)
			{
				return results;
			}

			try
			{
				for (const auto& t : toml[U"presets"].tableArrayView())
				{
					PresetEntry e;
					e.key = t[U"key"].getOpt<String>().value_or(U"custom");
					e.displayName = t[U"displayName"].getOpt<String>().value_or(U"Custom");
					e.description = t[U"description"].getOpt<String>().value_or(U"");

					Array<size_t> chain;
					for (const auto& effectNameNode : t[U"effects"].arrayView())
					{
						if (const auto effectName = effectNameNode.getOpt<String>())
						{
							const size_t idx = findEffectIndex(*effectName);
							if (idx != 0)
							{
								chain << idx;
							}
						}
					}
					if (chain.isEmpty())
					{
						chain = { 0 };
					}
					e.chain = chain;
					results << e;
				}
			}
			catch (const std::exception&)
			{
			}

			return results;
		}

		[[nodiscard]] double getPresetSectionBodyHeight() const
		{
			return 92.0 + (m_presets.size() + 1) * (ui::layout::AddButtonHeight + 4.0);
		}

		[[nodiscard]] double getChainSectionHeight(const size_t index) const
		{
			if (m_openEffectSelectIndex && (*m_openEffectSelectIndex == index))
			{
				return ChainSectionBaseHeight + m_effectNames.size() * EffectSelectRowHeight + 8.0;
			}
			return ChainSectionBaseHeight;
		}

		[[nodiscard]] int32 getParamRows(const pe::Effect& e) const
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
		}

		[[nodiscard]] double getParamBlockHeight(const pe::Effect& e) const
		{
			const int32 rows = getParamRows(e);
			if (rows <= 0)
			{
				return 0.0;
			}
          return (40.0 * rows);
		}

		[[nodiscard]] String getEffectTooltip(StringView effectName) const
		{
			if (effectName == U"なし")
			{
				return U"画面に変化を加えません。比較用の基準です。\n使いどころ: 他のエフェクトとの差を見たい時、段ごとの効き具合確認時。";
			}
			if (effectName == U"トゥーン")
			{
				return U"明暗の段数を減らして、セル画っぽい陰影にします。\n使いどころ: アニメ調、陰影を整理したい時、形をくっきり見せたい時。";
			}
			if (effectName == U"ピクセルアート")
			{
				return U"色の階調を整理して、簡略化されたレトロ画面寄りにします。\n使いどころ: レトロゲーム風、粗い質感を出したい時、情報量を減らしたい時。";
			}
			if (effectName == U"アウトライン")
			{
				return U"輪郭線を強調して、被写体の外形を見やすくします。\n使いどころ: トゥーン表現、読みやすさ重視、オブジェクトを目立たせたい時。";
			}
			if (effectName == U"油絵 (Kuwahara)")
			{
				return U"色のばらつきをならして、筆で塗ったような油絵風にします。\n使いどころ: 絵画調、夢っぽい画、現実感を少し崩したい時。";
			}
			if (effectName == U"CRT")
			{
				return U"湾曲、走査線、色マスクで古いモニタ風にします。\n使いどころ: レトロSF、ブラウン管演出、監視画面やゲーム内端末の表現。";
			}
			if (effectName == U"グレースケール")
			{
				return U"色を抜いて白黒にします。\n使いどころ: 回想、監視カメラ、雰囲気確認、明暗だけで見たい時。";
			}
			if (effectName == U"ポスタライズ")
			{
				return U"色数を減らして、ベタ塗り感の強い見た目にします。\n使いどころ: 印刷物風、グラフィック調、派手な記号化をしたい時。";
			}
			if (effectName == U"RGB シフト")
			{
				return U"色チャンネルを少しずらして、色収差やズレを出します。\n使いどころ: 軽いグリッチ、酔った視界、異常感やサイバー感を足したい時。";
			}
			if (effectName == U"Glitch")
			{
				return U"横ずれ、RGB分離、ブロックノイズでデジタル破綻風にします。\n使いどころ: SFハッキング、通信障害、異常演出、サイバー感を強く出したい時。";
			}
			if (effectName == U"スワール")
			{
				return U"画面をねじるように歪ませます。\n使いどころ: ワープ、魔法、違和感演出、画面遷移のアクセント。";
			}
			if (effectName == U"ブライトパス抽出")
			{
				return U"明るい部分だけを抜き出します。単体では地味ですが Bloom の素材になります。\n使いどころ: 発光部分の確認、Bloom 用の事前抽出、輝度マスクの確認。";
			}
			if (effectName == U"Bloom")
			{
				return U"明るい部分をにじませて、発光感を足します。\n使いどころ: 夜景、ネオン、魔法、映像を少しリッチに見せたい時。";
			}
			if (effectName == U"DoF")
			{
				return U"ピント面以外をぼかして、被写界深度を作ります。\n使いどころ: 被写体強調、ミニチュア風、シネマ風、視線誘導したい時。";
			}
			if (effectName == U"Tonemap (ACES)")
			{
				return U"明るさを整えて、白飛びを抑えつつ映画っぽい階調に寄せます。\n使いどころ: HDR感の整理、Bloom 後の画作り、全体を自然にまとめたい時。";
			}
			if (effectName == U"Vignette")
			{
				return U"画面周辺を暗くして、中央へ視線を集めます。\n使いどころ: シネマ風、視線誘導、緊張感や閉塞感を足したい時。";
			}
			if (effectName == U"Film Grain")
			{
				return U"細かな粒状ノイズを重ねて、フィルムや高感度撮影っぽくします。\n使いどころ: 映画風、アナログ感、無機質なCGを少し馴染ませたい時。";
			}
			if (effectName == U"FXAA")
			{
				return U"輪郭のジャギーを軽くぼかして目立ちにくくします。\n使いどころ: 最終段でのギザギザ軽減、画面を少し滑らかに見せたい時。";
			}
			if (effectName == U"Warm Grade")
			{
				return U"全体を暖色寄りに寄せ、乾いた古い洋ゲー風の色味にします。\n使いどころ: 夕景、荒野、懐かしい海外ゲーム風、土っぽい空気感を出したい時。";
			}

			return U"このエフェクトの説明は未設定です。\n使いどころ: 実際に ON/OFF して差分を確認してください。";
		}

		[[nodiscard]] size_t findEffectIndex(StringView name) const
		{
			for (size_t i = 0; i < m_effects.size(); ++i)
			{
				if (m_effects[i].name == name)
				{
					return i;
				}
			}
			return 0;
		}

		[[nodiscard]] String getCurrentPresetDisplayName() const
		{
			for (const auto& p : m_presets)
			{
				if (m_chain == p.chain)
				{
					return p.displayName;
				}
			}
			if ((m_chain.size() == 1) && (m_chain[0] == 0))
			{
				return U"なし";
			}
			return U"Custom";
		}

		[[nodiscard]] String getCurrentPresetDescription() const
		{
			for (const auto& p : m_presets)
			{
				if (m_chain == p.chain)
				{
					return p.description;
				}
			}
			if ((m_chain.size() == 1) && (m_chain[0] == 0))
			{
				return U"ポストエフェクトなし";
			}
			return U"手動編集されたチェイン";
		}

		Array<pe::Effect> m_effects;
		Array<String> m_effectNames;
		Array<PresetEntry> m_presets;
		Array<size_t> m_chain = { 0 };
       Array<bool> m_chainEnabled = { true };
       bool m_presetSectionCollapsed = false;
       Optional<size_t> m_openEffectSelectIndex;
		size_t m_prevLightingPresetIndex = 0;
		bool m_hasPrevLightingPreset = false;
	};
}
