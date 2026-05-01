# include <Siv3D.hpp> // Siv3D v0.6.16
# include "libs/AddonGaussian.h"
# include "Data/DefinitionLoaders.h"
# include "Data/UnitCatalog.h"
# include "Systems/BattleInputSystem.h"
# include "Systems/BattleSystems.h"
# include "Systems/CameraInputSystem.h"
# include "Systems/EditorInputSystem.h"
# include "UI/BattleRenderer.h"
# include "UI/MapEditor.h"
# include "UI/QuarterView.h"

namespace
{
	Vec2 ToWorldPos(const Vec2& screenPos)
	{
		return LT3::ToQuarterWorld(screenPos);
	}

	void UpdateClickDebugState(LT3::ClickDebugState& debugState, const LT3::MapEditorState& mapEditor)
	{
		debugState.currentScreen = Cursor::PosF();
		debugState.currentWorld = ToWorldPos(debugState.currentScreen);
		debugState.currentCell = LT3::PickMapEditorCell(mapEditor, debugState.currentScreen);

		if (MouseL.down())
		{
			debugState.lastLeftScreen = debugState.currentScreen;
			debugState.lastLeftWorld = debugState.currentWorld;
			debugState.lastLeftCell = debugState.currentCell;
		}
		if (MouseR.down())
		{
			debugState.lastRightScreen = debugState.currentScreen;
			debugState.lastRightWorld = debugState.currentWorld;
			debugState.lastRightCell = debugState.currentCell;
		}
	}

	void ProcessInput(LT3::BattleWorld& world, const LT3::DefinitionStores& defs, LT3::MapEditorState& mapEditor)
	{
		if (GaussianFSAddon::IsModalActive())
		{
			return;
		}

		LT3::UpdateQuarterViewCamera(mapEditor, world, defs);

		const Vec2 screenMouse = Cursor::PosF();
		const Vec2 worldMouse = ToWorldPos(screenMouse);
		if (LT3::HandleEditorInput(mapEditor, world, screenMouse))
		{
			return;
		}
		if (mapEditor.enabled)
		{
			return;
		}

		LT3::HandleBattleInput(world, defs, worldMouse);
	}
}

void Main()
{
#pragma region Addon
	Addon::Register<GaussianFSAddon>(U"GaussianFSAddon");
	GaussianFSAddon::Condition({ 1600,900 });
	GaussianFSAddon::SetLangSet({
		{ U"Japan",     U"日本語" },
		{ U"English",   U"English" },
		{ U"Deutsch",   U"Deutsch" },
		{ U"Test",      U"TestLang" },
		});
	GaussianFSAddon::SetLang(U"Japan");
	GaussianFSAddon::SetSceneSet({
		{ U"1600*900", U"1600",U"900"},
		{ U"1200*600", U"1200",U"600"},
		});
	GaussianFSAddon::SetScene(U"1600*900");
#pragma endregion

	Scene::SetBackground(ColorF{ 0.08, 0.14, 0.11 });
	const Font titleFont{ FontMethod::MSDF, 38, Typeface::Bold };
	const Font uiFont{ FontMethod::MSDF, 20, Typeface::Medium };

	LT3::UnitCatalog unitCatalog = LT3::LoadUnitCatalog();
	LT3::DefinitionStores defs = LT3::CreateDefaultDefinitions(unitCatalog);
	LT3::BattleRenderAssets renderAssets = LT3::BuildBattleRenderAssets(unitCatalog);
	LT3::BattleWorld world;
	LT3::SpawnDefaultBattle(world, defs);
	LT3::MapEditorState mapEditor;
	LT3::ClickDebugState clickDebug;
	LT3::LoadMapEditorAssets(mapEditor);

	while (System::Update())
	{
		{
			const double scale = GaussianFSAddon::GetSCALE();
			const Vec2 offset = GaussianFSAddon::GetOFFSET();
			const Transformer2D screenScaling{ Mat3x2::Scale(scale).translated(offset), TransformCursor::Yes };

			UpdateClickDebugState(clickDebug, mapEditor);
			ProcessInput(world, defs, mapEditor);
			if (!mapEditor.enabled)
			{
				LT3::UpdateBattleWorld(world, defs, Scene::DeltaTime());
			}
			LT3::DrawBattleWorld(world, defs, renderAssets, mapEditor, clickDebug, uiFont, titleFont);
			LT3::DrawMapEditorOverlay(mapEditor, unitCatalog, Cursor::PosF(), uiFont);
		}

#pragma region Addon
		if (GaussianFSAddon::TriggerOrDisplayESC()) break;
		if (GaussianFSAddon::TriggerOrDisplayLang()) break;
		if (GaussianFSAddon::TriggerOrDisplaySceneSize()) break;
		if (GaussianFSAddon::IsHide()) Window::Minimize();
		if (GaussianFSAddon::IsGameEnd()) break;
		GaussianFSAddon::DragProcessWindow();
#pragma endregion


	}
}
