# pragma once
# include <Siv3D.hpp>

namespace lighting
{
    // 時間帯プリセット
    enum class TimeOfDay : size_t
    {
        Morning = 0,
        Noon,
        MagicHour,
        Night,
        Count
    };

    struct Preset
    {
        String name;
        String description;
        Vec3 sunDirection;     // 光源方向 (光が「来る」向き、正規化済み)
        ColorF sunColor;       // sRGB 値 (適用時にリニア化)
        double sunIntensity;   // リニア空間での輝度倍率 (HDR 用)
        ColorF ambientColor;   // sRGB 値 (適用時にリニア化)
        ColorF backgroundColor;// sRGB 値 (適用時にリニア化)
    };

    inline const Array<Preset>& GetPresets()
    {
        static const Array<Preset> presets = {
            // 朝: 低めの太陽 + 青味のある空気
            Preset{
                U"朝",
                U"低めの太陽、ほのかに暖色、青い空気感",
                Vec3{ 0.70, 0.45, 0.30 }.normalized(),
                ColorF{ 1.00, 0.92, 0.80 },
                1.0,
                ColorF{ 0.24, 0.30, 0.40 },
                ColorF{ 0.55, 0.70, 0.85 },
            },
            // 昼: 高い太陽 + ニュートラル (Siv3D 既定に近い)
            Preset{
                U"昼",
                U"高い太陽、影は短くコントラスト弱め",
                Vec3{ 0.30, 0.90, 0.30 }.normalized(),
                ColorF{ 1.00, 0.98, 0.95 },
                1.0,
                ColorF{ 0.32, 0.34, 0.38 },
                ColorF{ 0.40, 0.60, 0.80 },
            },
            // マジックアワー: 低い太陽 + 強いコントラスト + 補色シャドウ
            //   N·L 崩壊を補うため sunIntensity=3.0 (HDR / ACES tonemap 前提)
            Preset{
                U"マジックアワー",
                U"低い太陽、強い暖色光と青いシャドウ (ACES 推奨)",
                Vec3{ 0.85, 0.30, 0.50 }.normalized(),
                ColorF{ 1.00, 0.62, 0.38 },
                3.0,
                ColorF{ 0.22, 0.28, 0.42 },
                ColorF{ 0.85, 0.55, 0.50 },
            },
            // 夜: 月光 (弱く青い) + 暗い ambient
            Preset{
                U"夜",
                U"月光、暗く青いトーン、暗部を沈める",
                Vec3{ -0.30, 0.55, -0.50 }.normalized(),
                ColorF{ 0.45, 0.55, 0.85 },
                1.0,
                ColorF{ 0.08, 0.10, 0.16 },
                ColorF{ 0.05, 0.07, 0.14 },
            },
        };
        return presets;
    }

    // プリセット適用 → リニア化された背景色を返す
    inline ColorF Apply(const Preset& p)
    {
        Graphics3D::SetSunDirection(p.sunDirection);
        Graphics3D::SetSunColor(p.sunColor.removeSRGBCurve() * p.sunIntensity);
        Graphics3D::SetGlobalAmbientColor(p.ambientColor.removeSRGBCurve());
        return p.backgroundColor.removeSRGBCurve();
    }

    // 8 方位 (水平方位)
    //   index: 0=N(-Z), 1=NE, 2=E(+X), 3=SE, 4=S(+Z), 5=SW, 6=W(-X), 7=NW
    inline constexpr const char32* DirectionLabels[8] = {
        U"N", U"NE", U"E", U"SE", U"S", U"SW", U"W", U"NW"
    };

    inline Vec2 DirectionXZ(size_t index)
    {
        constexpr Vec2 dirs[8] = {
            {  0, -1 }, {  1, -1 }, {  1,  0 }, {  1,  1 },
            {  0,  1 }, { -1,  1 }, { -1,  0 }, { -1, -1 },
        };
        return dirs[index % 8];
    }

    struct Overrides
    {
        double sunIntensityScale = 1.0;
        double ambientIntensityScale = 1.0;
        Optional<size_t> sunDirectionIndex; // 8 方位上書き (None ならプリセット方向)
    };

    // オーバーライド付き適用
    inline ColorF Apply(const Preset& p, const Overrides& o)
    {
        Vec3 dir = p.sunDirection;
        if (o.sunDirectionIndex)
        {
            // プリセットの太陽高度 (Y) を保ったまま、水平方位 (XZ) のみ差し替える
            const double y = dir.y;
            const double horizLen = std::sqrt(Max(0.0, 1.0 - y * y));
            const Vec2 xz = DirectionXZ(*o.sunDirectionIndex);
            const double xzLen = xz.length();
            if (xzLen > 0.0)
            {
                const Vec2 xzScaled = xz / xzLen * horizLen;
                dir = Vec3{ xzScaled.x, y, xzScaled.y };
            }
        }
        Graphics3D::SetSunDirection(dir.normalized());
        Graphics3D::SetSunColor(p.sunColor.removeSRGBCurve() * (p.sunIntensity * o.sunIntensityScale));
        Graphics3D::SetGlobalAmbientColor(p.ambientColor.removeSRGBCurve() * o.ambientIntensityScale);
        return p.backgroundColor.removeSRGBCurve();
    }
}
