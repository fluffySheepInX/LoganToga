#include "BonusRoomScene.h"
#include "AudioManager.h"
#include "ContinueRunSave.h"
#include "MenuButtonUi.h"
#include "SceneTransition.h"

namespace
{
	[[nodiscard]] MenuButtonStyle GetBonusButtonStyle()
	{
		MenuButtonStyle style;
		style.cornerRadius = 12.0;
		style.fillColor = ColorF{ 0.11, 0.13, 0.19, 0.97 };
		style.hoverFillColor = ColorF{ 0.16, 0.18, 0.24, 0.98 };
		style.pressedFillColor = ColorF{ 0.10, 0.12, 0.18, 0.98 };
		style.selectedFillColor = ColorF{ 0.28, 0.26, 0.16, 0.98 };
		style.selectedHoverFillColor = ColorF{ 0.36, 0.32, 0.18, 0.98 };
		style.frameColor = ColorF{ 0.62, 0.54, 0.30, 0.90 };
		style.hoverFrameColor = ColorF{ 0.92, 0.82, 0.44, 0.98 };
		style.selectedFrameColor = ColorF{ 1.0, 0.90, 0.56, 0.98 };
		style.accentColor = ColorF{ 0.82, 0.72, 0.38, 0.95 };
		style.selectedAccentColor = ColorF{ 0.98, 0.90, 0.60, 0.98 };
		return style;
	}

	[[nodiscard]] RectF GetBackButtonRect()
	{
		return RectF{ 40, 40, 180, 36 };
	}

	[[nodiscard]] RectF GetGalleryRoomRect(const int32 index)
	{
		return RectF{ 160, 160 + (index * 84), 820, 68 };
	}

	[[nodiscard]] RectF GetViewerPrevButtonRect()
	{
		return RectF{ 320, 640, 160, 40 };
	}

	[[nodiscard]] RectF GetViewerCloseButtonRect()
	{
		return RectF{ 560, 640, 160, 40 };
	}

	[[nodiscard]] RectF GetViewerNextButtonRect()
	{
		return RectF{ 800, 640, 160, 40 };
	}
}

BonusRoomScene::BonusRoomScene(const SceneBase::InitData& init)
	: SceneBase{ init }
{
	PlayMenuBgm();
}

void BonusRoomScene::update()
{
	if (UpdateSceneTransition(getData(), [this](const String& sceneName)
	{
		changeScene(sceneName);
	}))
	{
		return;
	}

	auto& data = getData();
	auto& progress = data.bonusRoomProgress;
	if (progress.activeRoomId.isEmpty())
	{
		if (progress.sceneMode == BonusRoomSceneMode::Selection)
		{
			updateSelection();
		}
		else
		{
			updateGallery();
		}
		return;
	}

	updateViewer();
}

void BonusRoomScene::draw() const
{
	Scene::Rect().draw(ColorF{ 0.06, 0.07, 0.10 });
	const auto& data = getData();
	const auto& progress = data.bonusRoomProgress;
	if (progress.activeRoomId.isEmpty())
	{
		if (progress.sceneMode == BonusRoomSceneMode::Selection)
		{
			drawSelection();
		}
		else
		{
			drawGallery();
		}
	}
	else
	{
		drawViewer();
	}

	DrawSceneTransitionOverlay(data);
}

RectF BonusRoomScene::getSelectionCardRect(const int32 index)
{
	const double cardWidth = 300.0;
	const double cardHeight = 260.0;
	const double gap = 28.0;
	const double totalWidth = (cardWidth * 3.0) + (gap * 2.0);
	const double startX = (Scene::Width() - totalWidth) * 0.5;
	return RectF{ startX + (index * (cardWidth + gap)), 230, cardWidth, cardHeight };
}

Array<const BonusRoomDefinition*> BonusRoomScene::viewedRooms() const
{
	const auto& data = getData();
	return CollectViewedBonusRooms(data.bonusRooms, data.bonusRoomProgress);
}

#include "BonusRoomSceneSelection.ipp"
#include "BonusRoomSceneGallery.ipp"
#include "BonusRoomSceneViewer.ipp"
