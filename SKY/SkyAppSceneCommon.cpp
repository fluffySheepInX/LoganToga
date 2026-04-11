# include "SkyAppInternal.hpp"
# include "MapData.hpp"

namespace SkyAppInternal
{
	void ReloadCampaigns(SkyAppData& data)
	{
		data.campaigns = SkyCampaign::LoadCampaignDefinitions();

		if (data.campaigns.isEmpty())
		{
			data.selectedCampaignIndex.reset();
			return;
		}

		if ((not data.selectedCampaignIndex) || (data.campaigns.size() <= *data.selectedCampaignIndex))
		{
			data.selectedCampaignIndex = 0;
		}
	}

	void PrepareNewCampaignDraft(SkyAppData& data)
	{
		data.campaignNameState = TextEditState{};
		data.campaignDescriptionState = TextEditState{};
		data.missionNameStates = { TextEditState{} };
		data.missionNameStates.front().text = U"Mission 1";
		data.missionMapPathStates = { TextEditState{} };
		data.missionMapPathStates.front().text = MainSupport::MapDataPath;
        data.missionPreDialogueStates = { TextEditState{} };
		data.missionPostDialogueStates = { TextEditState{} };
		data.selectedEditorMissionIndex = 0;
		data.editorMissionListOffset = 0;
		data.editorMessage.clear();
		data.editorMessageUntil = 0.0;
		data.editingCampaignId.reset();
       data.dialogueSceneTitle.clear();
		data.dialogueSceneLines.clear();
		data.dialogueSceneLineIndex = 0;
		data.dialogueNextScene = U"Title";
	}

	void LoadCampaignIntoEditor(SkyAppData& data, const SkyCampaign::CampaignDefinition& definition)
	{
		data.campaignNameState = TextEditState{};
		data.campaignNameState.text = definition.displayName;
		data.campaignDescriptionState = TextEditState{};
		data.campaignDescriptionState.text = definition.description;
		data.missionNameStates.clear();
		data.missionMapPathStates.clear();
		data.missionPreDialogueStates.clear();
		data.missionPostDialogueStates.clear();

		for (const auto& mission : definition.missions)
		{
			TextEditState missionNameState;
			missionNameState.text = mission.displayName;
			data.missionNameStates << missionNameState;

			TextEditState missionMapPathState;
			missionMapPathState.text = mission.mapFile;
			data.missionMapPathStates << missionMapPathState;

			TextEditState missionPreDialogueState;
			missionPreDialogueState.text = mission.preDialogueLines.join(U" | ");
			data.missionPreDialogueStates << missionPreDialogueState;

			TextEditState missionPostDialogueState;
			missionPostDialogueState.text = mission.postDialogueLines.join(U" | ");
			data.missionPostDialogueStates << missionPostDialogueState;
		}

		if (data.missionNameStates.isEmpty())
		{
			PrepareNewCampaignDraft(data);
		}
		else
		{
			data.selectedEditorMissionIndex = 0;
			data.editorMissionListOffset = 0;
			data.editorMessage.clear();
			data.editorMessageUntil = 0.0;
			data.editingCampaignId = definition.id;
		}
	}

	void ClampEditorMissionSelection(SkyAppData& data)
	{
		while (data.missionMapPathStates.size() < data.missionNameStates.size())
		{
			TextEditState state;
			state.text = MainSupport::MapDataPath;
			data.missionMapPathStates << state;
		}

		while (data.missionPreDialogueStates.size() < data.missionNameStates.size())
		{
			data.missionPreDialogueStates << TextEditState{};
		}

		while (data.missionPostDialogueStates.size() < data.missionNameStates.size())
		{
			data.missionPostDialogueStates << TextEditState{};
		}

		while (data.missionNameStates.size() < data.missionMapPathStates.size())
		{
			data.missionNameStates << TextEditState{};
		}

		if (data.missionNameStates.isEmpty())
		{
			PrepareNewCampaignDraft(data);
			return;
		}

		if ((not data.selectedEditorMissionIndex) || (data.missionNameStates.size() <= *data.selectedEditorMissionIndex))
		{
			data.selectedEditorMissionIndex = (data.missionNameStates.size() - 1);
		}

		const int32 selectedIndex = static_cast<int32>(*data.selectedEditorMissionIndex);
		const int32 maxOffset = Max(0, static_cast<int32>(data.missionNameStates.size()) - EditorVisibleMissionCount);
		data.editorMissionListOffset = Clamp(data.editorMissionListOffset, 0, maxOffset);

		if (selectedIndex < data.editorMissionListOffset)
		{
			data.editorMissionListOffset = selectedIndex;
		}
		else if ((data.editorMissionListOffset + EditorVisibleMissionCount) <= selectedIndex)
		{
			data.editorMissionListOffset = Max(0, (selectedIndex - EditorVisibleMissionCount + 1));
		}
	}

