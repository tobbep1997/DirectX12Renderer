struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 worldPos : WORLDPOS;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float4 texCord : TEXCORD;
};


float4 main(VS_OUTPUT input) : SV_TARGET
{
    return input.normal;
}