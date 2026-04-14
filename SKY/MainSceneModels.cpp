# include "MainScene.hpp"

namespace
{
	[[nodiscard]] const Mesh& RoadPlaneMesh()
	{
		static const Mesh mesh{ MeshData::OneSidedPlane(1.0, { 1, 1 }) };
		return mesh;
	}

	[[nodiscard]] const BlendState& GroundOverlayBlendState()
	{
		static const BlendState blendState = []
			{
				BlendState state = BlendState::Default2D;
             state.src = Blend::SrcAlpha;
				state.dst = Blend::InvSrcAlpha;
				state.op = BlendOp::Add;
				state.srcAlpha = Blend::One;
				state.dstAlpha = Blend::InvSrcAlpha;
				state.opAlpha = BlendOp::Add;
				return state;
			}();
		return blendState;
	}

	void DrawMillModel(const Model& model, const Mat4x4& mat)
	{
		const auto& materials = model.materials();

		for (const auto& object : model.objects())
		{
			Mat4x4 m = Mat4x4::Identity();

			if (object.name == U"Mill_Blades_Cube.007")
			{
				m *= Mat4x4::Rotate(Vec3{ 0, 0, -1 }, (Scene::Time() * -120_deg), Vec3{ 0, 9.37401, 0 });
			}

			const Transformer3D t{ (m * mat) };
			object.draw(materials);
		}
	}

	void DrawGroundContactGrime(const Vec3& position,
		const double outerRadius,
		const double innerRadius,
		const ColorF& outerColor,
		const ColorF& innerColor)
	{
        const ScopedRenderStates3D renderState{ GroundOverlayBlendState(), RasterizerState::SolidCullNone };
		Cylinder{ position.movedBy(0, 0.010, 0), outerRadius, 0.020 }.draw(outerColor.removeSRGBCurve());
		Cylinder{ position.movedBy(0, 0.015, 0), innerRadius, 0.014 }.draw(innerColor.removeSRGBCurve());
	}

	void DrawRoadShoulderGrime(const PlacedModel& placedModel, const double roadWidth, const double roadLength)
	{
        const ScopedRenderStates3D renderState{ GroundOverlayBlendState(), RasterizerState::SolidCullNone };
		const Transformer3D t{
			Mat4x4::Identity()
				.rotated(Quaternion::RotateY(static_cast<float>(placedModel.yaw)))
				.translated(placedModel.position.movedBy(0, 0.010, 0))
		};

		Box{ Vec3{ 0, 0, 0 }, (roadWidth + 1.8), 0.014, (roadLength + 0.9) }.draw(ColorF{ 0.24, 0.19, 0.14, 0.30 }.removeSRGBCurve());
		Box{ Vec3{ 0, 0.003, 0 }, (roadWidth + 0.8), 0.010, Max(1.0, (roadLength - 0.2)) }.draw(ColorF{ 0.36, 0.28, 0.18, 0.18 }.removeSRGBCurve());

		const double shoulderOffset = (roadWidth * 0.5 + 0.28);
		Box{ Vec3{ -shoulderOffset, 0.004, 0 }, 0.56, 0.012, (roadLength + 0.5) }.draw(ColorF{ 0.22, 0.18, 0.14, 0.42 }.removeSRGBCurve());
		Box{ Vec3{ shoulderOffset, 0.004, 0 }, 0.56, 0.012, (roadLength + 0.5) }.draw(ColorF{ 0.22, 0.18, 0.14, 0.42 }.removeSRGBCurve());

		const double rutOffset = Max(0.32, (roadWidth * 0.22));
		Box{ Vec3{ -rutOffset, 0.006, 0 }, 0.34, 0.008, Max(1.2, (roadLength - 0.6)) }.draw(ColorF{ 0.18, 0.15, 0.12, 0.20 }.removeSRGBCurve());
		Box{ Vec3{ rutOffset, 0.006, 0 }, 0.34, 0.008, Max(1.2, (roadLength - 0.6)) }.draw(ColorF{ 0.18, 0.15, 0.12, 0.20 }.removeSRGBCurve());
	}

  [[nodiscard]] ColorF MakeTireTrackTint(const MainSupport::ModelHeightSettings& modelHeightSettings, const MainSupport::TireTrackTextureSegment segment)
	{
		const double warmth = MainSupport::GetTireTrackWarmth(modelHeightSettings, segment);
		const double opacity = MainSupport::GetTireTrackOpacity(modelHeightSettings, segment);
		const ColorF charcoal{ 0.08, 0.08, 0.08, opacity };
		const ColorF earth{ 0.28, 0.20, 0.14, opacity };
		return charcoal.lerp(earth, warmth);
	}

