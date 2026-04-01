# include "FormationScene.h"

RectF FormationScene::GetUnitButton(const size_t index) const
{
	return RectF{ 185, (175 + (index * 62)), 220, 56 };
}

RectF FormationScene::GetSlotButton(const size_t index) const
{
	const double width = 190;
	const double spacingX = 20;
	const double spacingY = 20;
	const size_t column = (index % 2);
	const size_t row = (index / 2);
	return RectF{ (540 + (column * (width + spacingX))), (195 + (row * (52 + spacingY))), width, 52 };
}

RectF FormationScene::GetBackButton() const
{
	return RectF{ Arg::center = Scene::Center().movedBy(120, 274), 220, 52 };
}

RectF FormationScene::GetConfirmButton() const
{
	return RectF{ Arg::center = Scene::Center().movedBy(-120, 274), 220, 52 };
}

RectF FormationScene::GetPresetCard(const size_t index) const
{
	return RectF{ Arg::center = Scene::Center().movedBy((-260 + (260 * static_cast<double>(index))), 182), 220, 72 };
}

RectF FormationScene::GetPresetLoadButton(const size_t index) const
{
	return RectF{ Arg::center = GetPresetCard(index).center().movedBy(-52, 20), 92, 30 };
}

RectF FormationScene::GetPresetSaveButton(const size_t index) const
{
	return RectF{ Arg::center = GetPresetCard(index).center().movedBy(52, 20), 92, 30 };
}

RectF FormationScene::GetRandomButton() const
{
	return RectF{ 548, 154, 148, 34 };
}

RectF FormationScene::GetClearButton() const
{
	return RectF{ 764, 154, 148, 34 };
}

RectF FormationScene::GetUnitEditButton() const
{
	return RectF{ 185, 450, 220, 40 };
}

RectF FormationScene::GetWaveEditButton() const
{
	return RectF{ 185, 498, 220, 40 };
}
