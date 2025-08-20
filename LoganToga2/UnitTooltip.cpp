#include "UnitTooltip.h"

UnitTooltip::UnitTooltip(const Font& font)
	: m_font(font)
{
}

void UnitTooltip::show(const Vec2& pos, const String& text)
{
	// 同じ内容で既に表示中の場合は位置のみ更新
	if (m_isVisible && m_content == text)
	{
		m_position = pos;
		return; // タイマーをリセットしない
	}

	m_position = pos;
	m_content = text;
	m_isVisible = true;
	m_fadeTimer.restart();
	updateRenderTexture();
}

void UnitTooltip::hide()
{
	m_isVisible = false;
	// 非表示時に前回の内容をクリアして、次回必ず更新されるようにする
	m_lastRenderedContent.clear();
}

void UnitTooltip::updateRenderTexture()
{
	if (m_content.isEmpty()) return;

	// テキストの測定（原点基準で正確に取得）
	const auto lines = m_content.split(U'\n');
	int32 maxWidth = 0;
	int32 totalHeight = 0;

	// 原点基準でテキストサイズを測定
	for (const auto& line : lines)
	{
		// region(Vec2::Zero()) で原点基準のサイズを取得
		const auto textRegion = m_font(line).region(Vec2::Zero());

		maxWidth = Max(maxWidth, static_cast<int32>(textRegion.w));
		totalHeight += static_cast<int32>(textRegion.h) + 4;
	}

	// パディングとボーダーを考慮
	const int32 padding = 16;
	const int32 borderWidth = 3;
	const int32 width = maxWidth + padding * 2 + borderWidth * 2;
	const int32 height = totalHeight + padding * 2 + borderWidth * 2;

	// テクスチャサイズが同じで、内容が変わっていない場合は作成をスキップ
	if (m_renderTexture &&
		m_renderTexture.size() == Size(width, height) &&
		m_lastRenderedContent == m_content)
	{
		return;
	}

	// 前回の内容を記録
	m_lastRenderedContent = m_content;

	m_renderTexture = RenderTexture(width, height);

	{
		const ScopedRenderTarget2D target{ m_renderTexture };
		const ScopedRenderStates2D blend{ MakeBlendState() };

		// 外側のフレーム（光る効果）
		RectF(0, 0, width, height).draw(ColorF{ 0.3, 0.6, 1.0, 0.8 });

		// 内側の背景
		RectF(borderWidth, borderWidth, width - borderWidth * 2, height - borderWidth * 2)
			.draw(ColorF{ 0.05, 0.05, 0.15, 0.95 });

		// グラデーション効果
		for (int32 i = 0; i < borderWidth; ++i)
		{
			const double alpha = 0.3 * (1.0 - static_cast<double>(i) / borderWidth);
			RectF(i, i, width - i * 2, height - i * 2)
				.drawFrame(1, ColorF{ 0.5, 0.8, 1.0, alpha });
		}

		// テキストの描画（座標系を統一）
		int32 yOffset = borderWidth + padding;
		for (const auto& line : lines)
		{
			// 明示的に原点基準で描画
			m_font(line).draw(
				Vec2(borderWidth + padding, yOffset),
				Palette::White
			);

			// 次の行の位置を計算（同じregion()メソッドを使用）
			const auto textRegion = m_font(line).region(Vec2::Zero());
			yOffset += static_cast<int32>(textRegion.h) + 4;
		}
	}
}

void UnitTooltip::draw() const
{
	if (!m_isVisible || m_content.isEmpty()) return;

	// フェードイン効果
	const double fadeTime = 0.3;
	const double alpha = Min(m_fadeTimer.sF() / fadeTime, 1.0);

	// 画面端での位置調整
	Vec2 drawPos = m_position;
	const auto textureSize = m_renderTexture.size();

	if (drawPos.x + textureSize.x > Scene::Width())
		drawPos.x = Scene::Width() - textureSize.x - 10;
	if (drawPos.y + textureSize.y > Scene::Height())
		drawPos.y = m_position.y - textureSize.y - 10;

	// 描画
	m_renderTexture.draw(drawPos, ColorF{ 1.0, 1.0, 1.0, alpha });
}
