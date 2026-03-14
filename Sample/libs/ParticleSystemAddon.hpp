# pragma once
# include <Siv3D.hpp>

class ParticleSystemAddon : public IAddon
{
public:
	enum class Kind : uint8
	{
		Circle,
		Arc,
		Rect,
		Polygon,
		Count,
	};

	struct Preset
	{
		ParticleSystem2DParameters parameters;
		Texture texture;
		Vec2 force{ 0.0, 0.0 };
		double durationSec = 0.2;

		CircleEmitter2D circleEmitter;
		ArcEmitter2D arcEmitter;
		RectEmitter2D rectEmitter;
		PolygonEmitter2D polygonEmitter{ Polygon{} };
	};

	static constexpr StringView AddonName{ U"ParticleSystemAddon" };

	static void RegisterDefault()
	{
		Addon::Register<ParticleSystemAddon>(AddonName);
	}

	static void SetPreset(Kind kind, Preset preset)
	{
		if (auto p = Addon::GetAddon<ParticleSystemAddon>(AddonName))
		{
			p->setPresetInternal(kind, std::move(preset));
		}
	}

	static void Play(Kind kind, const Vec2& position)
	{
		if (auto p = Addon::GetAddon<ParticleSystemAddon>(AddonName))
		{
			p->playInternal(kind, position);
		}
	}

	static void SetDebugDraw(bool enabled)
	{
		if (auto p = Addon::GetAddon<ParticleSystemAddon>(AddonName))
		{
			p->setDebugDrawInternal(enabled);
		}
	}

private:
	struct Slot
	{
		Preset preset;
		std::unique_ptr<ParticleSystem2D> system;
		double timeLeft = 0.0;
		bool spawning = false;
	};

	std::array<Slot, static_cast<size_t>(Kind::Count)> m_slots;
	bool m_debugDraw = false;

	bool init() override
	{
		return true;
	}

	bool update() override
	{
		const double deltaTime = Scene::DeltaTime();
		for (auto& slot : m_slots)
		{
			if (!slot.system)
			{
				continue;
			}

			slot.system->update();

			if (!slot.spawning)
			{
				continue;
			}

			slot.timeLeft -= deltaTime;
			if (slot.timeLeft <= 0.0)
			{
				auto stopParameters = slot.preset.parameters;
				stopParameters.rate = 0.0;
				slot.system->setParameters(stopParameters);
				slot.spawning = false;
			}
		}

		return true;
	}

	void draw() const override
	{
		for (const auto& slot : m_slots)
		{
			if (!slot.system)
			{
				continue;
			}

			if (m_debugDraw)
			{
				slot.system->drawDebug();
			}
			else
			{
				slot.system->draw();
			}
		}
	}

	static size_t ToIndex(Kind kind)
	{
		return static_cast<size_t>(kind);
	}

	void setPresetInternal(Kind kind, Preset preset)
	{
		auto& slot = m_slots[ToIndex(kind)];
		slot.preset = std::move(preset);
		slot.system = std::make_unique<ParticleSystem2D>(Vec2::Zero(), slot.preset.force);
		applyEmitter(kind, *slot.system, slot.preset);
		slot.system->setTexture(slot.preset.texture);
		slot.system->setParameters(slot.preset.parameters);
		slot.system->prewarm();
		slot.spawning = false;
		slot.timeLeft = 0.0;
	}

	void playInternal(Kind kind, const Vec2& position)
	{
		auto& slot = m_slots[ToIndex(kind)];
		if (!slot.system)
		{
			return;
		}

		slot.system->setPosition(position);
		slot.system->setParameters(slot.preset.parameters);
		slot.timeLeft = slot.preset.durationSec;
		slot.spawning = true;
	}

	void setDebugDrawInternal(bool enabled)
	{
		m_debugDraw = enabled;
	}

	static void applyEmitter(Kind kind, ParticleSystem2D& system, const Preset& preset)
	{
		switch (kind)
		{
		case Kind::Circle:
			system.setEmitter(preset.circleEmitter);
			break;
		case Kind::Arc:
			system.setEmitter(preset.arcEmitter);
			break;
		case Kind::Rect:
			system.setEmitter(preset.rectEmitter);
			break;
		case Kind::Polygon:
			system.setEmitter(preset.polygonEmitter);
			break;
		default:
			break;
		}
	}
};
