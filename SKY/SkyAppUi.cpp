# include "SkyAppUi.hpp"
# include "MainSettings.hpp"
# include "MainUi.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
 namespace
	{
		[[nodiscard]] Vec2 ToMiniMapPoint(const RectF& mapRect, const double minX, const double minZ, const double worldSpan, const Vec3& worldPosition)
		{
			const double x = (worldPosition.x - minX) / worldSpan;
			const double z = (worldPosition.z - minZ) / worldSpan;
			return Vec2{ (mapRect.x + x * mapRect.w), (mapRect.bottomY() - z * mapRect.h) };
		}

		[[nodiscard]] ColorF GetResourceTypeColor(const ResourceType type)
		{
			switch (type)
			{
			case ResourceType::Budget:
				return ColorF{ 0.96, 0.82, 0.22, 0.78 };

			case ResourceType::Gunpowder:
				return ColorF{ 0.92, 0.42, 0.26, 0.78 };

			case ResourceType::Mana:
				return ColorF{ 0.42, 0.60, 0.98, 0.78 };

			default:
				return ColorF{ 0.70, 0.70, 0.70, 0.78 };
			}
		}

		[[nodiscard]] ColorF GetOwnerFrameColor(const Optional<UnitTeam>& ownerTeam)
		{
			if (ownerTeam && (*ownerTeam == UnitTeam::Player))
			{
				return ColorF{ 0.96, 0.96, 1.0, 0.95 };
			}

			if (ownerTeam && (*ownerTeam == UnitTeam::Enemy))
			{
				return ColorF{ 0.95, 0.35, 0.30, 0.95 };
			}

			return ColorF{ 0.65, 0.72, 0.82, 0.85 };
		}
	}

	void DrawSkySettingsPanel(Sky& sky, const SkyAppPanels& panels)
	{
		panels.skySettings.draw(ColorF{ 1.0, 0.92 });
		Rect{ 20, 20, 480, 76 }.draw();
		SimpleGUI::GetFont()(U"zenith:").draw(28, 24, ColorF{ 0.11 });
		Rect{ 100, 26, 28 }.draw(sky.zenithColor.gamma(2.2)).drawFrame(1, 0, ColorF{ 0.5 });
		SimpleGUI::GetFont()(U"horizon:").draw(148, 24, ColorF{ 0.11 });
		Rect{ 230, 26, 28 }.draw(sky.horizonColor.gamma(2.2)).drawFrame(1, 0, ColorF{ 0.5 });
		SimpleGUI::GetFont()(U"cloud:").draw(276, 24, ColorF{ 0.11 });
		Rect{ 340, 26, 28 }.draw(sky.cloudColor.gamma(2.2)).drawFrame(1, 0, ColorF{ 0.5 });
		SimpleGUI::GetFont()(U"sun:").draw(386, 24, ColorF{ 0.11 });
		Rect{ 430, 26, 28 }.draw(Graphics3D::GetSunColor().gamma(2.2)).drawFrame(1, 0, ColorF{ 0.5 });
		SimpleGUI::GetFont()(U"sunDir: {:.2f}   cloudTime: {:.1f}"_fmt(Graphics3D::GetSunDirection(), sky.cloudTime)).draw(28, 60, ColorF{ 0.11 });

		SimpleGUI::Slider(U"cloudiness: {:.3f}"_fmt(sky.cloudiness), sky.cloudiness, Vec2{ 20, 100 }, 180, 300);
		SimpleGUI::Slider(U"cloudScale: {:.2f}"_fmt(sky.cloudScale), sky.cloudScale, 0.0, 2.0, Vec2{ 20, 140 }, 180, 300);
		SimpleGUI::Slider(U"cloudHeight: {:.0f}"_fmt(sky.cloudPlaneHeight), sky.cloudPlaneHeight, 20.0, 6000.0, Vec2{ 20, 180 }, 180, 300);
		SimpleGUI::Slider(U"orientation: {:.0f}"_fmt(Math::ToDegrees(sky.cloudOrientation)), sky.cloudOrientation, 0.0, Math::TwoPi, Vec2{ 20, 220 }, 180, 300);
		SimpleGUI::Slider(U"fogHeightSky: {:.2f}"_fmt(sky.fogHeightSky), sky.fogHeightSky, Vec2{ 20, 260 }, 180, 300, false);
		SimpleGUI::Slider(U"star: {:.2f}"_fmt(sky.starBrightness), sky.starBrightness, Vec2{ 20, 300 }, 180, 300, false);
		SimpleGUI::Slider(U"starF: {:.2f}"_fmt(sky.starBrightnessFactor), sky.starBrightnessFactor, Vec2{ 20, 340 }, 180, 300);
		SimpleGUI::Slider(U"starSat: {:.2f}"_fmt(sky.starSaturation), sky.starSaturation, 0.0, 1.0, Vec2{ 20, 380 }, 180, 300);
		SimpleGUI::CheckBox(sky.sunEnabled, U"sun", Vec2{ 20, 420 }, 120, false);
		SimpleGUI::CheckBox(sky.cloudsEnabled, U"clouds", Vec2{ 150, 420 }, 120);
		SimpleGUI::CheckBox(sky.cloudsLightingEnabled, U"cloudsLighting", Vec2{ 280, 420 }, 220);
	}

	void DrawCameraSettingsPanel(DebugCamera3D& camera,
		CameraSettings& cameraSettings,
		BirdModel& birdModel,
		BirdModel& ashigaruModel,
		TimedMessage& cameraSaveMessage,
		const SkyAppPanels& panels)
	{
		panels.cameraSettings.draw(ColorF{ 1.0, 0.92 });
		SimpleGUI::GetFont()(U"Camera eye").draw(540, 28, ColorF{ 0.11 });
		SimpleGUI::Slider(U"eyeX: {:.2f}"_fmt(cameraSettings.eye.x), cameraSettings.eye.x, -50.0, 50.0, Vec2{ 540, 60 }, 140, 180);
		SimpleGUI::Slider(U"eyeY: {:.2f}"_fmt(cameraSettings.eye.y), cameraSettings.eye.y, -10.0, 50.0, Vec2{ 540, 100 }, 140, 180);
		SimpleGUI::Slider(U"eyeZ: {:.2f}"_fmt(cameraSettings.eye.z), cameraSettings.eye.z, -50.0, 50.0, Vec2{ 540, 140 }, 140, 180);
		SimpleGUI::GetFont()(U"Camera focus").draw(540, 190, ColorF{ 0.11 });
		SimpleGUI::Slider(U"focusX: {:.2f}"_fmt(cameraSettings.focus.x), cameraSettings.focus.x, -50.0, 50.0, Vec2{ 540, 220 }, 140, 180);
		SimpleGUI::Slider(U"focusY: {:.2f}"_fmt(cameraSettings.focus.y), cameraSettings.focus.y, -10.0, 50.0, Vec2{ 540, 260 }, 140, 180);
		SimpleGUI::Slider(U"focusZ: {:.2f}"_fmt(cameraSettings.focus.z), cameraSettings.focus.z, -50.0, 50.0, Vec2{ 540, 300 }, 140, 180);

		if (DrawTextButton(Rect{ 540, 330, 150, 30 }, U"Save TOML"))
		{
			cameraSaveMessage.show(SaveCameraSettings(cameraSettings)
				? U"Saved: {}"_fmt(CameraSettingsPath)
				: U"Save failed");
		}

		if (DrawTextButton(Rect{ 710, 330, 150, 30 }, U"視点初期化"))
		{
			cameraSettings.eye = DefaultCameraEye;
			cameraSettings.focus = DefaultCameraFocus;
			camera.setView(cameraSettings.eye, cameraSettings.focus);
			cameraSaveMessage.show(U"Camera reset");
		}

		if (cameraSaveMessage.isVisible())
		{
			SimpleGUI::GetFont()(cameraSaveMessage.text).draw(540, 372, ColorF{ 0.11 });
		}

		if (not birdModel.isLoaded())
		{
			SimpleGUI::GetFont()(U"bird.glb load failed").draw(540, 396, ColorF{ 0.75, 0.2, 0.2 });
		}

		if (not ashigaruModel.isLoaded())
		{
			SimpleGUI::GetFont()(U"ashigaru_v2.1.glb load failed").draw(540, 420, ColorF{ 0.75, 0.2, 0.2 });
		}

		DrawAnimationClipSelector(birdModel, U"Bird Clips", 540, 420, 150);
		DrawAnimationClipSelector(ashigaruModel, U"Ashigaru Clips", 710, 420, 150);
	}

	void DrawBlacksmithMenu(const SkyAppPanels& panels,
		Array<SpawnedSapper>& spawnedSappers,
     const Vec3& playerBasePosition,
		const Vec3& rallyPoint,
        ResourceStock& playerResources,
		const double sapperCost,
		TimedMessage& blacksmithMenuMessage)
	{
		panels.blacksmithMenu.draw(ColorF{ 0.98, 0.95 });
		panels.blacksmithMenu.drawFrame(2, 0, ColorF{ 0.25 });
		SimpleGUI::GetFont()(U"ユニット生産メニュー").draw((panels.blacksmithMenu.x + 16), (panels.blacksmithMenu.y + 12), ColorF{ 0.12 });
      SimpleGUI::GetFont()(U"予算: {:.0f}"_fmt(playerResources.budget)).draw((panels.blacksmithMenu.x + 16), (panels.blacksmithMenu.y + 34), ColorF{ 0.12 });

       const Rect produceSapperButton{ (panels.blacksmithMenu.x + 16), (panels.blacksmithMenu.y + 58), (panels.blacksmithMenu.w - 32), 32 };
		const Rect tierUpgradeButton{ (panels.blacksmithMenu.x + 16), (panels.blacksmithMenu.y + 96), (panels.blacksmithMenu.w - 32), 32 };

       if (DrawTextButton(produceSapperButton, U"工兵産出 ({:.0f})"_fmt(sapperCost)))
		{
          if (sapperCost <= playerResources.budget)
			{
					SpawnSapper(spawnedSappers, playerBasePosition, rallyPoint);
              playerResources.budget -= sapperCost;
				blacksmithMenuMessage.show(U"工兵を産出");
			}
			else
			{
				blacksmithMenuMessage.show(U"資源不足");
			}
		}

		if (DrawTextButton(tierUpgradeButton, U"ティアアップグレード"))
		{
			blacksmithMenuMessage.show(U"ティアアップグレードを選択");
		}

		if (blacksmithMenuMessage.isVisible())
		{
			SimpleGUI::GetFont()(blacksmithMenuMessage.text).draw((panels.blacksmithMenu.x + 16), (panels.blacksmithMenu.y - 28), ColorF{ 0.12 });
		}
	}

	void DrawMiniMap(const SkyAppPanels& panels,
		const DebugCamera3D& camera,
		const MapData& mapData,
		const Array<SpawnedSapper>& spawnedSappers,
     const Array<SpawnedSapper>& enemySappers,
     const Array<ResourceAreaState>& resourceAreaStates,
		const Array<size_t>& selectedSapperIndices)
	{
		const Rect panel = panels.miniMap;
		const RectF mapRect{ (panel.x + 10), (panel.y + 30), (panel.w - 20), (panel.h - 40) };
		Array<Vec3> boundsPoints{
         mapData.playerBasePosition,
			mapData.enemyBasePosition,
			mapData.sapperRallyPoint,
			camera.getEyePosition(),
			camera.getFocusPosition(),
		};

		for (const auto& sapper : spawnedSappers)
		{
			boundsPoints << GetSpawnedSapperBasePosition(sapper);
		}

		for (const auto& enemySapper : enemySappers)
		{
			boundsPoints << GetSpawnedSapperBasePosition(enemySapper);
		}

		for (const auto& placedModel : mapData.placedModels)
		{
			boundsPoints << placedModel.position;
		}

		for (const auto& resourceArea : mapData.resourceAreas)
		{
			boundsPoints << resourceArea.position;
		}

		double minX = boundsPoints.front().x;
		double maxX = boundsPoints.front().x;
		double minZ = boundsPoints.front().z;
		double maxZ = boundsPoints.front().z;

		for (const auto& point : boundsPoints)
		{
			minX = Min(minX, point.x);
			maxX = Max(maxX, point.x);
			minZ = Min(minZ, point.z);
			maxZ = Max(maxZ, point.z);
		}

		const double padding = 4.0;
		minX -= padding;
		maxX += padding;
		minZ -= padding;
		maxZ += padding;

		const double centerX = ((minX + maxX) * 0.5);
		const double centerZ = ((minZ + maxZ) * 0.5);
		const double worldSpan = Max({ (maxX - minX), (maxZ - minZ), 20.0 });
		minX = (centerX - worldSpan * 0.5);
		minZ = (centerZ - worldSpan * 0.5);

		panel.draw(ColorF{ 0.08, 0.10, 0.12, 0.88 });
		panel.drawFrame(2, 0, ColorF{ 0.75, 0.82, 0.90, 0.9 });
		SimpleGUI::GetFont()(U"Mini Map").draw((panel.x + 10), (panel.y + 6), Palette::White);
		SimpleGUI::GetFont()(U"N").drawAt((panel.x + panel.w * 0.5), (panel.y + 18), ColorF{ 0.85, 0.92, 1.0 });
		mapRect.draw(ColorF{ 0.02, 0.03, 0.05, 0.96 });
		mapRect.drawFrame(1, 0, ColorF{ 0.35, 0.45, 0.55, 0.85 });

		for (const auto& placedModel : mapData.placedModels)
		{
			Circle{ ToMiniMapPoint(mapRect, minX, minZ, worldSpan, placedModel.position), 2.0 }.draw(ColorF{ 0.48, 0.58, 0.48, 0.85 });
		}

		for (size_t i = 0; i < mapData.resourceAreas.size(); ++i)
		{
			const ResourceArea& resourceArea = mapData.resourceAreas[i];
			const Optional<UnitTeam> ownerTeam = ((i < resourceAreaStates.size()) ? resourceAreaStates[i].ownerTeam : none);
			const Vec2 resourcePoint = ToMiniMapPoint(mapRect, minX, minZ, worldSpan, resourceArea.position);
			const double radius = Max(5.0, (resourceArea.radius / worldSpan) * mapRect.w);
			Circle{ resourcePoint, radius }.draw(GetResourceTypeColor(resourceArea.type));
			Circle{ resourcePoint, radius }.drawFrame(2.0, GetOwnerFrameColor(ownerTeam));
		}

        const Vec2 blacksmithPoint = ToMiniMapPoint(mapRect, minX, minZ, worldSpan, mapData.playerBasePosition);
		RectF{ Arg::center = blacksmithPoint, 8, 8 }.draw(ColorF{ 0.92, 0.74, 0.28, 0.95 });

      const Vec2 enemyBasePoint = ToMiniMapPoint(mapRect, minX, minZ, worldSpan, mapData.enemyBasePosition);
		RectF{ Arg::center = enemyBasePoint, 8, 8 }.draw(ColorF{ 0.92, 0.28, 0.24, 0.95 });

		const Vec2 rallyPoint = ToMiniMapPoint(mapRect, minX, minZ, worldSpan, mapData.sapperRallyPoint);
		Quad{
			rallyPoint.movedBy(0, -6),
			rallyPoint.movedBy(6, 0),
			rallyPoint.movedBy(0, 6),
			rallyPoint.movedBy(-6, 0)
		}.draw(ColorF{ 0.45, 0.88, 0.98, 0.95 });

		for (size_t i = 0; i < spawnedSappers.size(); ++i)
		{
			const Vec2 sapperPoint = ToMiniMapPoint(mapRect, minX, minZ, worldSpan, GetSpawnedSapperBasePosition(spawnedSappers[i]));
           const bool isSelected = selectedSapperIndices.contains(i);

			if (isSelected)
			{
				Circle{ sapperPoint, 7.0 }.drawFrame(2.0, ColorF{ 1.0, 0.95, 0.35, 0.95 });
			}

			Circle{ sapperPoint, 4.0 }.draw(isSelected ? ColorF{ 1.0, 0.96, 0.55, 0.98 } : ColorF{ 0.96, 0.96, 1.0, 0.95 });
		}

		for (const auto& enemySapper : enemySappers)
		{
			const Vec2 enemyPoint = ToMiniMapPoint(mapRect, minX, minZ, worldSpan, GetSpawnedSapperBasePosition(enemySapper));
			RectF{ Arg::center = enemyPoint, 7, 7 }.draw(ColorF{ 0.95, 0.35, 0.30, 0.95 });
		}

		const Vec3 cameraPosition = camera.getEyePosition();
		Vec3 cameraDirection = (camera.getFocusPosition() - cameraPosition);
		cameraDirection.y = 0.0;

		if (cameraDirection.lengthSq() < 0.0001)
		{
			cameraDirection = Vec3{ 0, 0, 1 };
		}
		else
		{
			cameraDirection = cameraDirection.normalized();
		}

		const Vec3 cameraWing = Vec3{ -cameraDirection.z, 0, cameraDirection.x };
		const double cameraRange = Max(3.0, worldSpan * 0.12);
		const Vec2 cameraCenter = ToMiniMapPoint(mapRect, minX, minZ, worldSpan, cameraPosition);
		const Vec2 cameraTip = ToMiniMapPoint(mapRect, minX, minZ, worldSpan, (cameraPosition + cameraDirection * cameraRange));
		const Vec2 cameraLeft = ToMiniMapPoint(mapRect, minX, minZ, worldSpan, (cameraPosition + cameraDirection * cameraRange * 0.55 + cameraWing * cameraRange * 0.35));
		const Vec2 cameraRight = ToMiniMapPoint(mapRect, minX, minZ, worldSpan, (cameraPosition + cameraDirection * cameraRange * 0.55 - cameraWing * cameraRange * 0.35));

		Line{ cameraCenter, cameraTip }.draw(2.0, ColorF{ 0.98, 0.5, 0.32, 0.95 });
		Triangle{ cameraTip, cameraLeft, cameraRight }.draw(ColorF{ 0.98, 0.5, 0.32, 0.7 });
		Circle{ cameraCenter, 4.0 }.draw(ColorF{ 1.0, 0.72, 0.52, 0.98 });
	}

	void DrawSapperMenu(const SkyAppPanels& panels,
		Array<SpawnedSapper>& spawnedSappers,
     const Vec3& playerBasePosition,
		const Vec3& rallyPoint,
        ResourceStock& playerResources,
		const double sapperCost,
		TimedMessage& sapperMenuMessage)
	{
		panels.sapperMenu.draw(ColorF{ 0.97, 0.95 });
		panels.sapperMenu.drawFrame(2, 0, ColorF{ 0.25 });
		SimpleGUI::GetFont()(U"工兵メニュー").draw((panels.sapperMenu.x + 16), (panels.sapperMenu.y + 12), ColorF{ 0.12 });
       SimpleGUI::GetFont()(U"予算: {:.0f}"_fmt(playerResources.budget)).draw((panels.sapperMenu.x + 16), (panels.sapperMenu.y + 38), ColorF{ 0.12 });
		SimpleGUI::GetFont()(U"生産").draw((panels.sapperMenu.x + 16), (panels.sapperMenu.y + 66), ColorF{ 0.22 });
		SimpleGUI::GetFont()(U"スキル").draw((panels.sapperMenu.x + 16), (panels.sapperMenu.y + 156), ColorF{ 0.22 });

       const Rect produceSapperButton{ (panels.sapperMenu.x + 16), (panels.sapperMenu.y + 90), (panels.sapperMenu.w - 32), 28 };
		const Rect tierUpgradeButton{ (panels.sapperMenu.x + 16), (panels.sapperMenu.y + 122), (panels.sapperMenu.w - 32), 28 };
		const Rect scoutingSkillButton{ (panels.sapperMenu.x + 16), (panels.sapperMenu.y + 180), (panels.sapperMenu.w - 32), 28 };
		const Rect fortifySkillButton{ (panels.sapperMenu.x + 16), (panels.sapperMenu.y + 212), (panels.sapperMenu.w - 32), 28 };

       if (DrawTextButton(produceSapperButton, U"工兵産出 ({:.0f})"_fmt(sapperCost)))
		{
          if (sapperCost <= playerResources.budget)
			{
					SpawnSapper(spawnedSappers, playerBasePosition, rallyPoint);
              playerResources.budget -= sapperCost;
				sapperMenuMessage.show(U"工兵を産出");
			}
			else
			{
				sapperMenuMessage.show(U"資源不足");
			}
		}

		if (DrawTextButton(tierUpgradeButton, U"ティアアップグレード"))
		{
			sapperMenuMessage.show(U"ティアアップグレードを選択");
		}

		if (DrawTextButton(scoutingSkillButton, U"索敵スキル"))
		{
			sapperMenuMessage.show(U"工兵が索敵スキルを準備");
		}

		if (DrawTextButton(fortifySkillButton, U"陣地化スキル"))
		{
			sapperMenuMessage.show(U"工兵が陣地化スキルを準備");
		}

		if (sapperMenuMessage.isVisible())
		{
			SimpleGUI::GetFont()(sapperMenuMessage.text).draw((panels.sapperMenu.x + 16), (panels.sapperMenu.y - 28), ColorF{ 0.12 });
		}
	}
}
