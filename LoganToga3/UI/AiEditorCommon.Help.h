#pragma once
# include <Siv3D.hpp>
# include "AiEditorCommon.Layout.h"
# include "../Data/BattleAssetPaths.h"

namespace LT3
{
	// ヘルプアイコンの矩形を返す。
	inline RectF AiEditorHelpIconRect(const RectF& row)
	{
		return RectF{ row.x + row.w - 24.0, row.y + 8.0, 18.0, 18.0 };
	}

	// ヘルプポップアップの矩形を返す。
	inline RectF AiEditorHelpPopupRect(const RectF& iconRect)
	{
		const double w = 300.0;
		const double h = 42.0;
		const double offset = 8.0;
		const RectF detail = AiEditorDetailRect();
		const double x = Max(detail.x + 8.0, iconRect.x - w - offset);
		const double y = Clamp(iconRect.y - 10.0, detail.y + 8.0, detail.y + detail.h - h - 8.0);
		return RectF{ x, y, w, h };
	}

	// ヘルプアイコン用テクスチャを返す。
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

	// ヘルプアイコンを描画し、ホバー時の説明文を設定する。
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

	// 現在ホバー中のヘルプポップアップ矩形を返す。
	inline Optional<RectF> ResolveAiEditorHoveredHelpRect()
	{
		const RectF row{ Cursor::PosF().x - 9.0, Cursor::PosF().y - 9.0, 18.0, 18.0 };
		return AiEditorHelpPopupRect(row);
	}

	// 基本パラメータ行ごとのヘルプ文を返す。
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
			return U"戦闘の制限時間です。0 にはならず、時間切れでプレイヤー敗北になります。";
		case 9:
			return U"技術・上位戦力への比重です。今後の拡張を見据えた調整値です。";
		case 10:
			return U"このHP割合を下回ると退却判断に使う値です。";
		case 11:
			return U"AI側の資源獲得倍率です。高いほど展開が速くなります。";
		case 12:
			return U"資源消費なしで敵ユニットを生成できるかを切り替えます。";
		case 13:
			return U"会敵時に移動を優先して無視するか、移動を止めて戦闘後に目的地へ戻るかを選びます。";
		default:
			return U"";
		}
	}
}
