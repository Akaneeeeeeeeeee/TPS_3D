#include "DeferredLighting_Common.hlsli"

VS_OUT main(uint vid : SV_VertexID)
{
    float2 p[3] = { float2(-1, -1), float2(-1, 3), float2(3, -1) };
    float2 uv[3] = { float2(0, 1), float2(0, -1), float2(2, 1) };

    VS_OUT o;
    o.pos = float4(p[vid], 0, 1);
    o.uv = uv[vid];
    return o;
}