# include "SkyAppInternal.hpp"
# include "SkyAppTitleSceneInternal.hpp"

namespace SkyAppInternal
{
	namespace
	{
     using namespace TitleSceneDetail;

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

				if (data.pendingDeleteCampaignId)
				{
					if (IsRectButtonClicked(GetDeleteConfirmButton()))
					{
						if (SkyCampaign::DeleteCampaign(*data.pendingDeleteCampaignId))
						{
							data.titleMessage = U"Deleted: {}"_fmt(data.pendingDeleteCampaignName);
						}
						else
						{
							data.titleMessage = U"Delete failed: {}"_fmt(data.pendingDeleteCampaignName);
						}

						data.titleMessageUntil = (Scene::Time() + 3.0);
						data.pendingDeleteCampaignId.reset();
						data.pendingDeleteCampaignName.clear();
						ReloadCampaigns(data);
						return;
					}

					if (IsRectButtonClicked(GetDeleteCancelButton()) || KeyEscape.down())
					{
						data.pendingDeleteCampaignId.reset();
						data.pendingDeleteCampaignName.clear();
						return;
					}

					return;
				}

				if (data.pendingResetCampaignId)
				{
					if (IsRectButtonClicked(GetDeleteConfirmButton()))
					{
						if (SkyCampaign::DeleteCampaignProgress(*data.pendingResetCampaignId))
						{
							data.titleMessage = U"Progress reset: {}"_fmt(data.pendingResetCampaignName);
						}
						else
						{
							data.titleMessage = U"Reset failed: {}"_fmt(data.pendingResetCampaignName);
						}

						data.titleMessageUntil = (Scene::Time() + 3.0);
						data.pendingResetCampaignId.reset();
						data.pendingResetCampaignName.clear();
						return;
					}

					if (IsRectButtonClicked(GetDeleteCancelButton()) || KeyEscape.down())
					{
						data.pendingResetCampaignId.reset();
						data.pendingResetCampaignName.clear();
						return;
					}

					return;
				}

				if (not data.campaigns.isEmpty())
				{
					if (KeyUp.down())
					{
						const size_t currentIndex = data.selectedCampaignIndex.value_or(0);
						data.selectedCampaignIndex = ((currentIndex == 0) ? (data.campaigns.size() - 1) : (currentIndex - 1));
					}
					else if (KeyDown.down())
					{
						const size_t currentIndex = data.selectedCampaignIndex.value_or(0);
						data.selectedCampaignIndex = ((currentIndex + 1) % data.campaigns.size());
					}
				}

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

             const bool hasCurrentSelection = (data.selectedCampaignIndex && (*data.selectedCampaignIndex < data.campaigns.size()));

				if (IsRectButtonClicked(GetSelectedResetButton(), hasCurrentSelection && CanResetSelectedCampaign(data)))
				{
					if (const auto* campaign = FindSelectedCampaign(data))
					{
						data.pendingResetCampaignId = campaign->id;
						data.pendingResetCampaignName = campaign->displayName;
						return;
					}
				}

				if (IsRectButtonClicked(GetSelectedPlayButton(), hasCurrentSelection))
				{
					if (StartSelectedCampaign(data, false))
					{
						changeScene(data.dialogueSceneLines.isEmpty() ? U"Battle" : U"Dialogue", 0);
						return;
					}
				}

                if (IsRectButtonClicked(GetSelectedContinueButton(), hasCurrentSelection && CanContinueSelectedCampaign(data)))
				{
					if (StartSelectedCampaign(data, true))
					{
						changeScene(data.dialogueSceneLines.isEmpty() ? U"Battle" : U"Dialogue", 0);
						return;
					}
				}

                if (ShowDebugCampaignButtons && IsRectButtonClicked(GetSelectedEditButton(), hasCurrentSelection))
				{
					if (const auto* campaign = FindSelectedCampaign(data))
					{
						LoadCampaignIntoEditor(data, *campaign);
						changeScene(U"CampaignEditor", 0);
						return;
					}
				}

