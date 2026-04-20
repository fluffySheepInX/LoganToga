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
			}

			void update() override
			{
                   if (m_loadPhase != LoadPhase::Ready)
					{
						updateLoading();
						return;
					}

					SkyAppFlow::RunSkyAppFrame(*m_resources, m_state);
				auto& data = getData();

				if ((not m_campaignResultHandled) && data.activeCampaignId && m_state.battle.playerWon)
				{
					if (const auto* campaign = FindCampaignById(data, *data.activeCampaignId))
					{
						SkyCampaign::CampaignProgress progress = SkyCampaign::LoadCampaignProgress(*data.activeCampaignId);
						progress.campaignId = *data.activeCampaignId;

						if (*m_state.battle.playerWon)
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

				if (m_state.battle.playerWon && (not *m_state.battle.playerWon) && KeyR.down())
				{
					SkyAppFlow::ResetMatch(m_state);
					m_campaignResultHandled = false;
					m_state.messages[SkyAppSupport::MessageChannel::Restart].show(U"試合をリスタート");
					return;
				}

				if (data.activeCampaignId && m_state.battle.playerWon && (KeyEnter.down() || MouseL.down()))
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

				if ((not data.activeCampaignId) && m_state.battle.playerWon && (KeyEnter.down() || MouseL.down()))
				{
					const String returnScene = data.battleReturnScene;
					data.launchBattleInMapEditor = false;
					data.battleReturnScene = U"Title";
					changeScene(returnScene, 0);
					return;
				}

				if (m_state.hud.requestTitleScene)
				{
					m_state.hud.requestTitleScene = false;
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

			void draw() const override
			{
                  if (m_loadPhase != LoadPhase::Ready)
					{
						Scene::Rect().draw(ColorF{ 0.03, 0.05, 0.09 });
						const RectF loadingRect{ Arg::center = Scene::CenterF(), 420, 120 };
						loadingRect.rounded(18).draw(ColorF{ 0.04, 0.06, 0.10, 0.82 });
						loadingRect.rounded(18).drawFrame(2, 0, ColorF{ 0.78, 0.84, 0.94, 0.54 });
						SimpleGUI::GetFont()(GetLoadingMessage()).drawAt(loadingRect.center().movedBy(0, -12), Palette::White);
						SimpleGUI::GetFont()(U"Please wait...").drawAt(loadingRect.center().movedBy(0, 20), ColorF{ 0.82, 0.88, 0.96 });
						return;
					}

				if (not m_state.battle.playerWon)
				{
					return;
				}

				const auto& data = getData();
				Array<String> hintLines;

				if (*m_state.battle.playerWon)
				{
					if (data.activeCampaignId)
					{
						if (const auto* campaign = FindCampaignById(data, *data.activeCampaignId))
						{
							const size_t currentMissionIndex = Min(data.activeCampaignMissionIndex.value_or(0), (campaign->missions.size() - 1));
							const size_t nextMissionIndex = (currentMissionIndex + 1);
							if (nextMissionIndex < campaign->missions.size())
							{
								hintLines << U"Enter / Click: Next Mission";
								hintLines << U"Next: {}"_fmt(campaign->missions[nextMissionIndex].displayName);
							}
							else
							{
								hintLines << U"Enter / Click: Return to Title";
								hintLines << U"Campaign Complete";
							}
						}
					}
					else
					{
						hintLines << U"Enter / Click: Return to {}"_fmt(data.battleReturnScene);
					}
				}
				else
				{
					if (data.activeCampaignId)
					{
						if (const auto* campaign = FindCampaignById(data, *data.activeCampaignId))
						{
							const size_t missionIndex = Min(data.activeCampaignMissionIndex.value_or(0), (campaign->missions.size() - 1));
							hintLines << U"Enter / Click: Retry Mission";
							hintLines << U"Mission: {}"_fmt(campaign->missions[missionIndex].displayName);
						}
					}
					else
					{
						hintLines << U"Enter / Click: Return to {}"_fmt(data.battleReturnScene);
					}

					hintLines << U"R: Quick Retry";
				}

				const RectF infoRect{ Arg::center = Scene::CenterF().movedBy(0, 122), 420, (48 + hintLines.size() * 28) };
				infoRect.rounded(18).draw(ColorF{ 0.04, 0.06, 0.10, 0.74 });
				infoRect.rounded(18).drawFrame(2, 0, ColorF{ 0.78, 0.84, 0.94, 0.54 });

				for (size_t i = 0; i < hintLines.size(); ++i)
				{
					SimpleGUI::GetFont()(hintLines[i]).drawAt(infoRect.center().movedBy(0, (-14 + static_cast<double>(i) * 26.0)), Palette::White);
				}
			}

		private:
                enum class LoadPhase
				{
					InitializeState,
					LoadBattleMap,
					UpdateCampaignProgress,
					CreateResources,
					Ready,
				};

				void updateLoading()
				{
					auto& data = getData();

					switch (m_loadPhase)
					{
					case LoadPhase::InitializeState:
						SkyAppFlow::InitializeSkyAppState(m_state);
						if (data.launchBattleInMapEditor)
						{
							m_state.env.appMode = MainSupport::AppMode::EditMap;
						}
						m_loadPhase = LoadPhase::LoadBattleMap;
						return;

					case LoadPhase::LoadBattleMap:
					{
						const FilePath battleMapPath = data.pendingBattleMapPath;
						m_state.world.currentMapPath = battleMapPath.isEmpty() ? FilePath{ MainSupport::MapDataPath } : battleMapPath;
						if ((not battleMapPath.isEmpty()) && (battleMapPath != MainSupport::MapDataPath))
						{
							const MapDataLoadResult campaignMapLoad = LoadMapDataWithStatus(battleMapPath);
							m_state.world.mapData = campaignMapLoad.mapData;
							SkyAppFlow::ResetMatch(m_state);

							if (not campaignMapLoad.message.isEmpty())
							{
								m_state.messages[SkyAppSupport::MessageChannel::MapData].show(campaignMapLoad.message, 4.0);
							}
						}

						m_loadPhase = LoadPhase::UpdateCampaignProgress;
						return;
					}

					case LoadPhase::UpdateCampaignProgress:
						if (data.activeCampaignId)
						{
							SkyCampaign::CampaignProgress progress = SkyCampaign::LoadCampaignProgress(*data.activeCampaignId);
							progress.campaignId = *data.activeCampaignId;
							progress.currentMissionIndex = data.activeCampaignMissionIndex.value_or(0);
							progress.unlockedMissionCount = Max(progress.unlockedMissionCount, (progress.currentMissionIndex + 1));
							progress.completed = false;
							SkyCampaign::SaveCampaignProgress(progress);
						}
						m_loadPhase = LoadPhase::CreateResources;
						return;

					case LoadPhase::CreateResources:
						m_resources.emplace();
						m_loadPhase = LoadPhase::Ready;
						return;

					case LoadPhase::Ready:
					default:
						return;
					}
				}

				[[nodiscard]] StringView GetLoadingMessage() const
				{
					switch (m_loadPhase)
					{
					case LoadPhase::InitializeState:
						return U"Initializing battle state";

					case LoadPhase::LoadBattleMap:
						return U"Loading battle map";

					case LoadPhase::UpdateCampaignProgress:
						return U"Updating campaign progress";

					case LoadPhase::CreateResources:
						return U"Loading battle resources";

					case LoadPhase::Ready:
					default:
						return U"Ready";
					}
				}

				Optional<SkyAppFlow::SkyAppResources> m_resources;
			SkyAppFlow::SkyAppState m_state;
               LoadPhase m_loadPhase = LoadPhase::InitializeState;
			bool m_campaignResultHandled = false;
		};
	}

	void AddBattleScene(App& manager)
	{
		manager.add<SkyBattleScene>(U"Battle");
	}
}
