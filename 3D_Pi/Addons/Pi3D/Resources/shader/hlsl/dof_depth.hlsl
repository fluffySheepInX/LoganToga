//-----------------------------------------------
//  dof_depth.hlsl
//-----------------------------------------------

Texture2D		g_texture0 : register(t0);
SamplerState	g_sampler0 : register(s0);

namespace s3d
{
    struct VSInput
    {
        float4 position : POSITION;
        float3 normal : NORMAL;
        float2 uv : TEXCOORD0;
    };

    struct PSInput
    {
        float4 position : SV_POSITION;
        float3 worldPosition : TEXCOORD0;
        float2 uv : TEXCOORD1;
    };
}

cbuffer VSPerView : register(b1)
{
    row_major float4x4 g_worldToProjected;
}

cbuffer VSPerObject : register(b2)
{
    row_major float4x4 g_localToWorld;
}

cbuffer VSPerMaterial : register(b3)
{
    float4 g_uvTransform;
}

cbuffer PSPerView : register(b1)
{
    float3 g_eyePosition;
}

cbuffer PSPerMaterial : register(b3)
{
    float3 g_ambientColor;
    uint   g_hasTexture;
    float4 g_diffuseColor;
    float3 g_specularColor;
    float  g_shininess;
    float3 g_emissionColor;
}

s3d::PSInput VS(s3d::VSInput input)
{
    s3d::PSInput result;

    const float4 worldPosition = mul(input.position, g_localToWorld);

    result.position = mul(worldPosition, g_worldToProjected);
    result.worldPosition = worldPosition.xyz;
    result.uv = (input.uv * g_uvTransform.xy + g_uvTransform.zw);
    return result;
}

float4 PS(s3d::PSInput input) : SV_TARGET
{
    float alpha = g_diffuseColor.a;
    if (g_hasTexture)
    {
        alpha *= g_texture0.Sample(g_sampler0, input.uv).a;
    }
    clip(alpha - 0.1f);

    const float distanceToEye = distance(g_eyePosition, input.worldPosition);
    return float4(distanceToEye, 0.0f, 0.0f, 1.0f);
}
