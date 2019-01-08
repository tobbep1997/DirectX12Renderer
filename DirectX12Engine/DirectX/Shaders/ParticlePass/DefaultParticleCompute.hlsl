
cbuffer ParticleBuffer : register(b0)
{
    float4  CameraPosition;
    uint4 ParticleInfo;
    float4  ParticlePosition[256];
};

RWStructuredBuffer<float3x3> BufferOut : register(u0);

[numthreads(256, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float3 dir = normalize(ParticlePosition[DTid.x].xyz - CameraPosition.xyz);
    float3 right = normalize(cross(dir.xyz, float3(0, 1, 0)));
    float3 up = normalize(cross(dir.xyz, right.xyz));
    
    BufferOut[DTid.x] = float3x3(right, up, dir);
}