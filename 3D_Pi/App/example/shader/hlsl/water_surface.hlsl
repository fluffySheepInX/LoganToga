//	水面シェーダー（深度着色 + 海岸線フォーム）
//	h(x, z) は SeaTerrain.hpp の HeightAt と同一式を保つこと。

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

cbuffer PSWater : register(b2)
{
	//	x: seaLevel, y: shoreX, z: beachWidth, w: seaBasinDepth
	float4 g_waterTerrain;

	//	x: landHillHeight, y: fadeDepth, z: time, w: baseAlpha
	float4 g_waterParams;

	//	rgb: 浅瀬色(linear), a: フォーム強度
	float4 g_waterShallowColor;

	//	rgb: 深海色(linear), a: 予備
	float4 g_waterDeepColor;
}

//	SeaTerrain::HeightAt と同一の高さ場
float TerrainHeight(float x, float z)
{
	const float shoreX = g_waterTerrain.y;
	const float beachWidth = g_waterTerrain.z;
	const float basinDepth = g_waterTerrain.w;
	const float hillHeight = g_waterParams.x;

	const float hills = hillHeight * sin(x * 0.055) * cos(z * 0.047);
	const float basin = smoothstep(shoreX, shoreX + beachWidth, x);
	const float seabedRipple = 0.35 * sin(x * 0.11 + z * 0.07) * basin;

	return (hills * (1.0 - basin)) - (basinDepth * basin) + seabedRipple;
}

float4 PS(s3d::PSInput input) : SV_TARGET
{
	const float seaLevel = g_waterTerrain.x;
	const float fadeDepth = max(g_waterParams.y, 0.01);
	const float time = g_waterParams.z;
	const float baseAlpha = g_waterParams.w;

	const float x = input.worldPosition.x;
	const float z = input.worldPosition.z;

	//	水深
	const float depth = max(0.0, seaLevel - TerrainHeight(x, z));

	//	陸に食い込んだ水面は描かない
	clip(depth - 0.001);

	//	深度グラデーション（浅瀬 → 深海）
	const float depthT = saturate(depth / fadeDepth);
	float3 color = lerp(g_waterShallowColor.rgb, g_waterDeepColor.rgb, depthT);

	//	海岸線フォーム: 浅い帯に時間で揺れる白い縁
	const float foamBand = 1.0 - saturate(depth / 0.55);
	const float foamWave = 0.5 + 0.5 * sin((depth * 22.0) - (time * 1.6) + (sin(z * 0.35) * 2.0));
	const float foam = g_waterShallowColor.a * foamBand * foamWave;
	color = lerp(color, float3(0.92, 0.96, 0.98), saturate(foam));

	//	微細な表面きらめき
	const float sparkle = 0.04 * sin(x * 1.7 + time * 0.9) * sin(z * 1.9 - time * 1.1);
	color += sparkle;

	//	深いほど不透明に
	const float alpha = saturate(baseAlpha + 0.30 * depthT + 0.25 * saturate(foam));

	return float4(color, alpha);
}
