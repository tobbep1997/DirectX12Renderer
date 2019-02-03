struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 uv : TEXCORD;
};

cbuffer CAMERA_POSITION : register(b0)
{
    float4 CameraPosition;
}

float4 main(VS_OUTPUT input) : SV_TARGET
{
    return float4(1, 1, 1, 1);
}