struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 uv : TEXCORD;
};

cbuffer CAMERA_POSITION : register(b0)
{
    float4 CameraPosition;
}

SamplerState defualtSampler : register(s0);
SamplerComparisonState depthSampler : register(s1);

Texture2D positionTexture   : register(t0);
Texture2D albdeoTexture     : register(t1);
Texture2D normalTexture     : register(t2);
Texture2D metallicTexture   : register(t3);

#define ITERATION_DISTANCE = 0.01f
#define MAX_ITERATION = 10000

float4 main(VS_OUTPUT input) : SV_TARGET
{
	float4 worldPos = positionTexture.Sample(defualtSampler, input.uv.xy);
	float4 normal = normalTexture.Sample(defualtSampler, input.uv.xy);

	float4 InitialRayDirection = normalize(worldPos - CameraPosition);
	float4 RayDirection = normal * (InitialRayDirection * normal);


	


    return float4(abs(RayDirection.xyz), 1);
}