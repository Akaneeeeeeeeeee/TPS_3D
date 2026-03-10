#include "common.hlsl"

cbuffer LineWidthCB : register(b10)
{
    float gLineWidthPx; // ピクセル幅
    float2 gInvViewport; // (1/width, 1/height)
    float _pad;
}

[maxvertexcount(6)]
void main(line PS_IN input[2], inout TriangleStream<PS_IN> s)
{
    PS_IN o;
    o.WorldPos = float3(0, 0, 0);
    o.NormalW = float3(0, 0, 1);
    o.TexCoord = float2(0, 0);

    // dir は NDC で取る
    float2 p0 = input[0].Position.xy / input[0].Position.w;
    float2 p1 = input[1].Position.xy / input[1].Position.w;

    float2 dir = p1 - p0;
    float len2 = dot(dir, dir);
    if (len2 < 1e-12)
        return;
    dir *= rsqrt(len2);

    // ★ピクセル → NDC へ変換
    // NDCは[-1..1]なので「1ピクセル = 2/width, 2/height」
    float2 ndcPerPixel = float2(2.0f * gInvViewport.x, 2.0f * gInvViewport.y);

    // 太さは“半幅”で押し広げるのが自然
    float2 n = float2(-dir.y, dir.x) * (gLineWidthPx * 0.5f) * ndcPerPixel;

    // NDCオフセットをクリップ空間へ（wを掛ける）
    float4 p0L = input[0].Position;
    p0L.xy += n * input[0].Position.w;
    float4 p0R = input[0].Position;
    p0R.xy -= n * input[0].Position.w;
    float4 p1L = input[1].Position;
    p1L.xy += n * input[1].Position.w;
    float4 p1R = input[1].Position;
    p1R.xy -= n * input[1].Position.w;

    // tri1: p0L, p1L, p0R
    o.Diffuse = input[0].Diffuse;
    o.Position = p0L;
    s.Append(o);
    o.Diffuse = input[1].Diffuse;
    o.Position = p1L;
    s.Append(o);
    o.Diffuse = input[0].Diffuse;
    o.Position = p0R;
    s.Append(o);
    s.RestartStrip();

    // tri2: p0R, p1L, p1R
    o.Diffuse = input[0].Diffuse;
    o.Position = p0R;
    s.Append(o);
    o.Diffuse = input[1].Diffuse;
    o.Position = p1L;
    s.Append(o);
    o.Diffuse = input[1].Diffuse;
    o.Position = p1R;
    s.Append(o);
    s.RestartStrip();
}