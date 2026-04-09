# include "SkyAppInternal.hpp"

namespace SkyAppInternal
{
	namespace
	{
		void DrawCampaignRow(const RectF& rect, const Font& titleFont, const Font& infoFont, const SkyCampaign::CampaignDefinition& campaign, const bool selected)
		{
			const bool hovered = rect.mouseOver();
			rect.rounded(12).draw(selected
				? ColorF{ 0.20, 0.30, 0.44, 0.96 }
				: (hovered ? ColorF{ 0.14, 0.20, 0.30, 0.92 } : ColorF{ 0.10, 0.14, 0.22, 0.88 }));
			rect.rounded(12).drawFrame(2, 0, selected ? ColorF{ 0.92, 0.96, 1.0, 0.92 } : ColorF{ 0.42, 0.52, 0.64, 0.82 });
			titleFont(campaign.displayName).draw(rect.pos.movedBy(14, 10), Palette::White);
			infoFont(U"Missions: {}"_fmt(campaign.missions.size())).draw(rect.pos.movedBy(14, 38), ColorF{ 0.82, 0.89, 0.98, 0.92 });
		}

		class SkyTitleScene : public App::Scene
		{
		public:
			explicit SkyTitleScene(const InitData& init)
				: App::Scene{ init }
				, m_titleFont{ 42, Typeface::Heavy }
				, m_buttonFont{ 24, Typeface::Medium }
				, m_infoFont{ 18 }
				, m_campaignRowFont{ 14 }
			{
				ReloadCampaigns(getData());
			}

			void update() override
			{
				auto& data = getData();
				const bool hasSelection = (data.selectedCampaignIndex && (*data.selectedCampaignIndex < data.campaigns.size()));

				for (size_t i = 0; i < data.campaigns.size(); ++i)
				{
					if (GetCampaignRow(i).leftClicked())
					{
						data.selectedCampaignIndex = i;
					}
				}

				if (KeyEnter.down() || IsRectButtonClicked(GetBattleButton()))
				{
					data.pendingBattleMapPath = MainSupport::MapDataPath;
					data.activeCampaignId.reset();
					data.activeCampaignMissionIndex.reset();
					data.battleReturnScene = U"Title";
					data.launchBattleInMapEditor = false;
					changeScene(U"Battle", 0);
					return;
				}

				if (IsRectButtonClicked(GetCampaignEditorButton()))
				{
					PrepareNewCampaignDraft(data);
					changeScene(U"CampaignEditor", 0);
					return;
				}

				if (IsRectButtonClicked(GetEditCampaignButton(), hasSelection))
				{
					if (const auto* campaign = FindSelectedCampaign(data))
					{
						LoadCampaignIntoEditor(data, *campaign);
						changeScene(U"CampaignEditor", 0);
						return;
					}
				}

				if (IsRectButtonClicked(GetPlayCampaignButton(), hasSelection))
				{
					if (StartSelectedCampaign(data, false))
					{
						changeScene(U"Battle", 0);
						return;
					}
				}

				if (IsRectButtonClicked(GetContinueCampaignButton(), hasSelection && CanContinueSelectedCampaign(data)))
				{
					if (StartSelectedCampaign(data, true))
					{
						changeScene(U"Battle", 0);
						return;
					}
				}

				if (IsRectButtonClicked(GetExitButton()))
				{
					System::Exit();
					return;
				}

				if (IsRectButtonClicked(GetDeleteCampaignButton(), hasSelection))
				{
					if (const auto* campaign = FindSelectedCampaign(data))
					{
						if (SkyCampaign::DeleteCampaign(campaign->id))
						{
							data.titleMessage = U"Deleted: {}"_fmt(campaign->displayName);
						}
						else
						{
							data.titleMessage = U"Delete failed: {}"_fmt(campaign->displayName);
						}

						data.titleMessageUntil = (Scene::Time() + 3.0);
						ReloadCampaigns(data);
					}
				}
			}

