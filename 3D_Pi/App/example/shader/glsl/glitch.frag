//-----------------------------------------------
//  glitch.frag
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

float stripeMask(float y, float time, float blockiness)
{
    float rows = mix(18.0, 96.0, clamp(blockiness, 0.0, 1.0));
    float row = floor(y * rows);
    float n = hash21(vec2(row, floor(time * 18.0)));
    return smoothstep(0.60, 0.98, n);
}

void main()
{
    float intensity = clamp(g_params.y, 0.0, 1.0);
    float blockiness = clamp(g_params.z, 0.0, 1.0);
    float rgbShift = g_params.w * intensity;
    float time = g_params.x;

    vec2 uv = UV;
    float stripe = stripeMask(uv.y, time, blockiness) * intensity;
    float jitter = (hash21(vec2(floor(uv.y * mix(24.0, 140.0, blockiness)), floor(time * 24.0))) - 0.5)
        * mix(0.01, 0.08, blockiness) * stripe;

    float tear = step(0.985, hash21(vec2(floor(time * 12.0), floor(uv.y * 8.0)))) * intensity;
    uv.x += jitter + tear * 0.06 * sin(time * 80.0 + uv.y * 80.0);

    vec2 blockScale = vec2(80.0, mix(32.0, 120.0, blockiness));
    vec2 blockUv = floor(uv * blockScale) / blockScale;
    float blockNoise = hash21(blockUv + vec2(floor(time * 20.0)));
    float blockMask = smoothstep(0.86, 1.0, blockNoise) * intensity * blockiness;

    float scanNoise = hash21(vec2(floor(uv.y * 360.0), floor(time * 60.0))) - 0.5;
    uv.x += scanNoise * 0.006 * intensity;

    float r = texture(Texture0, uv + vec2(-rgbShift - blockMask * 0.025, 0.0)).r;
    float g = texture(Texture0, uv + vec2(jitter * 0.35, 0.0)).g;
    float b = texture(Texture0, uv + vec2(rgbShift + blockMask * 0.025, 0.0)).b;
    float a = texture(Texture0, uv).a;

    vec3 col = vec3(r, g, b);
    float digitalNoise = (hash21(UV * 900.0 + vec2(time * 41.0)) - 0.5) * 0.22 * intensity;
    col += digitalNoise;
    col = mix(col, col * vec3(0.55, 1.25, 1.45), blockMask * 0.7);

    FragColor = vec4(clamp(col, 0.0, 1.0), a) * Color + g_colorAdd;
}
