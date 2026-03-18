#pragma once

#include "GameSettings.h"

struct TitleUiLayout
{
	RectF panelRect{ 280, 190, 1040, 520 };
	Vec2 titlePos{ 800, 280 };
	Vec2 subtitlePos{ 800, 350 };
	Vec2 summaryLine1Pos{ 800, 430 };
	Vec2 summaryLine2Pos{ 800, 462 };
	Vec2 summaryLine3Pos{ 800, 494 };
	Vec2 viewedBonusRoomsPos{ 800, 526 };
	Vec2 enterHintPos{ 800, 562 };
	RectF continueButtonRect{ 690, 570, 220, 36 };
	RectF tutorialButtonRectWithContinue{ 690, 622, 220, 36 };
	RectF tutorialButtonRectWithoutContinue{ 690, 570, 220, 36 };
	RectF quickGuideButtonRectWithContinue{ 690, 674, 220, 36 };
	RectF quickGuideButtonRectWithoutContinue{ 690, 622, 220, 36 };
	RectF startButtonRectWithContinue{ 690, 726, 220, 36 };
	RectF startButtonRectWithoutContinue{ 690, 674, 220, 36 };
	RectF bonusButtonRectWithContinue{ 690, 778, 220, 36 };
	RectF bonusButtonRectWithoutContinue{ 690, 726, 220, 36 };
	Vec2 bonusRoomHintPosWithContinue{ 800, 606 };
	Vec2 bonusRoomHintPosWithoutContinue{ 800, 594 };
	RectF debugButtonRectWithContinue{ 690, 830, 220, 36 };
	RectF debugButtonRectWithoutContinue{ 690, 778, 220, 36 };
	RectF continuePreviewRect{ 956, 568, 308, 92 };
	RectF quickGuidePanelRect{ 370, 190, 860, 520 };
	Vec2 quickGuideBodyPos{ 408, 274 };
	Vec2 quickGuideFlowPos{ 800, 568 };
	RectF quickGuideTutorialButtonRect{ 580, 598, 200, 40 };
	RectF quickGuideCloseButtonRect{ 820, 598, 200, 40 };
	Vec2 quickGuideEscHintPos{ 800, 650 };
	RectF dataClearDialogRect{ 540, 345, 520, 210 };
	RectF dataClearDialogYesButtonRect{ 610, 492, 160, 40 };
	RectF dataClearDialogNoButtonRect{ 830, 492, 160, 40 };
	RectF exitDialogRect{ 590, 360, 420, 180 };
	RectF exitDialogYesButtonRect{ 640, 474, 140, 40 };
	RectF exitDialogNoButtonRect{ 820, 474, 140, 40 };
	Vec2 resolutionLabelPos{ 650, 700 };
	Vec2 resolutionValuePos{ 650, 724 };
	RectF resolutionSmallButtonRect{ 750, 696, 96, 32 };
	RectF resolutionMediumButtonRect{ 858, 696, 96, 32 };
	RectF resolutionLargeButtonRect{ 966, 696, 96, 32 };
	Vec2 saveLocationLabelPos{ 1050, 700 };
	Vec2 saveLocationValuePos{ 1050, 724 };
	RectF saveLocationButtonRect{ 1162, 696, 180, 32 };
	Vec2 dataManagementLabelPos{ 800, 628 };
	RectF clearContinueRunButtonRect{ 623, 646, 170, 32 };
	RectF clearSettingsButtonRect{ 807, 646, 170, 32 };
	RectF exitButtonRect{ 991, 646, 170, 32 };
	Vec2 dataManagementHintPos{ 800, 696 };
	Vec2 debugHintPosWithContinue{ 800, 806 };
	Vec2 debugHintPosWithoutContinue{ 800, 754 };
	RectF mapEditButtonRect{ 1170, 208, 128, 30 };
	RectF balanceEditButtonRect{ 1170, 246, 128, 30 };
	RectF transitionPresetButtonRect{ 1106, 284, 192, 30 };
	RectF titleUiEditorButtonRect{ 1106, 322, 192, 30 };
  RectF rewardEditorButtonRect{ 1106, 360, 192, 30 };
	RectF bonusRoomEditorButtonRect{ 1106, 398, 192, 30 };
};

namespace TitleUi
{
	inline constexpr int32 SchemaVersion = 1;

	[[nodiscard]] inline String GetLocalLayoutPath()
	{
		return U"save/title_ui_layout.toml";
	}

