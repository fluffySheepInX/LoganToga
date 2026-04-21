# version 410

uniform sampler2D Texture0;
uniform sampler2D Texture1;
uniform sampler2D Texture2;

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
    vec3 sharp = texture(Texture0, UV).rgb;
    vec3 blurred = texture(Texture2, UV).rgb;
    float depth = texture(Texture1, UV).r;

    float focusDistance = g_params.x;
    float focusRange = max(g_params.y, 1e-4);
    float nearTransition = max(g_params.z, 1e-4);
    float farTransition = max(g_params.w, 1e-4);

    float nearBlur = clamp((focusDistance - focusRange - depth) / nearTransition, 0.0, 1.0);
    float farBlur = clamp((depth - focusDistance - focusRange) / farTransition, 0.0, 1.0);
    float coc = max(nearBlur, farBlur);

    FragColor = vec4(mix(sharp, blurred, coc), 1.0);
}
