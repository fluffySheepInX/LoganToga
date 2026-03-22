#pragma once

#include "TitleUiLayout.Types.h"

namespace TitleUi
{
	inline constexpr int32 SchemaVersion = 1;

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

	[[nodiscard]] inline TitleUiLayout LoadTitleUiLayoutFromToml(const TOMLReader& toml)
	{
		TitleUiLayout layout = MakeDefaultTitleUiLayout();

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
}