	[[nodiscard]] inline String GetAppDataLayoutPath()
	{
		return FileSystem::PathAppend(GameSettings::GetSettingsDirectoryPath(), U"save/title_ui_layout.toml");
	}

	[[nodiscard]] inline String GetLayoutPathForLocation(const ContinueRunSaveLocation location)
	{
		switch (location)
		{
		case ContinueRunSaveLocation::AppData:
			return GetAppDataLayoutPath();
		case ContinueRunSaveLocation::Local:
		default:
			return GetLocalLayoutPath();
		}
	}

	[[nodiscard]] inline String GetLayoutPath()
	{
		return GetLayoutPathForLocation(GetContinueRunSaveLocation());
	}

	[[nodiscard]] inline TitleUiLayout MakeDefaultTitleUiLayout()
	{
		return {};
	}

	inline void AppendTomlLine(String& content, const String& key, const String& value)
	{
		content += key + U" = " + value + U"\n";
	}

	inline void AppendVec2Section(String& content, const String& section, const Vec2& value)
	{
		content += U"\n[" + section + U"]\n";
		AppendTomlLine(content, U"x", Format(value.x));
		AppendTomlLine(content, U"y", Format(value.y));
	}

	inline void AppendRectSection(String& content, const String& section, const RectF& value)
	{
		content += U"\n[" + section + U"]\n";
		AppendTomlLine(content, U"x", Format(value.x));
		AppendTomlLine(content, U"y", Format(value.y));
		AppendTomlLine(content, U"w", Format(value.w));
		AppendTomlLine(content, U"h", Format(value.h));
	}

	inline void TryLoadVec2Section(const TOMLReader& toml, const String& section, Vec2& value)
	{
		try
		{
			value.x = toml[section][U"x"].get<double>();
			value.y = toml[section][U"y"].get<double>();
		}
		catch (const std::exception&)
		{
		}
	}

	inline void RepairRectSizeFromDefault(RectF& value, const RectF& defaults, const double minWidth, const double minHeight)
	{
		if (value.w < minWidth)
		{
			value.w = defaults.w;
		}

		if (value.h < minHeight)
		{
			value.h = defaults.h;
		}
	}

	inline void RepairTitleUiLayout(TitleUiLayout& layout)
	{
		const TitleUiLayout defaults = MakeDefaultTitleUiLayout();
		auto repairButton = [](RectF& value, const RectF& defaults)
		{
			RepairRectSizeFromDefault(value, defaults, Max(80.0, defaults.w * 0.5), Max(24.0, defaults.h * 0.75));
		};

		repairButton(layout.continueButtonRect, defaults.continueButtonRect);
		repairButton(layout.tutorialButtonRectWithContinue, defaults.tutorialButtonRectWithContinue);
		repairButton(layout.tutorialButtonRectWithoutContinue, defaults.tutorialButtonRectWithoutContinue);
		repairButton(layout.quickGuideButtonRectWithContinue, defaults.quickGuideButtonRectWithContinue);
		repairButton(layout.quickGuideButtonRectWithoutContinue, defaults.quickGuideButtonRectWithoutContinue);
		repairButton(layout.startButtonRectWithContinue, defaults.startButtonRectWithContinue);
		repairButton(layout.startButtonRectWithoutContinue, defaults.startButtonRectWithoutContinue);
		repairButton(layout.bonusButtonRectWithContinue, defaults.bonusButtonRectWithContinue);
		repairButton(layout.bonusButtonRectWithoutContinue, defaults.bonusButtonRectWithoutContinue);
		repairButton(layout.debugButtonRectWithContinue, defaults.debugButtonRectWithContinue);
		repairButton(layout.debugButtonRectWithoutContinue, defaults.debugButtonRectWithoutContinue);
		repairButton(layout.quickGuideTutorialButtonRect, defaults.quickGuideTutorialButtonRect);
		repairButton(layout.quickGuideCloseButtonRect, defaults.quickGuideCloseButtonRect);
		repairButton(layout.dataClearDialogYesButtonRect, defaults.dataClearDialogYesButtonRect);
		repairButton(layout.dataClearDialogNoButtonRect, defaults.dataClearDialogNoButtonRect);
		repairButton(layout.exitDialogYesButtonRect, defaults.exitDialogYesButtonRect);
		repairButton(layout.exitDialogNoButtonRect, defaults.exitDialogNoButtonRect);
		repairButton(layout.resolutionSmallButtonRect, defaults.resolutionSmallButtonRect);
		repairButton(layout.resolutionMediumButtonRect, defaults.resolutionMediumButtonRect);
		repairButton(layout.resolutionLargeButtonRect, defaults.resolutionLargeButtonRect);
		repairButton(layout.saveLocationButtonRect, defaults.saveLocationButtonRect);
		repairButton(layout.clearContinueRunButtonRect, defaults.clearContinueRunButtonRect);
		repairButton(layout.clearSettingsButtonRect, defaults.clearSettingsButtonRect);
		repairButton(layout.exitButtonRect, defaults.exitButtonRect);
		repairButton(layout.mapEditButtonRect, defaults.mapEditButtonRect);
		repairButton(layout.balanceEditButtonRect, defaults.balanceEditButtonRect);
		repairButton(layout.transitionPresetButtonRect, defaults.transitionPresetButtonRect);
		repairButton(layout.titleUiEditorButtonRect, defaults.titleUiEditorButtonRect);
       repairButton(layout.rewardEditorButtonRect, defaults.rewardEditorButtonRect);
		repairButton(layout.bonusRoomEditorButtonRect, defaults.bonusRoomEditorButtonRect);
	}

