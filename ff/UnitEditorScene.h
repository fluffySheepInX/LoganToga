# pragma once
# include "AppState.h"

class UnitEditorScene : public App::Scene
{
public:
	using InitData = App::Scene::InitData;

	UnitEditorScene(const InitData& init);

	void update() override;
	void draw() const override;

private:
	enum class PendingAction
	{
		None,
		BackToFormation,
		SwitchUnit,
     SwitchEnemy,
		ReloadFromDisk,
		ResetToDefault,
	};

	bool HandleUnitSelection();
	bool HandlePrimaryActions();
	bool HandleFieldAdjustment();
	void HandleKeyboardShortcuts();
	bool HandlePendingActionConfirmation();
   bool RequestAction(PendingAction action, Optional<ff::UnitId> targetUnit = none, Optional<ff::EnemyKind> targetEnemy = none);
	void CommitPendingAction();
	void ClearPendingAction();
	void LoadUnit(ff::UnitId unitId);
   void LoadEnemy(ff::EnemyKind enemyKind);
	void SaveCurrentDefinition();

	void DrawUnitList() const;
	void DrawPreviewPanel() const;
	void DrawEditorPanel() const;
	void DrawInfoPanel(const ff::UnitDefinition& loadedDefinition, const ff::UnitDefinition& normalizedDefinition) const;
    void DrawInfoPanel(const ff::EnemyDefinition& loadedDefinition, const ff::EnemyDefinition& normalizedDefinition) const;
 void DrawStatusBar() const;
	void DrawBottomButtons() const;
	void DrawIconChip(const RectF& rect, const String& icon, const ColorF& fillColor, const ColorF& frameColor, const ColorF& iconColor = Palette::White) const;
	void DrawLabeledValue(const RectF& iconRect, const String& icon, const Vec2& pos, const String& value, const ColorF& textColor) const;
	void DrawFieldIcon(size_t index, const RectF& rect) const;
	void DrawColorChannelIcon(size_t index, const RectF& rect) const;
	void DrawHoverTooltip() const;
	[[nodiscard]] Optional<String> GetHoveredTooltipText() const;
	void DrawPendingActionDialog() const;

	[[nodiscard]] RectF GetUnitButton(size_t index) const;
	[[nodiscard]] RectF GetTopHelpIcon() const;
	[[nodiscard]] RectF GetUnitPanelHeaderIcon() const;
	[[nodiscard]] RectF GetPreviewPanelHeaderIcon() const;
	[[nodiscard]] RectF GetEditPanelHeaderIcon() const;
	[[nodiscard]] RectF GetInfoPanelHeaderIcon() const;
	[[nodiscard]] RectF GetUnitListHelpIcon() const;
	[[nodiscard]] RectF GetPreviewLineIcon(size_t index) const;
	[[nodiscard]] RectF GetLabelFieldIcon() const;
	[[nodiscard]] RectF GetRoleFieldIcon() const;
	[[nodiscard]] RectF GetFieldRow(size_t index) const;
	[[nodiscard]] RectF GetFieldIconRect(size_t index) const;
	[[nodiscard]] RectF GetDecreaseButton(size_t index) const;
	[[nodiscard]] RectF GetIncreaseButton(size_t index) const;
	[[nodiscard]] RectF GetColorRow(size_t index) const;
	[[nodiscard]] RectF GetColorIconRect(size_t index) const;
	[[nodiscard]] RectF GetColorDecreaseButton(size_t index) const;
	[[nodiscard]] RectF GetColorIncreaseButton(size_t index) const;
	[[nodiscard]] RectF GetColorPreview() const;
	[[nodiscard]] RectF GetColorPreviewIcon() const;
	[[nodiscard]] RectF GetResetButton() const;
	[[nodiscard]] RectF GetReloadButton() const;
	[[nodiscard]] RectF GetSaveButton() const;
	[[nodiscard]] RectF GetBackButton() const;
	[[nodiscard]] RectF GetDialogConfirmButton() const;
	[[nodiscard]] RectF GetDialogCancelButton() const;
	[[nodiscard]] RectF GetInfoLineIcon(size_t index) const;
	[[nodiscard]] RectF GetKeyboardHelpIcon() const;
	[[nodiscard]] RectF GetMetricIcon(size_t index) const;
	[[nodiscard]] RectF GetChangeListIcon() const;
	[[nodiscard]] StringView GetFieldLabel(size_t index) const;
	[[nodiscard]] StringView GetColorLabel(size_t index) const;
	[[nodiscard]] String GetFieldValue(size_t index) const;
	[[nodiscard]] String GetColorValue(size_t index) const;

