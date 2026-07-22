// CRT post effect (HLSL) - minimal safe version

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

// b1
//   x = curvature        (0..0.5)
//   y = scanlineStrength (0..1)
//   z = maskStrength     (0..1)
//   w = vignette         (0..1)
cbuffer PSEffectParams : register(b1)
{
    float4 g_param0;
}

float2 BarrelDistort(float2 uv, float curvature)
{
    float2 cc = uv * 2.0 - 1.0;
    float r2 = dot(cc, cc);
    cc *= 1.0 + curvature * r2;
    return cc * 0.5 + 0.5;
}

float4 PS(s3d::PSInput input) : SV_TARGET
{
    float curvature        = g_param0.x;
    float scanlineStrength = saturate(g_param0.y);
    float maskStrength     = saturate(g_param0.z);
    float vignette         = saturate(g_param0.w);

    float2 uv = BarrelDistort(input.uv, curvature);

    // out-of-range flag (no early return; keep uniform control flow)
    float inside = step(0.0, uv.x) * step(uv.x, 1.0)
                 * step(0.0, uv.y) * step(uv.y, 1.0);

    float2 sampleUV = saturate(uv);

    uint w, h;
    g_texture0.GetDimensions(w, h);
    float fw = (float)w;
    float fh = (float)h;

    float3 color = g_texture0.SampleLevel(g_sampler0, sampleUV, 0).rgb;

    // Scanlines: sin^2 along Y in pixel space
    float scan = sin(uv.y * fh * 3.14159265);
    scan = scan * scan;
    color *= lerp(1.0, scan, scanlineStrength);

    // RGB sub-pixel mask (3-pixel period along X)
    float subF = floor(uv.x * fw);
    float modF = subF - 3.0 * floor(subF / 3.0); // 0,1,2
    float3 mask = float3(0.55, 0.55, 0.55);
    if (modF < 0.5)        { mask = float3(1.0, 0.55, 0.55); }
    else if (modF < 1.5)   { mask = float3(0.55, 1.0, 0.55); }
    else                   { mask = float3(0.55, 0.55, 1.0); }
    color *= lerp(float3(1.0, 1.0, 1.0), mask, maskStrength);

    // Vignette
    float2 vc = uv * 2.0 - 1.0;
    float vd = dot(vc, vc);
    color *= lerp(1.0, saturate(1.0 - vd * 0.7), vignette);

    color *= inside;

    return float4(color, 1.0);
}
