#include "common.hlsl"
static const uint MAX_INSTANCE = 256;


struct VS_OUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR0;
    float3 normal : NORMAL0;
};


cbuffer InstanceCB : register(b8)
{
    row_major float4x4 gWorld[MAX_INSTANCE];
};

VS_OUT main(VS_IN vin, uint instanceId : SV_InstanceID)
{
    VS_OUT o;

    float4x4 world = gWorld[instanceId];

    float4 wpos = mul(vin.Position, world);
    float4 vpos = mul(wpos, View);
    o.pos = mul(vpos, Projection);

    float4 wN = mul(vin.Normal, world);
    o.normal = normalize(wN);
    o.color = vin.Diffuse;

    return o;
}
