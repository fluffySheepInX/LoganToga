//-----------------------------------------------
//
//	Pixel-art post effect (HLSL)
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

float4 PS(s3d::PSInput input) : SV_TARGET
{
    // カラーパレットの段階数 (チャンネル毎)
    const float levels = 5.0;

    float4 texColor = g_texture0.Sample(g_sampler0, input.uv);

    // カラー量子化 (ポスタリゼーション)
    texColor.rgb = floor(texColor.rgb * levels) / (levels - 1.0);
    texColor.rgb = saturate(texColor.rgb);

    return (texColor * input.color) + g_colorAdd;
}
