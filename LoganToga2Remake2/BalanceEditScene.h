#pragma once

#include "BattleConfigLookup.h"
#include "BattleConfigPathResolver.h"
#include "GameData.h"
#include "MenuButtonUi.h"
#include "SceneTransition.h"

class BalanceEditScene : public SceneBase
{
private:
	enum class Tab
	{
		Core,
		Units,
	};

	struct AdjustmentButtonRects
	{
		RectF minusLarge;
		RectF minusSmall;
		RectF plusSmall;
		RectF plusLarge;
	};

	struct HelpText
	{
		String title;
		String body;
	};

public:
	explicit BalanceEditScene(const SceneBase::InitData& init);

	void update() override;
	void draw() const override;

private:
	BattleConfigData m_editConfig;
	Tab m_tab = Tab::Core;
	int32 m_selectedUnitIndex = 0;
	String m_statusMessage = U"Ready";
	bool m_hasUnsavedChanges = false;

	void reloadFromDisk(const String& statusMessage);

	[[nodiscard]] bool saveEditorOverrides() const;
	[[nodiscard]] bool clearEditorOverrides() const;
	[[nodiscard]] bool clearAllOverrides() const;
	[[nodiscard]] static bool writeTextFile(const String& path, const String& content);
	[[nodiscard]] static bool removeFileIfExists(const String& path);
	[[nodiscard]] bool saveCoreEditorOverride() const;
	[[nodiscard]] bool saveUnitsEditorOverride() const;

	void drawLeftPanel() const;
	void drawCorePanel() const;
	void drawUnitPanel() const;
	void drawHelpPanel() const;

	void handleCoreInput();
	void handleUnitInput();
	void applyEditedState(bool changed);
	[[nodiscard]] bool handleUnitListInput();
	[[nodiscard]] bool handleSelectedProductionCostInput(int32 rowIndex, int32 smallStep, int32 largeStep);
	[[nodiscard]] int32 getSelectedProductionCost() const;
	[[nodiscard]] bool handleIntRowInput(int32 rowIndex, int32& value, int32 smallStep, int32 largeStep, int32 minValue);
	[[nodiscard]] HelpText getHoveredHelpText() const;
	[[nodiscard]] Optional<HelpText> getHoveredCoreRowHelp() const;
	[[nodiscard]] Optional<HelpText> getHoveredUnitRowHelp() const;
	[[nodiscard]] bool handleDoubleRowInput(int32 rowIndex, double& value, double smallStep, double largeStep, double minValue, double maxValue);

	void drawIntRow(int32 rowIndex, const String& label, int32 value, const Font& font) const;
	void drawDoubleRow(int32 rowIndex, const String& label, double value, const Font& font) const;
	void drawAdjustmentRow(const RectF& rowRect, const String& label, const String& valueText, const Font& font) const;
	[[nodiscard]] AdjustmentButtonRects getAdjustmentButtonRects(const RectF& rowRect) const;

	[[nodiscard]] UnitDefinition* getSelectedUnitDefinition();
	[[nodiscard]] const UnitDefinition* getSelectedUnitDefinition() const;
	[[nodiscard]] ProductionSlot* getSelectedProductionSlot();
	[[nodiscard]] const ProductionSlot* getSelectedProductionSlot() const;

	[[nodiscard]] static String getCoreEditorOverridePath();
	[[nodiscard]] static String getCoreOverridePath();
	[[nodiscard]] static String getUnitsEditorOverridePath();
	[[nodiscard]] static String getUnitsOverridePath();
	static void appendTomlLine(String& content, const String& key, const String& value);
	[[nodiscard]] static String quoteTomlString(const String& value);
	[[nodiscard]] static String toUnitArchetypeDisplayString(UnitArchetype archetype);
	[[nodiscard]] static String toUnitArchetypeTomlString(UnitArchetype archetype);

	[[nodiscard]] static bool isButtonClicked(const RectF& rect);
	static void drawButton(const RectF& rect, const String& label, const Font& font, bool selected = false);
	[[nodiscard]] static RectF getLeftPanelRect();
	[[nodiscard]] static RectF getRightPanelRect();
	[[nodiscard]] static RectF getEditorPanelRect();
	[[nodiscard]] static RectF getHelpPanelRect();
	[[nodiscard]] static RectF getTopButtonRect(int32 index);
	[[nodiscard]] static RectF getTabButtonRect(Tab tab);
	[[nodiscard]] static RectF getUnitButtonRect(int32 index);
	[[nodiscard]] static RectF getEditorRowRect(int32 index);
};
