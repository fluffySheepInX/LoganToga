//
//	Mosaic post effect (GLSL)
//

# version 410

//
//	Textures
//
uniform sampler2D Texture0;

//
//	PSInput
//
layout(location = 0) in vec4 Color;
layout(location = 1) in vec2 UV;

//
//	PSOutput
//
layout(location = 0) out vec4 FragColor;

//
//	Constant Buffer
//
layout(std140) uniform PSConstants2D
{
    vec4 g_colorAdd;
    vec4 g_sdfParam;
    vec4 g_sdfOutlineColor;
    vec4 g_sdfShadowColor;
    vec4 g_internal;
};

// ユーザー領域 (b1)
//   x = block size in pixels
layout(std140) uniform PSEffectParams
{
    vec4 g_param0;
};

void main()
{
    vec2 texSize = vec2(textureSize(Texture0, 0));
    float blockSize = max(g_param0.x, 1.0);

    vec2 pixel = UV * texSize;
    vec2 blockCenter = (floor(pixel / blockSize) + vec2(0.5)) * blockSize;
    vec2 sampleUV = clamp(blockCenter / texSize, vec2(0.0), vec2(1.0));

    vec4 c = texture(Texture0, sampleUV);

    FragColor = (c * Color) + g_colorAdd;
}
