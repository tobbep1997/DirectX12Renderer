
struct HS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 worldPos : WORLDPOS;
    float4 normal : NORMAL;
    float3x3 TBN : TBN;
    float4 texCord : TEXCORD;
	uint4 textureIndex : TEXTURE_INDEX;
};

struct PS_OUTPUT
{
    float4 worldPos : SV_target0;
    float4 albedo   : SV_target1;
    float4 normal   : SV_target2;
    float4 metallic : SV_target3;
};                             


SamplerState defaultSampler : register(s0);
Texture2D BindlessMap[] : register(t0);

PS_OUTPUT main(HS_OUTPUT input)
{
    PS_OUTPUT output = (PS_OUTPUT) 0;

	float4 albedo = BindlessMap[input.textureIndex.x + 0].Sample(defaultSampler, input.texCord.xy);
	float4 normal = float4(normalize(input.normal.xyz + mul((2.0f * BindlessMap[input.textureIndex.x + 1].Sample(defaultSampler, input.texCord.xy).xyz - 1.0f), input.TBN)), 0);
	float4 metallic = BindlessMap[input.textureIndex.x + 2].Sample(defaultSampler, input.texCord.xy);


    output.worldPos = input.worldPos;
    output.albedo = albedo;
    output.normal = normal;
    output.metallic = metallic;
    return output;
}