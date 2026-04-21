//-----------------------------------------------
//  fxaa.frag (FXAA Console 簡易版)
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
    vec2 ts = g_params.xy;
    float strength = max(g_params.z, 1e-3);

    vec3 rgbNW = texture(Texture0, UV + vec2(-ts.x, -ts.y)).rgb;
    vec3 rgbNE = texture(Texture0, UV + vec2( ts.x, -ts.y)).rgb;
    vec3 rgbSW = texture(Texture0, UV + vec2(-ts.x,  ts.y)).rgb;
    vec3 rgbSE = texture(Texture0, UV + vec2( ts.x,  ts.y)).rgb;
    vec3 rgbM  = texture(Texture0, UV).rgb;

    const vec3 luma = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * (1.0 / 8.0)), 1.0 / 128.0);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = clamp(dir * rcpDirMin, -8.0, 8.0) * ts * strength;

    vec3 rgbA = 0.5 * (
        texture(Texture0, UV + dir * (1.0 / 3.0 - 0.5)).rgb +
        texture(Texture0, UV + dir * (2.0 / 3.0 - 0.5)).rgb);
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture(Texture0, UV + dir * -0.5).rgb +
        texture(Texture0, UV + dir *  0.5).rgb);

    float lumaB = dot(rgbB, luma);
    vec3 col = ((lumaB < lumaMin) || (lumaB > lumaMax)) ? rgbA : rgbB;
    FragColor = vec4(col, 1.0);
}