	void DrawTexturedGroundStripSegment(const Texture& texture, const Vec3& position, const double width, const double length, const double yaw, const ColorF& tint, const double softness, const double yOffset = 0.018)
	{
		if ((width <= 0.01) || (length <= 0.01))
		{
			return;
		}

		const ScopedRenderStates3D renderState{ GroundOverlayBlendState(), RasterizerState::SolidCullNone };

		auto drawSegment = [&](const double drawWidth, const double drawLength, const double drawYOffset, const ColorF& drawTint)
			{
				const Transformer3D t{
					Mat4x4::Identity()
						.scaled(Float3{ static_cast<float>(drawWidth), 1.0f, static_cast<float>(drawLength) })
						.rotated(Quaternion::RotateY(static_cast<float>(yaw)))
						.translated(position.movedBy(0, drawYOffset, 0))
				};
				RoadPlaneMesh().draw(texture, drawTint.removeSRGBCurve());
			};

		if (0.001 < softness)
		{
			const double blurAlpha = (tint.a * softness * 0.42);
			drawSegment(width * (1.0 + softness * 0.20), length * (1.0 + softness * 0.10), (yOffset + 0.0004), ColorF{ tint.r, tint.g, tint.b, blurAlpha });
		}

		drawSegment(width, length, yOffset, tint);
	}

    void DrawTireTrackDecal(const PlacedModel& placedModel, const MainSupport::ModelHeightSettings& modelHeightSettings, const Texture& startTexture, const Texture& middleTexture, const Texture& endTexture)
	{
		const double decalLength = Clamp(placedModel.roadLength, 2.0, 80.0);
		const double decalWidth = Clamp(placedModel.roadWidth, 2.0, 80.0);
		const double capLength = Min(decalWidth, (decalLength * 0.5));
		const double centerLength = Max(0.0, (decalLength - (capLength * 2.0)));
        const double middleTileLength = Max(0.5, capLength);
       const double startYOffset = MainSupport::GetTireTrackYOffset(modelHeightSettings, MainSupport::TireTrackTextureSegment::Start);
		const double middleYOffset = MainSupport::GetTireTrackYOffset(modelHeightSettings, MainSupport::TireTrackTextureSegment::Middle);
		const double endYOffset = MainSupport::GetTireTrackYOffset(modelHeightSettings, MainSupport::TireTrackTextureSegment::End);
        const double startSoftness = MainSupport::GetTireTrackSoftness(modelHeightSettings, MainSupport::TireTrackTextureSegment::Start);
		const double middleSoftness = MainSupport::GetTireTrackSoftness(modelHeightSettings, MainSupport::TireTrackTextureSegment::Middle);
		const double endSoftness = MainSupport::GetTireTrackSoftness(modelHeightSettings, MainSupport::TireTrackTextureSegment::End);
		const ColorF startTint = MakeTireTrackTint(modelHeightSettings, MainSupport::TireTrackTextureSegment::Start);
		const ColorF middleTint = MakeTireTrackTint(modelHeightSettings, MainSupport::TireTrackTextureSegment::Middle);
		const ColorF endTint = MakeTireTrackTint(modelHeightSettings, MainSupport::TireTrackTextureSegment::End);
		const Vec3 direction{ Math::Sin(placedModel.yaw), 0.0, Math::Cos(placedModel.yaw) };
		const Vec3 startEdge = (placedModel.position - (direction * (decalLength * 0.5)));
		const Vec3 endEdge = (placedModel.position + (direction * (decalLength * 0.5)));

      DrawTexturedGroundStripSegment(startTexture, (startEdge + (direction * (capLength * 0.5))), decalWidth, capLength, placedModel.yaw, startTint, startSoftness, startYOffset);

		if (centerLength > 0.01)
		{
          Vec3 middleCursor = (startEdge + (direction * capLength));
			double remainingLength = centerLength;

			while (remainingLength > 0.01)
			{
				const double segmentLength = Min(middleTileLength, remainingLength);
                DrawTexturedGroundStripSegment(middleTexture, (middleCursor + (direction * (segmentLength * 0.5))), decalWidth, segmentLength, placedModel.yaw, middleTint, middleSoftness, middleYOffset);
				middleCursor += (direction * segmentLength);
				remainingLength -= segmentLength;
			}
		}

       DrawTexturedGroundStripSegment(endTexture, (endEdge - (direction * (capLength * 0.5))), decalWidth, capLength, placedModel.yaw, endTint, endSoftness, endYOffset);
	}
}

