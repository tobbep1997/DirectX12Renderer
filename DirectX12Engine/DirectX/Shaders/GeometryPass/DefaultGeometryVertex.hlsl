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
    float4 m_cameraPos;
}

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.pos = input.pos;
    output.color = input.color * m_cameraPos;
    return output;
}