			void draw() const override
			{
				const auto& data = getData();
				Scene::Rect().draw(ColorF{ 0.06, 0.10, 0.16 });
				const RectF leftPanel{ 80, 28, 360, 664 };
				const RectF rightPanel{ 480, 28, 720, 664 };
				leftPanel.rounded(24).draw(ColorF{ 0.08, 0.13, 0.21, 0.96 });
				leftPanel.rounded(24).drawFrame(2, 0, ColorF{ 0.56, 0.72, 0.92, 0.72 });
				rightPanel.rounded(24).draw(ColorF{ 0.08, 0.13, 0.21, 0.96 });
				rightPanel.rounded(24).drawFrame(2, 0, ColorF{ 0.56, 0.72, 0.92, 0.72 });

				m_titleFont(U"SKY").draw(leftPanel.pos.movedBy(24, 24), ColorF{ 0.96, 0.98, 1.0 });
				m_infoFont(U"Battle / Campaign の入口").draw(leftPanel.pos.movedBy(26, 84), ColorF{ 0.82, 0.90, 1.0, 0.92 });
				m_infoFont(U"起動直後は今まで通り Battle 直行").draw(leftPanel.pos.movedBy(26, 114), ColorF{ 0.84, 0.90, 0.98, 0.86 });
				m_infoFont(U"保存済み Campaign は右側に表示").draw(leftPanel.pos.movedBy(26, 142), ColorF{ 0.84, 0.90, 0.98, 0.86 });
				m_infoFont(U"Enter でもバトル開始").draw(leftPanel.pos.movedBy(26, 176), ColorF{ 1.0, 0.94, 0.72, 0.92 });

				DrawRectButton(GetBattleButton(), m_buttonFont, U"Battle Start");
				DrawRectButton(GetCampaignEditorButton(), m_buttonFont, U"Campaign Editor");
				DrawRectButton(GetEditCampaignButton(), m_buttonFont, U"Edit Selected Campaign", (data.selectedCampaignIndex && (*data.selectedCampaignIndex < data.campaigns.size())));
				DrawRectButton(GetPlayCampaignButton(), m_buttonFont, U"Play Selected Campaign", (data.selectedCampaignIndex && (*data.selectedCampaignIndex < data.campaigns.size())));
				DrawRectButton(GetContinueCampaignButton(), m_buttonFont, U"Continue Campaign", (data.selectedCampaignIndex && (*data.selectedCampaignIndex < data.campaigns.size()) && CanContinueSelectedCampaign(data)));
				DrawRectButton(GetDeleteCampaignButton(), m_buttonFont, U"Delete Selected Campaign", (data.selectedCampaignIndex && (*data.selectedCampaignIndex < data.campaigns.size())));
				DrawRectButton(GetExitButton(), m_buttonFont, U"Exit");

				m_buttonFont(U"Campaigns").draw(rightPanel.pos.movedBy(24, 20), ColorF{ 0.96, 0.98, 1.0 });
				if (data.campaigns.isEmpty())
				{
					m_infoFont(U"まだ保存済み Campaign はありません").draw(rightPanel.pos.movedBy(24, 72), ColorF{ 0.84, 0.90, 0.98, 0.86 });
					m_infoFont(U"Campaign Editor から新規作成できます").draw(rightPanel.pos.movedBy(24, 100), ColorF{ 0.84, 0.90, 0.98, 0.86 });
				}
				else
				{
					for (size_t i = 0; i < data.campaigns.size(); ++i)
					{
						DrawCampaignRow(GetCampaignRow(i), m_infoFont, m_campaignRowFont, data.campaigns[i], (data.selectedCampaignIndex && (*data.selectedCampaignIndex == i)));
					}

					if (data.selectedCampaignIndex && (*data.selectedCampaignIndex < data.campaigns.size()))
					{
						const auto& campaign = data.campaigns[*data.selectedCampaignIndex];
						const SkyCampaign::CampaignProgress progress = SkyCampaign::LoadCampaignProgress(campaign.id);
						m_infoFont(U"Selected: {}"_fmt(campaign.displayName)).draw(rightPanel.pos.movedBy(24, 420), Palette::White);
						m_infoFont(campaign.description.isEmpty() ? U"説明なし" : campaign.description).draw(rightPanel.pos.movedBy(24, 450), ColorF{ 0.82, 0.89, 0.98, 0.92 });
						m_infoFont(U"1st Map: {}"_fmt(campaign.missions.front().mapFile)).draw(rightPanel.pos.movedBy(24, 480), ColorF{ 0.76, 0.84, 0.94, 0.90 });
						m_infoFont(SkyCampaign::HasCampaignProgress(campaign.id)
							? (progress.completed
								? U"Progress: Complete"
								: U"Progress: Mission {} / {}"_fmt(progress.currentMissionIndex + 1, campaign.missions.size()))
							: U"Progress: Not started")
							.draw(rightPanel.pos.movedBy(24, 510), ColorF{ 1.0, 0.94, 0.72, 0.92 });
					}
				}

				if (Scene::Time() < data.titleMessageUntil)
				{
					m_infoFont(data.titleMessage).draw(leftPanel.pos.movedBy(26, 622), ColorF{ 1.0, 0.94, 0.72, 0.96 });
				}
			}

		private:
			Font m_titleFont;
			Font m_buttonFont;
			Font m_infoFont;
			Font m_campaignRowFont;

			[[nodiscard]] RectF GetBattleButton() const
			{
				return RectF{ 120, 246, 280, 52 };
			}

			[[nodiscard]] RectF GetCampaignEditorButton() const
			{
				return RectF{ 120, 314, 280, 52 };
			}

			[[nodiscard]] RectF GetEditCampaignButton() const
			{
				return RectF{ 120, 382, 280, 52 };
			}

			[[nodiscard]] RectF GetPlayCampaignButton() const
			{
				return RectF{ 120, 450, 280, 52 };
			}

			[[nodiscard]] RectF GetContinueCampaignButton() const
			{
				return RectF{ 120, 518, 280, 52 };
			}

			[[nodiscard]] RectF GetDeleteCampaignButton() const
			{
				return RectF{ 120, 586, 280, 52 };
			}

			[[nodiscard]] RectF GetExitButton() const
			{
				return RectF{ 120, 654, 280, 52 };
			}

			[[nodiscard]] RectF GetCampaignRow(const size_t index) const
			{
				return RectF{ 504, (150 + static_cast<double>(index) * 72.0), 672, 60 };
			}
		};
	}

	void AddTitleScene(App& manager)
	{
		manager.add<SkyTitleScene>(U"Title");
	}
}
