//-----------------------------------------------
//
//	Bayer dither threshold post effect (HLSL)
//
//-----------------------------------------------

//
//	Textures
//
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

//
//	Constant Buffer
//
cbuffer PSConstants2D : register(b0)
{
    float4 g_colorAdd;
    float4 g_sdfParam;
    float4 g_sdfOutlineColor;
    float4 g_sdfShadowColor;
    float4 g_internal;
}

// ユーザー領域 (b1)
//   x = threshold, y = dither scale, z = dither strength
cbuffer PSEffectParams : register(b1)
{
    float4 g_param0;
}

float Bayer4(int2 p)
{
    int x = p.x & 3;
    int y = p.y & 3;
    int index = x + y * 4;
    float values[16] = {
         0.0,  8.0,  2.0, 10.0,
        12.0,  4.0, 14.0,  6.0,
         3.0, 11.0,  1.0,  9.0,
        15.0,  7.0, 13.0,  5.0
    };
    return (values[index] + 0.5) / 16.0;
}

float4 PS(s3d::PSInput input) : SV_TARGET
{
    uint w, h;
    g_texture0.GetDimensions(w, h);
    float2 texSize = float2(w, h);

    float4 c = g_texture0.Sample(g_sampler0, input.uv);
    float luma = dot(c.rgb, float3(0.299, 0.587, 0.114));

    float threshold = saturate(g_param0.x);
    float scale = max(g_param0.y, 1.0);
    float strength = saturate(g_param0.z);
    int2 ditherPos = int2(floor(input.uv * texSize / scale));
    float dither = (Bayer4(ditherPos) - 0.5) * strength;
    float bw = step(threshold, luma + dither);

    c.rgb = float3(bw, bw, bw);

    return (c * input.color) + g_colorAdd;
}
