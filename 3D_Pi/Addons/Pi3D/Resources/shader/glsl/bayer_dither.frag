//
//	Bayer dither threshold post effect (GLSL)
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
//   x = threshold, y = dither scale, z = dither strength
layout(std140) uniform PSEffectParams
{
    vec4 g_param0;
};

float Bayer4(ivec2 p)
{
    int x = p.x & 3;
    int y = p.y & 3;
    int index = x + y * 4;
    float values[16] = float[16](
         0.0,  8.0,  2.0, 10.0,
        12.0,  4.0, 14.0,  6.0,
         3.0, 11.0,  1.0,  9.0,
        15.0,  7.0, 13.0,  5.0
    );
    return (values[index] + 0.5) / 16.0;
}

void main()
{
    vec2 texSize = vec2(textureSize(Texture0, 0));

    vec4 c = texture(Texture0, UV);
    float luma = dot(c.rgb, vec3(0.299, 0.587, 0.114));

    float threshold = clamp(g_param0.x, 0.0, 1.0);
    float scale = max(g_param0.y, 1.0);
    float strength = clamp(g_param0.z, 0.0, 1.0);
    ivec2 ditherPos = ivec2(floor(UV * texSize / scale));
    float dither = (Bayer4(ditherPos) - 0.5) * strength;
    float bw = step(threshold, luma + dither);

    c.rgb = vec3(bw);

    FragColor = (c * Color) + g_colorAdd;
}
