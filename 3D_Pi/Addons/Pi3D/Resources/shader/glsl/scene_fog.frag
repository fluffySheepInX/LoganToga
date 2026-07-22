# version 410

uniform sampler2D Texture0;
uniform sampler2D Texture1;

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

layout(std140) uniform PSFogColor
{
    vec4 g_fogColor;
};

void main()
{
    vec4 source = texture(Texture0, UV);
    float depth = texture(Texture1, UV).r;
    if (depth >= 99999.0)
    {
        float density = clamp(g_params.z, 0.0, 1.0);
        float horizonFog = smoothstep(0.25, 1.0, UV.y);
        float skyFog = density * mix(0.20, 0.65, horizonFog);
        FragColor = vec4(mix(source.rgb, g_fogColor.rgb, skyFog), source.a);
        return;
    }

    float startDistance = g_params.x;
    float endDistance = max(g_params.y, startDistance + 1e-3);
    float density = clamp(g_params.z, 0.0, 1.0);
    float fog = clamp((depth - startDistance) / (endDistance - startDistance), 0.0, 1.0) * density;

    FragColor = vec4(mix(source.rgb, g_fogColor.rgb, fog), source.a);
}
