#pragma once
# include <Siv3D.hpp>

namespace LT3
{
    inline constexpr Point QuarterLogicalSceneSize{ 1600, 900 };
    inline constexpr Vec2 QuarterViewOrigin{ 575.0, 180.0 };
    inline constexpr Vec2 QuarterViewScale{ 50.0 / 120.0, 25.0 / 120.0 };
    inline constexpr double QuarterTileStep = 120.0;
    inline constexpr Vec2 QuarterTileOffset{ 50.0, 25.0 };
    inline constexpr double QuarterTileThickness = 15.0;
    inline constexpr Vec2 QuarterBattleMapOrigin{ 200.0, 90.0 };
    inline constexpr double QuarterViewZoomMin = 0.5;
    inline constexpr double QuarterViewZoomMax = 2.5;
    inline constexpr double QuarterViewKeyboardPanPixelsPerSec = 600.0;
    inline Camera2D QuarterViewCamera2D{ Vec2{ 0.0, 0.0 }, 1.0, CameraControl::None_ };

    inline double QuarterLogicalSceneWidth()
    {
        return static_cast<double>(QuarterLogicalSceneSize.x);
    }

    inline double QuarterLogicalSceneHeight()
    {
        return static_cast<double>(QuarterLogicalSceneSize.y);
    }

    inline Vec2 ToQuarterIso(const Vec2& worldPos);
    inline Vec2 ToQuarterWorld(const Vec2& screenPos);
    inline void DragQuarterViewCamera(const Vec2& screenDelta);
    inline void MoveQuarterViewCamera(const Vec2& screenDelta);

    inline const Camera2DParameters& QuarterViewCameraParametersEnabled()
    {
        static const Camera2DParameters params = []
        {
            Camera2DParameters p = Camera2DParameters::Default();
            p.minScale = QuarterViewZoomMin;
            p.maxScale = QuarterViewZoomMax;
            p.wheelScaleFactor = 1.1;
            return p;
        }();
        return params;
    }

    inline const Camera2DParameters& QuarterViewCameraParametersDisabled()
    {
        static const Camera2DParameters params = []
        {
            Camera2DParameters p = Camera2DParameters::NoControl();
            p.minScale = QuarterViewZoomMin;
            p.maxScale = QuarterViewZoomMax;
            p.wheelScaleFactor = 1.1;
            return p;
        }();
        return params;
    }

    inline Vec2 GetQuarterViewKeyboardPanDirection()
    {
        Vec2 direction{ 0.0, 0.0 };

        if (KeyA.pressed())
        {
            direction.x -= 1.0;
        }
        if (KeyD.pressed())
        {
            direction.x += 1.0;
        }
        if (KeyW.pressed())
        {
            direction.y -= 1.0;
        }
        if (KeyS.pressed())
        {
            direction.y += 1.0;
        }

        if (direction.isZero())
        {
            return direction;
        }

        return direction.normalized();
    }

    inline void UpdateQuarterViewCamera2D(const bool enableControl, const bool allowWheelZoom = false)
    {
        QuarterViewCamera2D.setParameters(QuarterViewCameraParametersDisabled());

        static Optional<Vec2> previousMiddleDragScreen;

        if (enableControl && MouseM.pressed())
        {
            const Vec2 screenPos = Cursor::PosF();
            if (previousMiddleDragScreen)
            {
                DragQuarterViewCamera(screenPos - *previousMiddleDragScreen);
            }
            previousMiddleDragScreen = screenPos;
        }
        else
        {
            previousMiddleDragScreen.reset();
        }

        if (enableControl)
        {
            const Vec2 keyboardPanDirection = GetQuarterViewKeyboardPanDirection();
            if (!keyboardPanDirection.isZero())
            {
                const Vec2 screenDelta = keyboardPanDirection * QuarterViewKeyboardPanPixelsPerSec * Scene::DeltaTime();
                MoveQuarterViewCamera(screenDelta);
            }
        }

        if (enableControl || allowWheelZoom)
        {
            const double wheel = Mouse::Wheel();
            if (wheel != 0.0)
            {
                const Vec2 screenPos = Cursor::PosF();
                const Vec2 anchorWorld = ToQuarterWorld(screenPos);
                const double nextScale = Clamp((QuarterViewCamera2D.getScale() * Math::Pow(1.1, wheel)), QuarterViewZoomMin, QuarterViewZoomMax);
                const Vec2 nextCenter = (ToQuarterIso(anchorWorld) - ((screenPos - QuarterViewOrigin) / nextScale));
                QuarterViewCamera2D.jumpTo(nextCenter, nextScale);
            }
        }
    }

