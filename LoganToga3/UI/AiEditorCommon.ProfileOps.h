#pragma once
# include <Siv3D.hpp>
# include "AiEditorCommon.UnitWeight.h"
# include "../Data/Loaders/AiProfileDefLoader.h"

namespace LT3
{
	// 次のターゲット優先種別を返す。
	inline String NextAiTargetPriority(StringView value)
	{
		const String lower = String{ value }.lowercased();
		if (lower == U"base")
		{
			return U"resource";
		}
		if (lower == U"resource")
		{
			return U"unit";
		}
		return U"base";
	}

	// 次の会敵動作種別を返す。
	inline String NextAiContactBehavior(StringView value)
	{
		return (String{ value }.lowercased() == U"ignore") ? U"engage" : U"ignore";
	}

	// 前のターゲット優先種別を返す。
	inline String PreviousAiTargetPriority(StringView value)
	{
		const String lower = String{ value }.lowercased();
		if (lower == U"base")
		{
			return U"unit";
		}
		if (lower == U"unit")
		{
			return U"resource";
		}
		return U"base";
	}

	// 初期ユニット候補として選択可能かを判定する。
	inline bool CanAiEditorSelectInitialUnit(const DefinitionStores& defs, UnitDefId unitDefId)
	{
		if (!(0 <= unitDefId && unitDefId < defs.units.size()))
		{
			return false;
		}
		const UnitDef& def = defs.units[unitDefId];
		if (def.unit_id.isEmpty() || def.role == UnitRole::Base || def.role == UnitRole::Barrier || def.speed <= 0.0)
		{
			return false;
		}

		const String unitTag = def.unit_id.lowercased();
		for (const auto& action : defs.buildActions)
		{
			if (!action.enemyCanProduce || action.resultType != BuildActionResultType::Unit)
			{
				continue;
			}
			if (action.spawnUnit == unitDefId || action.spawnUnits.any([&](UnitDefId spawnUnit) { return spawnUnit == unitDefId; }))
			{
				return true;
			}
			if ((!action.spawnTag.isEmpty() && action.spawnTag.lowercased() == unitTag)
				|| action.spawnTags.any([&](const String& spawnTag) { return !spawnTag.isEmpty() && spawnTag.lowercased() == unitTag; }))
			{
				return true;
			}
		}

		return false;
	}

	// ユニットタグから表示名を解決する。
	inline String ResolveAiEditorUnitDisplayName(const DefinitionStores& defs, StringView unitTag)
	{
		const String lower = String{ unitTag }.lowercased();
		if (lower.isEmpty())
		{
			return U"<none>";
		}

		if (defs.unitByTag.contains(lower))
		{
			const UnitDef& def = defs.units[defs.unitByTag.at(lower)];
			return U"{} {}"_fmt(def.unit_id, def.name.isEmpty() ? def.unit_id : def.name);
		}

		return U"{} (?)"_fmt(lower);
	}

	// 初期ユニット候補タグ一覧を収集する。
	inline Array<String> CollectAiEditorInitialUnitTags(const DefinitionStores& defs)
	{
		Array<String> tags;
		for (UnitDefId unitDefId = 0; unitDefId < defs.units.size(); ++unitDefId)
		{
			if (CanAiEditorSelectInitialUnit(defs, unitDefId))
			{
				tags << defs.units[unitDefId].unit_id.lowercased();
			}
		}

		return tags;
	}

	// 初期ユニットタグを前後へ巡回して返す。
	inline String NextAiInitialUnitTag(StringView value, const DefinitionStores& defs, int32 direction)
	{
		const Array<String> tags = CollectAiEditorInitialUnitTags(defs);
		if (tags.isEmpty())
		{
			return String{ value }.lowercased();
		}

		const String lower = String{ value }.lowercased();
		int32 currentIndex = -1;
		for (int32 index = 0; index < static_cast<int32>(tags.size()); ++index)
		{
			if (tags[index] == lower)
			{
				currentIndex = index;
				break;
			}
		}
		if (currentIndex < 0)
		{
			currentIndex = 0;
		}
		else
		{
			currentIndex = (currentIndex + direction + static_cast<int32>(tags.size())) % static_cast<int32>(tags.size());
		}

		return tags[currentIndex];
	}

	// 未使用の初期ユニットタグ候補を返す。
	inline String FindNextAiInitialUnitTag(const AiProfileDef& profile, const DefinitionStores& defs)
	{
		const Array<String> tags = CollectAiEditorInitialUnitTags(defs);
		for (const String& tag : tags)
		{
			if (!profile.initialUnits.any([&](const String& initialUnit) { return initialUnit.lowercased() == tag; }))
			{
				return tag;
			}
		}

		return tags.isEmpty() ? U"" : tags.front();
	}

	// 未使用の unit weight タグ候補を返す。
	inline String FindNextAiUnitWeightTag(const AiProfileDef& profile, const DefinitionStores& defs)
	{
		const Array<String> tags = CollectAiEditorInitialUnitTags(defs);
		for (const String& unitTag : tags)
		{
			bool used = false;
			for (const auto& weight : profile.unitWeights)
			{
				if (weight.unitTag.lowercased() == unitTag)
				{
					used = true;
					break;
				}
			}

			if (!used)
			{
				return unitTag;
			}
		}

		return tags.isEmpty() ? U"" : tags.front();
	}

	// unit weight タグを前後へ巡回して返す。
	inline String NextAiUnitWeightTag(StringView value, const DefinitionStores& defs, int32 direction)
	{
		return NextAiInitialUnitTag(value, defs, direction);
	}

