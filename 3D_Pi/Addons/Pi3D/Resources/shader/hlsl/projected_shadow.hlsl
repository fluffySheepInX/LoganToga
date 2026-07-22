Texture2D g_texture0 : register(t0);
SamplerState g_sampler0 : register(s0);

namespace s3d
{
    struct PSInput
    {
        float4 position : SV_POSITION;
        float3 worldPosition : TEXCOORD0;
        float2 uv : TEXCOORD1;
        float3 normal : TEXCOORD2;
    };
}

cbuffer PSShadowEditor : register(b2)
{
    float4 g_shadowParams;
}

cbuffer PSPerMaterial : register(b3)
{
    float3 g_ambientColor;
    uint g_hasTexture;
    float4 g_diffuseColor;
    float3 g_specularColor;
    float g_shininess;
    float3 g_emissionColor;
}

float4 PS(s3d::PSInput input) : SV_TARGET
{
    float alpha = g_diffuseColor.a;

    if (g_hasTexture)
    {
        alpha *= g_texture0.Sample(g_sampler0, input.uv).a;
    }

    const float opacity = saturate(g_shadowParams.x);
    return float4(0.0, 0.0, 0.0, opacity * alpha);
}
