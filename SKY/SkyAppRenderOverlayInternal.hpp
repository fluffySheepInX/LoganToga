# pragma once
# include "SkyAppLoopInternal.hpp"
# include "SkyAppSupport.hpp"
# include "MainUi.hpp"
# include "MainScene.hpp"

namespace SkyAppFlow
{
	namespace OverlayDetail
	{
		[[nodiscard]] inline ColorF WithAlphaScaled(const ColorF& color, const double alphaScale)
		{
			return ColorF{ color.r, color.g, color.b, Math::Saturate(color.a * alphaScale) };
		}

		[[nodiscard]] inline ColorF GetTeamColor(const Optional<MainSupport::UnitTeam>& team)
		{
			if (team && (*team == MainSupport::UnitTeam::Player))
			{
				return ColorF{ 0.92, 0.95, 1.0, 0.95 };
			}

			if (team && (*team == MainSupport::UnitTeam::Enemy))
			{
				return ColorF{ 1.0, 0.78, 0.74, 0.95 };
			}

			return ColorF{ 0.72, 0.78, 0.84, 0.90 };
		}

		[[nodiscard]] inline StringView ToDisplayLabel(const MainSupport::ResourceType type)
		{
			switch (type)
			{
			case MainSupport::ResourceType::Budget:
				return U"予算";

			case MainSupport::ResourceType::Gunpowder:
				return U"火薬";

			case MainSupport::ResourceType::Mana:
				return U"魔力";

			default:
				return U"資源";
			}
		}

		[[nodiscard]] inline StringView ToOwnerLabel(const Optional<MainSupport::UnitTeam>& team)
		{
			if (team && (*team == MainSupport::UnitTeam::Player))
			{
				return U"自軍";
			}

			if (team && (*team == MainSupport::UnitTeam::Enemy))
			{
				return U"敵軍";
			}

			return U"中立";
		}

		[[nodiscard]] inline double& GetResourceValue(MainSupport::ResourceStock& resources, const MainSupport::ResourceType type)
		{
			switch (type)
			{
			case MainSupport::ResourceType::Budget:
				return resources.budget;

			case MainSupport::ResourceType::Gunpowder:
				return resources.gunpowder;

			case MainSupport::ResourceType::Mana:
				return resources.mana;

			default:
				return resources.budget;
			}
		}

		[[nodiscard]] inline double GetResourceAdjustStep(const MainSupport::ResourceType type)
		{
			switch (type)
			{
			case MainSupport::ResourceType::Budget:
				return 50.0;

			case MainSupport::ResourceType::Gunpowder:
			case MainSupport::ResourceType::Mana:
			default:
				return 10.0;
			}
		}

		[[nodiscard]] inline ColorF GetResourceTextColor(const MainSupport::ResourceType type)
		{
			switch (type)
			{
			case MainSupport::ResourceType::Budget:
				return ColorF{ 0.98, 0.90, 0.38 };

			case MainSupport::ResourceType::Gunpowder:
				return ColorF{ 0.98, 0.56, 0.42 };

			case MainSupport::ResourceType::Mana:
				return ColorF{ 0.58, 0.72, 1.0 };

			default:
				return Palette::White;
			}
		}

		[[nodiscard]] inline Optional<Vec2> ProjectToScreen(const MainSupport::AppCamera3D& camera, const Vec3& worldPosition)
		{
			const Float3 screenPosition = camera.worldToScreenPoint(Float3{ static_cast<float>(worldPosition.x), static_cast<float>(worldPosition.y), static_cast<float>(worldPosition.z) });

			if (screenPosition.z <= 0.0f)
			{
				return none;
			}

			return Vec2{ screenPosition.x, screenPosition.y };
		}

		[[nodiscard]] inline RectF GetSelectionRect(const Optional<Vec2>& selectionDragStart)
		{
			const Vec2 start = *selectionDragStart;
			const Vec2 end = Cursor::PosF();
			return RectF{ Min(start.x, end.x), Min(start.y, end.y), Abs(end.x - start.x), Abs(end.y - start.y) };
		}

		void DrawGroundContactOverlays(SkyAppResources& resources, SkyAppState& state, const SkyAppFrameState& frame);
		void DrawBattleOverlays(SkyAppState& state, const SkyAppFrameState& frame);
		void DrawResourceAreaOverlays(SkyAppState& state);
		void DrawResourcePanelOverlay(SkyAppState& state, const SkyAppFrameState& frame);
        void DrawResourceLoadWarnings(const SkyAppResources& resources);
        void DrawUiEditGridOverlay(const SkyAppState& state);
		void DrawBaseStatusOverlays(SkyAppState& state);
		void DrawModelHeightOverlay(SkyAppState& state, const SkyAppFrameState& frame);
		void DrawSelectionDragOverlay(const SkyAppState& state, const SkyAppFrameState& frame);
		void DrawMatchResultOverlay(const SkyAppState& state);
	}
}
