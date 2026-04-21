//-----------------------------------------------
//  film_grain.hlsl
//   g_params.x = time      (CPU から毎フレーム更新)
//   g_params.y = strength  (0..0.3)
//   g_params.z = luma_only (0/1)
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

float hash21(float2 p)
{
    p = frac(p * float2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return frac(p.x * p.y);
}

float4 PS(s3d::PSInput input) : SV_TARGET
{
    float3 col = g_texture0.Sample(g_sampler0, input.uv).rgb;

    float n = hash21(input.uv * 1024.0 + g_params.x * 17.0) - 0.5;

    if (g_params.z > 0.5)
    {
        col += n * g_params.y;
    }
    else
    {
        float3 n3 = float3(
            n,
            hash21(input.uv * 1024.0 + g_params.x * 31.0 + 7.0) - 0.5,
            hash21(input.uv * 1024.0 + g_params.x * 53.0 + 13.0) - 0.5
        );
        col += n3 * g_params.y;
    }

    return float4(saturate(col), 1.0);
}
