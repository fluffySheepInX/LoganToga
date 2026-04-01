# include "FormationScene.h"

FormationScene::FormationScene(const InitData& init)
	: App::Scene{ init }
	, m_titleFont{ 40, Typeface::Heavy }
	, m_buttonFont{ 22 }
	, m_infoFont{ 18 }
	, m_editingFormation{ MakeFormationEditState(getData()) }
{
	Scene::SetBackground(ColorF{ 0.08, 0.11, 0.18 });
}

void FormationScene::update()
{
	if (HandleSceneNavigation())
	{
		return;
	}

	if (HandleUtilityActions())
	{
		return;
	}

	if (HandlePresetInput())
	{
		return;
	}

	if (HandleUnitSelection())
	{
		return;
	}

	if (HandleSlotInput())
	{
		return;
	}
}
