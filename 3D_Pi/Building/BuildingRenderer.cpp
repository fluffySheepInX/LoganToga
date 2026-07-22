# include "../stdafx.h"
# include "BuildingRenderer.hpp"

namespace building
{
	namespace
	{
		[[nodiscard]] Vec3 LocalToWorld(const GeneratedBuilding& building, const Vec3& local)
		{
			const double angle = building.rotation01 * Math::TwoPi;
			const double c = Math::Cos(angle);
			const double s = Math::Sin(angle);
			return building.origin + Vec3{
				local.x * c - local.z * s,
				local.y,
				local.x * s + local.z * c
			};
		}

		[[nodiscard]] ColorF WithAlpha(ColorF color, const double alpha)
		{
			color.a = alpha;
			return color;
		}

		[[nodiscard]] ColorF StyleTint(const GeneratedBuilding& building, ColorF color)
		{
			switch (building.facadeStyle)
			{
			case BuildingFacadeStyle::Simple:
				break;
			case BuildingFacadeStyle::Warehouse:
				color.r *= 0.78;
				color.g *= 0.80;
				color.b *= 0.84;
				break;
			case BuildingFacadeStyle::Haunted:
				color.r *= 0.58;
				color.g *= 0.62;
				color.b *= 0.66;
				break;
			case BuildingFacadeStyle::OldTown:
			default:
				color.r *= 0.92;
				color.g *= 0.88;
				color.b *= 0.82;
				break;
			}
			return ColorF{ Clamp(color.r, 0.0, 1.0), Clamp(color.g, 0.0, 1.0), Clamp(color.b, 0.0, 1.0), color.a };
		}

		void DrawLocalBox(const GeneratedBuilding& building, const Vec3& localCenter, const Vec3& size, const ColorF& color)
		{
			const Transformer3D transform{
				Mat4x4::Identity()
					.rotated(Quaternion::RotateY(static_cast<float>(building.rotation01 * Math::TwoPi)))
					.translated(building.origin)
			};
			Box{ localCenter, size.x, size.y, size.z }.draw(color.removeSRGBCurve());
		}
	}

	void BuildingRenderer::draw(const BuildingDocument& document, const Optional<String>& selectedBuildingId, const Optional<GeneratedBuilding>& placementGhost) const
	{
		for (const auto& building : document.buildings)
		{
			drawBuilding(building, selectedBuildingId && (*selectedBuildingId == building.id), false);
		}

		if (placementGhost)
		{
			drawBuilding(*placementGhost, false, true);
		}
	}

	void BuildingRenderer::drawBuilding(const GeneratedBuilding& building, const bool selected, const bool ghost) const
	{
		GeneratedBuilding safeBuilding = building;
		Sanitize(safeBuilding);

		const double width = safeBuilding.footprintSize.x;
		const double depth = safeBuilding.footprintSize.y;
		const double wallHeight = safeBuilding.floors * safeBuilding.floorHeight;
		const double alpha = ghost ? 0.42 : 1.0;
		const ColorF wallColor = WithAlpha(StyleTint(safeBuilding, safeBuilding.materialSet.wallColor), alpha);
		const ColorF roofColor = WithAlpha(safeBuilding.materialSet.roofColor, alpha);
		const ColorF trimColor = WithAlpha(safeBuilding.materialSet.trimColor, alpha);

		const ScopedRenderStates3D renderState{ BlendState::OpaqueAlphaToCoverage, RasterizerState::SolidCullNone };
		DrawLocalBox(safeBuilding, Vec3{ 0, wallHeight * 0.5, 0 }, Vec3{ width, wallHeight, depth }, wallColor);

		for (int32 floor = 1; floor < safeBuilding.floors; ++floor)
		{
			DrawLocalBox(safeBuilding, Vec3{ 0, floor * safeBuilding.floorHeight, -depth * 0.5 - 0.012 }, Vec3{ width + 0.08, 0.05, 0.05 }, trimColor);
			DrawLocalBox(safeBuilding, Vec3{ 0, floor * safeBuilding.floorHeight, depth * 0.5 + 0.012 }, Vec3{ width + 0.08, 0.05, 0.05 }, trimColor);
		}

		if (safeBuilding.roofType == BuildingRoofType::Flat)
		{
			DrawLocalBox(safeBuilding, Vec3{ 0, wallHeight + 0.12, 0 }, Vec3{ width + 0.28, 0.24 + safeBuilding.roofHeight * 0.12, depth + 0.28 }, roofColor);
		}
		else
		{
			DrawLocalBox(safeBuilding, Vec3{ 0, wallHeight + safeBuilding.roofHeight * 0.32, 0 }, Vec3{ width + 0.34, Max(0.18, safeBuilding.roofHeight * 0.64), depth + 0.34 }, roofColor);
			const ColorF ridgeColor = ColorF{ Min(roofColor.r + 0.08, 1.0), Min(roofColor.g + 0.06, 1.0), Min(roofColor.b + 0.05, 1.0), roofColor.a };
			if (safeBuilding.roofType == BuildingRoofType::Gable)
			{
				DrawLocalBox(safeBuilding, Vec3{ 0, wallHeight + safeBuilding.roofHeight + 0.04, 0 }, Vec3{ width + 0.18, 0.10, 0.18 }, ridgeColor);
			}
			else
			{
				DrawLocalBox(safeBuilding, Vec3{ 0, wallHeight + safeBuilding.roofHeight + 0.02, 0 }, Vec3{ width * 0.36, 0.10, depth * 0.36 }, ridgeColor);
			}
		}

		drawOpenings(safeBuilding, wallHeight, ghost);

		if (selected)
		{
			drawSelectionBounds(safeBuilding);
		}
	}

