#include "ContinueRunSave.h"

String GetContinueResumeSceneLabel(const ContinueResumeScene scene)
{
	switch (scene)
	{
	case ContinueResumeScene::Reward:
		return U"Reward";
	case ContinueResumeScene::BonusRoom:
		return U"BonusRoom";
	case ContinueResumeScene::Battle:
	default:
		return U"Battle";
	}
}

String GetContinueResumeSceneName(const ContinueResumeScene scene)
{
	return GetContinueResumeSceneLabel(scene);
}

ContinueResumeScene ParseContinueResumeScene(const String& label)
{
	if (label == U"Reward")
	{
		return ContinueResumeScene::Reward;
	}

	if (label == U"BonusRoom")
	{
		return ContinueResumeScene::BonusRoom;
	}

	return ContinueResumeScene::Battle;
}
