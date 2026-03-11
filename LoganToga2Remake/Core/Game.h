#pragma once
# include <Siv3D.hpp>
# include <memory>

class AppScene
{
public:
    virtual ~AppScene() = default;
    virtual void handleInput() = 0;
    virtual void fixedUpdate(double deltaTime) = 0;
    virtual void draw() const = 0;
};

class FixedStepClock
{
public:
    explicit FixedStepClock(double stepSeconds)
        : m_stepSeconds{ stepSeconds } {}

    [[nodiscard]] double stepSeconds() const noexcept
    {
        return m_stepSeconds;
    }

    [[nodiscard]] size_t beginFrame()
    {
        m_accumulator += s3d::Min(s3d::Scene::DeltaTime(), 0.25);

        size_t stepCount = 0;
        while ((m_accumulator >= m_stepSeconds) && (stepCount < m_maxStepsPerFrame))
        {
            m_accumulator -= m_stepSeconds;
            ++stepCount;
        }

        return stepCount;
    }

private:
    double m_stepSeconds = 1.0 / 60.0;
    double m_accumulator = 0.0;
    size_t m_maxStepsPerFrame = 4;
};

class Game
{
public:
    explicit Game(std::unique_ptr<AppScene> scene)
        : m_scene{ std::move(scene) } {}

    void run()
    {
        FixedStepClock clock{ 1.0 / 60.0 };

        while (s3d::System::Update())
        {
            m_scene->handleInput();

            const size_t fixedSteps = clock.beginFrame();
            for (size_t i = 0; i < fixedSteps; ++i)
            {
                m_scene->fixedUpdate(clock.stepSeconds());
            }

            m_scene->draw();
        }
    }

private:
    std::unique_ptr<AppScene> m_scene;
};
