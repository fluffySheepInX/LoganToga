# pragma once

namespace SkyAppText
{
	enum class Language
	{
		Japanese,
		English,
	};

	enum class TextId
	{
		CommonDrag,
		CommonDragToAdjust,
       CommonDragToAdjustRange,
		CommonOn,
		CommonOff,
		CommonReset,
		CommonSave,
		CommonReload,
		CommonResetAll,
		CommonUi,
		HudEnemyPlanFixedResource,
		HudEnemyPlanFixedBase,
		HudEnemyPlanAutoResource,
		HudEnemyPlanAutoBase,
		HudEnemyPlanTooltipFixedResource,
		HudEnemyPlanTooltipFixedBase,
		HudEnemyPlanTooltipAuto,
		HudTooltipReturnToPlay,
		HudTooltipMapEditMode,
		HudTooltipModelHeightScale,
		HudTooltipUnitParameterEditor,
		HudTooltipSkySettingsPanel,
		HudTooltipCameraSettingsPanel,
		HudTooltipTerrainNoiseSettings,
		HudTooltipUiLayoutEdit,
		HudTooltipManualResourceAdjust,
		HudUiEditModeActivated,
		TerrainPanelMove,
		TerrainPanelMoveDescription,
		TerrainPageSurface,
		TerrainPageGrounding,
		TerrainPageNoise,
		TerrainPageSurfaceDescription,
		TerrainPageGroundingDescription,
		TerrainPageNoiseDescription,
		ModelHeightPanelTitle,
		ModelHeightTargets,
		ModelHeightItemYOffset,
		ModelHeightTargetCurrent,
		ModelHeightOffsetY,
		ModelHeightScaleLabel,
		ModelHeightResetTarget,
		ModelHeightWorldY,
		ModelHeightCurrentScale,
		ModelHeightRangeSummary,
		ModelHeightSavedWithPath,
		ModelHeightSaveFailed,
		ModelHeightReloaded,
		ModelHeightOffsetsScalesReset,
	};

	[[nodiscard]] inline Language& CurrentLanguageStorage()
	{
		static Language language = Language::Japanese;
		return language;
	}

	[[nodiscard]] inline Language& FallbackLanguageStorage()
	{
		static Language language = Language::English;
		return language;
	}

	inline void SetLanguage(const Language language)
	{
		CurrentLanguageStorage() = language;
	}

	inline void SetFallbackLanguage(const Language language)
	{
		FallbackLanguageStorage() = language;
	}

