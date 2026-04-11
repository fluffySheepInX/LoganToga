# include "SkyAppInternal.hpp"
# include "SkyAppCampaignEditorSceneInternal.hpp"

namespace SkyAppInternal
{
	void AddCampaignDescriptionEditorScene(App& manager);
	void AddMissionDialogueEditorScene(App& manager);

	namespace
	{
		using namespace CampaignEditorDetail;

		class SkyCampaignEditorScene : public App::Scene
		{
		public:
			explicit SkyCampaignEditorScene(const InitData& init)
				: App::Scene{ init }
				, m_titleFont{ 34, Typeface::Heavy }
				, m_labelFont{ 18, Typeface::Medium }
				, m_buttonFont{ 22, Typeface::Medium }
			{
				auto& data = getData();
				if (data.missionNameStates.isEmpty())
				{
					PrepareNewCampaignDraft(data);
				}

				ClampEditorMissionSelection(data);
			}

			void update() override
			{
				auto& data = getData();
              const Array<String> hintLines{
					U"Save: 現在の Campaign 構成を保存",
					U"Map Edit: 選択 Mission のマップを編集",
					U"Test Play: 選択 Mission のマップで確認",
				};
				ClampEditorMissionSelection(data);
				Scene::Rect().draw(ColorF{ 0.05, 0.09, 0.15 });
				const RectF panel{ 72, 32, 1140, 664 };
				const RectF campaignSection = GetCampaignSectionRect(panel);
				const RectF missionSection = GetMissionSectionRect(panel);
				const RectF detailSection = GetDetailSectionRect(panel);
				const RectF missionListRect = GetMissionListRect(panel);
				panel.rounded(24).draw(ColorF{ 0.08, 0.13, 0.21, 0.96 });
				panel.rounded(24).drawFrame(2, 0, ColorF{ 0.56, 0.72, 0.92, 0.72 });
				const size_t selectedMissionIndex = *data.selectedEditorMissionIndex;
				const int32 maxOffset = Max(0, static_cast<int32>(data.missionNameStates.size()) - EditorVisibleMissionCount);

				m_titleFont(U"Campaign Editor").draw(panel.pos.movedBy(28, 24), Palette::White);
				m_labelFont(U"Step 2: 複数 Mission 編集").draw(panel.pos.movedBy(30, 72), ColorF{ 0.84, 0.90, 0.98, 0.92 });
				DrawEditorSection(campaignSection, U"Campaign", m_labelFont);
				DrawEditorSection(missionSection, U"Mission List", m_labelFont);
				DrawEditorSection(detailSection, U"Mission Detail", m_labelFont);

				m_labelFont(U"Campaign Name").draw(campaignSection.pos.movedBy(18, 54), Palette::White);
				SimpleGUI::TextBox(data.campaignNameState, campaignSection.pos.movedBy(18, 82), 280, 48);

				m_labelFont(U"Description").draw(campaignSection.pos.movedBy(18, 142), Palette::White);
				const RectF descriptionPreviewRect{ campaignSection.x + 18, campaignSection.y + 170, 280, 128 };
				descriptionPreviewRect.rounded(12).draw(ColorF{ 0.07, 0.11, 0.18, 0.92 });
				descriptionPreviewRect.rounded(12).drawFrame(1, 0, ColorF{ 0.42, 0.52, 0.64, 0.82 });
				m_labelFont(data.campaignDescriptionState.text.trimmed().isEmpty()
					? U"No description"
					: MakePreviewText(data.campaignDescriptionState.text, 96)).draw(descriptionPreviewRect.pos.movedBy(14, 14), ColorF{ 0.88, 0.93, 1.0, 0.94 });
				DrawEditorActionButton(GetEditDescriptionButton(panel), m_labelFont, U"Edit Description", EditorActionButtonStyle::Secondary);

				missionListRect.rounded(12).draw(ColorF{ 0.07, 0.11, 0.18, 0.92 });
				missionListRect.rounded(12).drawFrame(1, 0, ColorF{ 0.42, 0.52, 0.64, 0.82 });

				for (int32 row = 0; row < EditorVisibleMissionCount; ++row)
				{
					const size_t missionIndex = static_cast<size_t>(data.editorMissionListOffset + row);
					if (data.missionNameStates.size() <= missionIndex)
					{
						break;
					}

					const RectF rowRect = GetMissionRowRect(row);
					const bool selected = (missionIndex == selectedMissionIndex);
					rowRect.rounded(10).draw(selected ? ColorF{ 0.20, 0.30, 0.44, 0.96 } : ColorF{ 0.11, 0.16, 0.24, 0.90 });
					rowRect.rounded(10).drawFrame(1, 0, selected ? ColorF{ 0.92, 0.96, 1.0, 0.92 } : ColorF{ 0.34, 0.44, 0.56, 0.78 });
					m_labelFont(U"{}. {}"_fmt(missionIndex + 1, data.missionNameStates[missionIndex].text.isEmpty() ? U"Mission {}"_fmt(missionIndex + 1) : data.missionNameStates[missionIndex].text)).draw(rowRect.pos.movedBy(12, 7), Palette::White);

					if (rowRect.leftClicked())
					{
						data.selectedEditorMissionIndex = missionIndex;
					}
				}

				DrawEditorActionButton(GetMissionAddButton(panel), m_labelFont, U"Add", EditorActionButtonStyle::Secondary);
				DrawEditorActionButton(GetMissionRemoveButton(panel), m_labelFont, U"Remove", EditorActionButtonStyle::Secondary, (1 < data.missionNameStates.size()));
				DrawEditorActionButton(GetMissionUpButton(panel), m_labelFont, U"Move Up", EditorActionButtonStyle::Secondary, (0 < selectedMissionIndex));
				DrawEditorActionButton(GetMissionDownButton(panel), m_labelFont, U"Move Down", EditorActionButtonStyle::Secondary, ((selectedMissionIndex + 1) < data.missionNameStates.size()));
				DrawEditorActionButton(GetMissionScrollUpButton(panel), m_labelFont, U"▲", EditorActionButtonStyle::Secondary, (0 < data.editorMissionListOffset));
				DrawEditorActionButton(GetMissionScrollDownButton(panel), m_labelFont, U"▼", EditorActionButtonStyle::Secondary, (data.editorMissionListOffset < maxOffset));

				if (IsRectButtonClicked(GetMissionAddButton(panel)))
				{
					AddEditorMission(data);
				}

				if (IsRectButtonClicked(GetMissionRemoveButton(panel), (1 < data.missionNameStates.size())))
				{
					RemoveSelectedEditorMission(data);
				}

				if (IsRectButtonClicked(GetMissionUpButton(panel), (0 < selectedMissionIndex)))
				{
					MoveSelectedEditorMission(data, -1);
				}

				if (IsRectButtonClicked(GetMissionDownButton(panel), ((selectedMissionIndex + 1) < data.missionNameStates.size())))
				{
					MoveSelectedEditorMission(data, 1);
				}

				if (IsRectButtonClicked(GetMissionScrollUpButton(panel), (0 < data.editorMissionListOffset)))
				{
					--data.editorMissionListOffset;
				}

				if (IsRectButtonClicked(GetMissionScrollDownButton(panel), (data.editorMissionListOffset < maxOffset)))
				{
					++data.editorMissionListOffset;
				}

				ClampEditorMissionSelection(data);
				const size_t currentMissionIndex = *data.selectedEditorMissionIndex;

				m_labelFont(U"Selected Mission: {} / {}"_fmt(currentMissionIndex + 1, data.missionNameStates.size())).draw(detailSection.pos.movedBy(18, 52), ColorF{ 0.88, 0.93, 1.0, 0.98 });
				m_labelFont(U"Mission Name").draw(detailSection.pos.movedBy(18, 92), Palette::White);
				SimpleGUI::TextBox(data.missionNameStates[currentMissionIndex], detailSection.pos.movedBy(18, 120), 418, 48);

				m_labelFont(U"Map Path").draw(detailSection.pos.movedBy(18, 180), Palette::White);
                SimpleGUI::TextBox(data.missionMapPathStates[currentMissionIndex], detailSection.pos.movedBy(18, 208), 326, 80);
				DrawEditorActionButton(GetBrowseMapPathButton(panel), m_labelFont, U"Browse", EditorActionButtonStyle::Secondary);

				m_labelFont(U"Dialogue").draw(detailSection.pos.movedBy(18, 268), Palette::White);
				const RectF dialoguePreviewRect{ detailSection.x + 18, detailSection.y + 296, 418, 122 };
				dialoguePreviewRect.rounded(12).draw(ColorF{ 0.07, 0.11, 0.18, 0.92 });
				dialoguePreviewRect.rounded(12).drawFrame(1, 0, ColorF{ 0.42, 0.52, 0.64, 0.82 });
				m_labelFont(U"Pre: {}"_fmt(MakeDialogueSummary(data.missionPreDialogueStates[currentMissionIndex].text))).draw(dialoguePreviewRect.pos.movedBy(14, 14), ColorF{ 0.88, 0.93, 1.0, 0.94 });
				m_labelFont(U"Post: {}"_fmt(MakeDialogueSummary(data.missionPostDialogueStates[currentMissionIndex].text))).draw(dialoguePreviewRect.pos.movedBy(14, 48), ColorF{ 0.84, 0.90, 0.98, 0.92 });
				DrawEditorActionButton(GetEditDialogueButton(panel), m_labelFont, U"Edit Dialogue", EditorActionButtonStyle::Secondary);

				DrawEditorActionButton(GetSaveButton(panel), m_buttonFont, U"Save", EditorActionButtonStyle::Primary);
				DrawEditorActionButton(GetMapEditButton(panel), m_labelFont, U"Map Edit", EditorActionButtonStyle::Secondary);
				DrawEditorActionButton(GetTestPlayButton(panel), m_labelFont, U"Test Play", EditorActionButtonStyle::Secondary);
				DrawEditorActionButton(GetBackButton(panel), m_labelFont, U"Back to Title", EditorActionButtonStyle::Back);
				DrawEditorActionButton(GetHintButton(panel), m_labelFont, U"Hint", EditorActionButtonStyle::Secondary);

				if (GetHintButton(panel).mouseOver())
				{
					DrawEditorHintPopup(GetHintButton(panel), m_labelFont, hintLines);
				}

				if (IsRectButtonClicked(GetEditDescriptionButton(panel)))
				{
					changeScene(U"CampaignDescriptionEditor", 0);
					return;
				}

				if (IsRectButtonClicked(GetEditDialogueButton(panel)))
				{
					changeScene(U"MissionDialogueEditor", 0);
					return;
				}

				if (IsRectButtonClicked(GetBrowseMapPathButton(panel)))
				{
					if (const Optional<FilePath> path = Dialog::OpenFile({ FileFilter::AllFiles() }))
					{
						data.missionMapPathStates[currentMissionIndex].text = *path;
					}
				}

				if (IsRectButtonClicked(GetMapEditButton(panel)))
				{
					data.pendingBattleMapPath = EnsureEditorMissionMapPath(data);
					data.activeCampaignId.reset();
					data.activeCampaignMissionIndex.reset();
					data.battleReturnScene = U"CampaignEditor";
					data.launchBattleInMapEditor = true;
					changeScene(U"Battle", 0);
					return;
				}

				if (IsRectButtonClicked(GetTestPlayButton(panel)))
				{
					data.pendingBattleMapPath = EnsureEditorMissionMapPath(data);
					data.activeCampaignId.reset();
					data.activeCampaignMissionIndex.reset();
					data.battleReturnScene = U"CampaignEditor";
					data.launchBattleInMapEditor = false;
					changeScene(U"Battle", 0);
					return;
				}

				if (IsRectButtonClicked(GetSaveButton(panel)))
				{
					SkyCampaign::CampaignDefinition definition = BuildCampaignDefinition(data);

					if (SkyCampaign::SaveCampaignDefinition(definition))
					{
						data.editingCampaignId = definition.id;
						data.editorMessage = U"Saved: {}"_fmt(definition.displayName);
						data.editorMessageUntil = (Scene::Time() + 3.0);
						ReloadCampaigns(data);

						for (size_t i = 0; i < data.campaigns.size(); ++i)
						{
							if (data.campaigns[i].id == definition.id)
							{
								data.selectedCampaignIndex = i;
								break;
							}
						}
					}
					else
					{
						data.editorMessage = U"Save failed";
						data.editorMessageUntil = (Scene::Time() + 3.0);
					}
				}

				if (IsRectButtonClicked(GetBackButton(panel)) || KeyEscape.down())
				{
					ReloadCampaigns(data);
					changeScene(U"Title", 0);
					return;
				}

				if (Scene::Time() < data.editorMessageUntil)
				{
					m_labelFont(data.editorMessage).draw(panel.pos.movedBy(30, 618), ColorF{ 1.0, 0.95, 0.72, 0.96 });
				}
			}

