#pragma once

#include "AudioManager.h"
#include "GameData.h"
#include "MenuButtonUi.h"
#include "RewardUiLayout.h"
#include "SceneTransition.h"

class RewardEditorScene : public SceneBase
{
private:
	struct EditableElement
	{
		String name;
		RectF RewardUiLayout::* rectMember = nullptr;
		Vec2 RewardUiLayout::* pointMember = nullptr;
	};

public:
	explicit RewardEditorScene(const SceneBase::InitData& init)
		: SceneBase{ init }
		, m_layout{ RewardUi::GetRewardUiLayout() }
	{
		PlayMenuBgm();
		resetPreviewState();
	}

	void update() override
	{
		auto& data = getData();
		if (UpdateSceneTransition(data, [this](const String& sceneName)
		{
			changeScene(sceneName);
		}))
		{
			return;
		}

		handleTopButtonInput();
		handleSelectionInput();
		handleDragInput();

		if (m_selectedCardIndex)
		{
			m_selectionEffectTime += Scene::DeltaTime();
			if (m_selectionEffectTime >= SelectionEffectDuration)
			{
				finishSelectedCardPreview();
			}
			return;
		}

		if (!isCursorOnControlPanel())
		{
			if (Key1.down())
			{
				choosePreviewCard(0);
				return;
			}
			if (Key2.down())
			{
				choosePreviewCard(1);
				return;
			}
			if (Key3.down())
			{
				choosePreviewCard(2);
				return;
			}
		}
	}

	void draw() const override
	{
		RewardUi::DrawRewardSelectionScreen(getData(), m_previewRunState, m_selectedCardIndex, m_selectionEffectTime, m_layout);
		drawPreviewGrid();
		drawSelectionHighlight();
		drawPanels();
		DrawSceneTransitionOverlay(getData());
	}

private:
	static constexpr double SelectionEffectDuration = 0.42;
	static constexpr int32 EditorGridCellSize = 8;
	static constexpr int32 EditorGridMajorLineSpan = 4;
	RewardUiLayout m_layout;
	RunState m_previewRunState;
	Optional<int32> m_selectedCardIndex;
	double m_selectionEffectTime = 0.0;
	String m_statusMessage = U"Reward editor ready";
	bool m_hasUnsavedChanges = false;
	int32 m_selectedElementIndex = 0;
	Optional<Vec2> m_dragOffset;
	bool m_isDraggingPoint = false;

	void initializePreviewRun()
	{
		m_previewRunState = {};
		m_previewRunState.isActive = true;
		m_previewRunState.currentBattleIndex = 0;
		m_previewRunState.totalBattles = 4;
		m_previewRunState.mapProgressionBattles = { 4, 2, 3 };
		m_selectedCardIndex.reset();
		m_selectionEffectTime = 0.0;
	}

	void rebuildPreviewChoices(const String& statusMessage)
	{
		auto& data = getData();
		if (data.rewardCards.isEmpty())
		{
			m_previewRunState.pendingRewardCardIds.clear();
			m_statusMessage = Localization::GetText(U"reward_editor.no_cards", U"表示できる報酬カードがありません", U"No reward cards available for preview");
			return;
		}

		m_previewRunState.pendingRewardCardIds = BuildRewardCardChoices(m_previewRunState, data.rewardCards);
		if (m_previewRunState.pendingRewardCardIds.isEmpty() && !m_previewRunState.selectedCardIds.isEmpty())
		{
			m_previewRunState.selectedCardIds.clear();
			m_previewRunState.pendingRewardCardIds = BuildRewardCardChoices(m_previewRunState, data.rewardCards);
		}

		m_statusMessage = statusMessage;
	}

	void resetPreviewState()
	{
		initializePreviewRun();
		rebuildPreviewChoices(Localization::GetText(U"reward_editor.status_reset_preview", U"プレビュー状態を初期化しました", U"Reset preview state"));
	}

	void choosePreviewCard(const int32 index)
	{
		if (m_selectedCardIndex || (index < 0) || (index >= static_cast<int32>(m_previewRunState.pendingRewardCardIds.size())))
		{
			return;
		}

		m_selectedCardIndex = index;
		m_selectionEffectTime = 0.0;
	}