	[[nodiscard]] inline StringView ToKey(const TextId id)
	{
		switch (id)
		{
		case TextId::CommonDrag: return U"CommonDrag";
		case TextId::CommonDragToAdjust: return U"CommonDragToAdjust";
      case TextId::CommonDragToAdjustRange: return U"CommonDragToAdjustRange";
		case TextId::CommonOn: return U"CommonOn";
		case TextId::CommonOff: return U"CommonOff";
		case TextId::CommonReset: return U"CommonReset";
		case TextId::CommonSave: return U"CommonSave";
		case TextId::CommonReload: return U"CommonReload";
		case TextId::CommonResetAll: return U"CommonResetAll";
		case TextId::CommonUi: return U"CommonUi";
		case TextId::HudEnemyPlanFixedResource: return U"HudEnemyPlanFixedResource";
		case TextId::HudEnemyPlanFixedBase: return U"HudEnemyPlanFixedBase";
		case TextId::HudEnemyPlanAutoResource: return U"HudEnemyPlanAutoResource";
		case TextId::HudEnemyPlanAutoBase: return U"HudEnemyPlanAutoBase";
		case TextId::HudEnemyPlanTooltipFixedResource: return U"HudEnemyPlanTooltipFixedResource";
		case TextId::HudEnemyPlanTooltipFixedBase: return U"HudEnemyPlanTooltipFixedBase";
		case TextId::HudEnemyPlanTooltipAuto: return U"HudEnemyPlanTooltipAuto";
		case TextId::HudTooltipReturnToPlay: return U"HudTooltipReturnToPlay";
		case TextId::HudTooltipMapEditMode: return U"HudTooltipMapEditMode";
		case TextId::HudTooltipModelHeightScale: return U"HudTooltipModelHeightScale";
		case TextId::HudTooltipUnitParameterEditor: return U"HudTooltipUnitParameterEditor";
		case TextId::HudTooltipSkySettingsPanel: return U"HudTooltipSkySettingsPanel";
		case TextId::HudTooltipCameraSettingsPanel: return U"HudTooltipCameraSettingsPanel";
		case TextId::HudTooltipTerrainNoiseSettings: return U"HudTooltipTerrainNoiseSettings";
		case TextId::HudTooltipUiLayoutEdit: return U"HudTooltipUiLayoutEdit";
		case TextId::HudTooltipManualResourceAdjust: return U"HudTooltipManualResourceAdjust";
		case TextId::HudUiEditModeActivated: return U"HudUiEditModeActivated";
		case TextId::TerrainPanelMove: return U"TerrainPanelMove";
		case TextId::TerrainPanelMoveDescription: return U"TerrainPanelMoveDescription";
		case TextId::TerrainPageSurface: return U"TerrainPageSurface";
		case TextId::TerrainPageGrounding: return U"TerrainPageGrounding";
		case TextId::TerrainPageNoise: return U"TerrainPageNoise";
		case TextId::TerrainPageSurfaceDescription: return U"TerrainPageSurfaceDescription";
		case TextId::TerrainPageGroundingDescription: return U"TerrainPageGroundingDescription";
		case TextId::TerrainPageNoiseDescription: return U"TerrainPageNoiseDescription";
		case TextId::ModelHeightPanelTitle: return U"ModelHeightPanelTitle";
		case TextId::ModelHeightTargets: return U"ModelHeightTargets";
		case TextId::ModelHeightItemYOffset: return U"ModelHeightItemYOffset";
		case TextId::ModelHeightTargetCurrent: return U"ModelHeightTargetCurrent";
		case TextId::ModelHeightOffsetY: return U"ModelHeightOffsetY";
		case TextId::ModelHeightScaleLabel: return U"ModelHeightScaleLabel";
		case TextId::ModelHeightResetTarget: return U"ModelHeightResetTarget";
		case TextId::ModelHeightWorldY: return U"ModelHeightWorldY";
		case TextId::ModelHeightCurrentScale: return U"ModelHeightCurrentScale";
		case TextId::ModelHeightRangeSummary: return U"ModelHeightRangeSummary";
		case TextId::ModelHeightSavedWithPath: return U"ModelHeightSavedWithPath";
		case TextId::ModelHeightSaveFailed: return U"ModelHeightSaveFailed";
		case TextId::ModelHeightReloaded: return U"ModelHeightReloaded";
		case TextId::ModelHeightOffsetsScalesReset: return U"ModelHeightOffsetsScalesReset";
		default: return U"UnknownTextId";
		}
	}

