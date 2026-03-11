# include "BattleRenderer.h"

namespace
{
    s3d::ColorF GetOwnerColor(const Owner owner)
    {
        return (owner == Owner::Player) ? s3d::ColorF{ 0.25, 0.75, 1.0 } : s3d::ColorF{ 1.0, 0.38, 0.32 };
    }

    s3d::String GetArchetypeLabel(const UnitArchetype archetype)
    {
        switch (archetype)
        {
        case UnitArchetype::Base:
            return U"BASE";
        case UnitArchetype::Worker:
            return U"WORKER";
        case UnitArchetype::Soldier:
            return U"SOLDIER";
        case UnitArchetype::Archer:
            return U"ARCHER";
        default:
            return U"UNIT";
        }
    }
}

void BattleRenderer::draw(const BattleState& state) const
{
    drawWorld(state);
    drawUnits(state);
    drawHud(state);
}

void BattleRenderer::drawWorld(const BattleState& state) const
{
    state.worldBounds.draw(s3d::ColorF{ 0.14, 0.17, 0.20 });

    for (s3d::int32 x = 0; x <= 1280; x += 64)
    {
        s3d::Line{ x, 0, x, 720 }.draw(1, s3d::ColorF{ 0.18, 0.21, 0.24 });
    }

    for (s3d::int32 y = 0; y <= 720; y += 64)
    {
        s3d::Line{ 0, y, 1280, y }.draw(1, s3d::ColorF{ 0.18, 0.21, 0.24 });
    }

    if (state.isSelecting)
    {
        state.selectionRect.drawFrame(2, s3d::Palette::Skyblue);
    }
}

void BattleRenderer::drawUnits(const BattleState& state) const
{
    for (const auto& unit : state.units)
    {
        if (!unit.isAlive)
        {
            continue;
        }

        const s3d::ColorF color = GetOwnerColor(unit.owner);
        s3d::Circle{ unit.position, unit.radius }.draw(color);

        if (unit.archetype == UnitArchetype::Base)
        {
            s3d::Circle{ unit.position, unit.radius + 10 }.drawFrame(4, color);
        }

        if (unit.isSelected)
        {
            s3d::Circle{ unit.position, unit.radius + 5 }.drawFrame(2, s3d::Palette::Yellow);
        }

        const double hpRate = (unit.maxHp > 0) ? (static_cast<double>(unit.hp) / unit.maxHp) : 0.0;
        const s3d::RectF barBack{ unit.position.x - 18, unit.position.y - unit.radius - 14, 36, 5 };
        barBack.draw(s3d::ColorF{ 0.1 });
        s3d::RectF{ barBack.pos, 36 * hpRate, barBack.h }.draw(s3d::ColorF{ 0.3, 0.95, 0.45 });

        m_smallFont(GetArchetypeLabel(unit.archetype)).drawAt(unit.position.movedBy(0, unit.radius + 10), s3d::Palette::White);
    }
}

void BattleRenderer::drawHud(const BattleState& state) const
{
    s3d::RoundRect{ 16, 16, 410, 128, 8 }.draw(s3d::ColorF{ 0.0, 0.0, 0.0, 0.55 });
    m_font(U"LoganToga2 Remake Prototype").draw(28, 26, s3d::Palette::White);
    m_smallFont(U"L drag: select / R click: move or attack").draw(28, 60, s3d::Palette::White);
    m_smallFont(U"1: Soldier (60G) / 2: Archer (80G)").draw(28, 82, s3d::Palette::White);
    m_smallFont(U"Gold: {}"_fmt(state.playerGold)).draw(28, 104, s3d::Palette::Gold);

    if (state.winner)
    {
        const s3d::String label = (*state.winner == Owner::Player) ? U"PLAYER WIN" : U"ENEMY WIN";
        m_font(label).drawAt(s3d::Scene::CenterF().movedBy(0, -20), s3d::Palette::White);
    }
}
