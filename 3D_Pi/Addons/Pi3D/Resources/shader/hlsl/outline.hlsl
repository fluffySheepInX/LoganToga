//-----------------------------------------------
//
//	Sobel ink outline (HLSL)
//	輝度勾配 (Sobel filter) でエッジを検出して黒線を被せる
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
//   x = threshold     (0..1) エッジ強度のしきい値
//   y = thickness     (0.5..3) サンプル間隔 (テクセル単位)
//   z = inkStrength   (0..1) 線の濃さ (0 で線なし)
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
    float threshold   = g_param0.x;
    float thickness   = max(g_param0.y, 0.5);
    float inkStrength = saturate(g_param0.z);

    uint w, h;
    g_texture0.GetDimensions(w, h);
    float2 texel = thickness / float2(w, h);

    // 3x3 近傍を一度ずつサンプル (s00..s22)
    float s00 = Luma(g_texture0.Sample(g_sampler0, input.uv + float2(-1, -1) * texel).rgb);
    float s10 = Luma(g_texture0.Sample(g_sampler0, input.uv + float2( 0, -1) * texel).rgb);
    float s20 = Luma(g_texture0.Sample(g_sampler0, input.uv + float2( 1, -1) * texel).rgb);
    float s01 = Luma(g_texture0.Sample(g_sampler0, input.uv + float2(-1,  0) * texel).rgb);
    float s21 = Luma(g_texture0.Sample(g_sampler0, input.uv + float2( 1,  0) * texel).rgb);
    float s02 = Luma(g_texture0.Sample(g_sampler0, input.uv + float2(-1,  1) * texel).rgb);
    float s12 = Luma(g_texture0.Sample(g_sampler0, input.uv + float2( 0,  1) * texel).rgb);
    float s22 = Luma(g_texture0.Sample(g_sampler0, input.uv + float2( 1,  1) * texel).rgb);

    // Sobel kernel
    //   Gx = [-1 0 1; -2 0 2; -1 0 1]
    //   Gy = [-1 -2 -1; 0 0 0; 1 2 1]
    float gx = (-s00 - 2.0 * s01 - s02) + (s20 + 2.0 * s21 + s22);
    float gy = (-s00 - 2.0 * s10 - s20) + (s02 + 2.0 * s12 + s22);
    float edge = sqrt(gx * gx + gy * gy);

    // しきい値で滑らかに 0..1 に正規化
    float edgeAmt = smoothstep(threshold, threshold + 0.1, edge) * inkStrength;

    // 元の色に黒線を合成
    float4 base = g_texture0.Sample(g_sampler0, input.uv);
    float3 inkColor = float3(0.0, 0.0, 0.0);
    base.rgb = lerp(base.rgb, inkColor, edgeAmt);

    return (base * input.color) + g_colorAdd;
}
