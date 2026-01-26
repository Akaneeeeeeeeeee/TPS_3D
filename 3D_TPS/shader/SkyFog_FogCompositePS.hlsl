#include "SkyFog_Common.hlsli"

Texture2D BeamTex : register(t6);

SamplerState BeamSamp : register(s2);

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

    float4 fog;
    if (edge > 0.002)
    {
        // ‚±‚±‚Ífog‚Ì‚Ý
        fog = FogCompute(uv, DepthFull, NoiseFogF, /*allowNear=*/true);
    }
    else
    {
        fog = low;
    }

    // BeamTex ‡¬irgb=F, a=‹­‚³j
    float4 beam = BeamTex.SampleLevel(BeamSamp, uv, 0);

    float outA = 1.0 - (1.0 - fog.a) * (1.0 - beam.a);
    float3 premul = fog.rgb * fog.a + beam.rgb * beam.a * (1.0 - fog.a);
    float3 outRgb = (outA > 1e-6) ? (premul / outA) : 0.0;

    return float4(outRgb, outA);
}
