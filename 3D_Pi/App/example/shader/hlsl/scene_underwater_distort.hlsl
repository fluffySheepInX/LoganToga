//-----------------------------------------------
//  scene_underwater_distort.hlsl
//   t0 = source HDR
//   t1 = scene depth (distance to eye)
//   g_params.x = distortion strength in UV
//   g_params.y = time
//   g_params.z = speed
//   g_params.w = scale
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

float4 PS(s3d::PSInput input) : SV_TARGET
{
	const float2 uv = input.uv;
	const float depth = g_texture1.Sample(g_sampler0, uv).r;
	const float time = g_params.y * g_params.z;
	const float scale = max(g_params.w, 1.0f);
	const float depthFactor = (depth >= 99999.0f) ? 0.55f : smoothstep(4.0f, 90.0f, depth);
	const float waveX = sin((uv.y * scale + time) * 6.2831853f);
	const float waveY = cos((uv.x * (scale * 0.73f) - time * 0.83f) * 6.2831853f);
	const float ripple = sin(((uv.x + uv.y) * (scale * 0.41f) + time * 0.57f) * 6.2831853f);
	const float2 offset = float2(waveX + ripple * 0.45f, waveY * 0.35f) * g_params.x * lerp(0.35f, 1.0f, depthFactor);
	const float2 sampleUV = clamp(uv + offset, float2(0.001f, 0.001f), float2(0.999f, 0.999f));

	return g_texture0.Sample(g_sampler0, sampleUV);
}
