# include "SkyAppUiParameterEditorInternal.hpp"
# include "SkyAppUiPanelFrameInternal.hpp"
# include "MainSettings.hpp"
# include "MainUi.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
   namespace
	{
		enum class MillEditorCategory
		{
			Attack,
			Suppression,
		};

		inline constexpr int32 EditorIconButtonSize = 24;
	}

	void DrawMillStatusEditor(const SkyAppPanels& panels,
		MapData& mapData,
		const size_t selectedMillIndex,
		const FilePathView path,
		TimedMessage& mapDataMessage)
	{
		using namespace UiParameterEditorDetail;

		if (mapData.placedModels.size() <= selectedMillIndex)
		{
			return;
		}

		PlacedModel& mill = mapData.placedModels[selectedMillIndex];
		const Rect panel = panels.millStatusEditor;
       const Rect headerRect{ panel.x, panel.y, panel.w, 56 };
		const Rect footerRect{ panel.x, (panel.bottomY() - 50), panel.w, 50 };
		const Rect contentPanel{ (panel.x + 8), (headerRect.bottomY() + 6), (panel.w - 16), (footerRect.y - headerRect.bottomY() - 12) };
		static MillEditorCategory currentCategory = MillEditorCategory::Attack;

     UiInternal::DrawNinePatchPanelFrame(panel, U"Mill Status Editor", ColorF{ 0.97, 0.96, 0.94 });
		headerRect.draw(ColorF{ 0.96, 0.95, 0.92, 0.84 });
		footerRect.draw(ColorF{ 0.95, 0.94, 0.91, 0.82 });
		Rect{ panel.x, (headerRect.bottomY() - 1), panel.w, 1 }.draw(ColorF{ 0.78, 0.76, 0.70 });
		Rect{ panel.x, footerRect.y, panel.w, 1 }.draw(ColorF{ 0.78, 0.76, 0.70 });
		SimpleGUI::GetFont()(U"Pos: {:.1f}, {:.1f}, {:.1f}"_fmt(mill.position.x, mill.position.y, mill.position.z)).draw((panel.x + 16), (panel.y + 32), UiInternal::EditorTextOnLightSecondaryColor());
		SimpleGUI::GetFont()(U"Owner: {}"_fmt(ToString(mill.ownerTeam))).draw((panel.x + 220), (panel.y + 32), UiInternal::EditorTextOnLightSecondaryColor());

		mill.attackRange = Clamp(mill.attackRange, 1.0, 20.0);
		mill.attackDamage = Clamp(mill.attackDamage, 1.0, 80.0);
		mill.attackInterval = Clamp(mill.attackInterval, 0.2, 5.0);
      mill.attackTargetCount = Clamp(mill.attackTargetCount, 1, 6);
		mill.suppressionDuration = Clamp(mill.suppressionDuration, 0.2, 10.0);
		mill.suppressionMoveSpeedMultiplier = Clamp(mill.suppressionMoveSpeedMultiplier, 0.1, 1.0);
		mill.suppressionAttackDamageMultiplier = Clamp(mill.suppressionAttackDamageMultiplier, 0.1, 1.0);
		mill.suppressionAttackIntervalMultiplier = Clamp(mill.suppressionAttackIntervalMultiplier, 1.0, 10.0);

		const auto drawCategoryButton = [&](const Rect& rect, const MillEditorCategory category, const StringView label)
			{
				const bool selected = (currentCategory == category);
				const bool hovered = rect.mouseOver();
				rect.draw(selected ? ColorF{ 0.33, 0.53, 0.82 } : (hovered ? ColorF{ 0.89, 0.92, 0.97 } : ColorF{ 0.84, 0.87, 0.92 }))
					.drawFrame(1, 0, selected ? ColorF{ 0.20, 0.32, 0.52 } : ColorF{ 0.40, 0.45, 0.52 });
				SimpleGUI::GetFont()(label).drawAt(rect.center(), selected ? ColorF{ 0.98 } : ColorF{ 0.16 });
				if (hovered && MouseL.down())
				{
					currentCategory = category;
				}
			};

		const Rect attackCategoryButton{ (contentPanel.x + 8), (contentPanel.y + 8), 136, 30 };
		const Rect suppressionCategoryButton{ (contentPanel.rightX() - 144), (contentPanel.y + 8), 136, 30 };
		drawCategoryButton(attackCategoryButton, MillEditorCategory::Attack, U"攻撃");
		drawCategoryButton(suppressionCategoryButton, MillEditorCategory::Suppression, U"制圧");
     SimpleGUI::GetFont()((currentCategory == MillEditorCategory::Attack) ? U"攻撃パラメータ" : U"制圧パラメータ")
			.draw((contentPanel.x + 12), (contentPanel.y + 46), UiInternal::EditorTextOnLightSecondaryColor());

       const auto drawParameterRow = [&](const double top, const int32 sliderId, double& value, const MillParameterEditorSpec& spec)
			{
                DrawMillParameterEditorCard(Rect{ (contentPanel.x + 4), static_cast<int32>(top), (contentPanel.w - 8), 92 }, sliderId, value, spec);
			};

     const double rowTop = (contentPanel.y + 70);
		const double rowStep = 88.0;
		if (currentCategory == MillEditorCategory::Attack)
		{
          double attackTargetCount = mill.attackTargetCount;
			drawParameterRow((rowTop + rowStep * 0), 0, attackTargetCount, MillParameterEditorSpec{ U"Targets", U"", 1.0, 6.0, 1.0, 1.0, 1.0, 1.0, 0 });
			mill.attackTargetCount = Clamp(static_cast<int32>(Math::Round(attackTargetCount)), 1, 6);

			drawParameterRow((rowTop + rowStep * 1), 1, mill.attackRange, MillParameterEditorSpec{ U"Range", U"", 1.0, 20.0, 0.5, 2.0, 5.0, 0.1, 1 });
			drawParameterRow((rowTop + rowStep * 2), 2, mill.attackDamage, MillParameterEditorSpec{ U"Damage", U"", 1.0, 80.0, 1.0, 5.0, 10.0, 1.0, 0 });
			drawParameterRow((rowTop + rowStep * 3), 3, mill.attackInterval, MillParameterEditorSpec{ U"Interval", U"s", 0.2, 5.0, 0.1, 0.25, 0.5, 0.05, 2 });
		}
		else
		{
            drawParameterRow((rowTop + rowStep * 0), 10, mill.suppressionDuration, MillParameterEditorSpec{ U"Suppress Time", U"s", 0.2, 10.0, 0.1, 0.5, 1.0, 0.05, 2 });
			drawParameterRow((rowTop + rowStep * 1), 11, mill.suppressionMoveSpeedMultiplier, MillParameterEditorSpec{ U"Move Rate", U"x", 0.1, 1.0, 0.05, 0.1, 0.2, 0.05, 2 });
			drawParameterRow((rowTop + rowStep * 2), 12, mill.suppressionAttackDamageMultiplier, MillParameterEditorSpec{ U"Atk Damage Rate", U"x", 0.1, 1.0, 0.05, 0.1, 0.2, 0.05, 2 });
			drawParameterRow((rowTop + rowStep * 3), 13, mill.suppressionAttackIntervalMultiplier, MillParameterEditorSpec{ U"Atk Interval Rate", U"x", 1.0, 10.0, 0.1, 0.5, 1.0, 0.05, 2 });
		}

        const Rect resetButton{ (footerRect.x + 12), (footerRect.y + 9), 136, 32 };
		const Rect editorTextColorsButton{ (footerRect.rightX() - EditorIconButtonSize - 12), (footerRect.y + 13), EditorIconButtonSize, EditorIconButtonSize };
		const Rect saveButton{ (editorTextColorsButton.x - 144), (footerRect.y + 9), 136, 32 };

		if (DrawTextButton(resetButton, U"推奨値に戻す"))
		{
			mill.attackRange = MillDefenseRange;
			mill.attackDamage = MillDefenseDamage;
			mill.attackInterval = MillDefenseInterval;
         mill.attackTargetCount = MillDefenseTargetCount;
			mill.suppressionDuration = MillSuppressionDuration;
			mill.suppressionMoveSpeedMultiplier = MillSuppressionMoveSpeedMultiplier;
			mill.suppressionAttackDamageMultiplier = MillSuppressionAttackDamageMultiplier;
			mill.suppressionAttackIntervalMultiplier = MillSuppressionAttackIntervalMultiplier;
			mapDataMessage.show(U"Mill ステータスを既定値に戻しました", 3.0);
		}

       if (DrawTextButton(saveButton, U"TOML保存"))
		{
			mapDataMessage.show(SaveMapData(mapData, path) ? U"Mill ステータスを保存" : U"Mill ステータス保存失敗", 3.0);
		}

		if (UiInternal::DrawEditorIconButton(editorTextColorsButton, U"色"))
		{
			UiInternal::OpenSharedEditorTextColorEditor();
		}
	}
}
