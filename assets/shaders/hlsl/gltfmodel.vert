// Copyright 2020 Google LLC

struct VSInput
{
[[vk::location(0)]] float3 Pos : POSITION0;
[[vk::location(1)]] float3 Normal : NORMAL0;
[[vk::location(2)]] float4 Tangent : TANGENT0;
[[vk::location(3)]] float2 UV : TEXCOORD0;
[[vk::location(4)]] float3 Color : COLOR0;
[[vk::location(5)]] float4 JointIndices : TEXCOORD1;
[[vk::location(6)]] float4 JointWeights : TEXCOORD2;
};

struct UBO
{
	float4x4 view;
	float4x4 proj;
	float3 ViewPos;
	float3 LightPos;
};

cbuffer ubo : register(b0) { UBO ubo; }

struct PushConsts {
	float4x4 model;
};
[[vk::push_constant]] PushConsts primitive;

StructuredBuffer<float4x4> jointMatrices : register(t1);

struct VSOutput
{
	float4 Pos : SV_POSITION;
[[vk::location(0)]] float3 Normal : NORMAL0;
[[vk::location(1)]] float4 Tangent : TANGENT0;
[[vk::location(2)]] float3 Color : COLOR0;
[[vk::location(3)]] float2 UV : TEXCOORD0;
[[vk::location(4)]] float3 ViewPos : TEXCOORD1;
[[vk::location(5)]] float3 LightPos : TEXCOORD2;
};

VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;
	output.Normal = input.Normal;
	output.Tangent = input.Tangent;
	output.UV = input.UV;
	output.Color = input.Color;
	output.ViewPos = ubo.ViewPos;
	output.LightPos = ubo.LightPos;
	
	float4x4 skinMat = input.JointWeights.x * jointMatrices[(uint)(input.JointIndices.x)]+
		input.JointWeights.y * jointMatrices[(uint)(input.JointIndices.y)]+
		input.JointWeights.z * jointMatrices[(uint)(input.JointIndices.z)]+
		input.JointWeights.w * jointMatrices[(uint)(input.JointIndices.w)];
		
	output.Pos = mul(ubo.proj, mul(ubo.view, mul(primitive.model, mul(skinMat, float4(input.Pos.xyz, 1.0)))));
	
	//output.Pos = mul(ubo.proj, mul(ubo.view, mul(primitive.model, float4(input.Pos.xyz, 1.0))));

	return output;
}