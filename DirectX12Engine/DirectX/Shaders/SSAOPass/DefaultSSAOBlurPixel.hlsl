#include "../ShaderIncludes/LightCalculations.hlsli"
struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 UV : TEXCORD;
};

SamplerState defaultSampler : register(s0);
Texture2D ssaoTexture : register(t0);

RWTexture2D<float4> blurOutput : register(u1);

float multiplier(uint2 index)
{
    float3x3 table =
    {
        1.0f,   4.0f,     1.0f,
        4.0f,   10.0f,    4.0f,
        1.0f,   4.0f,     1.0f
    };

    return table[index.x][index.y];
}

float multiplier(int2 pos, int radius, float centerStregth = 4.0f)
{
    float l = length(float2(pos.x, pos.y));
    return pow(radius - l, centerStregth);
}

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float finalColor = 0;
    float2 texelSize = TexelSize2(ssaoTexture);
    int sampleRadius = 1;

    //return float4(ssaoTexture.Sample(defaultSampler, input.UV.xy).r, 0, 0, 1);

    float2 smTex;
    float divider = 1.0f;
    for (int x = -sampleRadius; x <= sampleRadius; ++x)
    {
        for (int y = -sampleRadius; y <= sampleRadius; ++y)
        {
            smTex = input.UV.xy + (float2(x, y) * texelSize);
            uint2 index = ((float2(x, y) * 0.5) + 0.5) * 2;
            finalColor += ssaoTexture.Sample(defaultSampler, smTex).r;
            divider += 1.0f;
        }
    }
    finalColor /= divider;
    return float4(finalColor, 0, 0, 1);
}

