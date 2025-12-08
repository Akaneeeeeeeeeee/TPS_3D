#include "common.hlsl"

static const uint MAX_INSTANCE = 256;

// 1 本ぶんの情報
struct LineInstance
{
    float3 Start;
    float _pad0;
    float3 End;
    float _pad1;
    float4 Color;
};

cbuffer LineInstanceCB : register(b8)
{
    LineInstance gLine[MAX_INSTANCE];
}

// VS_IN / PS_IN は common.hlsl で定義されている前提
PS_IN main(VS_IN vin, uint instanceId : SV_InstanceID)
{
    PS_IN o;

    // CLineMesh 側で (0,0,0) → (0,0,1) の 2 頂点を持っている想定
    // z が 0 or 1 になっているので、それを 0～1 の補間値として使う
    float t = vin.Position.z;

    float3 start = gLine[instanceId].Start;
    float3 end = gLine[instanceId].End;

    float3 worldPos = lerp(start, end, t);

    float4 w = float4(worldPos, 1.0f);
    float4 v = mul(w, View);
    o.Position = mul(v, Projection);

    // 色はインスタンス側に持たせる
    o.Diffuse = gLine[instanceId].Color;
    o.TexCoord = float2(0, 0);

    return o;
}
