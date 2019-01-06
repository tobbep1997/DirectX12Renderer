struct VS_INPUT
{
    float4 pos : POSITION;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float4 texCord : TEXCORD;
};

cbuffer CAMERA_BUFFER : register(b0)
{
    float4x4 WorldMatrix;
}
cbuffer LIGHT_BUFFER : register(b1)
{
    uint4 LightType;
    float4x4 ViewProjection[6];
}

float4 main(VS_INPUT input) : POSITION
{
    return mul(input.pos, WorldMatrix);
}