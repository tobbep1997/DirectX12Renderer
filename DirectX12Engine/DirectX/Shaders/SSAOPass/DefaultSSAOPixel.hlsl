#include "../ShaderIncludes/LightCalculations.hlsli"
struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 uv : TEXCORD;
};

cbuffer CAMERA
{
    float4x4 ViewProjection;
};

SamplerState defaultSampler : register(s0);
SamplerComparisonState comparisonSampler : register(s1);

Texture2D worldPos : register(t0);
Texture2D depthStencil : register(t1);

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float4 position = worldPos.Sample(defaultSampler, input.uv.xy);

    float4 viewPosition = mul(position, ViewProjection);
    float depth = viewPosition.z / viewPosition.w;

    float2 smTex;
    float2 baseUV = input.uv.xy;
    float2 texelSize = TexelSize2(depthStencil);

    float currentAO = 0;
    float divider = 1;

    int ssaoSize = 10;

    for (int x = -ssaoSize; x <= ssaoSize; ++x)
    {
        for (int y = -ssaoSize; y <= ssaoSize; ++y)
        {
            smTex = baseUV + (float2(x, y) * texelSize);
            currentAO += depthStencil.SampleCmpLevelZero(comparisonSampler, smTex, depth).r;
            divider += 1.0f;
        }
    }
    currentAO /= divider;

    return float4(currentAO, 0, 0, 1);
}