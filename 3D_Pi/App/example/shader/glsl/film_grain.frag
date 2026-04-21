//-----------------------------------------------
//  film_grain.frag
//-----------------------------------------------

# version 410

uniform sampler2D Texture0;

layout(location = 0) in vec4 Color;
layout(location = 1) in vec2 UV;

layout(location = 0) out vec4 FragColor;

layout(std140) uniform PSConstants2D
{
    vec4 g_colorAdd;
    vec4 g_sdfParam;
    vec4 g_sdfOutlineColor;
    vec4 g_sdfShadowColor;
    vec4 g_internal;
};

layout(std140) uniform PSEffectParams
{
    vec4 g_params;
};

float hash21(vec2 p)
{
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

void main()
{
    vec3 col = texture(Texture0, UV).rgb;

    float n = hash21(UV * 1024.0 + g_params.x * 17.0) - 0.5;

    if (g_params.z > 0.5)
    {
        col += vec3(n) * g_params.y;
    }
    else
    {
        vec3 n3 = vec3(
            n,
            hash21(UV * 1024.0 + g_params.x * 31.0 + 7.0) - 0.5,
            hash21(UV * 1024.0 + g_params.x * 53.0 + 13.0) - 0.5
        );
        col += n3 * g_params.y;
    }

    FragColor = vec4(clamp(col, 0.0, 1.0), 1.0);
}
