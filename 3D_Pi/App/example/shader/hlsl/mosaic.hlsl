//-----------------------------------------------
//
//	Mosaic post effect (HLSL)
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
//   x = block size in pixels
cbuffer PSEffectParams : register(b1)
{
    float4 g_param0;
}

float4 PS(s3d::PSInput input) : SV_TARGET
{
    uint w, h;
    g_texture0.GetDimensions(w, h);
    float2 texSize = float2(w, h);
    float blockSize = max(g_param0.x, 1.0);

    float2 pixel = input.uv * texSize;
    float2 blockCenter = (floor(pixel / blockSize) + 0.5) * blockSize;
    float2 sampleUV = saturate(blockCenter / texSize);

    float4 c = g_texture0.Sample(g_sampler0, sampleUV);

    return (c * input.color) + g_colorAdd;
}
