struct HS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 worldPos : WORLDPOS;
    float4 normal : NORMAL;
    float3x3 TBN : TBN;
    float4 texCord : TEXCORD;
};

cbuffer LIGHT_BUFFER : register(b0, space1)
{
    float4 CameraPosition;
    uint4  LightType[256];
    float4 LightPosition[256];
    float4 LightColor[256];
    float4 LightVector[256];
};

cbuffer SHADOW_BUFFER : register(b1, space1)
{
    int4 values;
    float4x4 ShadowViewProjection;
}

Texture2D albedo : register(t0);
Texture2D normalmap : register(t1);
Texture2D metallicMap : register(t2);
Texture2D shadowMap : register(t3);

SamplerState defaultSampler : register(s0);

float4 LightCalculation(float4 albedo, float4 worldPos, float4 normal, float4 metallic, out float specular)
{
    float4 worldToCamera = normalize(CameraPosition - worldPos);
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

            //halfWayDir = normalize(posToLight + worldToCamera);
            
            //specular += pow(max(dot(normal, halfWayDir), 0.0f), 32.0f) * length(metallic.rgb);

        }
    }

    return finalColor;
}

void ShadowCalculations(float4 worldPos, out float shadowCoeff, float min = 0.0f, float max = 1.0f)
{   
    shadowCoeff = 1.0f;
    float4 fragmentLightPosition = mul(worldPos, ShadowViewProjection);
    fragmentLightPosition.xyz /= fragmentLightPosition.w;
    float2 smTex = float2(0.5f * fragmentLightPosition.x + 0.5f, -0.5f * fragmentLightPosition.y + 0.5f);
    float depth = fragmentLightPosition.z;

    if (abs(fragmentLightPosition.x) > 1.0f ||
        abs(fragmentLightPosition.y) > 1.0f)            
        return;

    float epsilon = 0.1;
    shadowCoeff = (shadowMap.Sample(defaultSampler, smTex).r - epsilon < depth) ? min : max;
}

float4 main(HS_OUTPUT input) : SV_TARGET
{
    float4 texColor = albedo.Sample(defaultSampler, input.texCord.xy);
    float4 normal = float4(normalize(input.normal.xyz + mul((2.0f * normalmap.Sample(defaultSampler, input.texCord.xy).xyz - 1.0f), input.TBN)), 0);
    float4 metallic = metallicMap.Sample(defaultSampler, input.texCord.xy);

    float4 ambient = float4(0.1f, 0.1f, 0.1f, 1.0f) * texColor;

    float specular;
    float shadowCoeff;
    ShadowCalculations(input.worldPos, shadowCoeff, -1.0f);
    float4 finalColor = LightCalculation(texColor, input.worldPos, normal, metallic, specular);
    return saturate((finalColor + specular) * shadowCoeff + ambient);
}