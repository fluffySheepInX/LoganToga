//
//	Pixel-art post effect (GLSL)
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
//   x = levels (色量子化段数)
layout(std140) uniform PSEffectParams
{
    vec4 g_param0;
};

void main()
{
    const int pixelScale = 4;
    float levels = max(g_param0.x, 2.0);

    vec2 texSize = vec2(textureSize(Texture0, 0));

    vec2 blockOrigin = floor(UV * texSize / float(pixelScale)) * float(pixelScale);

    vec4 sum = vec4(0.0);
    for (int y = 0; y < pixelScale; ++y)
    {
        for (int x = 0; x < pixelScale; ++x)
        {
            vec2 sampleUV = (blockOrigin + vec2(float(x) + 0.5, float(y) + 0.5)) / texSize;
            sum += texture(Texture0, sampleUV);
        }
    }
    vec4 c = sum / float(pixelScale * pixelScale);

    c.rgb = floor(c.rgb * levels) / (levels - 1.0);
    c.rgb = clamp(c.rgb, 0.0, 1.0);

    FragColor = (c * Color) + g_colorAdd;
}
