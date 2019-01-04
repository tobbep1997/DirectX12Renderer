
struct VS_INPUT
{
    float4 pos : POSITION;
    float4 uv : TEXCORD;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 uv : TEXCORD;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.pos = input.pos;
    output.uv = input.uv;
    return output;
}