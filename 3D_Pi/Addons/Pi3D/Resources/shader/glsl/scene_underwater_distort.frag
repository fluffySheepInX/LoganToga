# version 410

uniform sampler2D Texture0;
uniform sampler2D Texture1;

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

void main()
{
	vec2 uv = UV;
	float depth = texture(Texture1, uv).r;
	float time = g_params.y * g_params.z;
	float scale = max(g_params.w, 1.0);
	float depthFactor = (depth >= 99999.0) ? 0.55 : smoothstep(4.0, 90.0, depth);
	float waveX = sin((uv.y * scale + time) * 6.2831853);
	float waveY = cos((uv.x * (scale * 0.73) - time * 0.83) * 6.2831853);
	float ripple = sin(((uv.x + uv.y) * (scale * 0.41) + time * 0.57) * 6.2831853);
	vec2 offset = vec2(waveX + ripple * 0.45, waveY * 0.35) * g_params.x * mix(0.35, 1.0, depthFactor);
	vec2 sampleUV = clamp(uv + offset, vec2(0.001), vec2(0.999));

	FragColor = texture(Texture0, sampleUV);
}
