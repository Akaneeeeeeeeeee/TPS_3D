#include "SkyFog_Common.hlsli"

struct PS_IN
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

float4 main(PS_IN pin) : SV_Target
{
    float2 uv = saturate(pin.uv);
    return FogCompute(uv, DepthLowIn, NoiseFogL, true);
}
