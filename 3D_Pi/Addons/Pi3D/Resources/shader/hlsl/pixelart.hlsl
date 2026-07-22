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

// ユーザー領域 (b1)
//   x = levels (色量子化段数)
cbuffer PSEffectParams : register(b1)
{
    float4 g_param0;
}

float4 PS(s3d::PSInput input) : SV_TARGET
{
    // 1 ドットを何ピクセルにするか (シェーダ内固定: ループ展開のため)
    const int pixelScale = 4;
    // カラーパレットの段階数 (チャンネル毎、CPU 側から可変)
    float levels = max(g_param0.x, 2.0);

    uint w, h;
    g_texture0.GetDimensions(w, h);
    float2 texSize = float2(w, h);

    float2 blockOrigin = floor(input.uv * texSize / pixelScale) * pixelScale;

    float4 sum = float4(0, 0, 0, 0);
    [unroll] for (int y = 0; y < pixelScale; ++y)
    {
        [unroll] for (int x = 0; x < pixelScale; ++x)
        {
            float2 sampleUV = (blockOrigin + float2(x + 0.5, y + 0.5)) / texSize;
            sum += g_texture0.Sample(g_sampler0, sampleUV);
        }
    }
    float4 c = sum / (pixelScale * pixelScale);

    c.rgb = floor(c.rgb * levels) / (levels - 1.0);
    c.rgb = saturate(c.rgb);

    return (c * input.color) + g_colorAdd;
}
