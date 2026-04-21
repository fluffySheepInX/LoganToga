//-----------------------------------------------
//
//	Kuwahara filter (HLSL)
//	油絵風: 4 つの重なる小領域から最小分散の平均色を採用
//
//-----------------------------------------------

Texture2D		g_texture0 : register(t0);
SamplerState	g_sampler0 : register(s0);

namespace s3d
{
    struct PSInput
    {
        float4 position : SV_POSITION;
        float4 color    : COLOR0;
        float2 uv       : TEXCOORD0;
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
//   x = radius   (1..6)  小領域の半径（整数）
cbuffer PSEffectParams : register(b1)
{
    float4 g_param0;
}

float Luma(float3 c)
{
    return dot(c, float3(0.299, 0.587, 0.114));
}

float4 PS(s3d::PSInput input) : SV_TARGET
{
    int radius = (int)clamp(g_param0.x, 1.0, 6.0);

    uint w, h;
    g_texture0.GetDimensions(w, h);
    float2 texel = 1.0 / float2(w, h);

    // 4 つの重なる小領域 (右下/左下/右上/左上)
    int2 offsets[4] = {
        int2( 0,  0),
        int2(-radius,  0),
        int2( 0, -radius),
        int2(-radius, -radius)
    };

    float3 bestMean = float3(0.0, 0.0, 0.0);
    float  bestVar  = 1e20;

    [unroll]
    for (int k = 0; k < 4; ++k)
    {
        float3 sumC  = float3(0.0, 0.0, 0.0);
        float  sumL  = 0.0;
        float  sumL2 = 0.0;
        float  n = 0.0;

        for (int j = 0; j <= radius; ++j)
        {
            for (int i = 0; i <= radius; ++i)
            {
                float2 uv = input.uv + (offsets[k] + int2(i, j)) * texel;
                float3 c = g_texture0.Sample(g_sampler0, uv).rgb;
                float  l = Luma(c);
                sumC  += c;
                sumL  += l;
                sumL2 += l * l;
                n     += 1.0;
            }
        }

        float3 mean = sumC / n;
        float  meanL = sumL / n;
        float  variance = (sumL2 / n) - meanL * meanL;

        if (variance < bestVar)
        {
            bestVar  = variance;
            bestMean = mean;
        }
    }

    return float4(bestMean, 1.0);
}
