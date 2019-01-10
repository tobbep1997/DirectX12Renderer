
struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 worldPos : WORLDPOS;
    float4 UV : TEXCORD;
};

struct PS_OUTPUT
{
    float4 worldPos : SV_target0;
    float4 albedo : SV_target1;
    float4 normal : SV_target2;
    float4 metallic : SV_target3;
};

SamplerState defaultSampler : register(s0);
Texture2DArray textureArray : register(t0);

PS_OUTPUT main(VS_OUTPUT input)
{
    PS_OUTPUT output = (PS_OUTPUT) 0;

    output.worldPos = input.worldPos;
    output.albedo = textureArray.Sample(defaultSampler, input.UV.xyz);
    output.normal = float4(0, 0, 0, 0);
    output.metallic = float4(0, 0, 0, 0);

    return output;
}