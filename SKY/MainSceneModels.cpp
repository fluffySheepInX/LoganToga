# include "MainScene.hpp"

namespace
{
   struct PlacedModelRenderContext
	{
		const PlacedModel& placedModel;
		const MainSupport::ModelHeightSettings& modelHeightSettings;
		const MainSupport::PlacedModelRenderResources& renderResources;
	};

	struct LinearPlacedModelSegment
	{
		Vec3 start;
		Vec3 goal;
	};

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

	[[nodiscard]] Vec3 GetPlacedModelForward(const double yaw)
	{
		return Vec3{ Math::Sin(yaw), 0.0, Math::Cos(yaw) };
	}

	[[nodiscard]] LinearPlacedModelSegment GetLinearPlacedModelSegment(const Vec3& position, const double yaw, const double length)
	{
		const Vec3 halfDirection = (GetPlacedModelForward(yaw) * (length * 0.5));
		return LinearPlacedModelSegment{
			.start = (position - halfDirection),
			.goal = (position + halfDirection),
		};
	}

	[[nodiscard]] double GetPlacedWallLength(const PlacedModel& placedModel)
	{
		return Clamp(placedModel.wallLength, 2.0, 80.0);
	}

	[[nodiscard]] double GetPlacedRoadLength(const PlacedModel& placedModel)
	{
		return Clamp(placedModel.roadLength, 2.0, 80.0);
	}

	[[nodiscard]] double GetPlacedRoadWidth(const PlacedModel& placedModel)
	{
		return Clamp(placedModel.roadWidth, 2.0, 80.0);
	}

	void DrawAlphaToCoverageModel(const Model& model, const Vec3& position)
	{
		const ScopedRenderStates3D renderState{ BlendState::OpaqueAlphaToCoverage, RasterizerState::SolidCullNone };
		model.draw(position);
	}

