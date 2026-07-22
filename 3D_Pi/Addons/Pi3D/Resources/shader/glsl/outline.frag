//
//	Sobel ink outline (GLSL)
//

# version 410

uniform sampler2D Texture0;

layout(location = 0) in vec4 Color;
layout(location = 1) in vec2 UV;

layout(location = 0) out vec4 FragColor;

layout(std140) uniform PSConstants2D
{
    vec4 g_colorAdd;
    vec4 g_sdfParam;
    vec4 g_sdfOutlineColor;
    vec4 g_sdfShadowColor;
    vec4 g_internal;
};

// ユーザー領域 (b1)
//   x = threshold     (0..1)
//   y = thickness     (0.5..3) テクセル単位
//   z = inkStrength   (0..1)
layout(std140) uniform PSEffectParams
{
    vec4 g_param0;
};

float Luma(vec3 c)
{
    return dot(c, vec3(0.299, 0.587, 0.114));
}

void main()
{
    float threshold   = g_param0.x;
    float thickness   = max(g_param0.y, 0.5);
    float inkStrength = clamp(g_param0.z, 0.0, 1.0);

    vec2 texel = thickness / vec2(textureSize(Texture0, 0));

    float s00 = Luma(texture(Texture0, UV + vec2(-1.0, -1.0) * texel).rgb);
    float s10 = Luma(texture(Texture0, UV + vec2( 0.0, -1.0) * texel).rgb);
    float s20 = Luma(texture(Texture0, UV + vec2( 1.0, -1.0) * texel).rgb);
    float s01 = Luma(texture(Texture0, UV + vec2(-1.0,  0.0) * texel).rgb);
    float s21 = Luma(texture(Texture0, UV + vec2( 1.0,  0.0) * texel).rgb);
    float s02 = Luma(texture(Texture0, UV + vec2(-1.0,  1.0) * texel).rgb);
    float s12 = Luma(texture(Texture0, UV + vec2( 0.0,  1.0) * texel).rgb);
    float s22 = Luma(texture(Texture0, UV + vec2( 1.0,  1.0) * texel).rgb);

    float gx = (-s00 - 2.0 * s01 - s02) + (s20 + 2.0 * s21 + s22);
    float gy = (-s00 - 2.0 * s10 - s20) + (s02 + 2.0 * s12 + s22);
    float edge = sqrt(gx * gx + gy * gy);

    float edgeAmt = smoothstep(threshold, threshold + 0.1, edge) * inkStrength;

    vec4 base = texture(Texture0, UV);
    base.rgb = mix(base.rgb, vec3(0.0), edgeAmt);

    FragColor = (base * Color) + g_colorAdd;
}
