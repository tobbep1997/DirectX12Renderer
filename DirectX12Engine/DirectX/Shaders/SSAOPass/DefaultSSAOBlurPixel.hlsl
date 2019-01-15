
struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 UV : TEXCORD;
};

SamplerState defaultSampler : register(s0);
Texture2D ssaoTexture : register(t0);

RWTexture2D<float4> blurOutput : register(u1);

float4 main(VS_OUTPUT input) : SV_TARGET
{
    return float4(ssaoTexture.Sample(defaultSampler, input.UV.xy).r, 0, 0, 1);
}

