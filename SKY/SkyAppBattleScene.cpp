# include "SkyAppInternal.hpp"
# include "SkyAppLoop.hpp"
# include "SkyAppLoopInternal.hpp"

namespace SkyAppInternal
{
	namespace
	{
		class SkyBattleScene : public App::Scene
		{
		public:
			explicit SkyBattleScene(const InitData& init)
				: App::Scene{ init }
			{
				SkyAppFlow::InitializeSkyAppState(m_state);
				auto& data = getData();

				const FilePath battleMapPath = data.pendingBattleMapPath;
				m_state.currentMapPath = battleMapPath.isEmpty() ? FilePath{ MainSupport::MapDataPath } : battleMapPath;
				if ((not battleMapPath.isEmpty()) && (battleMapPath != MainSupport::MapDataPath))
				{
					const MapDataLoadResult campaignMapLoad = LoadMapDataWithStatus(battleMapPath);
					m_state.mapData = campaignMapLoad.mapData;
					SkyAppFlow::ResetMatch(m_state);

					if (not campaignMapLoad.message.isEmpty())
					{
						m_state.mapDataMessage.show(campaignMapLoad.message, 4.0);
					}
				}

				if (data.activeCampaignId)
				{
					SkyCampaign::CampaignProgress progress = SkyCampaign::LoadCampaignProgress(*data.activeCampaignId);
					progress.campaignId = *data.activeCampaignId;
					progress.currentMissionIndex = data.activeCampaignMissionIndex.value_or(0);
					progress.unlockedMissionCount = Max(progress.unlockedMissionCount, (progress.currentMissionIndex + 1));
					progress.completed = false;
					SkyCampaign::SaveCampaignProgress(progress);
				}

				if (data.launchBattleInMapEditor)
				{
					m_state.appMode = MainSupport::AppMode::EditMap;
				}
			}

			void update() override
			{
				SkyAppFlow::RunSkyAppFrame(m_resources, m_state);
				auto& data = getData();

				if ((not m_campaignResultHandled) && data.activeCampaignId && m_state.playerWon)
				{
					if (const auto* campaign = FindCampaignById(data, *data.activeCampaignId))
					{
						SkyCampaign::CampaignProgress progress = SkyCampaign::LoadCampaignProgress(*data.activeCampaignId);
						progress.campaignId = *data.activeCampaignId;

						if (*m_state.playerWon)
						{
							const size_t nextMissionIndex = Min((data.activeCampaignMissionIndex.value_or(0) + 1), campaign->missions.size());
							progress.unlockedMissionCount = Max(progress.unlockedMissionCount, nextMissionIndex + ((nextMissionIndex < campaign->missions.size()) ? 1 : 0));

							if (nextMissionIndex < campaign->missions.size())
							{
								progress.currentMissionIndex = nextMissionIndex;
								progress.completed = false;
							}
							else
							{
								progress.currentMissionIndex = (campaign->missions.size() - 1);
                              ++progress.clearCount;
								progress.completed = true;
							}
						}
						else
						{
							progress.currentMissionIndex = Min(data.activeCampaignMissionIndex.value_or(0), (campaign->missions.size() - 1));
						}

						SkyCampaign::SaveCampaignProgress(progress);
					}

					m_campaignResultHandled = true;
				}

				if (data.activeCampaignId && m_state.playerWon && (KeyEnter.down() || MouseL.down()))
				{
					if (const auto* campaign = FindCampaignById(data, *data.activeCampaignId))
					{
						const SkyCampaign::CampaignProgress progress = SkyCampaign::LoadCampaignProgress(*data.activeCampaignId);
						const size_t currentMissionIndex = Min(data.activeCampaignMissionIndex.value_or(0), (campaign->missions.size() - 1));
						Array<String> dialogueLines = campaign->missions[currentMissionIndex].postDialogueLines;

						if ((not progress.completed) && (progress.currentMissionIndex < campaign->missions.size()))
						{
                         dialogueLines.append(campaign->missions[progress.currentMissionIndex].preDialogueLines);
							data.activeCampaignMissionIndex = progress.currentMissionIndex;
							data.pendingBattleMapPath = campaign->missions[progress.currentMissionIndex].mapFile;

							if (not dialogueLines.isEmpty())
							{
								PrepareDialogueScene(data, campaign->missions[currentMissionIndex].displayName, dialogueLines, U"Battle");
								changeScene(U"Dialogue", 0);
								return;
							}

							changeScene(U"Battle", 0);
							return;
						}

						if (not dialogueLines.isEmpty())
						{
							PrepareDialogueScene(data, campaign->missions[currentMissionIndex].displayName, dialogueLines, U"Title");
							changeScene(U"Dialogue", 0);
							return;
						}
					}

					data.activeCampaignId.reset();
					data.activeCampaignMissionIndex.reset();
					changeScene(U"Title", 0);
					return;
				}

				if (m_state.requestTitleScene)
				{
					m_state.requestTitleScene = false;
					if (not data.activeCampaignId)
					{
						data.activeCampaignMissionIndex.reset();
					}
					const String returnScene = data.battleReturnScene;
					data.launchBattleInMapEditor = false;
					data.battleReturnScene = U"Title";
					changeScene(returnScene, 0);
				}
			}

		private:
			SkyAppFlow::SkyAppResources m_resources;
			SkyAppFlow::SkyAppState m_state;
			bool m_campaignResultHandled = false;
		};
	}

	void AddBattleScene(App& manager)
	{
		manager.add<SkyBattleScene>(U"Battle");
	}
}
