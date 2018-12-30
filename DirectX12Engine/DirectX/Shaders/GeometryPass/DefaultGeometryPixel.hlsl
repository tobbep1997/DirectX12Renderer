struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 worldPos : WORLDPOS;
    float4 normal : NORMAL;
    float3x3 TBN : TBN;
    float4 texCord : TEXCORD;
};

cbuffer LIGHT_BUFFER : register(b0)
{
    uint4 LightType[256];
    float4 LightPosition[256];
    float4 LightColor[256];
    float4 LightVector[256];
};

Texture2D albedo : register(t0);
Texture2D normalmap : register(t1);
SamplerState defaultSampler : register(s0);

float4 LightCalculation(float4 albedo, float4 worldPos, float4 normal)
{
    float4 posToLight = float4(0, 0, 0, 0);
    float4 finalColor = float4(0, 0, 0, 0);
    float distanceToLight = 0;
    float attenuation = 0;
    
    for (uint i = 0; i < LightType[0].x && i < 256; i++)
    {
        posToLight = LightPosition[i] - worldPos;
        distanceToLight = length(posToLight);
     
        if (LightType[i].y == 1) //pointlight
        {
            attenuation = LightVector[i].x / (1.0f + LightVector[i].y * pow(distanceToLight, LightVector[i].z));
            finalColor += max(dot(normal, normalize(posToLight)), 0.0f) * LightColor[i] * albedo * attenuation;
        }
    }
    

    return finalColor;
}

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float4 ambient = float4(0.02f, 0.02f, 0.02f, 1.0f);

    float4 texColor = albedo.Sample(defaultSampler, input.texCord.xy);
    float4 normal = float4(normalize(input.normal.xyz + mul((2.0f * normalmap.Sample(defaultSampler, input.texCord.xy).xyz - 1.0f), input.TBN)), 0);



    float4 finalColor = LightCalculation(texColor, input.worldPos, normal);
    return saturate((finalColor + ambient));
}