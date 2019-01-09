
cbuffer ParticleBuffer : register(b0)
{
    float4  CameraPosition;
    uint4 ParticleInfo;
    float4  ParticlePosition[256];
};

RWStructuredBuffer<float4> BufferOut : register(u0);

[numthreads(256, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float3 dir = normalize(ParticlePosition[DTid.x].xyz - CameraPosition.xyz);
    float3 right = normalize(cross(dir.xyz, float3(0, 1, 0)));
    float3 up = normalize(cross(dir.xyz, right.xyz));
    
    float4 upperLeft = ParticlePosition[DTid.x] + float4(-right + up, 1);
    float4 upperRight = ParticlePosition[DTid.x] + float4(right + up, 1);
    float4 lowerLeft = ParticlePosition[DTid.x] + float4(-right + -up, 1);
    float4 lowerRight = ParticlePosition[DTid.x] + float4(right + -up, 1);
    
    float4 upperLeftUV = float4(0, 0, 0, 0);
    float4 upperRightUV = float4(1, 0, 0, 0);
    float4 lowerLeftUV = float4(0, 1, 0, 0);
    float4 lowerRightUV = float4(1, 1, 0, 0);

    const uint baseIndex = DTid.x * 12;
        
    BufferOut[DTid.x + 0] = lowerLeft;
    BufferOut[DTid.x + 1] = lowerLeftUV;

    BufferOut[DTid.x + 2] = upperLeft;
    BufferOut[DTid.x + 3] = upperLeftUV;

    BufferOut[DTid.x + 4] = upperRight;
    BufferOut[DTid.x + 5] = upperRightUV;

    BufferOut[DTid.x + 6] = lowerRight;
    BufferOut[DTid.x + 7] = lowerRightUV;

    BufferOut[DTid.x + 8] = lowerLeft;
    BufferOut[DTid.x + 9] = lowerLeftUV;

    BufferOut[DTid.x + 10] = upperRight;
    BufferOut[DTid.x + 11] = upperRightUV;
}