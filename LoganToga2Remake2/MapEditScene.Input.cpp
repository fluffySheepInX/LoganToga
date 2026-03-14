#include "MapEditScene.h"
#include "SceneTransition.h"

void MapEditScene::update()
{
	if (UpdateSceneTransition(getData(), [this](const String& sceneName)
	{
		changeScene(sceneName);
	}))
	{
		return;
	}

	if (KeyEscape.down())
	{
		RequestSceneTransition(getData(), U"Title", [this](const String& sceneName)
		{
			changeScene(sceneName);
		});
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
