struct VS_INPUT
{
    float4 pos : POSITION;
    float4 color : COLOR;
};


struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

cbuffer CAMERA_BUFFER : register(b0)
{
    float4 CameraPos;
    float4x4 CameraViewProjection;

    float4 Padding[44];
}

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.pos = mul(input.pos, CameraViewProjection);
    output.color = input.color;
    return output;
}