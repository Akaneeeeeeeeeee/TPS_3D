static const uint MAX_INSTANCE = 256;

struct VS_IN
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR0;
    float3 normal : NORMAL0;
};

cbuffer CameraCB : register(b0)
{
    float4x4 gView;
    float4x4 gProj;
};

cbuffer InstanceCB : register(b1)
{
    float4x4 gWorld[MAX_INSTANCE];
};

VS_OUT main(VS_IN vin, uint instanceId : SV_InstanceID)
{
    VS_OUT o;

    float4x4 world = gWorld[instanceId];

    float4 wpos = mul(float4(vin.pos, 1.0f), world);
    float4 vpos = mul(wpos, gView);
    o.pos = mul(vpos, gProj);

    float3 wN = mul(vin.normal, (float3x3) world);
    o.normal = normalize(wN);
    o.color = vin.color;

    return o;
}
