//	Copyright (c) 2008-2025 Ryo Suzuki.
//	Copyright (c) 2016-2025 OpenSiv3D Project.
//	Licensed under the MIT License.

# version 410

//
//	Textures
//
uniform sampler2D Texture0;

//
//	PSInput
//
layout(location = 0) in vec3 WorldPosition;
layout(location = 1) in vec2 UV;
layout(location = 2) in vec3 Normal;

//
//	PSOutput
//
layout(location = 0) out vec4 FragColor;

//
//	Constant Buffer
//
layout(std140) uniform PSPerFrame // slot 0
{
	vec3 g_globalAmbientColor;
	vec3 g_sunColor;
	vec3 g_sunDirection;
};

layout(std140) uniform PSPerView // slot 1
{
	vec3 g_eyePosition;
};

layout(std140) uniform PSKicker // slot 2
{
	vec4 g_kickerDirectionIntensity;
	vec4 g_kickerColorEnable;
};

layout(std140) uniform PSPerMaterial // slot 3
{
	vec3  g_ambientColor;
	uint  g_hasTexture;
	vec4  g_diffuseColor;
	vec3  g_specularColor;
	float g_shininess;
	vec3  g_emissionColor;
};

//
//	Functions
//
vec4 GetDiffuseColor(vec2 uv)
{
	vec4 diffuseColor = g_diffuseColor;

	if (g_hasTexture == 1)
	{
		diffuseColor *= texture(Texture0, uv);
	}

	return diffuseColor;
}

vec3 CalculateDiffuseReflection(vec3 n, vec3 l, vec3 lightColor, vec3 diffuseColor, vec3 ambientColor)
{
	vec3 directColor = lightColor * max(dot(n, l), 0.0f);
	return ((ambientColor + directColor) * diffuseColor);
}

vec3 CalculateSpecularReflection(vec3 n, vec3 h, float shininess, float nl, vec3 lightColor, vec3 specularColor)
{
	float highlight = pow(max(dot(n, h), 0.0f), shininess) * float(0.0f < nl);
	return (lightColor * specularColor * highlight);
}

void main()
{
	vec3 lightColor		= g_sunColor;
	vec3 lightDirection	= g_sunDirection;

	vec3 n = normalize(Normal);
	vec3 l = lightDirection;
	vec4 diffuseColor = GetDiffuseColor(UV);
	vec3 ambientColor = (g_ambientColor * g_globalAmbientColor);

	// Diffuse
	vec3 diffuseReflection = CalculateDiffuseReflection(n, l, lightColor, diffuseColor.rgb, ambientColor);

	// Specular
	vec3 v = normalize(g_eyePosition - WorldPosition);
	vec3 h = normalize(v + lightDirection);
	vec3 specularReflection = CalculateSpecularReflection(n, h, g_shininess, dot(n, l), lightColor, g_specularColor);

	// Kicker
	float kickerEnabled = g_kickerColorEnable.w;
	vec3 kickerDirection = normalize(g_kickerDirectionIntensity.xyz);
	float kickerIntensity = max(g_kickerDirectionIntensity.w, 0.0);
	vec3 kickerColor = g_kickerColorEnable.rgb;
    float kickerDot = dot(n, kickerDirection);
	float kickerNL = max(kickerDot, 0.0f);
	float rim = pow(1.0f - max(dot(n, v), 0.0f), 1.5f);
	float wrappedKicker = clamp((kickerDot + 0.35f) / 1.35f, 0.0f, 1.0f) * rim * 0.85f;
	float kickerShape = max(kickerNL, wrappedKicker);
	vec3 kickerDiffuse = (kickerColor * kickerIntensity * kickerShape) * diffuseColor.rgb;
	vec3 kickerH = normalize(v + kickerDirection);
  vec3 kickerSpecular = CalculateSpecularReflection(n, kickerH, g_shininess, kickerDot, (kickerColor * kickerIntensity), g_specularColor);
	vec3 kickerReflection = (kickerDiffuse + kickerSpecular) * kickerEnabled;

 FragColor = vec4(diffuseReflection + specularReflection + kickerReflection + g_emissionColor, diffuseColor.a);
}
