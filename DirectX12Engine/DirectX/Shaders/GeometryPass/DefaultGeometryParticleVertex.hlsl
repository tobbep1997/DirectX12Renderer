
struct VS_INPUT
{
    float4 position : POSITION;
    float4 UV : TEXCORD;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 worldPos : WORLDPOS;
    float4 UV : TEXCORD;
};

cbuffer CAMERA_BUFFER : register(b0)
{
    float4 CameraPos;
    float4x4 ViewProjection;
}

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;

    output.position = mul(input.position, ViewProjection);
    output.worldPos = input.position;
    output.UV = input.UV;

    return output;
}