			void draw() const override
			{
			}

		private:
			Font m_titleFont;
			Font m_labelFont;
			Font m_buttonFont;

			[[nodiscard]] RectF GetCampaignSectionRect(const RectF& panel) const
			{
				return RectF{ panel.x + 24, panel.y + 108, 316, 380 };
			}

			[[nodiscard]] RectF GetMissionSectionRect(const RectF& panel) const
			{
				return RectF{ panel.x + 360, panel.y + 108, 256, 520 };
			}

			[[nodiscard]] RectF GetDetailSectionRect(const RectF& panel) const
			{
				return RectF{ panel.x + 636, panel.y + 108, 480, 520 };
			}

			[[nodiscard]] RectF GetMissionListRect(const RectF& panel) const
			{
				const RectF missionSection = GetMissionSectionRect(panel);
				return RectF{ missionSection.x + 14, missionSection.y + 52, 188, 362 };
			}

			[[nodiscard]] RectF GetMissionRowRect(const int32 row) const
			{
				return RectF{ 447, (194 + row * 48), 172, 38 };
			}

			[[nodiscard]] RectF GetMissionAddButton(const RectF& panel) const
			{
				const RectF missionSection = GetMissionSectionRect(panel);
				return RectF{ missionSection.x + 14, missionSection.y + 430, 104, 38 };
			}