	void AddEditorMission(SkyAppData& data)
	{
		TextEditState missionNameState;
		missionNameState.text = U"Mission {}"_fmt(data.missionNameStates.size() + 1);
		TextEditState missionMapPathState;
		missionMapPathState.text = MainSupport::MapDataPath;
		data.missionNameStates << missionNameState;
		data.missionMapPathStates << missionMapPathState;
      data.missionPreDialogueStates << TextEditState{};
		data.missionPostDialogueStates << TextEditState{};
		data.selectedEditorMissionIndex = (data.missionNameStates.size() - 1);
		ClampEditorMissionSelection(data);
	}

	void RemoveSelectedEditorMission(SkyAppData& data)
	{
		ClampEditorMissionSelection(data);

		if (data.missionNameStates.size() <= 1)
		{
			data.missionNameStates.front().text = U"Mission 1";
			data.missionMapPathStates.front().text = MainSupport::MapDataPath;
            data.missionPreDialogueStates.front().clear();
			data.missionPostDialogueStates.front().clear();
			data.selectedEditorMissionIndex = 0;
			return;
		}

		const size_t index = *data.selectedEditorMissionIndex;
		data.missionNameStates.erase(data.missionNameStates.begin() + index);
		data.missionMapPathStates.erase(data.missionMapPathStates.begin() + index);
		data.missionPreDialogueStates.erase(data.missionPreDialogueStates.begin() + index);
		data.missionPostDialogueStates.erase(data.missionPostDialogueStates.begin() + index);

		if (data.missionNameStates.size() <= index)
		{
			data.selectedEditorMissionIndex = (data.missionNameStates.size() - 1);
		}

		ClampEditorMissionSelection(data);
	}

	void MoveSelectedEditorMission(SkyAppData& data, const int32 direction)
	{
		ClampEditorMissionSelection(data);
		const int32 index = static_cast<int32>(*data.selectedEditorMissionIndex);
		const int32 targetIndex = Clamp((index + direction), 0, static_cast<int32>(data.missionNameStates.size()) - 1);

		if (index == targetIndex)
		{
			return;
		}

		std::swap(data.missionNameStates[index], data.missionNameStates[targetIndex]);
		std::swap(data.missionMapPathStates[index], data.missionMapPathStates[targetIndex]);
     std::swap(data.missionPreDialogueStates[index], data.missionPreDialogueStates[targetIndex]);
		std::swap(data.missionPostDialogueStates[index], data.missionPostDialogueStates[targetIndex]);
		data.selectedEditorMissionIndex = static_cast<size_t>(targetIndex);
		ClampEditorMissionSelection(data);
	}

	Array<String> SplitDialogueText(const StringView text)
	{
		Array<String> lines;
		for (const auto& part : String{ text }.split(U'|'))
		{
			const String trimmed = part.trimmed();
			if (not trimmed.isEmpty())
			{
				lines << trimmed;
			}
		}

		return lines;
	}

	void PrepareDialogueScene(SkyAppData& data, const StringView title, const Array<String>& lines, const StringView nextScene)
	{
		data.dialogueSceneTitle = title;
		data.dialogueSceneLines = lines;
		data.dialogueSceneLineIndex = 0;
		data.dialogueNextScene = nextScene;
	}

	SkyCampaign::CampaignDefinition BuildCampaignDefinition(const SkyAppData& data)
	{
		SkyCampaign::CampaignDefinition definition;
		definition.id = data.editingCampaignId.value_or(U"");
		definition.displayName = data.campaignNameState.text.trimmed();
		definition.description = data.campaignDescriptionState.text.trimmed();
		definition.missions.clear();

		for (size_t i = 0; i < data.missionNameStates.size(); ++i)
		{
			const String missionName = data.missionNameStates[i].text.trimmed();
			const String mapFile = data.missionMapPathStates[i].text.trimmed();
			definition.missions << SkyCampaign::CampaignMissionDefinition{
				.displayName = missionName.isEmpty() ? U"Mission {}"_fmt(i + 1) : missionName,
				.mapFile = mapFile.isEmpty() ? FilePath{ MainSupport::MapDataPath } : FilePath{ mapFile },
              .preDialogueLines = SplitDialogueText(data.missionPreDialogueStates[i].text),
				.postDialogueLines = SplitDialogueText(data.missionPostDialogueStates[i].text),
			};
		}

		if (definition.missions.isEmpty())
		{
			definition.missions << SkyCampaign::CampaignMissionDefinition{};
		}

		return definition;
	}

