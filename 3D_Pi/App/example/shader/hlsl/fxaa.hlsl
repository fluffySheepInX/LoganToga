//-----------------------------------------------
//  fxaa.hlsl  (FXAA Console 互換の簡易版 by Timothy Lottes)
//   g_params.xy = texelSize (1/width, 1/height)
//   g_params.z  = strength  (0..1) サンプル方向の強さ
//-----------------------------------------------

Texture2D		g_texture0 : register(t0);
SamplerState	g_sampler0 : register(s0);

namespace s3d
{
    struct PSInput
    {
        float4 position	: SV_POSITION;
        float4 color	: COLOR0;
        float2 uv		: TEXCOORD0;
    };
}

cbuffer PSConstants2D : register(b0)
{
    float4 g_colorAdd;
    float4 g_sdfParam;
    float4 g_sdfOutlineColor;
    float4 g_sdfShadowColor;
    float4 g_internal;
}

cbuffer PSEffectParams : register(b1)
{
    float4 g_params;
}

float4 PS(s3d::PSInput input) : SV_TARGET
{
    float2 ts = g_params.xy;
    float strength = max(g_params.z, 1e-3);

    float3 rgbNW = g_texture0.Sample(g_sampler0, input.uv + float2(-ts.x, -ts.y)).rgb;
    float3 rgbNE = g_texture0.Sample(g_sampler0, input.uv + float2( ts.x, -ts.y)).rgb;
    float3 rgbSW = g_texture0.Sample(g_sampler0, input.uv + float2(-ts.x,  ts.y)).rgb;
    float3 rgbSE = g_texture0.Sample(g_sampler0, input.uv + float2( ts.x,  ts.y)).rgb;
    float3 rgbM  = g_texture0.Sample(g_sampler0, input.uv).rgb;

    const float3 luma = float3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    float2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * (1.0 / 8.0)), 1.0 / 128.0);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = clamp(dir * rcpDirMin, -8.0, 8.0) * ts * strength;

    float3 rgbA = 0.5 * (
        g_texture0.Sample(g_sampler0, input.uv + dir * (1.0 / 3.0 - 0.5)).rgb +
        g_texture0.Sample(g_sampler0, input.uv + dir * (2.0 / 3.0 - 0.5)).rgb);
    float3 rgbB = rgbA * 0.5 + 0.25 * (
        g_texture0.Sample(g_sampler0, input.uv + dir * -0.5).rgb +
        g_texture0.Sample(g_sampler0, input.uv + dir *  0.5).rgb);

    float lumaB = dot(rgbB, luma);
    float3 col = ((lumaB < lumaMin) || (lumaB > lumaMax)) ? rgbA : rgbB;
    return float4(col, 1.0);
}
