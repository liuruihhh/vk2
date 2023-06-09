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

float3 calculateNormal(VSOutput input)
{
	float3 tangentNormal = normalMap.Sample(samplerNormalMap, input.UV).xyz * 2.0 - 1.0;
	return normalize(mul(tangentNormal, input.M_TBN));
}

float4 main(VSOutput input) : SV_TARGET
{
	float4 color = baseColorMap.Sample(samplerBaseColorMap, input.UV) * primitive.baseColorFactor;

	float3 N = calculateNormal(input);
	float3 L = normalize(input.LightDir);
	float3 V = normalize(input.ViewDir);
	float3 R = reflect(L, N);
	float3 diffuse = max(dot(N, L), 0.0) * input.Color;
	float3 specular = pow(max(dot(R, V), 0.0), 16.0) * float3(0.75, 0.75, 0.75);
	return float4(diffuse * color.rgb + specular, 1.0);
}