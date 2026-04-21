//
//	Toon (Cel) shading post effect (GLSL)
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
//   x = numBands  (セル段数, >=2)
//   y = shadowFloor (最暗バンドの輝度, 0..1)
layout(std140) uniform PSEffectParams
{
    vec4 g_param0;
};

void main()
{
    vec4 c = texture(Texture0, UV);

    float l = dot(c.rgb, vec3(0.299, 0.587, 0.114));

    float n = max(g_param0.x, 2.0);
    float floorV = clamp(g_param0.y, 0.0, 1.0);

    float idx = min(floor(l * n), n - 1.0);
    float band = mix(floorV, 1.0, idx / (n - 1.0));

    vec3 hue = (l > 1e-4) ? (c.rgb / l) : vec3(1.0);
    c.rgb = clamp(hue * band, 0.0, 1.0);

    FragColor = (c * Color) + g_colorAdd;
}
