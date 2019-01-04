struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 uv : TEXCORD;
};

SamplerState defaultSampler : register(s0);
Texture2DArray geometryTexture : register(t0);

float4 main(VS_OUTPUT input) : SV_Target
{
    return geometryTexture.Sample(defaultSampler, float3(input.uv.xy, 0));
}