	inline void TryLoadRectSection(const TOMLReader& toml, const String& section, RectF& value)
	{
		try
		{
			value.x = toml[section][U"x"].get<double>();
			value.y = toml[section][U"y"].get<double>();
			value.w = Max(8.0, toml[section][U"w"].get<double>());
			value.h = Max(8.0, toml[section][U"h"].get<double>());
		}
		catch (const std::exception&)
		{
		}
	}

	[[nodiscard]] inline String BuildTitleUiLayoutTomlContent(const TitleUiLayout& layout)
	{
		String content;
		AppendTomlLine(content, U"schemaVersion", Format(SchemaVersion));
		AppendRectSection(content, U"panel", layout.panelRect);
		AppendVec2Section(content, U"title", layout.titlePos);
		AppendVec2Section(content, U"subtitle", layout.subtitlePos);
		AppendVec2Section(content, U"summaryLine1", layout.summaryLine1Pos);
		AppendVec2Section(content, U"summaryLine2", layout.summaryLine2Pos);
		AppendVec2Section(content, U"summaryLine3", layout.summaryLine3Pos);
		AppendVec2Section(content, U"viewedBonusRooms", layout.viewedBonusRoomsPos);
		AppendVec2Section(content, U"enterHint", layout.enterHintPos);
		AppendRectSection(content, U"continueButton", layout.continueButtonRect);
		AppendRectSection(content, U"tutorialButtonWithContinue", layout.tutorialButtonRectWithContinue);
		AppendRectSection(content, U"tutorialButtonWithoutContinue", layout.tutorialButtonRectWithoutContinue);
		AppendRectSection(content, U"quickGuideButtonWithContinue", layout.quickGuideButtonRectWithContinue);
		AppendRectSection(content, U"quickGuideButtonWithoutContinue", layout.quickGuideButtonRectWithoutContinue);
		AppendRectSection(content, U"startButtonWithContinue", layout.startButtonRectWithContinue);
		AppendRectSection(content, U"startButtonWithoutContinue", layout.startButtonRectWithoutContinue);
		AppendRectSection(content, U"bonusButtonWithContinue", layout.bonusButtonRectWithContinue);
		AppendRectSection(content, U"bonusButtonWithoutContinue", layout.bonusButtonRectWithoutContinue);
		AppendVec2Section(content, U"bonusRoomHintWithContinue", layout.bonusRoomHintPosWithContinue);
		AppendVec2Section(content, U"bonusRoomHintWithoutContinue", layout.bonusRoomHintPosWithoutContinue);
		AppendRectSection(content, U"debugButtonWithContinue", layout.debugButtonRectWithContinue);
		AppendRectSection(content, U"debugButtonWithoutContinue", layout.debugButtonRectWithoutContinue);
		AppendRectSection(content, U"continuePreview", layout.continuePreviewRect);
		AppendRectSection(content, U"quickGuidePanel", layout.quickGuidePanelRect);
		AppendVec2Section(content, U"quickGuideBody", layout.quickGuideBodyPos);
		AppendVec2Section(content, U"quickGuideFlow", layout.quickGuideFlowPos);
		AppendRectSection(content, U"quickGuideTutorialButton", layout.quickGuideTutorialButtonRect);
		AppendRectSection(content, U"quickGuideCloseButton", layout.quickGuideCloseButtonRect);
		AppendVec2Section(content, U"quickGuideEscHint", layout.quickGuideEscHintPos);
		AppendRectSection(content, U"dataClearDialog", layout.dataClearDialogRect);
		AppendRectSection(content, U"dataClearDialogYesButton", layout.dataClearDialogYesButtonRect);
		AppendRectSection(content, U"dataClearDialogNoButton", layout.dataClearDialogNoButtonRect);
		AppendRectSection(content, U"exitDialog", layout.exitDialogRect);
		AppendRectSection(content, U"exitDialogYesButton", layout.exitDialogYesButtonRect);
		AppendRectSection(content, U"exitDialogNoButton", layout.exitDialogNoButtonRect);
		AppendVec2Section(content, U"resolutionLabel", layout.resolutionLabelPos);
		AppendVec2Section(content, U"resolutionValue", layout.resolutionValuePos);
		AppendRectSection(content, U"resolutionSmallButton", layout.resolutionSmallButtonRect);
		AppendRectSection(content, U"resolutionMediumButton", layout.resolutionMediumButtonRect);
		AppendRectSection(content, U"resolutionLargeButton", layout.resolutionLargeButtonRect);
		AppendVec2Section(content, U"saveLocationLabel", layout.saveLocationLabelPos);
		AppendVec2Section(content, U"saveLocationValue", layout.saveLocationValuePos);
		AppendRectSection(content, U"saveLocationButton", layout.saveLocationButtonRect);
		AppendVec2Section(content, U"dataManagementLabel", layout.dataManagementLabelPos);
		AppendRectSection(content, U"clearContinueRunButton", layout.clearContinueRunButtonRect);
		AppendRectSection(content, U"clearSettingsButton", layout.clearSettingsButtonRect);
		AppendRectSection(content, U"exitButton", layout.exitButtonRect);
		AppendVec2Section(content, U"dataManagementHint", layout.dataManagementHintPos);
		AppendVec2Section(content, U"debugHintWithContinue", layout.debugHintPosWithContinue);
		AppendVec2Section(content, U"debugHintWithoutContinue", layout.debugHintPosWithoutContinue);
		AppendRectSection(content, U"mapEditButton", layout.mapEditButtonRect);
		AppendRectSection(content, U"balanceEditButton", layout.balanceEditButtonRect);
		AppendRectSection(content, U"transitionPresetButton", layout.transitionPresetButtonRect);
		AppendRectSection(content, U"titleUiEditorButton", layout.titleUiEditorButtonRect);
     AppendRectSection(content, U"rewardEditorButton", layout.rewardEditorButtonRect);
		AppendRectSection(content, U"bonusRoomEditorButton", layout.bonusRoomEditorButtonRect);
		return content;
	}

