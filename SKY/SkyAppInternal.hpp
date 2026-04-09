# pragma once
# include <Siv3D.hpp>
# include "CampaignData.hpp"

namespace SkyAppInternal
{
	struct SkyAppData
	{
		Array<SkyCampaign::CampaignDefinition> campaigns;
		Optional<size_t> selectedCampaignIndex;
		Optional<String> activeCampaignId;
		Optional<size_t> activeCampaignMissionIndex;
		String battleReturnScene = U"Title";
		bool launchBattleInMapEditor = false;
		String titleMessage;
		double titleMessageUntil = 0.0;
		TextEditState campaignNameState;
		TextEditState campaignDescriptionState;
		Array<TextEditState> missionNameStates;
		Array<TextEditState> missionMapPathStates;
		Optional<size_t> selectedEditorMissionIndex;
		int32 editorMissionListOffset = 0;
		String editorMessage;
		double editorMessageUntil = 0.0;
		FilePath pendingBattleMapPath = FilePath{ MainSupport::MapDataPath };
		Optional<String> editingCampaignId;
	};

	using App = SceneManager<String, SkyAppData>;

	inline constexpr int32 EditorVisibleMissionCount = 6;
	inline constexpr FilePathView CampaignMapDirectoryPath = U"settings/campaign_maps";

	void ReloadCampaigns(SkyAppData& data);
	void PrepareNewCampaignDraft(SkyAppData& data);
	void LoadCampaignIntoEditor(SkyAppData& data, const SkyCampaign::CampaignDefinition& definition);
	void ClampEditorMissionSelection(SkyAppData& data);
	void AddEditorMission(SkyAppData& data);
	void RemoveSelectedEditorMission(SkyAppData& data);
	void MoveSelectedEditorMission(SkyAppData& data, int32 direction);
	[[nodiscard]] SkyCampaign::CampaignDefinition BuildCampaignDefinition(const SkyAppData& data);
	[[nodiscard]] const SkyCampaign::CampaignDefinition* FindSelectedCampaign(const SkyAppData& data);
	[[nodiscard]] const SkyCampaign::CampaignDefinition* FindCampaignById(const SkyAppData& data, StringView campaignId);
	[[nodiscard]] bool CanContinueSelectedCampaign(const SkyAppData& data);
	[[nodiscard]] bool StartSelectedCampaign(SkyAppData& data, bool continueFromProgress);
	[[nodiscard]] FilePath EnsureEditorMissionMapPath(SkyAppData& data);
	[[nodiscard]] bool IsRectButtonClicked(const RectF& rect, bool enabled = true);
	void DrawRectButton(const RectF& rect, const Font& font, StringView label, bool enabled = true);

	void AddTitleScene(App& manager);
	void AddCampaignEditorScene(App& manager);
	void AddBattleScene(App& manager);
}
