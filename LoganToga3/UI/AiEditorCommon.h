#pragma once
# include <Siv3D.hpp>
# include "../Data/Loaders/AiProfileDefLoader.h"
# include "../Data/BattleAssetPaths.h"
# include "MapEditorCoreTypes.h"
# include "MapEditorMapData.h"
# include "MapEditorToolbarLayoutRects.h"
# include "RectUiHelpers.h"

namespace LT3
{
	inline RectF AiEditorPanelRect()
	{
		return RectF{ 692.0, 72.0, 876.0, 610.0 };
	}

	inline RectF AiEditorListViewportRect()
	{
		const RectF panel = AiEditorPanelRect();
		return RectF{ panel.x + 20.0, panel.y + 58.0, 280.0, panel.h - 86.0 };
	}

	inline RectF AiEditorDetailRect()
	{
		const RectF panel = AiEditorPanelRect();
		return RectF{ panel.x + 320.0, panel.y + 58.0, panel.w - 340.0, panel.h - 86.0 };
	}

	inline RectF AiEditorProfileRowRect(const RectF& viewport, int32 index, double scroll)
	{
		return RectF{ viewport.x, viewport.y + index * 58.0 - scroll, viewport.w, 50.0 };
	}

	inline RectF AiEditorSaveRect()
	{
		const RectF panel = AiEditorPanelRect();
		return RectF{ panel.x + panel.w - 220.0, panel.y + 14.0, 96.0, 32.0 };
	}

	inline RectF AiEditorCloseRect()
	{
		const RectF panel = AiEditorPanelRect();
		return RectF{ panel.x + panel.w - 112.0, panel.y + 14.0, 88.0, 32.0 };
	}

	inline RectF AiEditorApplyRect()
	{
		const RectF panel = AiEditorPanelRect();
		return RectF{ panel.x + panel.w - 328.0, panel.y + 14.0, 96.0, 32.0 };
	}

	inline RectF AiEditorValueRowRect(int32 row, double scroll)
	{
		const RectF detail = AiEditorDetailRect();
		return RectF{ detail.x + 16.0, detail.y + 126.0 + row * 42.0 - scroll, detail.w - 32.0, 34.0 };
	}

	inline RectF AiEditorValueButtonRect(const RectF& row, int32 index)
	{
		const double w = 44.0;
		const double gap = 6.0;
		const double x = row.x + row.w - (w * 4.0 + gap * 3.0);
		return RectF{ x + index * (w + gap), row.y + 4.0, w, 26.0 };
	}

	inline RectF AiEditorHelpIconRect(const RectF& row)
	{
		return RectF{ row.x + 166.0, row.y + 6.0, 18.0, 18.0 };
	}

	inline Texture& AiEditorHelpIconTexture()
	{
		static bool loaded = false;
		static Texture texture;
		if (!loaded)
		{
			loaded = true;
			const FilePath path = ResolveSystemImagePath(U"hatena.png");
			if (FileSystem::Exists(path))
			{
				texture = Texture{ path };
			}
		}

		return texture;
	}

	inline void DrawAiEditorHelpIcon(const Font& uiFont, const RectF& row, StringView helpText, String& hoverHelp)
	{
		const RectF iconRect = AiEditorHelpIconRect(row);
		if (Texture& texture = AiEditorHelpIconTexture())
		{
			texture.resized(18, 18).draw(iconRect.pos);
		}
		else
		{
			uiFont(U"?").drawAt(14, iconRect.center(), Palette::White);
		}

		if (iconRect.mouseOver())
		{
			hoverHelp = String{ helpText };
		}
	}

