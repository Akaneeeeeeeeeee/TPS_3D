struct SpotLightGPU
{
    float4 Position;
    float4 Direction;
    float4 Color;
    float4 Params1; // range, innerCos, outerCos, intensity
    float4 Params2; // enabled, near, shadowSlice, unused
};

StructuredBuffer<SpotLightGPU> SpotLights : register(t6);

RWStructuredBuffer<uint> TileCount : register(u0);
RWStructuredBuffer<uint> TileIndex : register(u1);

cbuffer CBTile : register(b0)
{
    matrix ViewT; // row-vector用Transpose済み
    float2 Screen; // (1920,1080)
    float2 ProjScale; // (Proj._11, Proj._22) をCPUで入れる
    uint SpotCount;
    uint MaxPerTile;
    uint2 _pad;
};

[numthreads(64, 1, 1)]
void main(uint3 gid : SV_GroupID, uint3 tid : SV_GroupThreadID)
{
    uint tileId = gid.x; // Dispatch(tileCount,1,1)
    uint tileX = tileId % (uint) (Screen.x / 16);
    uint tileY = tileId / (uint) (Screen.x / 16);

    float2 tileMin = float2(tileX * 16, tileY * 16);
    float2 tileMax = tileMin + float2(16, 16);

    // 各スレッドがライトを分担
    for (uint li = tid.x; li < SpotCount; li += 64)
    {
        SpotLightGPU s = SpotLights[li];
        if (s.Params2.x < 0.5)
            continue; // enabled

        float3 wp = s.Position.xyz;
        float range = s.Params1.x;

        // view空間へ
        float4 v = mul(float4(wp, 1), ViewT);
        if (v.z <= 0.1)
            continue;

        // スクリーン中心（近似）：射影してuv→pixel
        float2 ndc;
        ndc.x = (v.x * ProjScale.x) / v.z;
        ndc.y = (v.y * ProjScale.y) / v.z;

        float2 uv = ndc * 0.5 + 0.5;
        uv.y = 1.0 - uv.y;

        float2 centerPx = uv * Screen;

        // 半径（近似）：rangeをスクリーンピクセル半径へ
        float radiusPx = range * ProjScale.x * (Screen.x * 0.5) / v.z;

        // 円と矩形の交差判定（簡易）
        float2 c = centerPx;
        float2 q = clamp(c, tileMin, tileMax);
        float2 d = c - q;
        if (dot(d, d) > radiusPx * radiusPx)
            continue;

        // 追加（atomic）
        uint idx;
        InterlockedAdd(TileCount[tileId], 1, idx);
        if (idx < MaxPerTile)
            TileIndex[tileId * MaxPerTile + idx] = li;
    }
}