	[[nodiscard]] inline TitleUiLayout LoadTitleUiLayoutFromDisk()
	{
		TitleUiLayout layout = MakeDefaultTitleUiLayout();
		const TOMLReader toml{ GetLayoutPath() };
		if (!toml)
		{
			return layout;
		}

		try
		{
			if (toml[U"schemaVersion"].get<int32>() != SchemaVersion)
			{
				return MakeDefaultTitleUiLayout();
			}
		}
		catch (const std::exception&)
		{
			return MakeDefaultTitleUiLayout();
		}

		TryLoadRectSection(toml, U"panel", layout.panelRect);
		TryLoadVec2Section(toml, U"title", layout.titlePos);
		TryLoadVec2Section(toml, U"subtitle", layout.subtitlePos);
		TryLoadVec2Section(toml, U"summaryLine1", layout.summaryLine1Pos);
		TryLoadVec2Section(toml, U"summaryLine2", layout.summaryLine2Pos);
		TryLoadVec2Section(toml, U"summaryLine3", layout.summaryLine3Pos);
		TryLoadVec2Section(toml, U"viewedBonusRooms", layout.viewedBonusRoomsPos);
		TryLoadVec2Section(toml, U"enterHint", layout.enterHintPos);
		TryLoadRectSection(toml, U"continueButton", layout.continueButtonRect);
		TryLoadRectSection(toml, U"tutorialButtonWithContinue", layout.tutorialButtonRectWithContinue);
		TryLoadRectSection(toml, U"tutorialButtonWithoutContinue", layout.tutorialButtonRectWithoutContinue);
		TryLoadRectSection(toml, U"quickGuideButtonWithContinue", layout.quickGuideButtonRectWithContinue);
		TryLoadRectSection(toml, U"quickGuideButtonWithoutContinue", layout.quickGuideButtonRectWithoutContinue);
		TryLoadRectSection(toml, U"startButtonWithContinue", layout.startButtonRectWithContinue);
		TryLoadRectSection(toml, U"startButtonWithoutContinue", layout.startButtonRectWithoutContinue);
		TryLoadRectSection(toml, U"bonusButtonWithContinue", layout.bonusButtonRectWithContinue);
		TryLoadRectSection(toml, U"bonusButtonWithoutContinue", layout.bonusButtonRectWithoutContinue);
		TryLoadVec2Section(toml, U"bonusRoomHintWithContinue", layout.bonusRoomHintPosWithContinue);
		TryLoadVec2Section(toml, U"bonusRoomHintWithoutContinue", layout.bonusRoomHintPosWithoutContinue);
		TryLoadRectSection(toml, U"debugButtonWithContinue", layout.debugButtonRectWithContinue);
		TryLoadRectSection(toml, U"debugButtonWithoutContinue", layout.debugButtonRectWithoutContinue);
		TryLoadRectSection(toml, U"continuePreview", layout.continuePreviewRect);
		TryLoadRectSection(toml, U"quickGuidePanel", layout.quickGuidePanelRect);
		TryLoadVec2Section(toml, U"quickGuideBody", layout.quickGuideBodyPos);
		TryLoadVec2Section(toml, U"quickGuideFlow", layout.quickGuideFlowPos);
		TryLoadRectSection(toml, U"quickGuideTutorialButton", layout.quickGuideTutorialButtonRect);
		TryLoadRectSection(toml, U"quickGuideCloseButton", layout.quickGuideCloseButtonRect);
		TryLoadVec2Section(toml, U"quickGuideEscHint", layout.quickGuideEscHintPos);
		TryLoadRectSection(toml, U"dataClearDialog", layout.dataClearDialogRect);
		TryLoadRectSection(toml, U"dataClearDialogYesButton", layout.dataClearDialogYesButtonRect);
		TryLoadRectSection(toml, U"dataClearDialogNoButton", layout.dataClearDialogNoButtonRect);
		TryLoadRectSection(toml, U"exitDialog", layout.exitDialogRect);
		TryLoadRectSection(toml, U"exitDialogYesButton", layout.exitDialogYesButtonRect);
		TryLoadRectSection(toml, U"exitDialogNoButton", layout.exitDialogNoButtonRect);
		TryLoadVec2Section(toml, U"resolutionLabel", layout.resolutionLabelPos);
		TryLoadVec2Section(toml, U"resolutionValue", layout.resolutionValuePos);
		TryLoadRectSection(toml, U"resolutionSmallButton", layout.resolutionSmallButtonRect);
		TryLoadRectSection(toml, U"resolutionMediumButton", layout.resolutionMediumButtonRect);
		TryLoadRectSection(toml, U"resolutionLargeButton", layout.resolutionLargeButtonRect);
		TryLoadVec2Section(toml, U"saveLocationLabel", layout.saveLocationLabelPos);
		TryLoadVec2Section(toml, U"saveLocationValue", layout.saveLocationValuePos);
		TryLoadRectSection(toml, U"saveLocationButton", layout.saveLocationButtonRect);
		TryLoadVec2Section(toml, U"dataManagementLabel", layout.dataManagementLabelPos);
		TryLoadRectSection(toml, U"clearContinueRunButton", layout.clearContinueRunButtonRect);
		TryLoadRectSection(toml, U"clearSettingsButton", layout.clearSettingsButtonRect);
		TryLoadRectSection(toml, U"exitButton", layout.exitButtonRect);
		TryLoadVec2Section(toml, U"dataManagementHint", layout.dataManagementHintPos);
		TryLoadVec2Section(toml, U"debugHintWithContinue", layout.debugHintPosWithContinue);
		TryLoadVec2Section(toml, U"debugHintWithoutContinue", layout.debugHintPosWithoutContinue);
		TryLoadRectSection(toml, U"mapEditButton", layout.mapEditButtonRect);
		TryLoadRectSection(toml, U"balanceEditButton", layout.balanceEditButtonRect);
		TryLoadRectSection(toml, U"transitionPresetButton", layout.transitionPresetButtonRect);
		TryLoadRectSection(toml, U"titleUiEditorButton", layout.titleUiEditorButtonRect);
        TryLoadRectSection(toml, U"rewardEditorButton", layout.rewardEditorButtonRect);
		TryLoadRectSection(toml, U"bonusRoomEditorButton", layout.bonusRoomEditorButtonRect);
		RepairTitleUiLayout(layout);
		return layout;
	}

