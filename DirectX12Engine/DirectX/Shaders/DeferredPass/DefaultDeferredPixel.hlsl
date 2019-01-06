#include "../ShaderIncludes/LightCalculations.hlsli"
struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 uv : TEXCORD;
};

cbuffer LIGHT_BUFFER : register(b0)
{
    float4 CameraPosition;
    uint4 LightType[256];
    float4 LightPosition[256];
    float4 LightColor[256];
    float4 LightVector[256];
};

cbuffer SHADOW_BUFFER : register(b0, space1)
{
    int4 values;
    float4x4 ShadowViewProjection[16];
}

Texture2DArray shadowMap : register(t0, space1);

SamplerState defaultSampler : register(s0);
SamplerComparisonState shadowSampler : register(s0, space1);

Texture2D positionTexture   : register(t0);
Texture2D albdeoTexture     : register(t1);
Texture2D normalTexture     : register(t2);
Texture2D metallicTexture   : register(t3);

float4 main(VS_OUTPUT input) : SV_Target
{
    float4 worldPos     = positionTexture.Sample(defaultSampler, input.uv.xy);
    float4 albedo       = albdeoTexture.Sample  (defaultSampler, input.uv.xy);
    float4 normal       = normalTexture.Sample  (defaultSampler, input.uv.xy);
    float4 metallic     = metallicTexture.Sample(defaultSampler, input.uv.xy);
    float4 ambient = float4(0.1f, 0.1f, 0.1f, 1.0f) * albedo;
    float4 specular = float4(0, 0, 0, 1.0f);
    float shadowCoeff = 1.0f;

    float4 finalColor = LightCalculation(LightType, 
                                    LightPosition, 
                                    LightColor, 
                                    LightVector, 
                                    CameraPosition, 
                                    worldPos, 
                                    albedo, 
                                    normal, 
                                    metallic, 
                                    specular);
    int divider = 1;
    for (int i = 0; i < values.x; i++)
    {
        divider += ShadowCalculations(shadowMap, 
            i, 
            shadowSampler, 
            TexelSize(shadowMap), 
            FragmentLightPos(worldPos, ShadowViewProjection[i]), 
            shadowCoeff,
            1);
    }
    shadowCoeff /= divider;
    return saturate((finalColor + specular) * shadowCoeff + ambient);;
}