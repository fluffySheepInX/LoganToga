# pragma once
# include "MainContext.hpp"

class BirdModel;

namespace SkyAppSupport
{
	struct TimedMessage
	{
		String text;
		double until = 0.0;

      void show(StringView message, double durationSeconds = 2.0);
		[[nodiscard]] bool isVisible() const;
	};

	struct SkyAppPanels
	{
       Rect miniMap{ 20, 20, 220, 220 };
		Rect skySettings{ 20, 20, 480, 430 };
		Rect cameraSettings{ 520, 20, 360, 380 };
         Rect mapEditor{ (Scene::Width() - 360), 20, 340, 500 };
       Rect blacksmithMenu{ (Scene::Width() - 320), (Scene::Height() - 224), 300, 184 };
		Rect sapperMenu{ (Scene::Width() - 340), (Scene::Height() - 324), 320, 284 };
         Rect millStatusEditor{ (Scene::Width() - 340), (Scene::Height() - 430), 320, 320 };
		Rect modelHeight{ 860, 20, 400, 300 };
      Rect resourcePanel{ 740, 20, 220, 84 };
		Rect uiToggle{ 20, (Scene::Height() - 100), 140, 36 };
		Rect mapModeToggle{ 180, (Scene::Height() - 100), 180, 36 };
		Rect modelHeightModeToggle{ 380, (Scene::Height() - 100), 220, 36 };
     Rect reloadMapButton{ 620, (Scene::Height() - 100), 160, 36 };
		   Rect restartButton{ 800, (Scene::Height() - 100), 160, 36 };
		Rect timeSlider{ 20, (Scene::Height() - 60), (Scene::Width() - 20), 36 };

      [[nodiscard]] bool isHoveringUi(bool showUI, bool isEditorMode, bool showBlacksmithMenu, bool showSapperMenu, bool showMillStatusEditor, bool modelHeightEditMode) const;
	};

	[[nodiscard]] Vec3 GetSpawnedSapperBasePosition(const MainSupport::SpawnedSapper& sapper);
	[[nodiscard]] double GetSpawnedSapperBounceOffset(const MainSupport::SpawnedSapper& sapper);
	[[nodiscard]] Vec3 GetSpawnedSapperRenderPosition(const MainSupport::SpawnedSapper& sapper);
    [[nodiscard]] double GetSpawnedSapperYaw(const MainSupport::SpawnedSapper& sapper);
   [[nodiscard]] bool IsSapperSuppressed(const MainSupport::SpawnedSapper& sapper);
	[[nodiscard]] double GetEffectiveSapperMoveSpeed(const MainSupport::SpawnedSapper& sapper);
	[[nodiscard]] double GetEffectiveSapperAttackDamage(const MainSupport::SpawnedSapper& sapper);
	[[nodiscard]] double GetEffectiveSapperAttackInterval(const MainSupport::SpawnedSapper& sapper);
    [[nodiscard]] ColorF GetSpawnedSapperTint(const MainSupport::SpawnedSapper& sapper, const ColorF& baseColor);
	void ApplySapperSuppression(MainSupport::SpawnedSapper& sapper, double durationSeconds, double moveSpeedMultiplier, double attackDamageMultiplier, double attackIntervalMultiplier);
	void UpdateSpawnedSappers(Array<MainSupport::SpawnedSapper>& spawnedSappers);
    void ResolveSapperSpacingAgainstUnits(Array<MainSupport::SpawnedSapper>& spawnedSappers, const Array<MainSupport::SpawnedSapper>& enemySappers);
	void ResolveSapperSpacingAgainstBase(Array<MainSupport::SpawnedSapper>& spawnedSappers, const Vec3& enemyBasePosition);
	void SetSpawnedSapperTarget(MainSupport::SpawnedSapper& sapper, const Vec3& targetPosition);
  void UpdateAutoCombat(Array<MainSupport::SpawnedSapper>& attackers, Array<MainSupport::SpawnedSapper>& defenders);
	void RemoveDefeatedSappers(Array<MainSupport::SpawnedSapper>& spawnedSappers);
  [[nodiscard]] Optional<size_t> HitTestSpawnedSapper(const Array<MainSupport::SpawnedSapper>& spawnedSappers, const MainSupport::AppCamera3D& camera);
	void DrawSelectedSapperRing(const MainSupport::AppCamera3D& camera, const MainSupport::SpawnedSapper& sapper);
	void DrawSelectedSapperIcon(const MainSupport::AppCamera3D& camera, const MainSupport::SpawnedSapper& sapper);
	 void DrawSapperHealthBars(const MainSupport::AppCamera3D& camera, const Array<MainSupport::SpawnedSapper>& spawnedSappers, const ColorF& fillColor);
	 void UpdateCameraWheelZoom(MainSupport::AppCamera3D& camera, MainSupport::CameraSettings& cameraSettings, const Vec3& playerBasePosition);
 void DrawSpawnedSappers(const Array<MainSupport::SpawnedSapper>& spawnedSappers, const BirdModel& sapperModel, const ColorF& color);
	void UpdateSkyFromTime(Sky& sky, double skyTime);
 void SpawnSapper(Array<MainSupport::SpawnedSapper>& spawnedSappers, const Vec3& spawnPosition, const Vec3& rallyPoint, MainSupport::SapperUnitType unitType = MainSupport::SapperUnitType::Infantry);
   void SpawnEnemySapper(Array<MainSupport::SpawnedSapper>& spawnedSappers, const Vec3& position, double facingYaw = MainSupport::BirdDisplayYaw);
}