	[[nodiscard]] inline TitleUiLayout& GetTitleUiLayoutStorage()
	{
		static TitleUiLayout layout = LoadTitleUiLayoutFromDisk();
		return layout;
	}

	[[nodiscard]] inline TitleUiLayout GetTitleUiLayout()
	{
		return GetTitleUiLayoutStorage();
	}

	[[nodiscard]] inline TitleUiLayout ReloadTitleUiLayout()
	{
		GetTitleUiLayoutStorage() = LoadTitleUiLayoutFromDisk();
		return GetTitleUiLayoutStorage();
	}

	[[nodiscard]] inline bool SaveTitleUiLayout(const TitleUiLayout& layout)
	{
		const String layoutPath = GetLayoutPath();
		FileSystem::CreateDirectories(FileSystem::ParentPath(layoutPath));

		TextWriter writer{ layoutPath };
		if (!writer)
		{
			return false;
		}

		writer.write(BuildTitleUiLayoutTomlContent(layout));
		GetTitleUiLayoutStorage() = layout;
		return true;
	}

	[[nodiscard]] inline bool ClearTitleUiLayout()
	{
		const String layoutPath = GetLayoutPath();
		if (FileSystem::Exists(layoutPath) && !FileSystem::Remove(layoutPath))
		{
			return false;
		}

		GetTitleUiLayoutStorage() = MakeDefaultTitleUiLayout();
		return true;
	}

