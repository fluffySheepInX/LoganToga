# include "SkyAppInternal.hpp"

namespace SkyAppInternal
{
	namespace
	{
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
				ClampEditorMissionSelection(data);
				Scene::Rect().draw(ColorF{ 0.05, 0.09, 0.15 });
				const RectF panel{ 96, 72, 1088, 576 };
				panel.rounded(24).draw(ColorF{ 0.08, 0.13, 0.21, 0.96 });
				panel.rounded(24).drawFrame(2, 0, ColorF{ 0.56, 0.72, 0.92, 0.72 });
				const size_t selectedMissionIndex = *data.selectedEditorMissionIndex;
				const int32 maxOffset = Max(0, static_cast<int32>(data.missionNameStates.size()) - EditorVisibleMissionCount);

				m_titleFont(U"Campaign Editor").draw(panel.pos.movedBy(28, 24), Palette::White);
				m_labelFont(U"Step 2: 複数 Mission 編集").draw(panel.pos.movedBy(30, 72), ColorF{ 0.84, 0.90, 0.98, 0.92 });

				m_labelFont(U"Campaign Name").draw(panel.pos.movedBy(30, 132), Palette::White);
				SimpleGUI::TextBox(data.campaignNameState, panel.pos.movedBy(30, 160), 340, 48);

				m_labelFont(U"Description").draw(panel.pos.movedBy(30, 220), Palette::White);
				SimpleGUI::TextBox(data.campaignDescriptionState, panel.pos.movedBy(30, 248), 500, 80);

				m_labelFont(U"Missions").draw(panel.pos.movedBy(30, 316), Palette::White);
				RectF{ 30 + panel.x, 344 + panel.y, 352, 244 }.draw(ColorF{ 0.07, 0.11, 0.18, 0.92 });
				RectF{ 30 + panel.x, 344 + panel.y, 352, 244 }.drawFrame(1, 0, ColorF{ 0.42, 0.52, 0.64, 0.82 });

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

				DrawRectButton(GetMissionAddButton(), m_buttonFont, U"Add");
				DrawRectButton(GetMissionRemoveButton(), m_buttonFont, U"Remove", (1 < data.missionNameStates.size()));
				DrawRectButton(GetMissionUpButton(), m_buttonFont, U"Up", (0 < selectedMissionIndex));
				DrawRectButton(GetMissionDownButton(), m_buttonFont, U"Down", ((selectedMissionIndex + 1) < data.missionNameStates.size()));
				DrawRectButton(GetMissionScrollUpButton(), m_buttonFont, U"▲", (0 < data.editorMissionListOffset));
				DrawRectButton(GetMissionScrollDownButton(), m_buttonFont, U"▼", (data.editorMissionListOffset < maxOffset));

				if (IsRectButtonClicked(GetMissionAddButton()))
				{
					AddEditorMission(data);
				}

				if (IsRectButtonClicked(GetMissionRemoveButton(), (1 < data.missionNameStates.size())))
				{
					RemoveSelectedEditorMission(data);
				}

				if (IsRectButtonClicked(GetMissionUpButton(), (0 < selectedMissionIndex)))
				{
					MoveSelectedEditorMission(data, -1);
				}

				if (IsRectButtonClicked(GetMissionDownButton(), ((selectedMissionIndex + 1) < data.missionNameStates.size())))
				{
					MoveSelectedEditorMission(data, 1);
				}

				if (IsRectButtonClicked(GetMissionScrollUpButton(), (0 < data.editorMissionListOffset)))
				{
					--data.editorMissionListOffset;
				}

				if (IsRectButtonClicked(GetMissionScrollDownButton(), (data.editorMissionListOffset < maxOffset)))
				{
					++data.editorMissionListOffset;
				}

				ClampEditorMissionSelection(data);
				const size_t currentMissionIndex = *data.selectedEditorMissionIndex;

				m_labelFont(U"Selected Mission: {} / {}"_fmt(currentMissionIndex + 1, data.missionNameStates.size())).draw(panel.pos.movedBy(430, 132), Palette::White);
				m_labelFont(U"Mission Name").draw(panel.pos.movedBy(430, 172), Palette::White);
				SimpleGUI::TextBox(data.missionNameStates[currentMissionIndex], panel.pos.movedBy(430, 200), 360, 48);

