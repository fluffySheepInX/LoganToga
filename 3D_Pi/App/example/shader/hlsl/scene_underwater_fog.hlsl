//-----------------------------------------------
//  scene_underwater_fog.hlsl
//   t0 = source HDR
//   t1 = scene depth (distance to eye)
//   g_params.x = start distance
//   g_params.y = end distance
//   g_params.z = density
//   g_params.w = time
//   g_fogColor.rgb = underwater fog color
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
	const float density = saturate(g_params.z);

	if (depth >= 99999.0f)
	{
		const float horizonFog = smoothstep(0.15f, 1.0f, input.uv.y);
		const float skyFog = density * lerp(0.35f, 0.78f, horizonFog);
		const float3 skyTint = lerp(source.rgb, source.rgb * float3(0.72f, 0.95f, 1.06f), 0.35f);
		return float4(lerp(skyTint, g_fogColor.rgb, skyFog), source.a);
	}

	const float startDistance = g_params.x;
	const float endDistance = max(g_params.y, startDistance + 1e-3f);
	const float distance01 = saturate((depth - startDistance) / (endDistance - startDistance));
	const float fog = pow(distance01, 0.82f) * density;
	const float contrastLoss = saturate(fog * 0.48f);
	const float luminance = dot(source.rgb, float3(0.299f, 0.587f, 0.114f));
	const float3 absorbed = source.rgb * float3(0.58f, 0.86f, 1.04f);
	const float3 lowContrast = lerp(absorbed, float3(luminance, luminance, luminance), contrastLoss);

	return float4(lerp(lowContrast, g_fogColor.rgb, fog), source.a);
}
