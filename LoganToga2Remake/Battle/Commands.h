#pragma once
# include <Siv3D.hpp>
# include <variant>

struct ClearSelectionCommand
{
};

struct SelectUnitsInRectCommand
{
    s3d::RectF rect;
    bool additive = false;
};

struct MoveUnitsCommand
{
    s3d::Array<s3d::int32> unitIds;
    s3d::Vec2 destination = s3d::Vec2::Zero();
};

struct AttackUnitCommand
{
    s3d::Array<s3d::int32> unitIds;
    s3d::int32 targetUnitId = -1;
};

using BattleCommand = std::variant<ClearSelectionCommand, SelectUnitsInRectCommand, MoveUnitsCommand, AttackUnitCommand>;