	[[nodiscard]] inline bool MoveTitleUiLayoutToLocation(const ContinueRunSaveLocation currentLocation, const ContinueRunSaveLocation nextLocation)
	{
		if (currentLocation == nextLocation)
		{
			return true;
		}

		const String currentPath = GetLayoutPathForLocation(currentLocation);
		const String nextPath = GetLayoutPathForLocation(nextLocation);
		FileSystem::CreateDirectories(FileSystem::ParentPath(nextPath));

		const auto layout = GetTitleUiLayout();

		TextWriter writer{ nextPath };
		if (!writer)
		{
			return false;
		}

		writer.write(BuildTitleUiLayoutTomlContent(layout));
		if (FileSystem::Exists(currentPath) && (currentPath != nextPath))
		{
			FileSystem::Remove(currentPath);
		}

		return true;
	}

	[[nodiscard]] inline RectF GetTutorialButtonRect(const TitleUiLayout& layout, const bool hasContinue)
	{
		return hasContinue ? layout.tutorialButtonRectWithContinue : layout.tutorialButtonRectWithoutContinue;
	}

	[[nodiscard]] inline RectF GetQuickGuideButtonRect(const TitleUiLayout& layout, const bool hasContinue)
	{
		return hasContinue ? layout.quickGuideButtonRectWithContinue : layout.quickGuideButtonRectWithoutContinue;
	}

	[[nodiscard]] inline RectF GetStartButtonRect(const TitleUiLayout& layout, const bool hasContinue)
	{
		return hasContinue ? layout.startButtonRectWithContinue : layout.startButtonRectWithoutContinue;
	}

	[[nodiscard]] inline RectF GetBonusButtonRect(const TitleUiLayout& layout, const bool hasContinue)
	{
		return hasContinue ? layout.bonusButtonRectWithContinue : layout.bonusButtonRectWithoutContinue;
	}

	[[nodiscard]] inline RectF GetDebugButtonRect(const TitleUiLayout& layout, const bool hasContinue)
	{
		return hasContinue ? layout.debugButtonRectWithContinue : layout.debugButtonRectWithoutContinue;
	}

	[[nodiscard]] inline Vec2 GetBonusRoomHintPos(const TitleUiLayout& layout, const bool hasContinue)
	{
		return hasContinue ? layout.bonusRoomHintPosWithContinue : layout.bonusRoomHintPosWithoutContinue;
	}

	[[nodiscard]] inline Vec2 GetQuickGuideHintPos(const TitleUiLayout& layout, const bool hasContinue)
	{
		const RectF buttonRect = GetQuickGuideButtonRect(layout, hasContinue);
		return Vec2{ buttonRect.center().x, buttonRect.center().y + 28 };
	}

	[[nodiscard]] inline RectF GetResolutionButtonRect(const TitleUiLayout& layout, const size_t index)
	{
		switch (index)
		{
		case 0:
			return layout.resolutionSmallButtonRect;
		case 2:
			return layout.resolutionLargeButtonRect;
		case 1:
		default:
			return layout.resolutionMediumButtonRect;
		}
	}
}
