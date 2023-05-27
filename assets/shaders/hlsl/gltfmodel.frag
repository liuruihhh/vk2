// Copyright 2020 Google LLC

Texture2D baseColorMap : register(t2);
SamplerState samplerBaseColorMap : register(s2);

struct VSOutput
{
[[vk::location(0)]] float3 Normal : NORMAL0;
[[vk::location(1)]] float4 Tangent : TANGENT0;
[[vk::location(2)]] float3 Color : COLOR0;
[[vk::location(3)]] float2 UV : TEXCOORD0;
[[vk::location(4)]] float3 ViewPos : TEXCOORD1;
[[vk::location(5)]] float3 LightPos : TEXCOORD2;
};


float4 main(VSOutput input) : SV_TARGET
{
	float4 color = baseColorMap.Sample(samplerBaseColorMap, input.UV) * float4(input.Color, 1.0);

	float3 N = normalize(input.Normal);
	float3 L = normalize(input.LightPos);
	float3 V = normalize(input.ViewPos);
	float3 R = reflect(L, N);
	float3 diffuse = max(dot(N, L), 0.0) * input.Color;
	float3 specular = pow(max(dot(R, V), 0.0), 16.0) * float3(0.75, 0.75, 0.75);
	return float4(diffuse * color.rgb + specular, 1.0);
}