				if (IsRectButtonClicked(GetExitButton()))
				{
					System::Exit();
					return;
				}

                if (ShowDebugCampaignButtons && IsRectButtonClicked(GetSelectedDeleteButton(), hasCurrentSelection))
				{
					if (const auto* campaign = FindSelectedCampaign(data))
					{
                      data.pendingDeleteCampaignId = campaign->id;
						data.pendingDeleteCampaignName = campaign->displayName;
						return;
					}
				}
			}

			void draw() const override
			{
				const auto& data = getData();
             Optional<std::pair<RectF, String>> hoveredTooltip;
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
				m_infoFont(U"↑↓ で Campaign 選択").draw(leftPanel.pos.movedBy(26, 204), ColorF{ 0.84, 0.90, 0.98, 0.86 });

				DrawRectButton(GetBattleButton(), m_buttonFont, U"Battle Start");
				DrawRectButton(GetCampaignEditorButton(), m_buttonFont, U"Campaign Editor");
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
                      const bool hasProgress = SkyCampaign::HasCampaignProgress(data.campaigns[i].id);
						const SkyCampaign::CampaignProgress progress = SkyCampaign::LoadCampaignProgress(data.campaigns[i].id);
						DrawCampaignRow(GetCampaignRow(i), m_infoFont, m_campaignRowFont, data.campaigns[i], progress, hasProgress, (data.selectedCampaignIndex && (*data.selectedCampaignIndex == i)));
					}

