//
//	Kuwahara filter (GLSL)
//

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

// ユーザー領域 (b1)
//   x = radius (1..6)
layout(std140) uniform PSEffectParams
{
    vec4 g_param0;
};

float Luma(vec3 c)
{
    return dot(c, vec3(0.299, 0.587, 0.114));
}

void main()
{
    int radius = int(clamp(g_param0.x, 1.0, 6.0));

    vec2 texel = 1.0 / vec2(textureSize(Texture0, 0));

    ivec2 offsets[4] = ivec2[4](
        ivec2( 0,  0),
        ivec2(-radius,  0),
        ivec2( 0, -radius),
        ivec2(-radius, -radius)
    );

    vec3 bestMean = vec3(0.0);
    float bestVar = 1e20;

    for (int k = 0; k < 4; ++k)
    {
        vec3  sumC  = vec3(0.0);
        float sumL  = 0.0;
        float sumL2 = 0.0;
        float n     = 0.0;

        for (int j = 0; j <= radius; ++j)
        {
            for (int i = 0; i <= radius; ++i)
            {
                vec2 uv = UV + vec2(offsets[k] + ivec2(i, j)) * texel;
                vec3 c = texture(Texture0, uv).rgb;
                float l = Luma(c);
                sumC  += c;
                sumL  += l;
                sumL2 += l * l;
                n     += 1.0;
            }
        }

        vec3 mean = sumC / n;
        float meanL = sumL / n;
        float variance = (sumL2 / n) - meanL * meanL;

        if (variance < bestVar)
        {
            bestVar  = variance;
            bestMean = mean;
        }
    }

    FragColor = vec4(bestMean, 1.0);
}
