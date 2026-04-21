//-----------------------------------------------
//  bloom_extract.frag
//  ソフトニーしきい値付きの bright-pass 抽出 (Bloom 前段)
//-----------------------------------------------

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

layout(std140) uniform PSEffectParams
{
    vec4 g_params;
};

void main()
{
    vec3 c = texture(Texture0, UV).rgb;
    float br = max(c.r, max(c.g, c.b));

    float threshold = g_params.x;
    float knee = max(g_params.y, 1e-4);

    float rq = clamp(br - threshold + knee, 0.0, 2.0 * knee);
    rq = rq * rq / (4.0 * knee + 1e-4);

    float mul = max(rq, br - threshold) / max(br, 1e-4);
    c = c * mul * g_params.z;

    FragColor = vec4(c, 1.0);
}
