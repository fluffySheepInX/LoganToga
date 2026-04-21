//-----------------------------------------------
//  vignette.hlsl
//   g_params.x = intensity   (0..1)
//   g_params.y = smoothness  (0.1..1.5)
//   g_params.z = roundness   (0..1) 0:画面比依存 / 1:真円
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
    float3 col = g_texture0.Sample(g_sampler0, input.uv).rgb;

    float2 d = (input.uv - 0.5) * 2.0;
    d.x *= lerp(1.7777, 1.0, g_params.z);
    float r = length(d);
    float v = smoothstep(1.0, 1.0 - max(g_params.y, 1e-3), r);
    float k = lerp(1.0, v, saturate(g_params.x));
    col *= k;

    return float4(col, 1.0);
}
