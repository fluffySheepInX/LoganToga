#include "MapEditScene.h"

void MapEditScene::update()
{
	if (KeyEscape.down())
	{
		changeScene(U"Title");
		return;
	}

	if (MouseL.up())
	{
		m_dragOffset.reset();
	}

	if (s3d::KeyDelete.down())
	{
		deleteSelection();
		return;
	}

	if ((s3d::KeyControl.pressed() && s3d::KeyS.down()) || s3d::KeyF5.down())
	{
		saveMap();
		return;
	}

	if (KeyR.down())
	{
		reloadConfig();
		return;
	}

	if (handleLeftPanelInput() || handleRightPanelInput())
	{
		return;
	}

	handleCanvasInput();
}