	[[nodiscard]] inline String TryGetText(const Language language, const TextId id)
	{
		switch (language)
		{
		case Language::Japanese:
			switch (id)
			{
			case TextId::CommonDrag: return U"drag";
			case TextId::CommonDragToAdjust: return U"drag to adjust";
            case TextId::CommonDragToAdjustRange: return U"drag to adjust  [{0}]";
			case TextId::CommonOn: return U"ON";
			case TextId::CommonOff: return U"OFF";
			case TextId::CommonReset: return U"Reset";
			case TextId::CommonSave: return U"Save";
			case TextId::CommonReload: return U"Reload";
			case TextId::CommonResetAll: return U"Reset All";
			case TextId::CommonUi: return U"UI";
			case TextId::HudEnemyPlanFixedResource: return U"固定:資源";
			case TextId::HudEnemyPlanFixedBase: return U"固定:拠点";
			case TextId::HudEnemyPlanAutoResource: return U"自動:資源";
			case TextId::HudEnemyPlanAutoBase: return U"自動:拠点";
			case TextId::HudEnemyPlanTooltipFixedResource: return U"敵全体方針: 資源確保に固定";
			case TextId::HudEnemyPlanTooltipFixedBase: return U"敵全体方針: 拠点攻撃に固定";
			case TextId::HudEnemyPlanTooltipAuto: return U"敵全体方針: 状況から自動決定";
			case TextId::HudTooltipReturnToPlay: return U"プレイ表示に戻す";
			case TextId::HudTooltipMapEditMode: return U"マップ編集モード";
			case TextId::HudTooltipModelHeightScale: return U"モデル高さ / スケール調整";
			case TextId::HudTooltipUnitParameterEditor: return U"ユニットパラメータエディタ";
			case TextId::HudTooltipSkySettingsPanel: return U"空設定パネル";
			case TextId::HudTooltipCameraSettingsPanel: return U"カメラ設定パネル";
			case TextId::HudTooltipTerrainNoiseSettings: return U"地面ノイズ設定";
			case TextId::HudTooltipUiLayoutEdit: return U"UI レイアウト編集";
			case TextId::HudTooltipManualResourceAdjust: return U"資源量の手動調整";
			case TextId::HudUiEditModeActivated: return U"UI Edit: drag panels / grid snap";
			case TextId::TerrainPanelMove: return U"パネル移動";
			case TextId::TerrainPanelMoveDescription: return U"`UI+` が ON の間、この drag ハンドルをドラッグすると Terrain Surface パネルを移動できます。項目操作と競合しにくいよう、移動開始位置を右上に分けています。";
			case TextId::TerrainPageSurface: return U"混合";
			case TextId::TerrainPageGrounding: return U"接地";
			case TextId::TerrainPageNoise: return U"ノイズ";
			case TextId::TerrainPageSurfaceDescription: return U"材質混合と配置物の浸食を調整します。境界のなじみと、建物や道が周囲地面へ書き込む強さを確認するページです。";
			case TextId::TerrainPageGroundingDescription: return U"摩耗と AO を調整します。接地感、踏み荒らし、重さの見え方を詰めるページです。";
			case TextId::TerrainPageNoiseDescription: return U"広域ノイズと細部ノイズを調整します。マップ全体のムラと近距離の粒感を扱います。";
			case TextId::ModelHeightPanelTitle: return U"Unit Height / Scale Editor";
			case TextId::ModelHeightTargets: return U"Targets";
			case TextId::ModelHeightItemYOffset: return U"Y {0}";
			case TextId::ModelHeightTargetCurrent: return U"Target: {0}";
			case TextId::ModelHeightOffsetY: return U"offset Y: {0}";
			case TextId::ModelHeightScaleLabel: return U"scale";
			case TextId::ModelHeightResetTarget: return U"Reset Target";
			case TextId::ModelHeightWorldY: return U"world Y: {0}";
			case TextId::ModelHeightCurrentScale: return U"scale: {0}";
			case TextId::ModelHeightRangeSummary: return U"Y [{0}, {1}] / Scale [{2}, {3}]";
			case TextId::ModelHeightSavedWithPath: return U"Saved: {0}";
			case TextId::ModelHeightSaveFailed: return U"Save failed";
			case TextId::ModelHeightReloaded: return U"Reloaded";
			case TextId::ModelHeightOffsetsScalesReset: return U"Offsets / scales reset";
			default: return U"";
			}

		case Language::English:
		default:
			switch (id)
			{
			case TextId::CommonDrag: return U"drag";
			case TextId::CommonDragToAdjust: return U"drag to adjust";
            case TextId::CommonDragToAdjustRange: return U"drag to adjust  [{0}]";
			case TextId::CommonOn: return U"ON";
			case TextId::CommonOff: return U"OFF";
			case TextId::CommonReset: return U"Reset";
			case TextId::CommonSave: return U"Save";
			case TextId::CommonReload: return U"Reload";
			case TextId::CommonResetAll: return U"Reset All";
			case TextId::CommonUi: return U"UI";
			case TextId::HudEnemyPlanFixedResource: return U"Lock: Res";
			case TextId::HudEnemyPlanFixedBase: return U"Lock: Base";
			case TextId::HudEnemyPlanAutoResource: return U"Auto: Res";
			case TextId::HudEnemyPlanAutoBase: return U"Auto: Base";
			case TextId::HudEnemyPlanTooltipFixedResource: return U"Enemy plan: lock to resource control";
			case TextId::HudEnemyPlanTooltipFixedBase: return U"Enemy plan: lock to base assault";
			case TextId::HudEnemyPlanTooltipAuto: return U"Enemy plan: decide automatically from the situation";
			case TextId::HudTooltipReturnToPlay: return U"Return to play view";
			case TextId::HudTooltipMapEditMode: return U"Map edit mode";
			case TextId::HudTooltipModelHeightScale: return U"Model height / scale adjustment";
			case TextId::HudTooltipUnitParameterEditor: return U"Unit parameter editor";
			case TextId::HudTooltipSkySettingsPanel: return U"Sky settings panel";
			case TextId::HudTooltipCameraSettingsPanel: return U"Camera settings panel";
			case TextId::HudTooltipTerrainNoiseSettings: return U"Terrain noise settings";
			case TextId::HudTooltipUiLayoutEdit: return U"UI layout edit";
			case TextId::HudTooltipManualResourceAdjust: return U"Manual resource adjustment";
			case TextId::HudUiEditModeActivated: return U"UI Edit: drag panels / grid snap";
			case TextId::TerrainPanelMove: return U"Move panel";
			case TextId::TerrainPanelMoveDescription: return U"While `UI+` is ON, drag this handle to move the Terrain Surface panel. The drag start area is separated to the top right to avoid interfering with parameter controls.";
			case TextId::TerrainPageSurface: return U"Blend";
			case TextId::TerrainPageGrounding: return U"Grounding";
			case TextId::TerrainPageNoise: return U"Noise";
			case TextId::TerrainPageSurfaceDescription: return U"Adjust material blending and placement imprint. Use this page to tune softer boundaries and how strongly buildings and roads imprint onto nearby ground.";
			case TextId::TerrainPageGroundingDescription: return U"Adjust wear and AO. Use this page to tune grounding, trampling, and the perceived weight of objects.";
			case TextId::TerrainPageNoiseDescription: return U"Adjust macro and detail noise. Use this page to tune broad terrain variation and close-range grain.";
			case TextId::ModelHeightPanelTitle: return U"Unit Height / Scale Editor";
			case TextId::ModelHeightTargets: return U"Targets";
			case TextId::ModelHeightItemYOffset: return U"Y {0}";
			case TextId::ModelHeightTargetCurrent: return U"Target: {0}";
			case TextId::ModelHeightOffsetY: return U"offset Y: {0}";
			case TextId::ModelHeightScaleLabel: return U"scale";
			case TextId::ModelHeightResetTarget: return U"Reset Target";
			case TextId::ModelHeightWorldY: return U"world Y: {0}";
			case TextId::ModelHeightCurrentScale: return U"scale: {0}";
			case TextId::ModelHeightRangeSummary: return U"Y [{0}, {1}] / Scale [{2}, {3}]";
			case TextId::ModelHeightSavedWithPath: return U"Saved: {0}";
			case TextId::ModelHeightSaveFailed: return U"Save failed";
			case TextId::ModelHeightReloaded: return U"Reloaded";
			case TextId::ModelHeightOffsetsScalesReset: return U"Offsets / scales reset";
			default: return U"";
			}
		}
	}

	[[nodiscard]] inline String Tr(const TextId id)
	{
		String text = TryGetText(CurrentLanguageStorage(), id);
		if (!text.isEmpty())
		{
			return text;
		}

		if (CurrentLanguageStorage() != FallbackLanguageStorage())
		{
			text = TryGetText(FallbackLanguageStorage(), id);
			if (!text.isEmpty())
			{
				return text;
			}
		}

		return U"[MissingText:{0}]"_fmt(ToKey(id));
	}

	template <class... Args>
	[[nodiscard]] inline String TrFormat(const TextId id, const Args&... args)
	{
		String text = Tr(id);
		const Array<String> replacements = { Format(args)... };
		for (size_t i = 0; i < replacements.size(); ++i)
		{
			text.replace((U"{" + Format(i) + U"}"), replacements[i]);
		}
		return text;
	}
}
