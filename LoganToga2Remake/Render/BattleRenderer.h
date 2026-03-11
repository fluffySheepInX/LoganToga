#pragma once
# include <Siv3D.hpp>
# include "../Battle/BattleState.h"

class BattleRenderer
{
public:
    void draw(const BattleState& state) const;

private:
    s3d::Font m_font{ 18, s3d::Typeface::Bold };
    s3d::Font m_smallFont{ 12 };

    void drawWorld(const BattleState& state) const;
    void drawUnits(const BattleState& state) const;
    void drawHud(const BattleState& state) const;
};