	// 未使用の build priority タグ候補を返す。
	inline String FindNextAiBuildPriorityTag(const AiProfileDef& profile, const DefinitionStores& defs)
	{
		for (const auto& action : defs.buildActions)
		{
			const String actionTag = action.tag.lowercased();
			if (actionTag.isEmpty())
			{
				continue;
			}

			bool used = false;
			for (const auto& buildPriority : profile.buildPriorities)
			{
				if (buildPriority.actionTag.lowercased() == actionTag)
				{
					used = true;
					break;
				}
			}

			if (!used)
			{
				return actionTag;
			}
		}

		return defs.buildActions.isEmpty() ? U"" : defs.buildActions.front().tag.lowercased();
	}

	// 選択中の AI プロファイルが有効かを判定する。
	inline bool HasSelectedAiProfile(const MapEditorState& editor, const DefinitionStores& defs)
	{
		return 0 <= editor.selectedAiProfileIndex && editor.selectedAiProfileIndex < static_cast<int32>(defs.aiProfiles.size());
	}

	// 選択中の AI プロファイル参照を返す。
	inline AiProfileDef& SelectedAiProfile(MapEditorState& editor, DefinitionStores& defs)
	{
		editor.selectedAiProfileIndex = Clamp(editor.selectedAiProfileIndex, 0, Max(0, static_cast<int32>(defs.aiProfiles.size()) - 1));
		return defs.aiProfiles[editor.selectedAiProfileIndex];
	}

	// タグから AI Editor の選択状態を同期する。
	inline void SyncAiEditorSelectionFromTag(MapEditorState& editor, const DefinitionStores& defs)
	{
		if (defs.aiProfiles.isEmpty())
		{
			editor.selectedAiProfileIndex = 0;
			editor.aiProfileSelectionInitialized = true;
			return;
		}

		if (editor.aiProfileSelectionInitialized)
		{
			return;
		}

		editor.selectedAiProfileTag = editor.selectedAiProfileTag.lowercased();
		if (!editor.selectedAiProfileTag.isEmpty() && defs.aiProfileByTag.contains(editor.selectedAiProfileTag))
		{
			editor.selectedAiProfileIndex = static_cast<int32>(defs.aiProfileByTag.at(editor.selectedAiProfileTag));
			editor.aiProfileSelectionInitialized = true;
			return;
		}

		editor.selectedAiProfileIndex = Clamp(editor.selectedAiProfileIndex, 0, static_cast<int32>(defs.aiProfiles.size()) - 1);
		editor.aiProfileSelectionInitialized = true;
	}

	// 選択中 AI プロファイルのインデックスを更新する。
	inline void SetSelectedAiProfile(MapEditorState& editor, const AiProfileDef& profile, int32 profileIndex)
	{
		editor.selectedAiProfileIndex = profileIndex;
		editor.aiProfileSelectionInitialized = true;
	}

	// 選択中 AI プロファイルのタグを editor へ反映する。
	inline void ApplySelectedAiProfileTag(MapEditorState& editor, const DefinitionStores& defs)
	{
		if (!HasSelectedAiProfile(editor, defs))
		{
			return;
		}

		editor.selectedAiProfileTag = defs.aiProfiles[editor.selectedAiProfileIndex].tag.lowercased();
	}

	// 基本パラメータ行の差分を AI プロファイルへ適用する。
	inline void ApplyAiEditorDelta(AiProfileDef& profile, int32 rowIndex, double delta)
	{
		switch (rowIndex)
		{
		case 0:
			profile.openingDelaySec = Max(0.0, profile.openingDelaySec + delta);
			break;
		case 1:
			profile.spawnIntervalSec = Max(0.25, profile.spawnIntervalSec + delta);
			break;
		case 2:
			profile.attackWaveIntervalSec = Max(1.0, profile.attackWaveIntervalSec + delta);
			break;
		case 3:
			profile.aggression = Clamp(profile.aggression + delta * 0.01, 0.0, 1.0);
			break;
		case 4:
			profile.economyFocus = Clamp(profile.economyFocus + delta * 0.01, 0.0, 1.0);
			break;
		case 5:
			profile.defenseFocus = Clamp(profile.defenseFocus + delta * 0.01, 0.0, 1.0);
			break;
		case 6:
			profile.attackGroupSize = Max(1, profile.attackGroupSize + static_cast<int32>(delta));
			break;
		case 7:
			profile.maxArmySize = Max(1, profile.maxArmySize + static_cast<int32>(delta));
			break;
		case 8:
			profile.battleTimeLimitSec = Max(60.0, profile.battleTimeLimitSec + delta * 60.0);
			break;
		case 9:
			profile.techFocus = Clamp(profile.techFocus + delta * 0.01, 0.0, 1.0);
			break;
		case 10:
			profile.retreatHpRatio = Clamp(profile.retreatHpRatio + delta * 0.01, 0.0, 1.0);
			break;
		case 11:
			profile.resourceMultiplier = Max(0.0, profile.resourceMultiplier + delta * 0.01);
			break;
		}
		if (profile.maxArmySize < profile.attackGroupSize)
		{
			profile.maxArmySize = profile.attackGroupSize;
		}
	}

	// 詳細パネルの最大スクロール量を返す。
	inline double AiEditorDetailMaxScroll(const AiProfileDef& profile)
	{
		const int32 rowCount = 18
			+ static_cast<int32>(profile.initialUnits.size())
			+ static_cast<int32>(profile.unitWeights.size())
			+ static_cast<int32>(profile.buildPriorities.size())
			+ static_cast<int32>(profile.targetPriority.size());
		return Max(0.0, rowCount * 42.0 - AiEditorDetailRect().h + 126.0);
	}
}