	void BuildingRenderer::drawOpenings(const GeneratedBuilding& building, const double wallHeight, const bool ghost) const
	{
		for (const auto& opening : building.openings)
		{
			const int32 repeatCount = Clamp(opening.repeatCount, 1, 12);
			for (int32 i = 0; i < repeatCount; ++i)
			{
				drawOpening(building, opening, i, wallHeight, ghost);
			}
		}
	}

	void BuildingRenderer::drawOpening(const GeneratedBuilding& building, const BuildingOpening& opening, const int32 repeatIndex, const double wallHeight, const bool ghost) const
	{
		const double width = building.footprintSize.x;
		const double depth = building.footprintSize.y;
		const double baseX = Math::Lerp(-width * 0.38, width * 0.38, Clamp(opening.normalizedX, 0.0, 1.0));
		const double xOffset = (repeatIndex - (opening.repeatCount - 1) * 0.5) * opening.repeatSpacing;
		const double floorBase = Clamp(opening.floorIndex, 0, building.floors - 1) * building.floorHeight;
		const double centerY = (opening.type == BuildingOpeningType::Door) ? (opening.size.y * 0.5) : (floorBase + building.floorHeight * 0.55);
		const Vec2 size{ Clamp(opening.size.x, 0.2, 4.0), Clamp(opening.size.y, 0.2, building.floorHeight * 0.9) };
		const ColorF openingColor = WithAlpha(opening.type == BuildingOpeningType::Door ? ColorF{ 0.16, 0.08, 0.035, 1.0 } : ColorF{ 0.08, 0.16, 0.22, 1.0 }, ghost ? 0.38 : 1.0);

		Vec3 localCenter{ baseX + xOffset, Clamp(centerY, 0.25, wallHeight - 0.12), -depth * 0.5 - 0.028 };
		Vec3 localSize{ size.x, size.y, 0.055 };

		if (opening.wallSide == BuildingWallSide::Back)
		{
			localCenter.z = depth * 0.5 + 0.028;
		}
		else if (opening.wallSide == BuildingWallSide::Left)
		{
			localCenter = Vec3{ -width * 0.5 - 0.028, localCenter.y, baseX + xOffset };
			localSize = Vec3{ 0.055, size.y, size.x };
		}
		else if (opening.wallSide == BuildingWallSide::Right)
		{
			localCenter = Vec3{ width * 0.5 + 0.028, localCenter.y, baseX + xOffset };
			localSize = Vec3{ 0.055, size.y, size.x };
		}

		DrawLocalBox(building, localCenter, localSize, openingColor);
	}

	void BuildingRenderer::drawSelectionBounds(const GeneratedBuilding& building) const
	{
		const double width = building.footprintSize.x + 0.18;
		const double depth = building.footprintSize.y + 0.18;
		const double height = building.floors * building.floorHeight + building.roofHeight + 0.18;
		const ColorF color{ 0.24, 0.58, 1.0, 1.0 };
		const ScopedRenderStates3D wireState{ BlendState::Opaque, RasterizerState::WireframeCullNone };
		DrawLocalBox(building, Vec3{ 0, height * 0.5, 0 }, Vec3{ width, height, depth }, color);
	}
}
