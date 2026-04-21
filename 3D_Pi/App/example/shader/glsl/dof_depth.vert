# version 410

layout(location = 0) in vec4 VertexPosition;
layout(location = 1) in vec3 VertexNormal;
layout(location = 2) in vec2 VertexUV;

layout(location = 0) out vec3 WorldPosition;
layout(location = 1) out vec2 UV;
out gl_PerVertex
{
    vec4 gl_Position;
};

layout(std140) uniform VSPerView
{
    mat4x4 g_worldToProjected;
};

layout(std140) uniform VSPerObject
{
    mat4x4 g_localToWorld;
};

layout(std140) uniform VSPerMaterial
{
    vec4 g_uvTransform;
};

void main()
{
    vec4 worldPosition = VertexPosition * g_localToWorld;
    gl_Position = worldPosition * g_worldToProjected;
    WorldPosition = worldPosition.xyz;
    UV = (VertexUV * g_uvTransform.xy + g_uvTransform.zw);
}
