
#pragma warning (disable : 4000)

struct LIGHT_STRUCT
{
    uint4 LightType;
    float4 LightPosition;
    float4 LightColor;
    float4 LightVector;
};

float4 SingelLightCalculations(in LIGHT_STRUCT light,
	in float4 cameraPosition,
	in float4 worldPosition,
	in float4 albedo,
	in float4 normal,
	in float4 metallic,
	inout float4 specular)
{
    float4 worldToCamera = normalize(cameraPosition - worldPosition);
    float4 posToLight = light.LightPosition - worldPosition;
    float4 finalColor = float4(0, 0, 0, 0);
    float4 halfWayDir = 0;
    float distanceToLight = length(posToLight);
    float attenuation = 0;
	

    if (light.LightType.x == 0) //pointlight
    {
        attenuation = light.LightVector.x / (1.0f + light.LightVector.y * pow(distanceToLight, light.LightVector.z));

        halfWayDir = normalize(posToLight + worldToCamera);

        specular += pow(max(dot(normal, halfWayDir), 0.0f), 128.0f) * length(metallic.rgb) * attenuation * light.LightColor;

        if (distanceToLight < light.LightVector.w)
        {
            finalColor = max(dot(normal, normalize(posToLight)), 0.0f) * light.LightColor * albedo * attenuation;
        }
    }
    else if (light.LightType.x == 1) //Dir
    {
        finalColor = max(dot(normal, normalize(-float4(light.LightVector.xyz, 0))), 0.0f) * light.LightColor * albedo * light.LightVector.w;
    }
    return finalColor;
}

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
    inout float4 specular)
{
    float4 worldToCamera = normalize(cameraPosition - worldPos);
    float4 posToLight = float4(0, 0, 0, 0);
    float4 finalColor = float4(0, 0, 0, 0);
    float4 halfWayDir = 0;
    float distanceToLight = 0;
    float attenuation = 0;   

    for (uint i = 0; i < LightType[0].x && i < 256; i++)
    {
        posToLight = LightPosition[i] - worldPos;
        distanceToLight = length(posToLight);
     
        if (LightType[i].y == 0) //pointlight
        {
            attenuation = LightVector[i].x / (1.0f + LightVector[i].y * pow(distanceToLight, LightVector[i].z));

            halfWayDir = normalize(posToLight + worldToCamera);
            
            specular += pow(max(dot(normal, halfWayDir), 0.0f), 128.0f) * length(metallic.rgb) * attenuation * LightColor[i];

            if (distanceToLight < LightVector[i].w)
            {
                finalColor += max(dot(normal, normalize(posToLight)), 0.0f) * LightColor[i] * albedo * attenuation;
            }
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

float TexelSize(Texture2DArray tTexture)
{
    float width, height, element, numberOfElements;
    tTexture.GetDimensions(0, width, height, element, numberOfElements);
    return 1.0f / width;
}

float TexelSize(Texture2D tTexture)
{
    float width, height, element;
    tTexture.GetDimensions(0, width, height, element);
    return 1.0f / width;
}

float2 TexelSize2(Texture2D tTexture)
{
    float width, height, element;
    tTexture.GetDimensions(0, width, height, element);
    return float2(1.0f / width, 1.0f / height);
}

int ShadowCalculations(Texture2DArray shadowMap, uint index, SamplerComparisonState samplerState, in float texelSize, in float4 fragmentLightPos, inout float shadowCoeff, in int PFCSampleRate = 1, in float min = 0.0f, in float max = 1.0f)
{    
    
    if (abs(fragmentLightPos.x) > 1 || abs(fragmentLightPos.y) > 1)
        return 0;

    float2 smTex;
    float2 baseUV = FragmentLightUV(fragmentLightPos);
    float depth = FragmentLightDepth(fragmentLightPos);
    float epsilon = 0.01f;
    float divider = 0.0f;
    float currentShadowCoeff = 1.0f;
       

    for (int x = -PFCSampleRate; x <= PFCSampleRate; ++x)
    {
        for (int y = -PFCSampleRate; y <= PFCSampleRate; ++y)
        {
            smTex = baseUV + (float2(x, y) * texelSize);
            currentShadowCoeff += shadowMap.SampleCmpLevelZero(samplerState, float3(smTex.x, smTex.y, index), depth - epsilon).r;
            divider += 1.0f;
        }
    }
    currentShadowCoeff /= divider;
    shadowCoeff += currentShadowCoeff;
    return 1;
}