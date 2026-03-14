#pragma once
#include <Siv3D.hpp>

class AddonEffFs : public IAddon
{
public:
	enum class EffectKind {
		Wind,
		Rain,
	};

	static inline constexpr StringView Name = U"AddonEffFs";
	/// @brief 
	static void Condition();
	/// @brief アクティブかどうかを返します。
	/// @return アクティブな場合は true、それ以外の場合は false
	static bool IsActive();
	/// @brief アクティブにします。
	static void SetActive();
	/// @brief 非アクティブにします。
	static void SetNotActive();
	/// @brief 
	static void SetDeltaTime(double dt);

	// 風向き（単位ベクトルで想定）
	static void SetDirection(const Vec2& dir);
	// 一時的な強風インパルスを与える
	// 強さ + 維持時間
	static void AddImpulse(double strength, double holdTime = 0.5);
	// 描画色・密度なども開けておける
	static void SetColor(const ColorF& col);
	static void SetSpawnDensity(double base, double mul);
	static void SetKind(EffectKind kind);
private:
	/// @brief 毎フレームの更新処理を行います。
	/// @return 更新が成功した場合は true、それ以外の場合は false
	bool update() override;
	/// @brief 毎フレームの描画処理を行います。
	void draw() const override;

	void spawnParticles(int32 count);

	bool m_active = true;

	// 粒子
	struct P { Vec2 pos, vel; float life, age; float size; ColorF col; };
	Array<P> m_ps;

	double m_dt = 0.0;
	double m_time = 0.0;
	double m_strength = 0.0;
	double m_targetStrength = 0.0;
	double m_strengthHold = 0.0; // 強風維持タイマー
	Vec2 m_dir{ 1,0 };
	double m_speed = 1.0;// まだ未使用。将来の拡張で使う予定

	ColorF m_baseColor = ColorF(1.0, 1.0, 1.0, 0.5);
	double m_spawnBaseProb = 0.05;
	double m_spawnStrengthMul = 0.20;
	EffectKind m_kind = EffectKind::Wind;
};