				m_labelFont(U"Map Path").draw(panel.pos.movedBy(430, 264), Palette::White);
				SimpleGUI::TextBox(data.missionMapPathStates[currentMissionIndex], panel.pos.movedBy(430, 292), 520, 80);

				m_labelFont(U"Mission は Save 時に配列として TOML へ保存されます").draw(panel.pos.movedBy(430, 384), ColorF{ 0.82, 0.89, 0.98, 0.90 });
				m_labelFont(U"Map Edit でその Mission 専用マップへ直接移動できます").draw(panel.pos.movedBy(430, 414), ColorF{ 0.82, 0.89, 0.98, 0.90 });

				DrawRectButton(GetMapEditButton(), m_buttonFont, U"Map Edit");
				DrawRectButton(GetTestPlayButton(), m_buttonFont, U"Test Play");
				DrawRectButton(GetSaveButton(), m_buttonFont, U"Save");
				DrawRectButton(GetBackButton(), m_buttonFont, U"Back to Title");

				if (IsRectButtonClicked(GetMapEditButton()))
				{
					data.pendingBattleMapPath = EnsureEditorMissionMapPath(data);
					data.activeCampaignId.reset();
					data.activeCampaignMissionIndex.reset();
					data.battleReturnScene = U"CampaignEditor";
					data.launchBattleInMapEditor = true;
					changeScene(U"Battle", 0);
					return;
				}

				if (IsRectButtonClicked(GetTestPlayButton()))
				{
					data.pendingBattleMapPath = EnsureEditorMissionMapPath(data);
					data.activeCampaignId.reset();
					data.activeCampaignMissionIndex.reset();
					data.battleReturnScene = U"CampaignEditor";
					data.launchBattleInMapEditor = false;
					changeScene(U"Battle", 0);
					return;
				}

				if (IsRectButtonClicked(GetSaveButton()))
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

				if (IsRectButtonClicked(GetBackButton()) || KeyEscape.down())
				{
					ReloadCampaigns(data);
					changeScene(U"Title", 0);
					return;
				}

				if (Scene::Time() < data.editorMessageUntil)
				{
					m_labelFont(data.editorMessage).draw(panel.pos.movedBy(30, 520), ColorF{ 1.0, 0.95, 0.72, 0.96 });
				}
			}

			void draw() const override
			{
			}

		private:
			Font m_titleFont;
			Font m_labelFont;
			Font m_buttonFont;

			[[nodiscard]] RectF GetMissionRowRect(const int32 row) const
			{
				return RectF{ 134, (430 + row * 38), 276, 32 };
			}

			[[nodiscard]] RectF GetMissionAddButton() const
			{
				return RectF{ 134, 598, 88, 36 };
			}

			[[nodiscard]] RectF GetMissionRemoveButton() const
			{
				return RectF{ 230, 598, 108, 36 };
			}

			[[nodiscard]] RectF GetMissionUpButton() const
			{
				return RectF{ 346, 598, 64, 36 };
			}

			[[nodiscard]] RectF GetMissionDownButton() const
			{
				return RectF{ 418, 598, 74, 36 };
			}

			[[nodiscard]] RectF GetMissionScrollUpButton() const
			{
				return RectF{ 454, 430, 38, 32 };
			}

			[[nodiscard]] RectF GetMissionScrollDownButton() const
			{
				return RectF{ 454, 468, 38, 32 };
			}

			[[nodiscard]] RectF GetSaveButton() const
			{
				return RectF{ 860, 432, 260, 52 };
			}

			[[nodiscard]] RectF GetMapEditButton() const
			{
				return RectF{ 860, 500, 124, 52 };
			}

			[[nodiscard]] RectF GetTestPlayButton() const
			{
				return RectF{ 996, 500, 124, 52 };
			}

			[[nodiscard]] RectF GetBackButton() const
			{
				return RectF{ 860, 568, 260, 52 };
			}
		};
	}

	void AddCampaignEditorScene(App& manager)
	{
		manager.add<SkyCampaignEditorScene>(U"CampaignEditor");
	}
}
