struct VS_INPUT
{
    float4 pos : POSITION;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float4 texCord : TEXCORD;

    float4x4 worldMatrix : WORLD;
	uint4 textureIndex : TEXTURE_INDEX;
};


struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 worldPos : WORLDPOS;
    float4 normal : NORMAL;
    float3x3 TBN : TBN;
    float4 texCord : TEXCORD;
	uint4 textureIndex : TEXTURE_INDEX;

    float tessFactor : TESSFACTOR;
};

cbuffer CAMERA_BUFFER : register(b0)
{
    float4 CameraPos;
    float4x4 ViewProjection;
}

#define MAX_TESS 128
#define MIN_TESS 128
#define MIN_TESS_DIST 2
#define MAX_TESS_DIST 10

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.pos = mul(input.pos, mul(input.worldMatrix, ViewProjection));
    output.worldPos = mul(input.pos, input.worldMatrix);
    output.normal = normalize(mul(input.normal, input.worldMatrix));

    float3 tangent = normalize(mul(input.tangent, input.worldMatrix).xyz);
    tangent = normalize(tangent - dot(tangent, output.normal.xyz) * output.normal.xyz).xyz;
    float3 bitangent = cross(output.normal.xyz, tangent);
    float3x3 TBN = float3x3(tangent, bitangent, output.normal.xyz);

    output.TBN = TBN;
    output.texCord = input.texCord;

    float distanceToCamera = length(CameraPos - output.worldPos);

    float tess = saturate((MIN_TESS_DIST - distanceToCamera) / (MIN_TESS_DIST - MAX_TESS_DIST));
    output.tessFactor = tess * (MAX_TESS - MIN_TESS);
    output.tessFactor = MAX_TESS - output.tessFactor;
	output.textureIndex = input.textureIndex;

    return output;
}