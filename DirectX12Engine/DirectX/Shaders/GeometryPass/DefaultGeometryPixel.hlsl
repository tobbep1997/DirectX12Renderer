#include "../ShaderIncludes/LightCalculations.hlsli"

struct HS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 worldPos : WORLDPOS;
    float4 normal : NORMAL;
    float3x3 TBN : TBN;
    float4 texCord : TEXCORD;
};

struct PS_OUTPUT
{
    float4 worldPos : SV_target0;
    float4 albedo   : SV_target1;
    float4 normal   : SV_target2;
    float4 metallic : SV_target3;
};                             



Texture2D albedo : register(t0);
Texture2D normalmap : register(t1);
Texture2D metallicMap : register(t2);

Texture2D shadowMap : register(t0, space1);

SamplerState defaultSampler : register(s0);
SamplerComparisonState shadowSampler : register(s0, space1);


PS_OUTPUT main(HS_OUTPUT input)
{
    PS_OUTPUT output = (PS_OUTPUT) 0;

    float4 texColor = albedo.Sample(defaultSampler, input.texCord.xy);
    float4 normal = float4(normalize(input.normal.xyz + mul((2.0f * normalmap.Sample(defaultSampler, input.texCord.xy).xyz - 1.0f), input.TBN)), 0);
    float4 metallic = metallicMap.Sample(defaultSampler, input.texCord.xy);
        
    output.worldPos = input.worldPos;
    output.albedo = texColor;
    output.normal = normal;
    output.metallic = metallic;
    return output;
}