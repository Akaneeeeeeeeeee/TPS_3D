struct SpotLightGPU
{
    float4 Position;
    float4 Direction;
    float4 Color;
    float4 Params1; // range, innerCos, outerCos, intensity
    float4 Params2; // enabled, near, shadowSlice, unused
};

SamplerState Samp : register(s0);

Texture2D DepthTex : register(t3);

StructuredBuffer<SpotLightGPU> SpotLights : register(t6);
StructuredBuffer<uint> TileCountSRV : register(t7);
StructuredBuffer<uint> TileIndexSRV : register(t8);

RWTexture2D<float4> BeamOut : register(u0);

cbuffer CBDeferred : register(b9)
{
    matrix InvViewT;
    matrix InvProjT;
    float4 CameraWorldPos;
    float4 Screen; // x=W,y=H
}

cbuffer CBTileInfo : register(b1)
{
    uint SpotCount;
    uint MaxPerTile;
    uint TileW;
    uint TileH;
}

cbuffer CBBeam : register(b0)
{
    float beamMaxDist; // 例: 3000
    float stepLenWanted; // 例: 50
    float kBeam; // 例: 0.002
    float beamTint; // 例: 1.0

    float2 BeamSize; // (BEAM_W, BEAM_H)
    float2 _pad;
};

float3 WorldDirFromUV(float2 uv)
{
    float2 ndc = uv * 2.0 - 1.0;
    ndc.y = -ndc.y;

    float4 clip = float4(ndc, 1.0, 1.0);
    float4 view = mul(clip, InvProjT);
    view.xyz /= max(view.w, 1e-6);

    float3 wdir = mul(float4(view.xyz, 0.0), InvViewT).xyz;
    return normalize(wdir);
}

float3 WorldPosFromDepth(float2 uv, float depth01)
{
    float2 ndc = uv * 2.0 - 1.0;
    ndc.y = -ndc.y;
    float4 clip = float4(ndc, depth01, 1.0);

    float4 view = mul(clip, InvProjT);
    view.xyz /= max(view.w, 1e-6);

    float3 wp = mul(float4(view.xyz, 1.0), InvViewT).xyz;
    return wp;
}

// Fogの SpotCone01 を dist2 先判定で軽くした版
float SpotCone01(float3 p, SpotLightGPU s)
{
    float3 lp = s.Position.xyz;
    float3 sd = normalize(s.Direction.xyz);

    float3 toP = p - lp;
    float dist2 = dot(toP, toP);

    float range = s.Params1.x;
    if (dist2 > range * range)
        return 0.0;

    float dist = sqrt(dist2);

    float innerCos = s.Params1.y;
    float outerCos = s.Params1.z;
    float nearD = s.Params2.y;

    if (dist <= nearD || dist > range)
        return 0.0;

    float3 dirTo = toP / max(dist, 1e-6);
    float cosAng = dot(dirTo, sd);
    if (cosAng < outerCos)
        return 0.0;

    float angle01 = saturate((cosAng - outerCos) / max(innerCos - outerCos, 1e-6));

    float dist01 = saturate(1.0 - (dist - nearD) / max(range - nearD, 1e-6));
    dist01 = sqrt(dist01);

    return angle01 * dist01;
}

[numthreads(8, 8, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    if (id.x >= (uint) BeamSize.x || id.y >= (uint) BeamSize.y)
        return;

    float2 uv = (float2(id.xy) + 0.5) / BeamSize;

    float depth01 = DepthTex.SampleLevel(Samp, uv, 0).r;

    float3 camW = CameraWorldPos.xyz;

    float3 dir;
    float distMax;

    if (depth01 >= 0.99999)
    {
        dir = WorldDirFromUV(uv);
        distMax = beamMaxDist;
    }
    else
    {
        float3 wp = WorldPosFromDepth(uv, depth01);
        float3 v = wp - camW;
        float dist = length(v);
        if (dist <= 1e-4)
        {
            BeamOut[id.xy] = 0;
            return;
        }
        dir = v / dist;
        distMax = min(dist, beamMaxDist);
    }

    // full解像度でのtile選択（Tileは 1920x1080基準）
    uint2 pixFull = uint2(uv.x * Screen.x, uv.y * Screen.y);
    uint tileX = pixFull.x / 16;
    uint tileY = pixFull.y / 16;
    if (tileX >= TileW || tileY >= TileH)
    {
        BeamOut[id.xy] = 0;
        return;
    }

    uint tileId = tileY * TileW + tileX;
    uint count = min(TileCountSRV[tileId], MaxPerTile);

    int steps = (int) ceil(distMax / max(stepLenWanted, 1.0));
    steps = clamp(steps, 8, 128);
    float stepLen = distMax / steps;

    float beamEnergy = 0.0;
    float3 beamPremul = 0.0;

    [loop]
    for (int i = 0; i < steps; ++i)
    {
        float tt = (i + 0.5) * stepLen;
        float3 p = camW + dir * tt;

        [loop]
        for (uint k = 0; k < count; ++k)
        {
            uint li = TileIndexSRV[tileId * MaxPerTile + k];
            if (li >= (uint) SpotCount)
                continue;

            SpotLightGPU s = SpotLights[li];
            if (s.Params2.x < 0.5)
                continue;

            float cone = SpotCone01(p, s);
            if (cone <= 0.0)
                continue;

            float w = cone * s.Params1.w * stepLen; // intensity
            beamEnergy += w;
            beamPremul += s.Color.rgb * w;
        }
    }

    float a = saturate(1.0 - exp(-kBeam * beamEnergy));
    float3 rgb = (beamEnergy > 1e-6) ? (beamPremul / beamEnergy) * beamTint : 0.0;

    BeamOut[id.xy] = float4(rgb, a);
}