	void AdjustValue(size_t index, double direction);
	void AdjustColor(size_t index, double direction);
	void SyncEditorsFromDefinition();
	void SyncNumericEditorsFromDefinition();
	void ApplyEditorTextsToDefinition();
	[[nodiscard]] bool ApplyNumericEditor(size_t index);
	[[nodiscard]] bool HasUnsavedChanges() const;
	[[nodiscard]] bool HasPendingTextStateChanges() const;
	[[nodiscard]] bool DoesNumericEditorMatch(size_t index) const;
	[[nodiscard]] bool IsDirty() const;
	[[nodiscard]] bool IsFieldChanged(size_t index) const;
	[[nodiscard]] ff::UnitDefinition GetNormalizedEditingDefinition() const;
    [[nodiscard]] ff::EnemyDefinition GetNormalizedEditingEnemyDefinition() const;
	[[nodiscard]] Array<String> BuildChangeLines(const ff::UnitDefinition& loadedDefinition, const ff::UnitDefinition& editingDefinition) const;
   [[nodiscard]] Array<String> BuildChangeLines(const ff::EnemyDefinition& loadedDefinition, const ff::EnemyDefinition& editingDefinition) const;
	[[nodiscard]] static String FormatColor(const ColorF& color);
	[[nodiscard]] String GetPendingActionMessage() const;
	[[nodiscard]] static String FormatDefinitionSummary(const ff::UnitDefinition& definition);
  [[nodiscard]] static String FormatDefinitionSummary(const ff::EnemyDefinition& definition);
	[[nodiscard]] static bool AreSameDefinition(const ff::UnitDefinition& lhs, const ff::UnitDefinition& rhs);
	[[nodiscard]] static bool AreSameDefinition(const ff::EnemyDefinition& lhs, const ff::EnemyDefinition& rhs);
	[[nodiscard]] bool IsEnemyEditor() const;
	[[nodiscard]] String GetCurrentDefinitionPath() const;
	[[nodiscard]] String GetCurrentStableId() const;
	[[nodiscard]] String GetCurrentDisplayName() const;
	[[nodiscard]] String GetBackButtonTooltipText() const;

	static constexpr size_t FieldCount = 5;
	static constexpr size_t ColorChannelCount = 4;

	Font m_titleFont;
	Font m_buttonFont;
	Font m_infoFont;
	ff::UnitId m_unitId;
 Optional<ff::EnemyKind> m_enemyKind;
	bool m_editingEnemyDefinitions = false;
	ff::UnitDefinition m_editingDefinition;
 ff::EnemyDefinition m_editingEnemyDefinition;
	mutable TextEditState m_labelEditState;
	mutable TextEditState m_roleEditState;
	mutable Array<TextEditState> m_numericEditStates = Array<TextEditState>(FieldCount);
	String m_debugStatus;
	String m_validationStatus;
	PendingAction m_pendingAction = PendingAction::None;
	Optional<ff::UnitId> m_pendingUnitId;
    Optional<ff::EnemyKind> m_pendingEnemyKind;
	bool m_lastSaveSucceeded = true;
	bool m_hasValidationError = false;
	size_t m_selectedFieldIndex = 0;
};
