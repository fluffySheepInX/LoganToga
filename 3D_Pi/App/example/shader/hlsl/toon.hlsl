//-----------------------------------------------
//
//	Toon (Cel) shading post effect (HLSL)
//	輝度を数段階に量子化して色相を保ちつつ陰影をセル化
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

// ユーザー領域 (b1)
//   x = numBands  (セル段数, >=2)
//   y = shadowFloor (最暗バンドの輝度, 0..1)
cbuffer PSEffectParams : register(b1)
{
    float4 g_param0;
}

float4 PS(s3d::PSInput input) : SV_TARGET
{
    float4 c = g_texture0.Sample(g_sampler0, input.uv);

    float l = dot(c.rgb, float3(0.299, 0.587, 0.114));

    float n = max(g_param0.x, 2.0);
    float floorV = saturate(g_param0.y);

    // l in [0,1] -> band index in [0, n-1]
    float idx = min(floor(l * n), n - 1.0);
    // 等間隔で floorV..1.0 にマッピング
    float band = lerp(floorV, 1.0, idx / (n - 1.0));

    float3 hue = (l > 1e-4) ? (c.rgb / l) : float3(1.0, 1.0, 1.0);
    c.rgb = saturate(hue * band);

    return (c * input.color) + g_colorAdd;
}
