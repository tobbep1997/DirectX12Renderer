struct VS_INPUT
{
    float4 pos : POSITION;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float4 texCord : TEXCORD;
};

float4 main(VS_INPUT input) : SV_POSITION
{
    return input.pos;
}