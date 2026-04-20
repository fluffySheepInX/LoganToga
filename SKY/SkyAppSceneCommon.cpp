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
		data.missionDrafts = { MissionEditDraft{} };
		data.missionDrafts.front().name.text = U"Mission 1";
		data.missionDrafts.front().mapPath.text = MainSupport::MapDataPath;
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
		data.missionDrafts.clear();

		for (const auto& mission : definition.missions)
		{
			MissionEditDraft draft;
			draft.name.text = mission.displayName;
			draft.mapPath.text = mission.mapFile;
			draft.preDialogue.text = mission.preDialogueLines.join(U" | ");
			draft.postDialogue.text = mission.postDialogueLines.join(U" | ");
			data.missionDrafts << draft;
		}

		if (data.missionDrafts.isEmpty())
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
		if (data.missionDrafts.isEmpty())
		{
			PrepareNewCampaignDraft(data);
			return;
		}

		if ((not data.selectedEditorMissionIndex) || (data.missionDrafts.size() <= *data.selectedEditorMissionIndex))
		{
			data.selectedEditorMissionIndex = (data.missionDrafts.size() - 1);
		}

		const int32 selectedIndex = static_cast<int32>(*data.selectedEditorMissionIndex);
		const int32 maxOffset = Max(0, static_cast<int32>(data.missionDrafts.size()) - EditorVisibleMissionCount);
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
		MissionEditDraft draft;
		draft.name.text = U"Mission {}"_fmt(data.missionDrafts.size() + 1);
		draft.mapPath.text = MainSupport::MapDataPath;
		data.missionDrafts << draft;
		data.selectedEditorMissionIndex = (data.missionDrafts.size() - 1);
		ClampEditorMissionSelection(data);
	}

	void DuplicateSelectedEditorMission(SkyAppData& data)
	{
		ClampEditorMissionSelection(data);
		const size_t index = *data.selectedEditorMissionIndex;
		const size_t insertIndex = (index + 1);

		MissionEditDraft draft = data.missionDrafts[index];
		if (draft.name.text.trimmed().isEmpty())
		{
			draft.name.text = U"Mission {}"_fmt(insertIndex + 1);
		}

		data.missionDrafts.insert(data.missionDrafts.begin() + insertIndex, draft);
		data.selectedEditorMissionIndex = insertIndex;
		ClampEditorMissionSelection(data);
	}

	void RemoveSelectedEditorMission(SkyAppData& data)
	{
		ClampEditorMissionSelection(data);

		if (data.missionDrafts.size() <= 1)
		{
			auto& draft = data.missionDrafts.front();
			draft.name.text = U"Mission 1";
			draft.mapPath.text = MainSupport::MapDataPath;
			draft.preDialogue.clear();
			draft.postDialogue.clear();
			data.selectedEditorMissionIndex = 0;
			return;
		}

		const size_t index = *data.selectedEditorMissionIndex;
		data.missionDrafts.erase(data.missionDrafts.begin() + index);

		if (data.missionDrafts.size() <= index)
		{
			data.selectedEditorMissionIndex = (data.missionDrafts.size() - 1);
		}

		ClampEditorMissionSelection(data);
	}

	void MoveSelectedEditorMission(SkyAppData& data, const int32 direction)
	{
		ClampEditorMissionSelection(data);
		const int32 index = static_cast<int32>(*data.selectedEditorMissionIndex);
		const int32 targetIndex = Clamp((index + direction), 0, static_cast<int32>(data.missionDrafts.size()) - 1);

		if (index == targetIndex)
		{
			return;
		}

		std::swap(data.missionDrafts[index], data.missionDrafts[targetIndex]);
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

		for (size_t i = 0; i < data.missionDrafts.size(); ++i)
		{
			const auto& draft = data.missionDrafts[i];
			const String missionName = draft.name.text.trimmed();
			const String mapFile = draft.mapPath.text.trimmed();
			definition.missions << SkyCampaign::CampaignMissionDefinition{
				.displayName = missionName.isEmpty() ? U"Mission {}"_fmt(i + 1) : missionName,
				.mapFile = mapFile.isEmpty() ? FilePath{ MainSupport::MapDataPath } : FilePath{ mapFile },
				.preDialogueLines = SplitDialogueText(draft.preDialogue.text),
				.postDialogueLines = SplitDialogueText(draft.postDialogue.text),
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
		String path = data.missionDrafts[missionIndex].mapPath.text.trimmed();

		if (path.isEmpty() || (path == MainSupport::MapDataPath))
		{
			FileSystem::CreateDirectories(CampaignMapDirectoryPath);
			const String campaignKey = data.editingCampaignId.value_or(SkyCampaign::Detail::SanitizeCampaignId(data.campaignNameState.text));
			path = FileSystem::PathAppend(CampaignMapDirectoryPath, U"{}_mission_{}.toml"_fmt(campaignKey.isEmpty() ? U"campaign" : campaignKey, missionIndex + 1));
			data.missionDrafts[missionIndex].mapPath.text = path;
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
