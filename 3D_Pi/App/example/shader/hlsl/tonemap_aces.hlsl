//-----------------------------------------------
//  tonemap_aces.hlsl
//   g_params.x = exposure (0..4)
//   g_params.y = gamma    (1..2.4) ※リニア RT に書く時の手動ガンマ用
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

float3 ACESFilm(float3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

float4 PS(s3d::PSInput input) : SV_TARGET
{
    float3 col = g_texture0.Sample(g_sampler0, input.uv).rgb;
    col = ACESFilm(col * g_params.x);
    col = pow(max(col, 0.0), 1.0 / max(g_params.y, 1e-3));
    return float4(col, 1.0);
}