					if (data.selectedCampaignIndex && (*data.selectedCampaignIndex < data.campaigns.size()))
					{
						const auto& campaign = data.campaigns[*data.selectedCampaignIndex];
						const SkyCampaign::CampaignProgress progress = SkyCampaign::LoadCampaignProgress(campaign.id);
                     const bool canResetSelection = SkyCampaign::HasCampaignProgress(campaign.id);
                        const bool hasSelection = true;
                        m_infoFont(U"Selected: {}"_fmt(FormatCampaignTitle(campaign, progress.clearCount))).draw(rightPanel.pos.movedBy(24, 420), Palette::White);
                     DrawCampaignActionButton(GetSelectedResetButton(), CampaignActionIcon::Reset, canResetSelection);
						DrawPlayAttentionEffect(GetSelectedPlayButton());
                        DrawCampaignActionButton(GetSelectedPlayButton(), CampaignActionIcon::Play, hasSelection);
						DrawCampaignActionButton(GetSelectedContinueButton(), CampaignActionIcon::Continue, CanContinueSelectedCampaign(data));

                        if (GetSelectedResetButton().mouseOver())
						{
							hoveredTooltip = std::pair{ GetSelectedResetButton(), String{ U"Reset" } };
						}
						else if (GetSelectedPlayButton().mouseOver())
						{
							hoveredTooltip = std::pair{ GetSelectedPlayButton(), String{ U"Play" } };
						}
						else if (GetSelectedContinueButton().mouseOver())
						{
							hoveredTooltip = std::pair{ GetSelectedContinueButton(), String{ U"Continue" } };
						}

						if (ShowDebugCampaignButtons)
						{
							DrawCampaignActionButton(GetSelectedEditButton(), CampaignActionIcon::Edit, hasSelection);
							DrawCampaignActionButton(GetSelectedDeleteButton(), CampaignActionIcon::Delete, hasSelection);

							if (GetSelectedEditButton().mouseOver())
							{
								hoveredTooltip = std::pair{ GetSelectedEditButton(), String{ U"Edit" } };
							}
							else if (GetSelectedDeleteButton().mouseOver())
							{
								hoveredTooltip = std::pair{ GetSelectedDeleteButton(), String{ U"Delete" } };
							}
						}

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

				if (hoveredTooltip)
				{
					DrawCampaignActionTooltip(hoveredTooltip->first, hoveredTooltip->second);
				}

				if (data.pendingDeleteCampaignId)
				{
					Scene::Rect().draw(ColorF{ 0.0, 0.0, 0.0, 0.42 });
					const RectF dialog = GetDeleteDialogRect();
					dialog.rounded(20).draw(ColorF{ 0.08, 0.13, 0.21, 0.98 });
					dialog.rounded(20).drawFrame(2, 0, ColorF{ 0.78, 0.84, 0.94, 0.84 });
					m_buttonFont(U"Delete Campaign?").draw(dialog.pos.movedBy(22, 18), Palette::White);
					m_infoFont(U"{} を削除します"_fmt(data.pendingDeleteCampaignName)).draw(dialog.pos.movedBy(24, 64), ColorF{ 0.90, 0.94, 1.0, 0.96 });
					m_infoFont(U"progress も削除されます").draw(dialog.pos.movedBy(24, 92), ColorF{ 1.0, 0.88, 0.76, 0.96 });
					DrawRectButton(GetDeleteConfirmButton(), m_infoFont, U"Delete");
					DrawRectButton(GetDeleteCancelButton(), m_infoFont, U"Cancel");
				}

				if (data.pendingResetCampaignId)
				{
					Scene::Rect().draw(ColorF{ 0.0, 0.0, 0.0, 0.42 });
					const RectF dialog = GetDeleteDialogRect();
					dialog.rounded(20).draw(ColorF{ 0.08, 0.13, 0.21, 0.98 });
					dialog.rounded(20).drawFrame(2, 0, ColorF{ 0.78, 0.84, 0.94, 0.84 });
					m_buttonFont(U"Reset Progress?").draw(dialog.pos.movedBy(22, 18), Palette::White);
					m_infoFont(U"{} の progress をリセットします"_fmt(data.pendingResetCampaignName)).draw(dialog.pos.movedBy(24, 64), ColorF{ 0.90, 0.94, 1.0, 0.96 });
					m_infoFont(U"クリア回数と Continue 状態が消えます").draw(dialog.pos.movedBy(24, 92), ColorF{ 1.0, 0.88, 0.76, 0.96 });
					DrawRectButton(GetDeleteConfirmButton(), m_infoFont, U"Reset");
					DrawRectButton(GetDeleteCancelButton(), m_infoFont, U"Cancel");
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

           [[nodiscard]] RectF GetSelectedEditButton() const
			{
              return RectF{ 1010, 414, 40, 40 };
			}

			[[nodiscard]] RectF GetSelectedResetButton() const
			{
				return RectF{ 962, 414, 40, 40 };
			}

           [[nodiscard]] RectF GetSelectedPlayButton() const
			{
              return RectF{ 1058, 414, 40, 40 };
			}

           [[nodiscard]] RectF GetSelectedContinueButton() const
			{
              return RectF{ 1106, 414, 40, 40 };
			}

         [[nodiscard]] RectF GetSelectedDeleteButton() const
			{
              return RectF{ 1154, 414, 40, 40 };
			}

			[[nodiscard]] RectF GetExitButton() const
			{
				return RectF{ 120, 654, 280, 52 };
			}

			[[nodiscard]] RectF GetDeleteDialogRect() const
			{
				return RectF{ Arg::center = Scene::CenterF(), 360, 180 };
			}

			[[nodiscard]] RectF GetDeleteConfirmButton() const
			{
				const RectF dialog = GetDeleteDialogRect();
				return RectF{ dialog.x + 32, dialog.y + 122, 128, 36 };
			}

			[[nodiscard]] RectF GetDeleteCancelButton() const
			{
				const RectF dialog = GetDeleteDialogRect();
				return RectF{ dialog.x + 200, dialog.y + 122, 128, 36 };
			}

			[[nodiscard]] RectF GetCampaignRow(const size_t index) const
			{
				return RectF{ 504, (150 + static_cast<double>(index) * 72.0), 672, 60 };
			}

			[[nodiscard]] bool CanResetSelectedCampaign(const SkyAppData& data) const
			{
				if (const auto* campaign = FindSelectedCampaign(data))
				{
					return SkyCampaign::HasCampaignProgress(campaign->id);
				}

				return false;
			}
		};

	}

	void AddTitleScene(App& manager)
	{
		manager.add<SkyTitleScene>(U"Title");
	}

}
