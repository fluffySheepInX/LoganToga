# pragma once
# include "AppState.h"

class FormationScene : public App::Scene
{
public:
	using InitData = App::Scene::InitData;

	FormationScene(const InitData& init);

	void update() override;
	void draw() const override;

private:
	RectF GetUnitButton(size_t index) const;
	RectF GetSlotButton(size_t index) const;
    RectF GetConfirmButton() const;
	RectF GetBackButton() const;
	RectF GetPresetCard(size_t index) const;
	RectF GetPresetLoadButton(size_t index) const;
	RectF GetPresetSaveButton(size_t index) const;
	RectF GetRandomButton() const;
	RectF GetClearButton() const;

	Font m_titleFont;
	Font m_buttonFont;
	Font m_infoFont;
  Array<Optional<ff::AllyBehavior>> m_editingFormationSlots;
	Optional<ff::AllyBehavior> m_selectedFormationUnit;
};
