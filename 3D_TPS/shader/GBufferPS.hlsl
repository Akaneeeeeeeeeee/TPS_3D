SamplerState Samp : register(s0);
Texture2D DiffuseMap : register(t0);

struct MATERIAL
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Emission;
    float Shininess;
    bool TextureEnable;
    float2 Dummy;
};
cbuffer MaterialBuffer : register(b3)
{
    MATERIAL Material;
}

struct PS_IN
{
    float4 PosH : SV_Position;
    float3 NrmW : TEXCOORD0;
    float2 UV : TEXCOORD1;
    float4 Col : TEXCOORD2;
};

struct PS_OUT
{
    float4 RT0 : SV_Target0; // albedo
    float4 RT1 : SV_Target1; // normal + rough
    float4 RT2 : SV_Target2; // emission + (unused)
};

PS_OUT main(PS_IN i)
{
    PS_OUT o;

    float3 albedo = Material.Diffuse.rgb * i.Col.rgb;

    if (Material.TextureEnable)
        albedo *= DiffuseMap.Sample(Samp, i.UV).rgb;

    float3 N = normalize(i.NrmW);
    float3 encN = N * 0.5 + 0.5;

    float rough = saturate(1.0 - Material.Shininess / 256.0);

    o.RT0 = float4(albedo, 1.0);
    o.RT1 = float4(encN, rough);
    o.RT2 = float4(Material.Emission.rgb, 1.0);
    return o;
}
