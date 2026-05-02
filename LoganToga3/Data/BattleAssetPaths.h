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

    inline FilePath ResolveSystemImagePath(const String& imageName)
    {
        return ResolveAssetPath(
            U"000_Warehouse/000_DefaultGame/000_SystemImage/" + imageName,
            U"App/000_Warehouse/000_DefaultGame/000_SystemImage/" + imageName);
    }
}
