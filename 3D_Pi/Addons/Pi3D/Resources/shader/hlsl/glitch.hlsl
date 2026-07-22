//-----------------------------------------------
//  glitch.hlsl
//   g_params.x = time
//   g_params.y = intensity
//   g_params.z = blockiness
//   g_params.w = rgb shift
//-----------------------------------------------

Texture2D       g_texture0 : register(t0);
SamplerState   g_sampler0 : register(s0);

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

float stripeMask(float y, float time, float blockiness)
{
    float rows = lerp(18.0, 96.0, saturate(blockiness));
    float row = floor(y * rows);
    float n = hash21(float2(row, floor(time * 18.0)));
    float pulse = smoothstep(0.60, 0.98, n);
    return pulse;
}

float4 PS(s3d::PSInput input) : SV_TARGET
{
    float intensity = saturate(g_params.y);
    float blockiness = saturate(g_params.z);
    float rgbShift = g_params.w * intensity;
    float time = g_params.x;

    float2 uv = input.uv;
    float stripe = stripeMask(uv.y, time, blockiness) * intensity;
    float jitter = (hash21(float2(floor(uv.y * lerp(24.0, 140.0, blockiness)), floor(time * 24.0))) - 0.5)
        * lerp(0.01, 0.08, blockiness) * stripe;

    float tear = step(0.985, hash21(float2(floor(time * 12.0), floor(uv.y * 8.0)))) * intensity;
    uv.x += jitter + tear * 0.06 * sin(time * 80.0 + uv.y * 80.0);

    float2 blockUv = floor(uv * float2(80.0, lerp(32.0, 120.0, blockiness))) / float2(80.0, lerp(32.0, 120.0, blockiness));
    float blockNoise = hash21(blockUv + floor(time * 20.0));
    float blockMask = smoothstep(0.86, 1.0, blockNoise) * intensity * blockiness;

    float scanNoise = hash21(float2(floor(uv.y * 360.0), floor(time * 60.0))) - 0.5;
    uv.x += scanNoise * 0.006 * intensity;

    float r = g_texture0.Sample(g_sampler0, uv + float2(-rgbShift - blockMask * 0.025, 0.0)).r;
    float g = g_texture0.Sample(g_sampler0, uv + float2(jitter * 0.35, 0.0)).g;
    float b = g_texture0.Sample(g_sampler0, uv + float2(rgbShift + blockMask * 0.025, 0.0)).b;
    float a = g_texture0.Sample(g_sampler0, uv).a;

    float3 col = float3(r, g, b);
    float digitalNoise = (hash21(input.uv * 900.0 + time * 41.0) - 0.5) * 0.22 * intensity;
    col += digitalNoise;
    col = lerp(col, col * float3(0.55, 1.25, 1.45), blockMask * 0.7);

    return float4(saturate(col), a) * input.color + g_colorAdd;
}
