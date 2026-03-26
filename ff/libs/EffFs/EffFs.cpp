#include "../AddonEffFs.h"

namespace
{
	AddonEffFs* GetEffFs()
	{
		if (auto p = Addon::GetAddon<AddonEffFs>(AddonEffFs::Name)) return p;
		return nullptr;
	}
}

void AddonEffFs::Condition()
{
	if (auto p = GetEffFs())
	{
		// まだ未使用。将来の拡張で使う予定
	}
}
/// @brief アクティブかどうか
/// @return 
[[nodiscard]]
bool AddonEffFs::IsActive()
{
	if (auto p = GetEffFs())
	{
		return p->m_active;
	}
	else
	{
		return false;
	}
}
/// @brief アクティブにします
void AddonEffFs::SetActive()
{
	if (auto p = GetEffFs())
	{
		p->m_active = true;
	}
}
/// @brief 非アクティブにします
void AddonEffFs::SetNotActive()
{
	if (auto p = GetEffFs())
	{
		p->m_active = false;
	}
}
void AddonEffFs::SetDeltaTime(double dt)
{
	if (auto p = GetEffFs())
	{
		p->m_dt = dt;
	}
}
void AddonEffFs::SetKind(EffectKind kind)
{
	if (auto p = GetEffFs())
	{
		p->m_kind = kind;
	}
}
/// @brief 
/// @return 
[[nodiscard]]
bool AddonEffFs::update()
{
	const double dt = (m_dt > 0.0) ? m_dt : Scene::DeltaTime();
	m_dt = 0.0;
	const double gravity = (m_kind == EffectKind::Rain ? 1500.0 : 0.0);

	if (m_active)
	{
		m_time += dt;

		if (m_strengthHold > 0.0)
		{
			m_strengthHold -= dt;
		}
		else
		{
			// ターゲット自体を少しずつゼロに戻す
			m_targetStrength = Max(0.0, m_targetStrength - dt * 0.15);
		}

		// 現在強度をターゲットへイージング
		m_strength += (m_targetStrength - m_strength) * (dt * 3.0);
		m_strength = Clamp(m_strength, 0.0, 1.0);
		// 風速
		const Vec2 v = m_dir * (m_speed * (60.0 + 240.0 * m_strength));

		// 自然発生
		const double prob = Clamp(m_spawnBaseProb + m_spawnStrengthMul * m_strength, 0.0, 1.0);
		if (RandomBool(prob))
		{
			spawnParticles(2 + Random(0, 2));
		}

	}

	// 粒子の寿命処理だけは常にやる
	for (auto& p : m_ps)
	{
		p.age += static_cast<float>(dt);
		if (m_kind == EffectKind::Rain)
		{
			// 雨：重力を下方向に加速
			p.vel.y += gravity * dt;

			// 風成分をちょっとだけ X に足しても良い
			p.vel.x += m_dir.x * 50.0 * dt * m_speed;

			p.pos += p.vel * dt;
		}
		else // Wind モード
		{
			p.pos += p.vel * dt;

			// 画面ループ（ホコリ用）
			p.pos.x = Fmod((p.pos.x + Scene::Width()), (double)Scene::Width());
			p.pos.y = Fmod((p.pos.y + Scene::Height()), (double)Scene::Height());
		}
	}
	m_ps.remove_if([this](const P& p)
		{
			if (p.age >= p.life)
				return true;
			if (m_kind == EffectKind::Rain)
			{
				// 画面下まで落ちたら消す（+ margin）
				if (p.pos.y > Scene::Height() + 50.0)
					return true;
			}
			return false;
		});

	return true;
}
/// @brief
void AddonEffFs::draw() const
{
	if (m_ps.isEmpty())
		return;

	const ScopedRenderStates2D blend{ BlendState::Additive };

	if (m_kind == EffectKind::Rain)
	{
		for (const auto& p : m_ps)
		{
			const double lifeT = (p.age / p.life);  // 0→1
			const double a = (1.0 - lifeT);     // フェードアウト

			// 速度方向の角度
			const double angle = std::atan2(p.vel.y, p.vel.x);

			// size = 太さ、長さは速度と強さで決める
			const double len = p.size * (8.0 + 12.0 * m_strength);
			const double w = p.size * 1.5;

			const double alpha = a * p.col.a;

			RectF{ Arg::center = p.pos, len, w }
				.rotated(angle)
				.draw(p.col.withAlpha(alpha));
		}
	}
	else // Wind
	{
		const double angle = m_dir.getAngle();

		for (const auto& p : m_ps)
		{
			const double lifeT = (p.age / p.life); // 0 → 1
			const double a = (1.0 - lifeT);    // フェードアウト
			const double s = p.size * (1.0 + 2.0 * m_strength);

			const double alpha = a * p.col.a;

			RectF{ Arg::center = p.pos, s * 8.0, s * 2.0 }
				.rotated(angle)
				.draw(p.col.withAlpha(alpha));
		}
	}
}
void AddonEffFs::spawnParticles(int32 count)
{
	for (int i = 0; i < count; ++i)
	{
		P p;
		const double baseSpeed = Random(60.0, 140.0);
		// 雨の初速（落下 + 風の横成分）
		const double fallSpeed = Random(500.0, 800.0); // 落ちる速さ
		const double windX = m_dir.x * 150.0 * m_speed; // 風による横ブレ
		switch (m_kind)
		{
		case AddonEffFs::EffectKind::Wind:
			p.pos = Vec2{ Random(0.0, (double)Scene::Width()),
						   Random(0.0, (double)Scene::Height()) };
			p.vel = m_dir * (baseSpeed * m_speed);
			p.life = static_cast<float>(Random(0.8, 1.8));
			p.size = static_cast<float>(Random(1.0, 2.2));
			break;
		case AddonEffFs::EffectKind::Rain:
			// 画面上端～ちょい上から出す
			p.pos.x = Random(0.0, (double)Scene::Width());
			p.pos.y = Random(-50.0, 0.0);


			p.vel = Vec2{ windX, fallSpeed };

			p.life = static_cast<float>(Random(0.6, 1.2));  // 短め
			p.size = static_cast<float>(Random(0.8, 1.3));  // しずくの「太さ」用
			break;
		default:
			break;
		}

		p.age = 0.0f;
		// ベース色 + 強さで α を掛ける
		const double baseAlpha = (m_kind == EffectKind::Rain ? 0.5 : 0.35);
		const double alpha = (baseAlpha + 0.35 * m_strength) * m_baseColor.a;
		p.col = ColorF(m_baseColor.r, m_baseColor.g, m_baseColor.b, alpha);

		m_ps << p;
	}
}

void AddonEffFs::SetDirection(const Vec2& dir)
{
	if (auto p = GetEffFs())
	{
		// 0 ベクトルガード & 正規化
		if (not dir.isZero())
		{
			p->m_dir = dir.normalized();
		}
	}
}
void AddonEffFs::AddImpulse(double strength, double holdTime)
{
	if (auto p = GetEffFs())
	{
		// 強さを [0,1] くらいにクランプ（お好み）
		strength = Clamp(strength, 0.0, 1.0);
		holdTime = Max(0.0, holdTime);

		// 「今の target より強ければ上書き」くらいにしておくと自然
		p->m_targetStrength = Max(p->m_targetStrength, strength);
		p->m_strengthHold = Max(p->m_strengthHold, holdTime);
	}
}
void AddonEffFs::SetColor(const ColorF& col)
{
	if (auto p = GetEffFs())
	{
		p->m_baseColor = col;
	}
}
void AddonEffFs::SetSpawnDensity(double base, double mul)
{
	if (auto p = GetEffFs())
	{
		p->m_spawnBaseProb = Max(0.0, base);
		p->m_spawnStrengthMul = Max(0.0, mul);
	}
}
