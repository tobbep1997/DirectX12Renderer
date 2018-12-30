struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 worldPos : WORLDPOS;
    float4 normal : NORMAL;
    float3x3 TBN : TBN;
    float4 texCord : TEXCORD;
};

Texture2D albedo : register(t0);
Texture2D normalmap : register(t1);
SamplerState defaultSampler : register(s0);

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float4 lightPos = float4(5.0f, 0.0f, -5.0f, 1.0f);
    float4 lightColor = float4(1, 1, 1, 1);
    float4 ambient = float4(0.02f, 0.02f, 0.02f, 1.0f);
    float4 texColor = albedo.Sample(defaultSampler, input.texCord.xy);
    float4 normal = float4(normalize(input.normal.xyz + mul((2.0f * normalmap.Sample(defaultSampler, input.texCord.xy).xyz - 1.0f), input.TBN)), 0);

    float4 posToLight = lightPos - input.worldPos;
    float distanceToLight = length(posToLight);
    float attenuation = 10.0f / (1.0f + 1.0f * pow(distanceToLight, 1.5f));

    float4 finalColor = max(dot(normal, normalize(posToLight)), 0.0f) * lightColor * texColor;
    return saturate((finalColor + ambient) * attenuation);
}