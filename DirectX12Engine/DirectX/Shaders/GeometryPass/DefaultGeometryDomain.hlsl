struct DS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 worldPos : WORLDPOS;
    float4 normal : NORMAL;
    float3x3 TBN : TBN;
    float4 texCord : TEXCORD;
	uint4 textureIndex : TEXTURE_INDEX;
};

// Output control point
struct HS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 worldPos : WORLDPOS;
    float4 normal : NORMAL;
    float3x3 TBN : TBN;
    float4 texCord : TEXCORD;
	uint4 textureIndex : TEXTURE_INDEX;
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]			: SV_TessFactor; // e.g. would be [4] for a quad domain
	float InsideTessFactor			: SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
	// TODO: change/add other stuff
};

#define NUM_CONTROL_POINTS 3

cbuffer CAMERA_BUFFER : register(b0)
{
    float4 CameraPos;
    float4x4 ViewProjection;
}

SamplerState defaultSampler : register(s0);

Texture2D BindlessMap[] : register(t0);

[domain("tri")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float3 domain : SV_DomainLocation,
	const OutputPatch<HS_OUTPUT, NUM_CONTROL_POINTS> patch)
{
    DS_OUTPUT output = (DS_OUTPUT)0;

    output.worldPos = domain.x * patch[0].worldPos + 
        domain.y * patch[1].worldPos + 
        domain.z * patch[2].worldPos;
    output.worldPos.w = 1;
    
    output.normal = domain.x * patch[0].normal +
        domain.y * patch[1].normal +
        domain.z * patch[2].normal;
    output.normal = normalize(output.normal);

    output.texCord = domain.x * patch[0].texCord +
        domain.y * patch[1].texCord +
        domain.z * patch[2].texCord;
    output.texCord.z = 0;
    output.texCord.w = 0;

    output.TBN = patch[0].TBN;
	output.textureIndex = patch[0].textureIndex;

	float height = length(BindlessMap[patch[0].textureIndex.x + 3].SampleLevel(defaultSampler, output.texCord.xy, 0).rgb);
    height = clamp(height, 0.0f, 1.0f);

    float4 normal = float4(normalize(output.normal.xyz + mul((2.0f * BindlessMap[patch[0].textureIndex.x + 1].SampleLevel(defaultSampler, output.texCord.xy, 0).xyz - 1.0f), output.TBN)), 0);

    output.worldPos += (0.05f * (height - 1.0f)) * normal;

    output.pos = mul(output.worldPos, ViewProjection);
	return output;
}

