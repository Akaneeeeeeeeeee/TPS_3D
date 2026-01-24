SamplerState Samp : register(s0);

// GBuffer
Texture2D GAlbedo : register(t0);
Texture2D GNormalR : register(t1);
Texture2D GEmissive : register(t2);
Texture2D DepthTex : register(t3); // R32_FLOAT

// Light buffers
struct LIGHT
{
    bool Enable;
    bool3 Dummy;
    float4 Direction;
    float4 Diffuse;
    float4 Ambient;
};
cbuffer LightBuffer : register(b4)
{
    LIGHT Light;
}

#define MAX_SPOT_LIGHT 8
struct SpotLightGPU
{
    float4 Position;
    float4 Direction;
    float4 Color;
    float4 Params1; // range, innerCos, outerCos, intensity
    float4 Params2; // enabled, near, ...
};
cbuffer SpotLightBuffer : register(b6)
{
    SpotLightGPU SpotLights[MAX_SPOT_LIGHT];
    int SpotCount;
    float3 _SpotPad;
}

// Deferred CB
cbuffer CBDeferred : register(b9)
{
    matrix InvViewT;
    matrix InvProjT;
    float4 CameraWorldPos;
    float4 Screen;
}

struct VS_OUT
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

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
