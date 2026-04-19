# include "SkyAppSupport.hpp"
# include <memory>
# include "BirdModel.hpp"
# include "MainUi.hpp"
# include "MainScene.hpp"

using namespace MainSupport;

namespace SkyAppSupport
{
	namespace
	{
        struct CustomUnitRenderModelSet
		{
			UnitModel idleModel;
			UnitModel moveModel;
			UnitModel attackModel;
			double lastUpdatedAt = -1.0;

			CustomUnitRenderModelSet(const FilePath& modelPath, const UnitRenderModel baseRenderModel)
				: idleModel{ modelPath, BirdDisplayHeight, GetUnitRenderModelProceduralAnimationType(baseRenderModel) }
				, moveModel{ modelPath, BirdDisplayHeight, GetUnitRenderModelProceduralAnimationType(baseRenderModel) }
				, attackModel{ modelPath, BirdDisplayHeight, GetUnitRenderModelProceduralAnimationType(baseRenderModel) } {}
		};

		void ApplyConfiguredAnimationClip(UnitModel& renderModel, const int32 configuredClipIndex)
		{
			if (not renderModel.hasAnimations())
			{
				return;
			}

			if (configuredClipIndex < 0)
			{
				renderModel.clearClipIndex();
				return;
			}

			const size_t resolvedClipIndex = Min(static_cast<size_t>(Max(configuredClipIndex, 0)), (renderModel.animations().size() - 1));
			if (renderModel.currentClipIndex() != resolvedClipIndex)
			{
				renderModel.setClipIndex(resolvedClipIndex);
			}
		}

		[[nodiscard]] UnitModelAnimationRole ResolveSapperAnimationRole(const SpawnedSapper& sapper)
		{
			if (0.0 < sapper.moveDuration)
			{
				return UnitModelAnimationRole::Move;
			}

			if ((Scene::Time() - sapper.lastAttackAt) < GetEffectiveSapperAttackInterval(sapper))
			{
				return UnitModelAnimationRole::Attack;
			}

			return UnitModelAnimationRole::Idle;
		}

		[[nodiscard]] const UnitModel& SelectUnitRenderModel(const UnitRenderModelRegistryView& renderModels, const UnitRenderModel renderModel, const UnitModelAnimationRole role)
		{
			const size_t index = GetUnitRenderModelIndex(renderModel);
			const auto& models = (role == UnitModelAnimationRole::Move)
				? renderModels.moveModels
				: ((role == UnitModelAnimationRole::Attack)
					? renderModels.attackModels
					: renderModels.idleModels);

			if ((index < models.size()) && models[index])
			{
				return *models[index];
			}

			return *renderModels.idleModels[GetUnitRenderModelIndex(UnitRenderModel::Bird)];
		}

		[[nodiscard]] const UnitModel* SelectCustomRoleModel(const CustomUnitRenderModelSet& models, const UnitModelAnimationRole role)
		{
			switch (role)
			{
			case UnitModelAnimationRole::Move:
				return models.moveModel.isLoaded() ? &models.moveModel : nullptr;

			case UnitModelAnimationRole::Attack:
				return models.attackModel.isLoaded() ? &models.attackModel : nullptr;

			case UnitModelAnimationRole::Idle:
			default:
				return models.idleModel.isLoaded() ? &models.idleModel : nullptr;
			}
		}

