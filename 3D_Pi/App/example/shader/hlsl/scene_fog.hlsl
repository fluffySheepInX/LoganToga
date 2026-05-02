//-----------------------------------------------
//  scene_fog.hlsl
//   t0 = source HDR
//   t1 = scene depth (distance to eye)
//   g_params.x = start distance
//   g_params.y = end distance
//   g_params.z = density
//   g_fogColor.rgb = fog color
//-----------------------------------------------

Texture2D		g_texture0 : register(t0);
Texture2D		g_texture1 : register(t1);
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

cbuffer PSFogColor : register(b2)
{
    float4 g_fogColor;
}

float4 PS(s3d::PSInput input) : SV_TARGET
{
    const float4 source = g_texture0.Sample(g_sampler0, input.uv);
    const float depth = g_texture1.Sample(g_sampler0, input.uv).r;
    if (depth >= 99999.0f)
    {
        const float density = saturate(g_params.z);
        const float horizonFog = smoothstep(0.25f, 1.0f, input.uv.y);
        const float skyFog = density * lerp(0.20f, 0.65f, horizonFog);
        return float4(lerp(source.rgb, g_fogColor.rgb, skyFog), source.a);
    }

    const float startDistance = g_params.x;
    const float endDistance = max(g_params.y, startDistance + 1e-3f);
    const float density = saturate(g_params.z);
    const float fog = saturate((depth - startDistance) / (endDistance - startDistance)) * density;

    return float4(lerp(source.rgb, g_fogColor.rgb, fog), source.a);
}