	const SkyCampaign::CampaignDefinition* FindSelectedCampaign(const SkyAppData& data)
	{
		if (data.selectedCampaignIndex && (*data.selectedCampaignIndex < data.campaigns.size()))
		{
			return &data.campaigns[*data.selectedCampaignIndex];
		}

		return nullptr;
	}

	const SkyCampaign::CampaignDefinition* FindCampaignById(const SkyAppData& data, const StringView campaignId)
	{
		for (const auto& campaign : data.campaigns)
		{
			if (campaign.id == campaignId)
			{
				return &campaign;
			}
		}

		return nullptr;
	}

	bool CanContinueSelectedCampaign(const SkyAppData& data)
	{
		const SkyCampaign::CampaignDefinition* campaign = FindSelectedCampaign(data);
		if (not campaign)
		{
			return false;
		}

		if (not SkyCampaign::HasCampaignProgress(campaign->id))
		{
			return false;
		}

		const SkyCampaign::CampaignProgress progress = SkyCampaign::LoadCampaignProgress(campaign->id);
		return (not progress.completed) && (progress.currentMissionIndex < campaign->missions.size());
	}

	bool StartSelectedCampaign(SkyAppData& data, const bool continueFromProgress)
	{
		const SkyCampaign::CampaignDefinition* campaign = FindSelectedCampaign(data);
		if (not campaign)
		{
			return false;
		}

		size_t missionIndex = 0;
		if (continueFromProgress)
		{
			const SkyCampaign::CampaignProgress progress = SkyCampaign::LoadCampaignProgress(campaign->id);
			if (progress.completed)
			{
				return false;
			}

			missionIndex = Min(progress.currentMissionIndex, (campaign->missions.size() - 1));
		}

		data.activeCampaignId = campaign->id;
		data.activeCampaignMissionIndex = missionIndex;
		data.battleReturnScene = U"Title";
		data.launchBattleInMapEditor = false;
		data.pendingBattleMapPath = campaign->missions[missionIndex].mapFile;
		data.dialogueSceneTitle.clear();
		data.dialogueSceneLines.clear();
		data.dialogueSceneLineIndex = 0;
		data.dialogueNextScene = U"Title";

		if (not campaign->missions[missionIndex].preDialogueLines.isEmpty())
		{
			PrepareDialogueScene(data, campaign->missions[missionIndex].displayName, campaign->missions[missionIndex].preDialogueLines, U"Battle");
		}
		return true;
	}

	FilePath EnsureEditorMissionMapPath(SkyAppData& data)
	{
		ClampEditorMissionSelection(data);
		const size_t missionIndex = *data.selectedEditorMissionIndex;
		String path = data.missionMapPathStates[missionIndex].text.trimmed();

		if (path.isEmpty() || (path == MainSupport::MapDataPath))
		{
			FileSystem::CreateDirectories(CampaignMapDirectoryPath);
			const String campaignKey = data.editingCampaignId.value_or(SkyCampaign::Detail::SanitizeCampaignId(data.campaignNameState.text));
			path = FileSystem::PathAppend(CampaignMapDirectoryPath, U"{}_mission_{}.toml"_fmt(campaignKey.isEmpty() ? U"campaign" : campaignKey, missionIndex + 1));
			data.missionMapPathStates[missionIndex].text = path;
		}

		if (not FileSystem::Exists(path))
		{
			SaveMapData(MakeDefaultMapData(), path);
		}

		return FilePath{ path };
	}

	bool IsRectButtonClicked(const RectF& rect, const bool enabled)
	{
		return enabled && rect.leftClicked();
	}

	void DrawRectButton(const RectF& rect, const Font& font, const StringView label, const bool enabled)
	{
		const bool hovered = enabled && rect.mouseOver();
		ColorF fillColor{ 0.16, 0.25, 0.40, 1.0 };
		ColorF frameColor{ 0.78, 0.88, 1.0, 0.88 };
		ColorF textColor{ 1.0, 1.0, 1.0, 1.0 };

		if (not enabled)
		{
			fillColor = ColorF{ 0.14, 0.16, 0.20, 0.72 };
			frameColor = ColorF{ 0.46, 0.52, 0.60, 0.72 };
			textColor = ColorF{ 0.72, 0.76, 0.82, 1.0 };
		}
		else if (hovered)
		{
			fillColor = ColorF{ 0.22, 0.36, 0.56, 1.0 };
		}

		rect.rounded(16).draw(fillColor);
		rect.rounded(16).drawFrame(2, 0, frameColor);
		font(label).drawAt(rect.center(), textColor);
	}
}
