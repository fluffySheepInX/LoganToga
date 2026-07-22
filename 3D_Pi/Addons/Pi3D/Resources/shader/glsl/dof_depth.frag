# version 410

uniform sampler2D Texture0;

layout(location = 0) in vec3 WorldPosition;
layout(location = 1) in vec2 UV;

layout(location = 0) out vec4 FragColor;

layout(std140) uniform PSPerView
{
    vec3 g_eyePosition;
};

layout(std140) uniform PSPerMaterial
{
    vec3  g_ambientColor;
    uint  g_hasTexture;
    vec4  g_diffuseColor;
    vec3  g_specularColor;
    float g_shininess;
    vec3  g_emissionColor;
};

void main()
{
    float alpha = g_diffuseColor.a;
    if (g_hasTexture == 1u)
    {
        alpha *= texture(Texture0, UV).a;
    }
    if (alpha < 0.1)
    {
        discard;
    }

    float distanceToEye = distance(g_eyePosition, WorldPosition);
    FragColor = vec4(distanceToEye, 0.0, 0.0, 1.0);
}
