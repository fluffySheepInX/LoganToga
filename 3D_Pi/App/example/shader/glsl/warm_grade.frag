//-----------------------------------------------
//  warm_grade.frag
//  2000s 海外ゲーム風 セピア寄りカラーグレーディング
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
    // x: warmth   y: saturation   z: contrast   w: shadow lift
    vec4 g_params;
};

void main()
{
    vec3 col = texture(Texture0, UV).rgb;

    col = col + g_params.w * (1.0 - col);
    col = clamp((col - 0.5) * g_params.z + 0.5, 0.0, 1.0);

    float luma = dot(col, vec3(0.299, 0.587, 0.114));
    col = mix(vec3(luma), col, g_params.y);

    vec3 warmTint   = vec3(1.10, 0.98, 0.78);
    vec3 coolShadow = vec3(0.92, 0.96, 1.05);
    vec3 graded = col * mix(coolShadow, warmTint, luma);
    col = mix(col, graded, g_params.x);

    FragColor = vec4(col, 1.0);
}
