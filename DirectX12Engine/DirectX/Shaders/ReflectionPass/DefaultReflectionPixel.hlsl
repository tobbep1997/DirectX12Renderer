#include "../ShaderIncludes/LightCalculations.hlsli"
struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 uv : TEXCORD;
};

cbuffer CAMERA_POSITION : register(b0)
{
    float4 CameraPosition;
	float4x4 ViewProjection;
}

SamplerState defualtSampler : register(s0);
SamplerComparisonState depthSampler : register(s1);

Texture2D positionTexture   : register(t0);
Texture2D albdeoTexture     : register(t1);
Texture2D normalTexture     : register(t2);
Texture2D metallicTexture   : register(t3);
Texture2D depthTexture   : register(t4);



float4 main(VS_OUTPUT input) : SV_TARGET
{

	uint MAX_ITERATION = 100;
	float ITERATION_DISTANCE = 0.01f;

	float4 worldPos = positionTexture.Sample(defualtSampler, input.uv.xy);
	float4 normal = normalTexture.Sample(defualtSampler, input.uv.xy);
	float4 metallic = metallicTexture.Sample(defualtSampler, input.uv.xy);

	float4 InitialRayDirection = worldPos - CameraPosition;
	float4 RayDirection = normal * (InitialRayDirection * normal);

	float4 color = float4(0,0,0,0);
	if (length(metallic) < .5f)
		return color;

	
	float4 pos;
	for (uint i = 0; i < MAX_ITERATION; i++)
	{
		float4 ray = RayDirection * i * ITERATION_DISTANCE;
		pos = mul(worldPos + ray, ViewProjection);

		pos /= pos.w;
		float depth = FragmentLightDepth(pos);
		float2 uv = FragmentLightUV(pos);

		if (depthTexture.Sample(defualtSampler, uv).r < depth)
		{
			color = albdeoTexture.Sample(defualtSampler, uv);
			break;
		}
	}

    return color * metallic;
}