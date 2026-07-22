//-----------------------------------------------
//  dof_combine.hlsl
//   t0 = source HDR
//   t1 = scene depth (distance to eye)
//   t2 = blurred scene
//   g_params.x = focusDistance
//   g_params.y = focusRange
//   g_params.z = nearTransition
//   g_params.w = farTransition
//-----------------------------------------------

Texture2D		g_texture0 : register(t0);
Texture2D		g_texture1 : register(t1);
Texture2D		g_texture2 : register(t2);
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
    const float3 sharp = g_texture0.Sample(g_sampler0, input.uv).rgb;
    const float3 blurred = g_texture2.Sample(g_sampler0, input.uv).rgb;
    const float depth = g_texture1.Sample(g_sampler0, input.uv).r;

    const float focusDistance = g_params.x;
    const float focusRange = max(g_params.y, 1e-4);
    const float nearTransition = max(g_params.z, 1e-4);
    const float farTransition = max(g_params.w, 1e-4);

    const float nearBlur = saturate((focusDistance - focusRange - depth) / nearTransition);
    const float farBlur = saturate((depth - focusDistance - focusRange) / farTransition);
    const float coc = max(nearBlur, farBlur);

    return float4(lerp(sharp, blurred, coc), 1.0f);
}
