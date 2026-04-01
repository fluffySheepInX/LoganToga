# pragma once
# include "AppState.h"
# include "WaveDefinitions.h"

class WaveEditorScene : public App::Scene
{
public:
	using InitData = App::Scene::InitData;

	WaveEditorScene(const InitData& init);

	void update() override;
	void draw() const override;

private:
	enum class PendingAction
	{
		None,
		BackToFormation,
     OpenEnemyEditor,
		ReloadFromDisk,
	};

	static constexpr size_t GlobalTimingFieldCount = 3;
	static constexpr size_t WaveNumericFieldCount = 5;
	static constexpr size_t SpawnKindCount = 3;

	bool HandlePendingActionConfirmation();
	bool HandleWaveSelection();
	bool HandlePrimaryActions();
	bool HandleAdjustments();
	void HandleKeyboardShortcuts();
   bool RequestAction(PendingAction action, Optional<ff::EnemyKind> targetEnemyKind = none);
	void CommitPendingAction();
	void ClearPendingAction();
	void ReloadEditingConfig();
	void SaveCurrentDefinition();
	void ResetCurrentWave();
	void SyncEditorsFromCurrentWave();
	void ApplyEditorTextsToCurrentWave();
	void ApplyTypePreset(ff::WaveType type);
	void ApplyTraitPreset(ff::WaveTrait trait);
	void AdjustGlobalTiming(size_t index, double direction);
	void AdjustWaveNumeric(size_t index, double direction);
	void AdjustSpawnCount(size_t index, int32 delta);
	void CycleWaveType(int32 direction);
	void CycleWaveTrait(int32 direction);

	[[nodiscard]] ff::WaveDefinition& CurrentWave();
	[[nodiscard]] const ff::WaveDefinition& CurrentWave() const;
	[[nodiscard]] bool HasUnsavedChanges() const;
	[[nodiscard]] String GetPendingActionMessage() const;
	[[nodiscard]] Array<int32> GetSpawnCounts() const;
	void SetSpawnCounts(const Array<int32>& counts);

	void DrawWaveList() const;
	void DrawEditorPanel() const;
	void DrawSpawnPanel() const;
	void DrawStatusBar() const;
	void DrawPendingActionDialog() const;

	[[nodiscard]] RectF GetWaveButton(size_t index) const;
	[[nodiscard]] RectF GetGlobalTimingRow(size_t index) const;
	[[nodiscard]] RectF GetGlobalTimingDecreaseButton(size_t index) const;
	[[nodiscard]] RectF GetGlobalTimingIncreaseButton(size_t index) const;
	[[nodiscard]] RectF GetTypeButton() const;
	[[nodiscard]] RectF GetTypeDecreaseButton() const;
	[[nodiscard]] RectF GetTypeIncreaseButton() const;
	[[nodiscard]] RectF GetTraitButton() const;
	[[nodiscard]] RectF GetTraitDecreaseButton() const;
	[[nodiscard]] RectF GetTraitIncreaseButton() const;
	[[nodiscard]] RectF GetWaveNumericRow(size_t index) const;
	[[nodiscard]] RectF GetWaveNumericDecreaseButton(size_t index) const;
	[[nodiscard]] RectF GetWaveNumericIncreaseButton(size_t index) const;
	[[nodiscard]] RectF GetSpawnRow(size_t index) const;
	[[nodiscard]] RectF GetSpawnDecreaseButton(size_t index) const;
	[[nodiscard]] RectF GetSpawnIncreaseButton(size_t index) const;
 [[nodiscard]] RectF GetSpawnEditButton(size_t index) const;
	[[nodiscard]] RectF GetResetButton() const;
	[[nodiscard]] RectF GetReloadButton() const;
	[[nodiscard]] RectF GetSaveButton() const;
	[[nodiscard]] RectF GetBackButton() const;
	[[nodiscard]] RectF GetDialogConfirmButton() const;
	[[nodiscard]] RectF GetDialogCancelButton() const;

	[[nodiscard]] StringView GetGlobalTimingLabel(size_t index) const;
	[[nodiscard]] String GetGlobalTimingValue(size_t index) const;
	[[nodiscard]] StringView GetWaveNumericLabel(size_t index) const;
	[[nodiscard]] String GetWaveNumericValue(size_t index) const;
	[[nodiscard]] StringView GetSpawnLabel(size_t index) const;
	[[nodiscard]] String GetTypeLabel() const;
	[[nodiscard]] String GetTraitLabel() const;

	Font m_titleFont;
	Font m_buttonFont;
	Font m_infoFont;
	ff::WaveConfig m_editingConfig;
	int32 m_selectedWave = 1;
 mutable TextEditState m_labelEditState;
	mutable TextEditState m_descriptionEditState;
	String m_debugStatus;
	PendingAction m_pendingAction = PendingAction::None;
    Optional<ff::EnemyKind> m_pendingEnemyKind;
	bool m_lastSaveSucceeded = true;
};
