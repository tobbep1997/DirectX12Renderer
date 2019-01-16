
cbuffer ParticleBuffer : register(b0)
{
    float4      CameraPosition;
    float4x4    WorldMatrix;

    float4 ParticleInfo[256]; //X = deltaTime Y = TimeAlive Z = TimeToLive
    float4 ParticlePosition[256];
    float4 ParticleDirection[256];    //W = velocity
    float4 ParticleSize[256];
};

RWStructuredBuffer<float4> VertexBufferOut : register(u0);
RWStructuredBuffer<float4> CalcBufferOut : register(u1);

void ParticleCalculations(inout float4 position, inout float4 info, in float4 direction)
{
    position = float4(position.xyz + (direction.xyz * direction.w * info.x), 1);
    position.w = 1;
    info.y += info.x;
}



[numthreads(256, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint textures = 3;

    float4 particleInfo = ParticleInfo[DTid.x];
    float4 particlePos  = ParticlePosition[DTid.x];

    ParticleCalculations(particlePos, particleInfo, ParticleDirection[DTid.x]);


    float4 particleWorldPos = mul(particlePos, WorldMatrix);
    float4 cameraPos = float4(CameraPosition.x * 2.0f, CameraPosition.y * 2.0f, CameraPosition.z * 2.0f, 1);

    float3 dir      = normalize(particleWorldPos.xyz - cameraPos.xyz);
    float3 right    =  normalize(cross(dir.xyz, float3(0, 1, 0)));
    float3 up       =     normalize(cross(dir.xyz, right.xyz));
    
    float4 upperLeft    = particleWorldPos + float4((-right * ParticleSize[DTid.x].x) + (up * ParticleSize[DTid.x].y), 0);
    upperLeft.w = 1;
    float4 upperRight   = particleWorldPos + float4((right * ParticleSize[DTid.x].x) + (up * ParticleSize[DTid.x].y), 0);
    upperRight.w = 1;
    float4 lowerLeft    = particleWorldPos + float4((-right * ParticleSize[DTid.x].x) + (-up * ParticleSize[DTid.x].y), 0);
    lowerLeft.w = 1;
    float4 lowerRight   = particleWorldPos + float4((right * ParticleSize[DTid.x].x) + (-up * ParticleSize[DTid.x].y), 0);
    lowerRight.w = 1;

    float third = particleInfo.z / 3.0f;
    uint textureIndex = 0;
    if (particleInfo.y > third)
        textureIndex = 1;
    if (particleInfo.y > third*2)
        textureIndex = 2;

    float4 upperLeftUV = float4(0, 0, textureIndex, 0);
    float4 upperRightUV = float4(1, 0, textureIndex, 0);
    float4 lowerLeftUV = float4(0, 1, textureIndex, 0);
    float4 lowerRightUV = float4(1, 1, textureIndex, 0);

    if (ParticleInfo[DTid.x].x > 0)
    {
        const uint baseVertexIndex = DTid.x * 12;
        
        VertexBufferOut[baseVertexIndex + 0] = lowerLeft;
        VertexBufferOut[baseVertexIndex + 1] = lowerLeftUV;

        VertexBufferOut[baseVertexIndex + 2] = upperLeft;
        VertexBufferOut[baseVertexIndex + 3] = upperLeftUV;

        VertexBufferOut[baseVertexIndex + 4] = upperRight;
        VertexBufferOut[baseVertexIndex + 5] = upperRightUV;

        VertexBufferOut[baseVertexIndex + 6] = lowerRight;
        VertexBufferOut[baseVertexIndex + 7] = lowerRightUV;

        VertexBufferOut[baseVertexIndex + 8] = lowerLeft;
        VertexBufferOut[baseVertexIndex + 9] = lowerLeftUV;

        VertexBufferOut[baseVertexIndex + 10] = upperRight;
        VertexBufferOut[baseVertexIndex + 11] = upperRightUV;

        const uint baseCalcIndex = DTid.x * 2;

        CalcBufferOut[baseCalcIndex + 0] = particlePos;
        CalcBufferOut[baseCalcIndex + 1] = particleInfo;
    }

}