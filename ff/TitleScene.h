# pragma once
# include "AppState.h"

class TitleScene : public App::Scene
{
public:
	using InitData = App::Scene::InitData;

	TitleScene(const InitData& init);

	void update() override;
	void draw() const override;

private:
	RectF GetStartButton() const;
	RectF GetFormationButton() const;
	RectF GetExitButton() const;

	Font m_titleFont;
	Font m_buttonFont;
};