			[[nodiscard]] RectF GetMissionRemoveButton(const RectF& panel) const
			{
				const RectF missionSection = GetMissionSectionRect(panel);
				return RectF{ missionSection.x + 128, missionSection.y + 430, 114, 38 };
			}

			[[nodiscard]] RectF GetMissionUpButton(const RectF& panel) const
			{
				const RectF missionSection = GetMissionSectionRect(panel);
				return RectF{ missionSection.x + 14, missionSection.y + 476, 104, 38 };
			}

			[[nodiscard]] RectF GetMissionDownButton(const RectF& panel) const
			{
				const RectF missionSection = GetMissionSectionRect(panel);
				return RectF{ missionSection.x + 128, missionSection.y + 476, 114, 38 };
			}

			[[nodiscard]] RectF GetMissionScrollUpButton(const RectF& panel) const
			{
				const RectF missionListRect = GetMissionListRect(panel);
				return RectF{ missionListRect.rightX() + 10, missionListRect.y + 12, 30, 30 };
			}

			[[nodiscard]] RectF GetMissionScrollDownButton(const RectF& panel) const
			{
				const RectF missionListRect = GetMissionListRect(panel);
				return RectF{ missionListRect.rightX() + 10, missionListRect.y + 50, 30, 30 };
			}

			[[nodiscard]] RectF GetSaveButton(const RectF& panel) const
			{
				const RectF detailSection = GetDetailSectionRect(panel);
				return RectF{ detailSection.x + 238, detailSection.y + 430, 220, 52 };
			}