namespace MainSupport
{
  void DrawPlacedModel(const PlacedModel& placedModel, const ModelHeightSettings& modelHeightSettings, const Model& millModel, const Model& treeModel, const Model& pineModel, const Model& grassPatchModel, const Texture& roadTexture, const Texture& tireTrackStartTexture, const Texture& tireTrackMiddleTexture, const Texture& tireTrackEndTexture)
	{
		switch (placedModel.type)
		{
		case PlaceableModelType::Mill:
          DrawGroundContactGrime(placedModel.position, 4.4, 3.1, ColorF{ 0.22, 0.18, 0.14, 0.36 }, ColorF{ 0.34, 0.28, 0.20, 0.22 });
			DrawMillModel(millModel, Mat4x4::Translate(placedModel.position));
			return;

		case PlaceableModelType::Tree:
		{
            DrawGroundContactGrime(placedModel.position, 1.42, 0.84, ColorF{ 0.18, 0.17, 0.13, 0.28 }, ColorF{ 0.26, 0.24, 0.18, 0.18 });
			const ScopedRenderStates3D renderState{ BlendState::OpaqueAlphaToCoverage, RasterizerState::SolidCullNone };
			treeModel.draw(placedModel.position);
			return;
		}

		case PlaceableModelType::Pine:
		{
            DrawGroundContactGrime(placedModel.position, 1.28, 0.76, ColorF{ 0.16, 0.16, 0.12, 0.26 }, ColorF{ 0.24, 0.22, 0.17, 0.16 });
			const ScopedRenderStates3D renderState{ BlendState::OpaqueAlphaToCoverage, RasterizerState::SolidCullNone };
			pineModel.draw(placedModel.position);
			return;
		}

		case PlaceableModelType::GrassPatch:
		{
			const ScopedRenderStates3D renderState{ BlendState::OpaqueAlphaToCoverage, RasterizerState::SolidCullNone };
			grassPatchModel.draw(Mat4x4::Translate(placedModel.position));
			return;
		}

		case PlaceableModelType::Rock:
		{
          DrawGroundContactGrime(placedModel.position, 2.6, 1.9, ColorF{ 0.15, 0.15, 0.14, 0.32 }, ColorF{ 0.28, 0.26, 0.24, 0.16 });
			Cylinder{ placedModel.position.movedBy(0, 0.45, 0), 2.1, 0.9 }.draw(ColorF{ 0.36, 0.39, 0.42 }.removeSRGBCurve());
			Sphere{ placedModel.position.movedBy(0, 1.45, 0), 1.75 }.draw(ColorF{ 0.48, 0.50, 0.54 }.removeSRGBCurve());
			Sphere{ placedModel.position.movedBy(-0.8, 1.1, 0.6), 0.82 }.draw(ColorF{ 0.42, 0.45, 0.50 }.removeSRGBCurve());
			return;
		}

		case PlaceableModelType::Wall:
		{
			const double wallLength = Clamp(placedModel.wallLength, 2.0, 80.0);
			const Vec3 direction{ Math::Sin(placedModel.yaw), 0.0, Math::Cos(placedModel.yaw) };
			const Vec3 halfDirection = (direction * (wallLength * 0.5));
			const Vec3 start = (placedModel.position - halfDirection);
			const Vec3 goal = (placedModel.position + halfDirection);
			Line3D{ start.movedBy(0, 0.12, 0), goal.movedBy(0, 0.12, 0) }.draw(ColorF{ 0.62, 0.66, 0.72, 0.95 }.removeSRGBCurve());
			Cylinder{ start.movedBy(0, 0.55, 0), 0.26, 2.2 }.draw(ColorF{ 0.52, 0.54, 0.58 }.removeSRGBCurve());
			Cylinder{ goal.movedBy(0, 0.55, 0), 0.26, 2.2 }.draw(ColorF{ 0.52, 0.54, 0.58 }.removeSRGBCurve());

			const int32 segmentCount = Max(1, static_cast<int32>(Math::Ceil(wallLength / 1.2)));
			for (int32 i = 0; i <= segmentCount; ++i)
			{
				const double t = (static_cast<double>(i) / segmentCount);
				const Vec3 position = start.lerp(goal, t);
				Cylinder{ position.movedBy(0, 0.42, 0), 0.34, 0.84 }.draw(ColorF{ 0.58, 0.60, 0.65 }.removeSRGBCurve());
			}
			return;
		}

		case PlaceableModelType::Road:
		{
			const double roadLength = Clamp(placedModel.roadLength, 2.0, 80.0);
			const double roadWidth = Clamp(placedModel.roadWidth, 2.0, 80.0);
           DrawRoadShoulderGrime(placedModel, roadWidth, roadLength);
			const ScopedRenderStates3D renderState{ BlendState::Opaque, RasterizerState::SolidCullNone };
			const Transformer3D t{
				Mat4x4::Identity()
					.scaled(Float3{ static_cast<float>(roadWidth), 1.0f, static_cast<float>(roadLength) })
					.rotated(Quaternion::RotateY(static_cast<float>(placedModel.yaw)))
					.translated(placedModel.position.movedBy(0, 0.025, 0))
			};
			RoadPlaneMesh().draw(roadTexture);
			return;
		}

		case PlaceableModelType::TireTrackDecal:
            DrawTireTrackDecal(placedModel, modelHeightSettings, tireTrackStartTexture, tireTrackMiddleTexture, tireTrackEndTexture);
			return;

		default:
			return;
		}
	}
}
