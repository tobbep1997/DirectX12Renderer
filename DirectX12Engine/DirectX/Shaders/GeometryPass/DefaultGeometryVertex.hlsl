struct VS_INPUT
{
    float4 pos : POSITION;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float4 texCord : TEXCORD;
};


struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 worldPos : WORLDPOS;
    float4 normal : NORMAL;
    float3x3 TBN : TBN;
    float4 texCord : TEXCORD;
};

cbuffer CAMERA_BUFFER : register(b0)
{
    float4 CameraPos;
    float4x4 WorldMatrix;
    float4x4 ViewProjection;
    
    float4 Padding[40];
}

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.pos = mul(input.pos, mul(WorldMatrix, ViewProjection));
    output.worldPos = mul(input.pos, WorldMatrix);
    output.normal = normalize(mul(input.normal, WorldMatrix));

    float3 tangent = normalize(mul(input.tangent, WorldMatrix).xyz);
    tangent = normalize(tangent - dot(tangent, output.normal.xyz) * output.normal.xyz).xyz;
    float3 bitangent = cross(output.normal.xyz, tangent);
    float3x3 TBN = float3x3(tangent, bitangent, output.normal.xyz);
    output.TBN = TBN;

    output.texCord = input.texCord;
    return output;
}