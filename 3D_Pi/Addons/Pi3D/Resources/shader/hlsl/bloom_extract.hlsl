//-----------------------------------------------
//
//  bloom_extract.hlsl
//  ソフトニーしきい値付きの bright-pass 抽出 (Bloom 前段)
//    g_params.x = threshold
//    g_params.y = knee
//    g_params.z = preScale (1.0 で素通し)
//
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
    float3 c = g_texture0.Sample(g_sampler0, input.uv).rgb;
    float br = max(c.r, max(c.g, c.b));

    float threshold = g_params.x;
    float knee = max(g_params.y, 1e-4);

    float rq = clamp(br - threshold + knee, 0.0, 2.0 * knee);
    rq = rq * rq / (4.0 * knee + 1e-4);

    float mul = max(rq, br - threshold) / max(br, 1e-4);
    c = c * mul * g_params.z;

    return float4(c, 1.0);
}