    inline Vec2 ToQuarterIso(const Vec2& worldPos)
    {
        return Vec2{
            (worldPos.x - worldPos.y) * QuarterViewScale.x,
            (worldPos.x + worldPos.y) * QuarterViewScale.y
        };
    }

    inline Vec2 FromQuarterIso(const Vec2& isoPos)
    {
        const double a = isoPos.x / QuarterViewScale.x;
        const double b = isoPos.y / QuarterViewScale.y;
        return Vec2{ (a + b) * 0.5, (b - a) * 0.5 };
    }

    inline Vec2 ToQuarterScreen(const Vec2& worldPos)
    {
        return QuarterViewOrigin + ToQuarterIso(worldPos);
    }

    inline Vec2 ToQuarterViewportScreen(const Vec2& worldPos)
    {
        const double scale = QuarterViewCamera2D.getScale();
        const Vec2 screenPos = ToQuarterScreen(worldPos);
        return QuarterViewOrigin + ((screenPos - QuarterViewOrigin) * scale) - (QuarterViewCamera2D.getCenter() * scale);
    }

    inline Vec2 ToQuarterWorld(const Vec2& screenPos)
    {
        return FromQuarterIso(((screenPos - QuarterViewOrigin) / QuarterViewCamera2D.getScale()) + QuarterViewCamera2D.getCenter());
    }

    inline Vec2 ToQuarterPreCameraScreen(const Vec2& screenPos)
    {
        return QuarterViewOrigin + (((screenPos - QuarterViewOrigin) / QuarterViewCamera2D.getScale()) + QuarterViewCamera2D.getCenter());
    }

    inline Vec2 ScreenDeltaToQuarterWorld(const Vec2& screenDelta)
    {
        return FromQuarterIso(screenDelta / QuarterViewCamera2D.getScale());
    }

    inline Vec2 ScreenDeltaToQuarterIso(const Vec2& screenDelta)
    {
        return (screenDelta / QuarterViewCamera2D.getScale());
    }

    inline Vec2 GetQuarterViewCameraIsoCenter()
    {
        return QuarterViewCamera2D.getCenter();
    }

    inline double GetQuarterViewCameraScale()
    {
        return QuarterViewCamera2D.getScale();
    }

    inline void SetQuarterViewCameraIsoCenter(const Vec2& center)
    {
        QuarterViewCamera2D.jumpTo(center, QuarterViewCamera2D.getScale());
    }

    inline void MoveQuarterViewCamera(const Vec2& screenDelta)
    {
        SetQuarterViewCameraIsoCenter(QuarterViewCamera2D.getCenter() + ScreenDeltaToQuarterIso(screenDelta));
    }

    inline void DragQuarterViewCamera(const Vec2& screenDelta)
    {
        SetQuarterViewCameraIsoCenter(QuarterViewCamera2D.getCenter() - ScreenDeltaToQuarterIso(screenDelta));
    }

    inline void ZoomQuarterViewAt(const Vec2& screenPos, const double wheel)
    {
        if (wheel == 0.0)
        {
            return;
        }

        const Vec2 anchorWorld = ToQuarterWorld(screenPos);
        const double nextScale = Clamp((QuarterViewCamera2D.getScale() * Math::Pow(1.1, wheel)), QuarterViewZoomMin, QuarterViewZoomMax);
        const Vec2 nextCenter = (ToQuarterIso(anchorWorld) - ((screenPos - QuarterViewOrigin) / nextScale));
        QuarterViewCamera2D.jumpTo(nextCenter, nextScale);
    }