			[[nodiscard]] RectF GetMapEditButton(const RectF& panel) const
			{
				const RectF detailSection = GetDetailSectionRect(panel);
				return RectF{ detailSection.x + 18, detailSection.y + 430, 100, 42 };
			}

			[[nodiscard]] RectF GetTestPlayButton(const RectF& panel) const
			{
				const RectF detailSection = GetDetailSectionRect(panel);
				return RectF{ detailSection.x + 128, detailSection.y + 430, 100, 42 };
			}

			[[nodiscard]] RectF GetBackButton(const RectF& panel) const
			{
				const RectF detailSection = GetDetailSectionRect(panel);
               return RectF{ detailSection.x + 18, detailSection.y + 482, 332, 42 };
			}

			[[nodiscard]] RectF GetHintButton(const RectF& panel) const
			{
				const RectF detailSection = GetDetailSectionRect(panel);
				return RectF{ detailSection.x + 362, detailSection.y + 482, 96, 42 };
			}

			[[nodiscard]] RectF GetEditDescriptionButton(const RectF& panel) const
			{
				const RectF campaignSection = GetCampaignSectionRect(panel);
				return RectF{ campaignSection.x + 18, campaignSection.y + 314, 280, 42 };
			}

			[[nodiscard]] RectF GetEditDialogueButton(const RectF& panel) const
			{
				const RectF detailSection = GetDetailSectionRect(panel);
				return RectF{ detailSection.x + 18, detailSection.y + 380, 418, 42 };
			}

			[[nodiscard]] RectF GetBrowseMapPathButton(const RectF& panel) const
			{
				const RectF detailSection = GetDetailSectionRect(panel);
				return RectF{ detailSection.x + 352, detailSection.y + 208, 84, 42 };
			}
		};
	}

	void AddCampaignEditorScene(App& manager)
	{
		manager.add<SkyCampaignEditorScene>(U"CampaignEditor");
		AddCampaignDescriptionEditorScene(manager);
		AddMissionDialogueEditorScene(manager);
	}
}
