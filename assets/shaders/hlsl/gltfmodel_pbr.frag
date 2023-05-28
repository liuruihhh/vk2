// Copyright 2020 Google LLC

Texture2D baseColorMap : register(t2);
SamplerState samplerBaseColorMap : register(s2);
Texture2D metallicRoughnessMap : register(t3);
SamplerState samplerMetallicRoughnessMap : register(s3);
Texture2D normalMap : register(t4);
SamplerState samplerNormalMap : register(s4);

struct PushConsts {
	float4x4 model;
	float4 baseColorFactor;
};
[[vk::push_constant]] PushConsts primitive;

struct VSOutput
{
[[vk::location(0)]] float3 Normal : NORMAL0;
[[vk::location(1)]] float3 Tangent : TANGENT0;
[[vk::location(2)]] float3 Color : COLOR0;
[[vk::location(3)]] float2 UV : TEXCOORD0;
[[vk::location(4)]] float3 ViewDir : TEXCOORD1;
[[vk::location(5)]] float3 LightDir : TEXCOORD2;
[[vk::location(6)]] float3x3 M_TBN : TEXCOORD3;
};

#define PI 3.1415926535897932384626433832795

float3 albedo(float2 uv){
 	return pow(baseColorMap.Sample(samplerBaseColorMap, uv).rgb, float3(2.2, 2.2, 2.2));
}

// Normal Distribution function
float D_GGX(float dotNH, float roughness)
{
	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;
	float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
	return (alpha2)/(PI * denom*denom);
}

// Geometric Shadowing function
float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;
	float GL = dotNL / (dotNL * (1.0 - k) + k);
	float GV = dotNV / (dotNV * (1.0 - k) + k);
	return GL * GV;
}

// Fresnel function
float3 F_Schlick(float cosTheta, float3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
float3 F_SchlickR(float cosTheta, float3 F0, float roughness)
{
	return F0 + (max((1.0 - roughness).xxx, F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 specularContribution(float2 inUV, float3 L, float3 V, float3 N, float3 F0, float metallic, float roughness)
{
	float3 H = normalize (V + L);
	float dotNH = clamp(dot(N, H), 0.0, 1.0);
	float dotNV = clamp(dot(N, V), 0.0, 1.0);
	float dotNL = clamp(dot(N, L), 0.0, 1.0);
	float3 lightColor = float3(4.0, 4.0, 4.0);
	float3 color = float3(0.0, 0.0, 0.0);
	if (dotNL > 0.0) {
		float D = D_GGX(dotNH, roughness);
		float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
		float3 F = F_Schlick(dotNV, F0);
		float3 spec = D * F * G / (4.0 * dotNL * dotNV + 0.001);
		float3 kD = (float3(1.0, 1.0, 1.0) - F) * (1.0 - metallic);
		color += (kD * albedo(inUV) / PI + spec) * dotNL * lightColor;
	}
	return color;
}

float3 calculateNormal(VSOutput input)
{
	float3 tangentNormal = normalMap.Sample(samplerNormalMap, input.UV).xyz * 2.0 - 1.0;
	return normalize(mul(tangentNormal, input.M_TBN));
}

float3 Uncharted2Tonemap(float3 color)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	float W = 11.2;
	return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}

float4 main(VSOutput input) : SV_TARGET
{
	float3 N = calculateNormal(input);
	float3 L = normalize(input.LightDir);
	float3 V = normalize(input.ViewDir);
	
	float4 baseColor = baseColorMap.Sample(samplerBaseColorMap, input.UV)*primitive.baseColorFactor;

	float4 metallicRoughness = metallicRoughnessMap.Sample(samplerMetallicRoughnessMap, input.UV);
	float metallic = metallicRoughness.b;
	float roughness = metallicRoughness.g;
	
	float3 F0 = float3(0.04, 0.04, 0.04);
	F0 = lerp(F0, albedo(input.UV), metallic);
	
	float3 Lo = specularContribution(input.UV, L, V, N, F0, metallic, roughness);
	
	float3 diffuse = albedo(input.UV);
	float3 F = F_SchlickR(max(dot(N, V), 0.0), F0, roughness);
	
	float3 kD = 1.0 - F;
	kD *= 1.0 - metallic;
	
	float3 ambient = (kD * diffuse);
	
	float3 color = ambient + Lo;
	
	float exposure = 5.0;

	color = Uncharted2Tonemap(color * exposure);
	color = color * (1.0 / Uncharted2Tonemap((11.2).xxx));
	
	return float4(color, baseColor.a);
}