    inline Transformer2D CreateQuarterViewTransformer()
    {
        return Transformer2D{
            Mat3x2::Scale(QuarterViewCamera2D.getScale(), QuarterViewOrigin)
                .translated(-(QuarterViewCamera2D.getScale() * QuarterViewCamera2D.getCenter())),
            TransformCursor::Yes,
            Transformer2D::Target::PushCamera
        };
    }

    inline Quad ToQuarterTile(const Vec2& worldCenter)
    {
        const Vec2 bottomCenter = ToQuarterScreen(worldCenter);
        const Vec2 top = bottomCenter.movedBy(0, -QuarterTileThickness).movedBy(0, -QuarterTileOffset.y * 2);
        const Vec2 right = bottomCenter.movedBy(0, -QuarterTileThickness).movedBy(QuarterTileOffset.x, -QuarterTileOffset.y);
        const Vec2 bottom = bottomCenter.movedBy(0, -QuarterTileThickness);
        const Vec2 left = bottomCenter.movedBy(0, -QuarterTileThickness).movedBy(-QuarterTileOffset.x, -QuarterTileOffset.y);
        return Quad{ top, right, bottom, left };
    }

    inline Vec2 QuarterTileFaceCenterScreen(const Vec2& worldCenter)
    {
        return ToQuarterScreen(worldCenter).movedBy(0, -(QuarterTileOffset.y + QuarterTileThickness));
    }

    inline Vec2 QuarterBattleCellCenter(int32 x, int32 y)
    {
        return QuarterBattleMapOrigin + Vec2{ x * QuarterTileStep, y * QuarterTileStep };
    }

    inline Point QuarterWorldToBattleCell(const Vec2& worldPosition, int32 width, int32 height)
    {
        const Vec2 local = worldPosition - QuarterBattleMapOrigin;
        const int32 col = Clamp(static_cast<int32>(Math::Round(local.x / QuarterTileStep)), 0, Max(0, width - 1));
        const int32 row = Clamp(static_cast<int32>(Math::Round(local.y / QuarterTileStep)), 0, Max(0, height - 1));
        return Point{ col, row };
    }

    inline Point QuarterScreenToBattleCell(const Vec2& screenPos, int32 width, int32 height)
    {
        const Vec2 preCameraScreenPos = ToQuarterPreCameraScreen(screenPos);
        const Vec2 worldPos = FromQuarterIso(preCameraScreenPos - QuarterViewOrigin);
        const Vec2 local = worldPos - QuarterBattleMapOrigin;

        const int32 approxCol = Clamp(static_cast<int32>(Math::Round(local.x / QuarterTileStep)), 0, Max(0, width - 1));
        const int32 approxRow = Clamp(static_cast<int32>(Math::Round(local.y / QuarterTileStep)), 0, Max(0, height - 1));

        Optional<Point> bestCell;
        int32 bestDiagonal = INT32_MIN;
        int32 bestXOnDiagonal = INT32_MIN;

        for (int32 dy = -2; dy <= 2; ++dy)
        {
            for (int32 dx = -2; dx <= 2; ++dx)
            {
                const int32 col = approxCol + dx;
                const int32 row = approxRow + dy;
                if (col < 0 || row < 0 || col >= width || row >= height)
                {
                    continue;
                }

                if (ToQuarterTile(QuarterBattleCellCenter(col, row)).intersects(preCameraScreenPos))
                {
                    const int32 diagonal = col + row;
                    if (!bestCell
                        || diagonal > bestDiagonal
                        || (diagonal == bestDiagonal && col > bestXOnDiagonal))
                    {
                        bestCell = Point{ col, row };
                        bestDiagonal = diagonal;
                        bestXOnDiagonal = col;
                    }
                }
            }
        }

        if (bestCell)
        {
            return *bestCell;
        }

        return Point{ approxCol, approxRow };
    }

    inline RectF BuildCommandRect(int32 index)
    {
        return RectF{ 590.0 + index * 150.0, 836.0, 136.0, 46.0 };
    }
}
