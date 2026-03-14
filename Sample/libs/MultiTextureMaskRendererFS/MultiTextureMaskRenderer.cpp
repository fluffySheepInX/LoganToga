# include "../MultiTextureMaskRenderer.h"

MultiTextureMaskRenderer::MultiTextureMaskRenderer(const Size& size, const FilePath& hlslPath, const FilePath& glslPath)
	: m_maskTexture1{ size }
	, m_maskTexture2{ size }
	, m_shader{ HLSL{ hlslPath, U"PS" } | GLSL{ glslPath, {{U"PSConstants2D", 0}} } }
{
}

bool MultiTextureMaskRenderer::IsValid() const
{
	return static_cast<bool>(m_shader);
}

const RenderTexture& MultiTextureMaskRenderer::GetMaskTexture1() const
{
	return m_maskTexture1;
}

const RenderTexture& MultiTextureMaskRenderer::GetMaskTexture2() const
{
	return m_maskTexture2;
}

void MultiTextureMaskRenderer::UpdateMaskTexture1(const std::function<void()>& draw)
{
	m_maskTexture1.clear(ColorF{ 0.0, 0.0 });
	const ScopedRenderTarget2D target{ m_maskTexture1 };
	const ScopedRenderStates2D blend{ MakeAlphaMaxBlendState() };
	draw();
}

void MultiTextureMaskRenderer::UpdateMaskTexture2(const std::function<void()>& draw)
{
	m_maskTexture2.clear(ColorF{ 0.0, 1.0 });
	const ScopedRenderTarget2D target{ m_maskTexture2 };
	draw();
}

void MultiTextureMaskRenderer::DrawMasked(const Texture& texture, const Vec2& pos)
{
	Graphics2D::SetPSTexture(1, m_maskTexture2);
	const ScopedCustomShader2D shader{ m_shader };
	texture.draw(pos);
}

bool MultiTextureMaskRenderer::init()
{
	return IsValid();
}

bool MultiTextureMaskRenderer::update()
{
	return true;
}

void MultiTextureMaskRenderer::draw() const
{
}

BlendState MultiTextureMaskRenderer::MakeAlphaMaxBlendState()
{
	BlendState blendState = BlendState::Default2D;
	blendState.srcAlpha = Blend::SrcAlpha;
	blendState.dstAlpha = Blend::DestAlpha;
	blendState.opAlpha = BlendOp::Max;
	return blendState;
}
