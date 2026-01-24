#include "SkyFog_Common.hlsli"

struct PSIn
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

float4 main(PSIn pin) : SV_Target
{
    float2 uv = saturate(pin.uv);

    float4 low = FogLowTex.SampleLevel(Samp, uv, 0);

    float2 px = float2(1.0 / gFogScreen.x, 1.0 / gFogScreen.y);
    float d0 = DepthFull.SampleLevel(Samp, uv, 0).r;
    float dx = DepthFull.SampleLevel(Samp, uv + float2(px.x, 0), 0).r;
    float dy = DepthFull.SampleLevel(Samp, uv + float2(0, px.y), 0).r;

    float edge = max(abs(d0 - dx), abs(d0 - dy));

    if (edge > 0.002)
    {
        return FogCompute(uv, DepthFull, NoiseFogF, /*allowNear=*/true);
    }

    return low;
}
