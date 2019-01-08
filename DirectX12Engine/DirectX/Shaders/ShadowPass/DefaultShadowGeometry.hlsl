struct GSOutput
{
	float4 pos : SV_POSITION;
    uint RTIndex : SV_RenderTargetArrayIndex;
};

#define MAX_RENDERTARGETS 6

cbuffer LIGHT_BUFFER : register(b0)
{
    uint4 LightType;
    float4x4 ViewProjection[6];
}

[maxvertexcount(MAX_RENDERTARGETS * 3)]
void main(
	triangle float4 input[3] : POSITION, 
	inout TriangleStream<GSOutput> output
)
{
    uint index = 1;
    if (LightType.x == 0)
        index = 6;
    else if (LightType.x == 1)
        index = 1;

    for (uint i = 0; i < index; i++)
	{
        for (uint j = 0; j < 3; j++)
        {
            GSOutput element = (GSOutput) 0;
            element.pos = mul(input[j], ViewProjection[i]);
            element.RTIndex = i;
		    output.Append(element);
        }
        output.RestartStrip();
    }
}