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
    float4 tangent : TANGENT;
    float4 texCord : TEXCORD;
};

cbuffer CAMERA_BUFFER : register(b0)
{
    float4 CameraPos;
    float4x4 CameraViewProjection;
    float4x4 WorldViewProjection;
    
    float4 Padding[40];
}

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.pos = mul(input.pos, CameraViewProjection);
    
    output.normal = input.normal;
    return output;
}