	void DrawAlphaToCoverageModel(const Model& model, const Mat4x4& transform)
	{
		const ScopedRenderStates3D renderState{ BlendState::OpaqueAlphaToCoverage, RasterizerState::SolidCullNone };
		model.draw(transform);
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

	void DrawMillPlacedModel(const PlacedModelRenderContext& context)
	{
		DrawGroundContactGrime(context.placedModel.position, 4.4, 3.1, ColorF{ 0.22, 0.18, 0.14, 0.36 }, ColorF{ 0.34, 0.28, 0.20, 0.22 });
		DrawMillModel(context.renderResources.millModel, Mat4x4::Translate(context.placedModel.position));
	}

	void DrawTreePlacedModel(const PlacedModelRenderContext& context)
	{
		DrawGroundContactGrime(context.placedModel.position, 1.42, 0.84, ColorF{ 0.18, 0.17, 0.13, 0.28 }, ColorF{ 0.26, 0.24, 0.18, 0.18 });
		DrawAlphaToCoverageModel(context.renderResources.treeModel, context.placedModel.position);
	}

	void DrawPinePlacedModel(const PlacedModelRenderContext& context)
	{
		DrawGroundContactGrime(context.placedModel.position, 1.28, 0.76, ColorF{ 0.16, 0.16, 0.12, 0.26 }, ColorF{ 0.24, 0.22, 0.17, 0.16 });
		DrawAlphaToCoverageModel(context.renderResources.pineModel, context.placedModel.position);
	}

	void DrawGrassPatchPlacedModel(const PlacedModelRenderContext& context)
	{
		DrawAlphaToCoverageModel(context.renderResources.grassPatchModel, Mat4x4::Translate(context.placedModel.position));
	}

	void DrawRockPlacedModel(const PlacedModel& placedModel)
	{
		DrawGroundContactGrime(placedModel.position, 2.6, 1.9, ColorF{ 0.15, 0.15, 0.14, 0.32 }, ColorF{ 0.28, 0.26, 0.24, 0.16 });
		Cylinder{ placedModel.position.movedBy(0, 0.45, 0), 2.1, 0.9 }.draw(ColorF{ 0.36, 0.39, 0.42 }.removeSRGBCurve());
		Sphere{ placedModel.position.movedBy(0, 1.45, 0), 1.75 }.draw(ColorF{ 0.48, 0.50, 0.54 }.removeSRGBCurve());
		Sphere{ placedModel.position.movedBy(-0.8, 1.1, 0.6), 0.82 }.draw(ColorF{ 0.42, 0.45, 0.50 }.removeSRGBCurve());
	}

	void DrawWallPlacedModel(const PlacedModel& placedModel)
	{
		const double wallLength = GetPlacedWallLength(placedModel);
		const LinearPlacedModelSegment segment = GetLinearPlacedModelSegment(placedModel.position, placedModel.yaw, wallLength);
		Line3D{ segment.start.movedBy(0, 0.12, 0), segment.goal.movedBy(0, 0.12, 0) }.draw(ColorF{ 0.62, 0.66, 0.72, 0.95 }.removeSRGBCurve());
		Cylinder{ segment.start.movedBy(0, 0.55, 0), 0.26, 2.2 }.draw(ColorF{ 0.52, 0.54, 0.58 }.removeSRGBCurve());
		Cylinder{ segment.goal.movedBy(0, 0.55, 0), 0.26, 2.2 }.draw(ColorF{ 0.52, 0.54, 0.58 }.removeSRGBCurve());

		const int32 segmentCount = Max(1, static_cast<int32>(Math::Ceil(wallLength / 1.2)));
		for (int32 i = 0; i <= segmentCount; ++i)
		{
			const double t = (static_cast<double>(i) / segmentCount);
			const Vec3 position = segment.start.lerp(segment.goal, t);
			Cylinder{ position.movedBy(0, 0.42, 0), 0.34, 0.84 }.draw(ColorF{ 0.58, 0.60, 0.65 }.removeSRGBCurve());
		}
	}

	void DrawRoadPlacedModel(const PlacedModelRenderContext& context)
	{
		const double roadLength = GetPlacedRoadLength(context.placedModel);
		const double roadWidth = GetPlacedRoadWidth(context.placedModel);
		DrawRoadShoulderGrime(context.placedModel, roadWidth, roadLength);
		const ScopedRenderStates3D renderState{ BlendState::Opaque, RasterizerState::SolidCullNone };
		const Transformer3D t{
			Mat4x4::Identity()
				.scaled(Float3{ static_cast<float>(roadWidth), 1.0f, static_cast<float>(roadLength) })
				.rotated(Quaternion::RotateY(static_cast<float>(context.placedModel.yaw)))
				.translated(context.placedModel.position.movedBy(0, 0.025, 0))
		};
		RoadPlaneMesh().draw(context.renderResources.roadTexture);
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
        const double decalLength = GetPlacedRoadLength(placedModel);
		const double decalWidth = GetPlacedRoadWidth(placedModel);
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
        const Vec3 direction = GetPlacedModelForward(placedModel.yaw);
		const LinearPlacedModelSegment segment = GetLinearPlacedModelSegment(placedModel.position, placedModel.yaw, decalLength);

      DrawTexturedGroundStripSegment(startTexture, (segment.start + (direction * (capLength * 0.5))), decalWidth, capLength, placedModel.yaw, startTint, startSoftness, startYOffset);

		if (centerLength > 0.01)
		{
            Vec3 middleCursor = (segment.start + (direction * capLength));
			double remainingLength = centerLength;

			while (remainingLength > 0.01)
			{
				const double segmentLength = Min(middleTileLength, remainingLength);
             DrawTexturedGroundStripSegment(middleTexture, (middleCursor + (direction * (segmentLength * 0.5))), decalWidth, segmentLength, placedModel.yaw, middleTint, middleSoftness, middleYOffset);
				middleCursor += (direction * segmentLength);
				remainingLength -= segmentLength;
			}
		}

       DrawTexturedGroundStripSegment(endTexture, (segment.goal - (direction * (capLength * 0.5))), decalWidth, capLength, placedModel.yaw, endTint, endSoftness, endYOffset);
	}
}

namespace MainSupport
{
  void DrawPlacedModel(const PlacedModel& placedModel, const ModelHeightSettings& modelHeightSettings, const PlacedModelRenderResources& renderResources)
	{
       const PlacedModelRenderContext context{ placedModel, modelHeightSettings, renderResources };

		switch (placedModel.type)
		{
		case PlaceableModelType::Mill:
           DrawMillPlacedModel(context);
			return;

		case PlaceableModelType::Tree:
           DrawTreePlacedModel(context);
			return;

		case PlaceableModelType::Pine:
           DrawPinePlacedModel(context);
			return;

		case PlaceableModelType::GrassPatch:
           DrawGrassPatchPlacedModel(context);
			return;

		case PlaceableModelType::Rock:
           DrawRockPlacedModel(placedModel);
			return;

		case PlaceableModelType::Wall:
           DrawWallPlacedModel(placedModel);
			return;

		case PlaceableModelType::Road:
           DrawRoadPlacedModel(context);
			return;

		case PlaceableModelType::TireTrackDecal:
            DrawTireTrackDecal(placedModel, modelHeightSettings, renderResources.tireTrackStartTexture, renderResources.tireTrackMiddleTexture, renderResources.tireTrackEndTexture);
			return;

		default:
			return;
		}
	}
}
