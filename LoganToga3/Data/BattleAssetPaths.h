#pragma once
# include <Siv3D.hpp>

namespace LT3
{
    inline FilePath ResolveAssetPath(const FilePath& appPath, const FilePath& repoPath)
    {
        if (FileSystem::Exists(appPath))
        {
            return appPath;
        }

        if (FileSystem::Exists(repoPath))
        {
            return repoPath;
        }

        return appPath;
    }

    inline FilePath ResolveBuildIconPath(const String& iconName)
    {
        return ResolveAssetPath(
            U"000_Warehouse/000_DefaultGame/043_ChipImageBuild/" + iconName,
            U"App/000_Warehouse/000_DefaultGame/043_ChipImageBuild/" + iconName);
    }

    inline FilePath ResolveUnitChipPath(const String& imageName)
    {
        return ResolveAssetPath(
            U"000_Warehouse/000_DefaultGame/040_ChipImage/" + imageName,
            U"App/000_Warehouse/000_DefaultGame/040_ChipImage/" + imageName);
    }

    inline FilePath ResolveBuildingChipPath(const String& imageName)
    {
        return ResolveAssetPath(
            U"000_Warehouse/000_DefaultGame/015_BattleMapCellImage/" + imageName,
            U"App/000_Warehouse/000_DefaultGame/015_BattleMapCellImage/" + imageName);
    }

    inline FilePath ResolveCatalogVisualPath(const String& kind, const String& imageName)
    {
        if (imageName.isEmpty())
        {
            return FilePath{};
        }

        if (kind.lowercased() == U"building")
        {
            const FilePath buildingPath = ResolveBuildingChipPath(imageName);
            if (FileSystem::Exists(buildingPath))
            {
                return buildingPath;
            }

            return ResolveUnitChipPath(imageName);
        }

        const FilePath unitPath = ResolveUnitChipPath(imageName);
        if (FileSystem::Exists(unitPath))
        {
            return unitPath;
        }

        return ResolveBuildingChipPath(imageName);
    }

    inline FilePath ResolveUnitPortraitPath(const String& imageName)
    {
        if (imageName.isEmpty())
        {
            return FilePath{};
        }

        return ResolveAssetPath(
            U"000_Warehouse/000_DefaultGame/044_PortraitImage/" + imageName,
            U"App/000_Warehouse/000_DefaultGame/044_PortraitImage/" + imageName);
    }

    inline FilePath ResolveUnitVoicePath(const String& fileName)
    {
        if (fileName.isEmpty())
        {
            return FilePath{};
        }

        return ResolveAssetPath(
            U"000_Warehouse/000_DefaultGame/045_Voice/" + fileName,
            U"App/000_Warehouse/000_DefaultGame/045_Voice/" + fileName);
    }

    inline FilePath ResolveDecalAmbientSoundPath(const String& fileName)
    {
        if (fileName.isEmpty())
        {
            return FilePath{};
        }

        return ResolveAssetPath(
            U"000_Warehouse/000_DefaultGame/046_AmbientSound/" + fileName,
            U"App/000_Warehouse/000_DefaultGame/046_AmbientSound/" + fileName);
    }

    inline FilePath ResolveSystemImagePath(const String& imageName)
    {
        return ResolveAssetPath(
            U"000_Warehouse/000_DefaultGame/000_SystemImage/" + imageName,
            U"App/000_Warehouse/000_DefaultGame/000_SystemImage/" + imageName);
    }
}
