// CRT post effect (GLSL) - minimal safe version
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
    vec4 g_param0;
};

vec2 BarrelDistort(vec2 uv, float curvature)
{
    vec2 cc = uv * 2.0 - 1.0;
    float r2 = dot(cc, cc);
    cc *= 1.0 + curvature * r2;
    return cc * 0.5 + 0.5;
}

void main()
{
    float curvature        = g_param0.x;
    float scanlineStrength = clamp(g_param0.y, 0.0, 1.0);
    float maskStrength     = clamp(g_param0.z, 0.0, 1.0);
    float vignette         = clamp(g_param0.w, 0.0, 1.0);

    vec2 uv = BarrelDistort(UV, curvature);

    float inside = step(0.0, uv.x) * step(uv.x, 1.0)
                 * step(0.0, uv.y) * step(uv.y, 1.0);

    vec2 sampleUV = clamp(uv, 0.0, 1.0);

    ivec2 size = textureSize(Texture0, 0);
    vec3 color = textureLod(Texture0, sampleUV, 0.0).rgb;

    float scan = sin(uv.y * float(size.y) * 3.14159265);
    scan = scan * scan;
    color *= mix(1.0, scan, scanlineStrength);

    float subF = floor(uv.x * float(size.x));
    float modF = mod(subF, 3.0);
    vec3 mask = vec3(0.55);
    if (modF < 0.5)      { mask = vec3(1.0, 0.55, 0.55); }
    else if (modF < 1.5) { mask = vec3(0.55, 1.0, 0.55); }
    else                 { mask = vec3(0.55, 0.55, 1.0); }
    color *= mix(vec3(1.0), mask, maskStrength);

    vec2 vc = uv * 2.0 - 1.0;
    float vd = dot(vc, vc);
    color *= mix(1.0, clamp(1.0 - vd * 0.7, 0.0, 1.0), vignette);

    color *= inside;

    FragColor = vec4(color, 1.0);
}
