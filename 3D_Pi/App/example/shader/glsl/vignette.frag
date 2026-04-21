//-----------------------------------------------
//  vignette.frag
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
    vec3 col = texture(Texture0, UV).rgb;

    vec2 d = (UV - 0.5) * 2.0;
    d.x *= mix(1.7777, 1.0, g_params.z);
    float r = length(d);
    float v = smoothstep(1.0, 1.0 - max(g_params.y, 1e-3), r);
    float k = mix(1.0, v, clamp(g_params.x, 0.0, 1.0));
    col *= k;

    FragColor = vec4(col, 1.0);
}
