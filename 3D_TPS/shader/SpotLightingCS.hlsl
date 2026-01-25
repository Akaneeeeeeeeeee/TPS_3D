struct SpotLightGPU
{
    float4 Position;
    float4 Direction;
    float4 Color;
    float4 Params1; // range, innerCos, outerCos, intensity
    float4 Params2; // enabled, near, shadowSlice, unused
};

SamplerState Samp : register(s0);
SamplerComparisonState ShadowCmp : register(s1);

Texture2D GAlbedo : register(t0);
Texture2D GNormalR : register(t1);
Texture2D DepthTex : register(t3);

StructuredBuffer<SpotLightGPU> SpotLights : register(t6);
StructuredBuffer<uint> TileCountSRV : register(t7);
StructuredBuffer<uint> TileIndexSRV : register(t8);

// 近いK本だけ影を持つ：配列影
Texture2DArray<float> SpotShadowTex : register(t9);

RWTexture2D<float4> SpotAccum : register(u0);

// カメラ復元
cbuffer CBDeferred : register(b9)
{
    matrix InvViewT;
    matrix InvProjT;
    float4 CameraWorldPos;
    float4 Screen; // x=W,y=H
}

// タイル情報
cbuffer CBTileInfo : register(b1)
{
    uint SpotCount;
    uint MaxPerTile;
    uint TileW;
    uint TileH;
}

// スポット影行列（K=16まで）
cbuffer CBSpotShadow : register(b2)
{
    matrix SpotLightViewProjT[16]; // row-vector用Transpose済み
    float4 SpotShadowTexel; // x=1/w, y=1/h
    float4 SpotShadowParams; // x=bias, y=normalBias, z=pcfRadius, w=K
}

float3 WorldPosFromDepth(float2 uv, float depth01)
{
    float2 ndc = uv * 2.0 - 1.0;
    ndc.y = -ndc.y;
    float4 clip = float4(ndc, depth01, 1.0);

    float4 view = mul(clip, InvProjT);
    view.xyz /= max(view.w, 1e-6);

    float4 w = mul(float4(view.xyz, 1.0), InvViewT);
    return w.xyz;
}

float SpotShadowPCF(int slice, float3 worldPos, float3 N)
{
    float3 wp = worldPos + N * SpotShadowParams.y;

    float4 lc = mul(float4(wp, 1), SpotLightViewProjT[slice]);
    if (abs(lc.w) < 1e-6)
        return 1.0;

    float3 ndc = lc.xyz / lc.w;

    float2 suv = ndc.xy * 0.5 + 0.5;
    suv.y = 1.0 - suv.y;

    float depth = ndc.z;
    if (depth < 0 || depth > 1)
        return 1.0;
    if (suv.x < 0 || suv.x > 1 || suv.y < 0 || suv.y > 1)
        return 1.0;

    int r = (int) SpotShadowParams.z;
    float2 texel = SpotShadowTexel.xy;
    float bias = SpotShadowParams.x;

    float lit = 0;
    int cnt = 0;
    [loop]
    for (int y = -r; y <= r; ++y)
    {
        [loop]
        for (int x = -r; x <= r; ++x)
        {
            float2 uv = suv + float2(x, y) * texel;
            lit += SpotShadowTex.SampleCmpLevelZero(ShadowCmp, float3(uv, slice), depth - bias);
            cnt++;
        }
    }
    return lit / max(cnt, 1);
}

[numthreads(16, 16, 1)]
void main(uint3 gid : SV_GroupID, uint3 tid : SV_GroupThreadID)
{
    uint tileX = gid.x;
    uint tileY = gid.y;
    if (tileX >= TileW || tileY >= TileH)
        return;

    uint2 pix = uint2(tileX * 16 + tid.x, tileY * 16 + tid.y);
    if (pix.x >= (uint) Screen.x || pix.y >= (uint) Screen.y)
        return;

    float2 uv = (float2(pix) + 0.5) / Screen.xy;

    float depth01 = DepthTex.SampleLevel(Samp, uv, 0).r;
    if (depth01 >= 0.99999)
    {
        SpotAccum[pix] = float4(0, 0, 0, 0);
        return;
    }

    float3 al = GAlbedo.SampleLevel(Samp, uv, 0).rgb;
    float3 nr = GNormalR.SampleLevel(Samp, uv, 0).xyz;
    float3 N = normalize(nr * 2.0 - 1.0);
    float3 worldPos = WorldPosFromDepth(uv, depth01);

    uint tileId = tileY * TileW + tileX;
    uint count = TileCountSRV[tileId];
    count = min(count, MaxPerTile);

    float3 sum = 0;

    [loop]
    for (uint i = 0; i < count; ++i)
    {
        uint li = TileIndexSRV[tileId * MaxPerTile + i];
        if (li >= SpotCount)
            continue;

        SpotLightGPU s = SpotLights[li];
        if (s.Params2.x < 0.5)
            continue;

        float3 lp = s.Position.xyz;
        float3 sd = normalize(s.Direction.xyz);

        float3 toP = worldPos - lp;
        float dist2 = dot(toP, toP);

        float range = s.Params1.x;
        float range2 = range * range;
        if (dist2 > range2)
            continue;

        float dist = sqrt(dist2);
        float nearD = s.Params2.y;
        if (dist <= nearD)
            continue;

        float3 dirTo = toP / max(dist, 1e-6);
        float cosAng = dot(dirTo, sd);

        float innerCos = s.Params1.y;
        float outerCos = s.Params1.z;
        if (cosAng < outerCos)
            continue;

        float angle01 = saturate((cosAng - outerCos) / max(innerCos - outerCos, 1e-6));

        float dist01 = 1.0 - (dist - nearD) / max(range - nearD, 1e-6);
        dist01 = saturate(dist01);
        dist01 *= dist01;

        // ndl（面に当たる）
        float ndl = saturate(dot(N, normalize(lp - worldPos)));

        float atten = angle01 * dist01 * s.Params1.w * ndl;

        // 影（近いK本だけ）
        float shadow = 1.0;
        int slice = (int) s.Params2.z;
        if (slice >= 0 && slice < (int) SpotShadowParams.w)
            shadow = SpotShadowPCF(slice, worldPos, N);

        sum += al * (s.Color.rgb * atten * shadow);
    }

    SpotAccum[pix] = float4(sum, 1);
}
