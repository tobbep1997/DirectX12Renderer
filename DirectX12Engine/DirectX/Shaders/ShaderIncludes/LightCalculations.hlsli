
float4 LightCalculation(
    uint4 LightType[256],
    float4 LightPosition[256], 
    float4 LightColor[256], 
    float4 LightVector[256],
    float4 cameraPosition, 
    float4 worldPos, 
    float4 albedo, 
    float4 normal, 
    float4 metallic, 
    out float specular)
{
    float4 worldToCamera = normalize(cameraPosition - worldPos);
    float4 posToLight = float4(0, 0, 0, 0);
    float4 finalColor = float4(0, 0, 0, 0);
    float4 halfWayDir = 0;
    float distanceToLight = 0;
    float attenuation = 0;
    specular = 0;

    for (uint i = 0; i < LightType[0].x && i < 256; i++)
    {
        posToLight = LightPosition[i] - worldPos;
        distanceToLight = length(posToLight);
     
        if (LightType[i].y == 0) //pointlight
        {
            attenuation = LightVector[i].x / (1.0f + LightVector[i].y * pow(distanceToLight, LightVector[i].z));
            finalColor += max(dot(normal, normalize(posToLight)), 0.0f) * LightColor[i] * albedo * attenuation;

            halfWayDir = normalize(posToLight + worldToCamera);
            
            specular += pow(max(dot(normal, halfWayDir), 0.0f), 32.0f) * length(metallic.rgb);

        }
        else if (LightType[i].y == 1) //Dir
        {
            attenuation = LightVector[i].w;
            finalColor += max(dot(normal, normalize(-float4(LightVector[i].xyz, 0))), 0.0f) * LightColor[i] * albedo * attenuation;
        }
    }

    return finalColor;
}

float4 FragmentLightPos(float4 worldPos, float4x4 ShadowViewProjection)
{
    float4 fragmentLightPosition =mul(worldPos, ShadowViewProjection);
    fragmentLightPosition.xyz /= fragmentLightPosition.w;
    return fragmentLightPosition;
}

float2 FragmentLightUV(float4 fragmentLightPos)
{
    return float2(0.5f * fragmentLightPos.x + 0.5f, -0.5f * fragmentLightPos.y + 0.5f);
}

float FragmentLightDepth(float4 fragmentLightPos)
{
    return fragmentLightPos.z;
}

float TexelSize(Texture2D tTexture)
{
    float width, height, element;
    tTexture.GetDimensions(0, width, height, element);
    return 1.0f / width;
}

void ShadowCalculations(Texture2D shadowMap, SamplerComparisonState samplerState, in float texelSize, in float4 fragmentLightPos, out float shadowCoeff, in float min = 0.0f, in float max = 1.0f)
{
    shadowCoeff = 0.0f;
       
    float2 smTex = FragmentLightUV(fragmentLightPos);
    float depth = FragmentLightDepth(fragmentLightPos);
       
    float epsilon = 0.01f;
    float divider = 0.0f;
    int sampleSize = 1;

    for (int x = -sampleSize; x <= sampleSize; ++x)
    {
        for (int y = -sampleSize; y <= sampleSize; ++y)
        {
            shadowCoeff += shadowMap.SampleCmpLevelZero(samplerState, smTex + (float2(x, y) * texelSize), depth - epsilon).r;
            divider += 1.0f;
        }
    }

    shadowCoeff /= divider;
}