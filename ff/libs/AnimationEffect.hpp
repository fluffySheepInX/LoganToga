#pragma once
#include <Siv3D.hpp>

/// @brief アニメーション処理をカプセル化したクラス
template <class TShape>
class AnimationEffect
{
public:
	// t の値に応じた描画処理を外部から渡す（再利用・カスタム可能）
	using DrawFunction = std::function<void(TShape, double)>;

	AnimationEffect(const DrawFunction& drawFunc, int32 seconds = 5, double speed = 5.0)
		: m_speed(speed), m_drawFunc(drawFunc)
	{
		m_stopwatch.set(std::chrono::seconds(seconds));
	}

	// 毎フレーム呼び出してアニメーション描画を行う
	void update(const TShape& shape, bool doTrigger = false)
	{
		if (doTrigger)
		{
			trigger();
		}

		// 経過時間に speed を掛けて t を計算
		const double t = m_stopwatch.sF() * m_speed;
		m_drawFunc(shape, t);
	}

	// ボタン押下などでアニメーションをリセットする
	void trigger()
	{
		m_stopwatch.restart();
	}

private:
	Stopwatch m_stopwatch;  // アニメーション用タイマー
	double m_speed;         // アニメーションの進行速度
	DrawFunction m_drawFunc;
};