     [[nodiscard]] const UnitModel* TrySelectCustomUnitRenderModel(const UnitEditorSettings& unitEditorSettings, const ModelHeightSettings& modelHeightSettings, const SpawnedSapper& sapper, const UnitModelAnimationRole role)
		{
			const FilePath& modelPath = GetUnitModelPath(unitEditorSettings, sapper.team, sapper.unitType);
			if (modelPath.isEmpty() || (not FileSystem::Exists(modelPath)))
			{
				return nullptr;
			}

           static HashTable<FilePath, std::shared_ptr<CustomUnitRenderModelSet>> cache;
            const UnitRenderModel fallbackRenderModel = TryGetDefaultModelRenderModel(modelPath).value_or(GetSpawnedSapperRenderModel(sapper));

			if (const auto it = cache.find(modelPath); it != cache.end())
			{
             if (it->second)
				{
					if (it->second->lastUpdatedAt != Scene::Time())
					{
                      ApplyConfiguredAnimationClip(it->second->idleModel, GetModelAnimationClipIndex(modelHeightSettings, modelPath, UnitModelAnimationRole::Idle));
						ApplyConfiguredAnimationClip(it->second->moveModel, GetModelAnimationClipIndex(modelHeightSettings, modelPath, UnitModelAnimationRole::Move));
						ApplyConfiguredAnimationClip(it->second->attackModel, GetModelAnimationClipIndex(modelHeightSettings, modelPath, UnitModelAnimationRole::Attack));
						it->second->idleModel.update(Scene::DeltaTime());
						it->second->moveModel.update(Scene::DeltaTime());
						it->second->attackModel.update(Scene::DeltaTime());
						it->second->lastUpdatedAt = Scene::Time();
					}

					return SelectCustomRoleModel(*it->second, role);
				}

				return nullptr;
			}

          auto loadedModels = std::make_shared<CustomUnitRenderModelSet>(modelPath, fallbackRenderModel);
			ApplyConfiguredAnimationClip(loadedModels->idleModel, GetModelAnimationClipIndex(modelHeightSettings, modelPath, UnitModelAnimationRole::Idle));
			ApplyConfiguredAnimationClip(loadedModels->moveModel, GetModelAnimationClipIndex(modelHeightSettings, modelPath, UnitModelAnimationRole::Move));
			ApplyConfiguredAnimationClip(loadedModels->attackModel, GetModelAnimationClipIndex(modelHeightSettings, modelPath, UnitModelAnimationRole::Attack));
			loadedModels->idleModel.update(Scene::DeltaTime());
			loadedModels->moveModel.update(Scene::DeltaTime());
			loadedModels->attackModel.update(Scene::DeltaTime());
			loadedModels->lastUpdatedAt = Scene::Time();
			const UnitModel* loadedModelPtr = SelectCustomRoleModel(*loadedModels, role);
			cache.emplace(modelPath, std::move(loadedModels));
			return loadedModelPtr;
		}
		constexpr ColorF SuppressionAuraColor{ 0.34, 0.68, 1.0, 0.18 };
	}

    void DrawSpawnedSappers(const Array<SpawnedSapper>& spawnedSappers, const UnitEditorSettings& unitEditorSettings, const UnitRenderModelRegistryView& renderModels, const ModelHeightSettings& modelHeightSettings, const ColorF& color, const FogOfWarState* fogOfWar, const bool requireVisible)
	{
		for (const auto& sapper : spawnedSappers)
		{
            if ((sapper.hitPoints <= 0.0) || IsSapperRetreatedHidden(sapper))
			{
				continue;
			}

			if (fogOfWar && (requireVisible ? (not IsFogVisibleAt(*fogOfWar, GetSpawnedSapperBasePosition(sapper))) : (not IsFogExploredAt(*fogOfWar, GetSpawnedSapperBasePosition(sapper)))))
			{
				continue;
			}

			const double elapsed = (Scene::Time() - sapper.spawnedAt);
			const double popIn = Min(elapsed / 0.25, 1.0);
			const Vec3 renderPosition = GetSpawnedSapperRenderPosition(sapper);
            const UnitRenderModel renderModel = GetSpawnedSapperRenderModel(sapper);
          const UnitModelAnimationRole animationRole = ResolveSapperAnimationRole(sapper);
          const FilePath& configuredModelPath = GetUnitModelPath(unitEditorSettings, sapper.team, sapper.unitType);
			const UnitModel* customModel = TrySelectCustomUnitRenderModel(unitEditorSettings, modelHeightSettings, sapper, animationRole);
			const UnitModel& drawModel = (customModel ? *customModel : SelectUnitRenderModel(renderModels, renderModel, animationRole));
            const double drawScale = customModel ? GetModelScale(modelHeightSettings, configuredModelPath) : GetModelScale(modelHeightSettings, renderModel);

         if (drawModel.isLoaded())
			{
				const double appearance = Max(0.72, (0.72 + 0.28 * EaseOutBack(popIn)));
             const ColorF unitColor = GetSpawnedSapperTint(sapper, color);
				const ColorF tint{ unitColor.r * appearance, unitColor.g * appearance, unitColor.b * appearance, unitColor.a };

				if (IsSapperSuppressed(sapper))
				{
					const double pulse = (0.82 + 0.18 * Periodic::Sine0_1(2.0s));
					Sphere{ renderPosition.movedBy(0, 1.25, 0), (1.05 + 0.10 * pulse) }.draw(ColorF{ SuppressionAuraColor.r, SuppressionAuraColor.g, SuppressionAuraColor.b, (SuppressionAuraColor.a + 0.05 * pulse) }.removeSRGBCurve());
				}

                drawModel.draw(renderPosition, GetSpawnedSapperYaw(sapper), tint.removeSRGBCurve(), drawScale);
			}
			else
			{
				const double radius = (0.22 + (0.68 * EaseOutBack(popIn)));
               Sphere{ renderPosition.movedBy(0, radius, 0), radius }.draw(GetSpawnedSapperTint(sapper, color).removeSRGBCurve());
			}
		}
	}
}