	inline String AiEditorRowHelpText(int32 rowIndex)
	{
		switch (rowIndex)
		{
		case 0:
			return U"戦闘開始直後にAIが本格行動を始めるまでの待ち時間です。";
		case 1:
			return U"敵ユニット生成の基本間隔です。短いほど頻繁に補充します。";
		case 2:
			return U"攻撃波をまとめて出す間隔です。短いほど攻勢が激しくなります。";
		case 3:
			return U"前進・交戦を優先する度合いです。高いほど積極的に攻めます。";
		case 4:
			return U"資源確保や内政寄りの比重です。高いほど戦力増強を優先します。";
		case 5:
			return U"拠点防衛や守備維持の比重です。高いほど守り寄りになります。";
		case 6:
			return U"攻撃波としてまとまって進軍する最低部隊数です。";
		case 7:
			return U"同時に保持したい最大軍勢規模です。";
		case 8:
			return U"技術・上位戦力への比重です。今後の拡張を見据えた調整値です。";
		case 9:
			return U"このHP割合を下回ると退却判断に使う値です。";
		case 10:
			return U"AI側の資源獲得倍率です。高いほど展開が速くなります。";
		case 11:
			return U"資源消費なしで敵ユニットを生成できるかを切り替えます。";
		case 12:
			return U"会敵時に移動を優先して無視するか、移動を止めて戦闘後に目的地へ戻るかを選びます。";
		default:
			return U"";
		}
	}

	inline RectF AiEditorInlineButtonRect(const RectF& row, int32 index, int32 count)
	{
		const double w = 58.0;
		const double gap = 6.0;
		const double x = row.x + row.w - (w * count + gap * (count - 1));
		return RectF{ x + index * (w + gap), row.y + 4.0, w, 26.0 };
	}

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

	inline String NextAiContactBehavior(StringView value)
	{
		return (String{ value }.lowercased() == U"ignore") ? U"engage" : U"ignore";
	}

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

	inline String FindNextAiUnitWeightTag(const AiProfileDef& profile, const DefinitionStores& defs)
	{
		for (const auto& unit : defs.units)
		{
			const String unitTag = unit.unit_id.lowercased();
			if (unitTag.isEmpty())
			{
				continue;
			}

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

		return U"kouhei";
	}

	inline bool HasSelectedAiProfile(const MapEditorState& editor, const DefinitionStores& defs)
	{
		return 0 <= editor.selectedAiProfileIndex && editor.selectedAiProfileIndex < static_cast<int32>(defs.aiProfiles.size());
	}

	inline AiProfileDef& SelectedAiProfile(MapEditorState& editor, DefinitionStores& defs)
	{
		editor.selectedAiProfileIndex = Clamp(editor.selectedAiProfileIndex, 0, Max(0, static_cast<int32>(defs.aiProfiles.size()) - 1));
		return defs.aiProfiles[editor.selectedAiProfileIndex];
	}

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

	inline void SetSelectedAiProfile(MapEditorState& editor, const AiProfileDef& profile, int32 profileIndex)
	{
		editor.selectedAiProfileIndex = profileIndex;
		editor.aiProfileSelectionInitialized = true;
	}

	inline void ApplySelectedAiProfileTag(MapEditorState& editor, const DefinitionStores& defs)
	{
		if (!HasSelectedAiProfile(editor, defs))
		{
			return;
		}

		editor.selectedAiProfileTag = defs.aiProfiles[editor.selectedAiProfileIndex].tag.lowercased();
	}

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
			profile.techFocus = Clamp(profile.techFocus + delta * 0.01, 0.0, 1.0);
			break;
		case 9:
			profile.retreatHpRatio = Clamp(profile.retreatHpRatio + delta * 0.01, 0.0, 1.0);
			break;
		case 10:
			profile.resourceMultiplier = Max(0.0, profile.resourceMultiplier + delta * 0.01);
			break;
		}
		if (profile.maxArmySize < profile.attackGroupSize)
		{
			profile.maxArmySize = profile.attackGroupSize;
		}
	}

	inline double AiEditorDetailMaxScroll(const AiProfileDef& profile)
	{
		const int32 rowCount = 15 + static_cast<int32>(profile.unitWeights.size()) + static_cast<int32>(profile.targetPriority.size());
		return Max(0.0, rowCount * 42.0 - AiEditorDetailRect().h + 126.0);
	}
}
