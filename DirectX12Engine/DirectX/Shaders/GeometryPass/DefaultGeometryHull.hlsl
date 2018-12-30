struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 worldPos : WORLDPOS;
    float4 normal : NORMAL;
    float3x3 TBN : TBN;
    float4 texCord : TEXCORD;

    float tessFactor : TESSFACTOR;
};

struct HS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 worldPos : WORLDPOS;
    float4 normal : NORMAL;
    float3x3 TBN : TBN;
    float4 texCord : TEXCORD;
};

struct HS_CONSTANT_DATA_OUTPUT
{
    float EdgeTessFactor[3] : SV_TessFactor; // e.g. would be [4] for a quad domain
    float InsideTessFactor : SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
	// TODO: change/add other stuff
};

HS_CONSTANT_DATA_OUTPUT PatchHS(InputPatch<VS_OUTPUT, 3> patch,
                  uint patchID : SV_PrimitiveID)
{
    HS_CONSTANT_DATA_OUTPUT pt = (HS_CONSTANT_DATA_OUTPUT) 0;
    
    // Average tess factors along edges, and pick an edge tess factor for 
    // the interior tessellation.  It is important to do the tess factor
    // calculation based on the edge properties so that edges shared by 
    // more than one triangle will have the same tessellation factor.  
    // Otherwise, gaps can appear.
    pt.EdgeTessFactor[0] = 0.5f * (patch[1].tessFactor + patch[2].tessFactor);
    pt.EdgeTessFactor[1] = 0.5f * (patch[2].tessFactor + patch[0].tessFactor);
    pt.EdgeTessFactor[2] = 0.5f * (patch[0].tessFactor + patch[1].tessFactor);

    //float tessFactor = 50;
    
    //pt.EdgeTessFactor[0] = tessFactor;
    //pt.EdgeTessFactor[1] = tessFactor;
    //pt.EdgeTessFactor[2] = tessFactor;
    pt.InsideTessFactor = pt.EdgeTessFactor[0];
    
    return pt;
}

//[partitioning("fractional_odd")]
//[partitioning("pow2")]
//[partitioning("integer")]
[domain("tri")]
[partitioning("pow2")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchHS")]
HS_OUTPUT main(InputPatch<VS_OUTPUT, 3> p,
           uint i : SV_OutputControlPointID,
           uint patchId : SV_PrimitiveID)
{
    HS_OUTPUT hout;
    
    // Pass through shader.
    hout.pos = p[i].pos;
    hout.worldPos = p[i].worldPos;
    hout.normal = p[i].normal;
    hout.TBN = p[i].TBN;
    hout.texCord = p[i].texCord;

    
    return hout;
}