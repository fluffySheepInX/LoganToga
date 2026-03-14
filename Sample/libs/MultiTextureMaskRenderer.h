# pragma once
# include <Siv3D.hpp>

class MultiTextureMaskRenderer : public IAddon
{
public:

	static constexpr StringView AddonName{ U"MultiTextureMaskRenderer" };

	MultiTextureMaskRenderer(const Size& size, const FilePath& hlslPath, const FilePath& glslPath);

	[[nodiscard]]
	bool IsValid() const;

	[[nodiscard]]
	const RenderTexture& GetMaskTexture1() const;

	[[nodiscard]]
	const RenderTexture& GetMaskTexture2() const;

	void UpdateMaskTexture1(const std::function<void()>& draw);

	void UpdateMaskTexture2(const std::function<void()>& draw);

	void DrawMasked(const Texture& texture, const Vec2& pos);

private:

	bool init() override;

	bool update() override;

	void draw() const override;

	static BlendState MakeAlphaMaxBlendState();

	RenderTexture m_maskTexture1;
	RenderTexture m_maskTexture2;
	PixelShader m_shader;
};