	void finishSelectedCardPreview()
	{
		auto& data = getData();
		if (!m_selectedCardIndex)
		{
			return;
		}

		const int32 index = *m_selectedCardIndex;
		m_selectedCardIndex.reset();
		m_selectionEffectTime = 0.0;
		if ((index < 0) || (index >= static_cast<int32>(m_previewRunState.pendingRewardCardIds.size())))
		{
			return;
		}

		ApplyRewardCardChoice(m_previewRunState, data.rewardCards, m_previewRunState.pendingRewardCardIds[index]);
		m_previewRunState.currentBattleIndex = ((m_previewRunState.currentBattleIndex + 1) % Max(m_previewRunState.totalBattles, 1));
		rebuildPreviewChoices(Localization::GetText(U"reward_editor.status_applied", U"報酬選択をプレビューへ反映しました", U"Applied reward choice to preview"));
	}

	void requestReturnToTitle()
	{
		RequestSceneTransition(getData(), U"Title", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
	}

	void handleTopButtonInput()
	{
		if (isButtonClicked(getTopButtonRect(0)))
		{
			if (RewardUi::SaveRewardUiLayout(m_layout))
			{
				m_hasUnsavedChanges = false;
				m_statusMessage = Localization::GetText(U"reward_editor.status_saved_layout", U"リワード画面レイアウトを保存しました", U"Saved reward screen layout");
			}
			else
			{
				m_statusMessage = Localization::GetText(U"reward_editor.status_save_failed", U"リワード画面レイアウトの保存に失敗しました", U"Failed to save reward screen layout");
			}
			return;
		}

		if (isButtonClicked(getTopButtonRect(1)))
		{
			m_layout = RewardUi::ReloadRewardUiLayout();
			m_hasUnsavedChanges = false;
			m_statusMessage = Localization::GetText(U"reward_editor.status_reloaded_layout", U"リワード画面レイアウトを再読み込みしました", U"Reloaded reward screen layout");
			return;
		}

		if (isButtonClicked(getTopButtonRect(2)))
		{
			m_layout = RewardUi::MakeDefaultRewardUiLayout();
			m_hasUnsavedChanges = true;
			m_statusMessage = Localization::GetText(U"reward_editor.status_reset_layout", U"リワード画面レイアウトを既定値へ戻しました", U"Reset reward screen layout to defaults");
			return;
		}

		if (isButtonClicked(getTopButtonRect(3)) || KeyR.down())
		{
			rebuildPreviewChoices(Localization::GetText(U"reward_editor.status_rerolled", U"報酬候補を再抽選しました", U"Rerolled reward choices"));
			return;
		}

		if (isButtonClicked(getTopButtonRect(4)) || KeyC.down())
		{
			resetPreviewState();
			return;
		}

		if (isButtonClicked(getTopButtonRect(5)) || KeyEscape.down())
		{
			requestReturnToTitle();
		}
	}

	void handleSelectionInput()
	{
		const auto& elements = getEditableElements();
		for (int32 i = 0; i < static_cast<int32>(elements.size()); ++i)
		{
			if (isButtonClicked(getSelectionRowRect(i)))
			{
				m_selectedElementIndex = i;
				return;
			}
		}
	}

	void handleDragInput()
	{
		if (isCursorOnControlPanel())
		{
			if (!MouseL.pressed())
			{
				m_dragOffset.reset();
			}
			return;
		}

		if (Vec2* point = getSelectedPoint())
		{
			const Circle handle{ *point, 10.0 };
			if (!m_dragOffset && MouseL.down() && handle.mouseOver())
			{
				m_dragOffset = (Cursor::PosF() - *point);
				m_isDraggingPoint = true;
			}

			if (m_dragOffset)
			{
				if (MouseL.pressed())
				{
					*point = snapToGrid(Cursor::PosF() - *m_dragOffset);
					m_hasUnsavedChanges = true;
					m_statusMessage = Localization::GetText(U"reward_editor.status_dragging", U"要素を移動中...", U"Moving selected element...");
				}
				else
				{
					m_dragOffset.reset();
				}
			}
			return;
		}

		if (RectF* rect = getSelectedRect())
		{
			if (!m_dragOffset && MouseL.down() && rect->mouseOver())
			{
				m_dragOffset = (Cursor::PosF() - rect->pos);
				m_isDraggingPoint = false;
			}

			if (m_dragOffset)
			{
				if (MouseL.pressed())
				{
					const Vec2 snappedPos = snapToGrid(Cursor::PosF() - *m_dragOffset);
					rect->x = snappedPos.x;
					rect->y = snappedPos.y;
					m_hasUnsavedChanges = true;
					m_statusMessage = Localization::GetText(U"reward_editor.status_dragging", U"要素を移動中...", U"Moving selected element...");
				}
				else
				{
					m_dragOffset.reset();
				}
			}
		}
	}

	[[nodiscard]] RectF* getSelectedRect()
	{
		const EditableElement& element = getEditableElements()[m_selectedElementIndex];
		return element.rectMember ? &(m_layout.*(element.rectMember)) : nullptr;
	}

	[[nodiscard]] const RectF* getSelectedRect() const
	{
		const EditableElement& element = getEditableElements()[m_selectedElementIndex];
		return element.rectMember ? &(m_layout.*(element.rectMember)) : nullptr;
	}

	[[nodiscard]] Vec2* getSelectedPoint()
	{
		const EditableElement& element = getEditableElements()[m_selectedElementIndex];
		return element.pointMember ? &(m_layout.*(element.pointMember)) : nullptr;
	}

	[[nodiscard]] const Vec2* getSelectedPoint() const
	{
		const EditableElement& element = getEditableElements()[m_selectedElementIndex];
		return element.pointMember ? &(m_layout.*(element.pointMember)) : nullptr;
	}

	void drawPanels() const
	{
		const RectF panel = getLeftPanelRect();
		panel.draw(ColorF{ 0.05, 0.07, 0.09, 0.96 });
		panel.drawFrame(2.0, ColorF{ 0.42, 0.60, 0.92 });

		const auto& data = getData();
		data.uiFont(Localization::GetText(U"reward_editor.panel_title", U"Reward UI Editor", U"Reward UI Editor")).draw(panel.x + 16, panel.y + 14, Palette::White);
		data.smallFont(m_hasUnsavedChanges
			? Localization::GetText(U"reward_editor.unsaved_changes", U"未保存の変更があります", U"Unsaved changes")
			: Localization::GetText(U"reward_editor.saved_state", U"保存済みレイアウト", U"Layout saved"))
			.draw(panel.x + 16, panel.y + 46, ColorF{ 0.82, 0.88, 0.96 });

		drawButton(getTopButtonRect(0), Localization::GetText(U"reward_editor.save_layout_button", U"Save Layout", U"Save Layout"), data.smallFont, true);
		drawButton(getTopButtonRect(1), Localization::GetText(U"reward_editor.reload_layout_button", U"Reload Layout", U"Reload Layout"), data.smallFont);
		drawButton(getTopButtonRect(2), Localization::GetText(U"reward_editor.reset_layout_button", U"Reset Layout", U"Reset Layout"), data.smallFont);
		drawButton(getTopButtonRect(3), Localization::GetText(U"reward_editor.reroll_button", U"再抽選", U"Reroll"), data.smallFont);
		drawButton(getTopButtonRect(4), Localization::GetText(U"reward_editor.reset_preview_button", U"プレビュー初期化", U"Reset Preview"), data.smallFont);
		drawButton(getTopButtonRect(5), Localization::GetText(U"reward_editor.back_button", U"タイトルへ戻る", U"Back to Title"), data.smallFont);

		data.smallFont(Localization::GetText(U"reward_editor.element_list_title", U"編集対象", U"Editable Elements")).draw(panel.x + 16, panel.y + 236, Palette::White);
		const auto& elements = getEditableElements();
		for (int32 i = 0; i < static_cast<int32>(elements.size()); ++i)
		{
			drawButton(getSelectionRowRect(i), elements[i].name, data.smallFont, (i == m_selectedElementIndex));
		}

		data.smallFont(Localization::GetText(U"reward_editor.editor_hint", U"左の一覧で要素を選択し、画面上でドラッグして移動", U"Select an element on the left and drag it in the preview"))
			.draw(panel.x + 16, panel.bottomY() - 74, ColorF{ 0.88, 0.92, 1.0 });
		data.smallFont(Localization::GetText(U"reward_editor.grid_hint", U"8px グリッド表示 / ドラッグ中は升目にスナップ", U"8px grid shown / dragging snaps to cells"))
			.draw(panel.x + 16, panel.bottomY() - 60, ColorF{ 0.84, 0.90, 0.98 });
		data.smallFont(Localization::FormatText(U"reward_editor.selected_cards", U"選択済みカード: {0}", U"Selected cards: {0}", m_previewRunState.selectedCardIds.size()))
			.draw(panel.x + 16, panel.bottomY() - 38, ColorF{ 0.88, 0.93, 1.0 });
		data.smallFont(m_statusMessage).draw(panel.x + 16, panel.bottomY() - 20, ColorF{ 0.84, 0.90, 0.98 });
	}

	void drawPreviewGrid() const
	{
		const RectF gridRect = getPreviewGridRect();
		for (double x = gridRect.x; x <= gridRect.rightX(); x += EditorGridCellSize)
		{
			const int32 cellIndex = static_cast<int32>(Math::Round((x - gridRect.x) / EditorGridCellSize));
			const bool isMajorLine = ((Abs(cellIndex) % EditorGridMajorLineSpan) == 0);
			Line{ x, gridRect.y, x, gridRect.bottomY() }.draw(1.0, isMajorLine ? ColorF{ 0.42, 0.55, 0.78, 0.16 } : ColorF{ 0.32, 0.40, 0.56, 0.08 });
		}

		for (double y = gridRect.y; y <= gridRect.bottomY(); y += EditorGridCellSize)
		{
			const int32 cellIndex = static_cast<int32>(Math::Round((y - gridRect.y) / EditorGridCellSize));
			const bool isMajorLine = ((Abs(cellIndex) % EditorGridMajorLineSpan) == 0);
			Line{ gridRect.x, y, gridRect.rightX(), y }.draw(1.0, isMajorLine ? ColorF{ 0.42, 0.55, 0.78, 0.16 } : ColorF{ 0.32, 0.40, 0.56, 0.08 });
		}
	}

	void drawSelectionHighlight() const
	{
		const RectF* rect = getSelectedRect();
		if (rect)
		{
			rect->stretched(4.0).drawFrame(3.0, 0, ColorF{ 0.98, 0.90, 0.42, 0.95 });
			return;
		}

		const Vec2* point = getSelectedPoint();
		if (point)
		{
			Circle{ *point, 9.0 }.draw(ColorF{ 0.98, 0.90, 0.42, 0.26 });
			Circle{ *point, 5.0 }.drawFrame(2.0, ColorF{ 0.98, 0.90, 0.42, 0.95 });
			Line{ point->movedBy(-12, 0), point->movedBy(12, 0) }.draw(2.0, ColorF{ 0.98, 0.90, 0.42, 0.95 });
			Line{ point->movedBy(0, -12), point->movedBy(0, 12) }.draw(2.0, ColorF{ 0.98, 0.90, 0.42, 0.95 });
		}
	}

	[[nodiscard]] bool isCursorOnControlPanel() const
	{
		return getLeftPanelRect().mouseOver();
	}

	[[nodiscard]] static bool isButtonClicked(const RectF& rect)
	{
		return IsMenuButtonClicked(rect);
	}

	static void drawButton(const RectF& rect, const String& label, const Font& font, const bool selected = false)
	{
		DrawMenuButton(rect, label, font, selected);
	}

	[[nodiscard]] static RectF getLeftPanelRect()
	{
		return RectF{ 12, 12, 300, Scene::Height() - 24 };
	}

	[[nodiscard]] static RectF getPreviewGridRect()
	{
		const RectF panel = getLeftPanelRect();
		return RectF{ panel.rightX() + 12, 12, Scene::Width() - panel.rightX() - 24, Scene::Height() - 24 };
	}

	[[nodiscard]] static double snapToGridValue(const double value)
	{
		return (Math::Round(value / EditorGridCellSize) * EditorGridCellSize);
	}

	[[nodiscard]] static Vec2 snapToGrid(const Vec2& value)
	{
		return Vec2{ snapToGridValue(value.x), snapToGridValue(value.y) };
	}

	[[nodiscard]] static RectF getTopButtonRect(const int32 index)
	{
		const RectF panel = getLeftPanelRect();
		return RectF{ panel.x + 16, panel.y + 76 + (index * 30), panel.w - 32, 26 };
	}

	[[nodiscard]] static RectF getSelectionRowRect(const int32 index)
	{
		const RectF panel = getLeftPanelRect();
		return RectF{ panel.x + 16, panel.y + 262 + (index * 24), panel.w - 32, 22 };
	}

	[[nodiscard]] static const Array<EditableElement>& getEditableElements()
	{
		static const Array<EditableElement> elements =
		{
			{ U"Title", nullptr, &RewardUiLayout::titlePos },
			{ U"Subtitle", nullptr, &RewardUiLayout::subtitlePos },
			{ U"Hint", nullptr, &RewardUiLayout::hintPos },
			{ U"Card 1", &RewardUiLayout::card1Rect, nullptr },
			{ U"Card 2", &RewardUiLayout::card2Rect, nullptr },
			{ U"Card 3", &RewardUiLayout::card3Rect, nullptr },
			{ U"Acquired Title", nullptr, &RewardUiLayout::acquiredLabelPos },
			{ U"Acquired Name", nullptr, &RewardUiLayout::acquiredCardNamePos },
		};
		return elements;
	}
};
