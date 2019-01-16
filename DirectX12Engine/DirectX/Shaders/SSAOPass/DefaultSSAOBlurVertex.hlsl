
struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 UV : TEXCORD;
};

struct VS_INPUT
{
    float4 position : POSITION;
    float4 UV : TEXCORD;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    
    output.position = input.position;
    output.UV = input.UV;

    return output;
}