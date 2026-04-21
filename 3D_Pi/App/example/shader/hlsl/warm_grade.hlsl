//-----------------------------------------------
//
//	warm_grade.hlsl
//	2000s 海外ゲーム風 セピア寄りカラーグレーディング
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
    // x: warmth (0..1)   y: saturation (0..1.5)
    // z: contrast (0.5..2.0)   w: shadow lift (0..0.3)
    float4 g_params;
}

float4 PS(s3d::PSInput input) : SV_TARGET
{
    float3 col = g_texture0.Sample(g_sampler0, input.uv).rgb;

    // シャドウを少し持ち上げてコントラストを稼ぐ
    col = col + g_params.w * (1.0 - col);

    // コントラスト
    col = saturate((col - 0.5) * g_params.z + 0.5);

    // 彩度
    float luma = dot(col, float3(0.299, 0.587, 0.114));
    col = lerp(float3(luma, luma, luma), col, g_params.y);

    // 暖色寄りのティント (シャドウ:青寄り / ハイライト:黄寄りの弱 split-tone)
    float3 warmTint = float3(1.10, 0.98, 0.78);
    float3 coolShadow = float3(0.92, 0.96, 1.05);
    float3 graded = col * lerp(coolShadow, warmTint, luma);
    col = lerp(col, graded, g_params.x);

    return float4(col, 1.0);
}
