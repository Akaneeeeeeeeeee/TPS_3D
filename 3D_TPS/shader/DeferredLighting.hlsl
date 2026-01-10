SamplerState Samp : register(s0);

// GBuffer
Texture2D GAlbedo : register(t0);
Texture2D GNormalR : register(t1);
Texture2D GEmissive : register(t2);
Texture2D DepthTex : register(t3); // Renderer::DepthSRV (R32_FLOAT)

// Light buffers (既存)
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

struct VSOut
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

VSOut VS_Fullscreen(uint vid : SV_VertexID)
{
    float2 p[3] = { float2(-1, -1), float2(-1, 3), float2(3, -1) };
    float2 uv[3] = { float2(0, 1), float2(0, -1), float2(2, 1) };

    VSOut o;
    o.pos = float4(p[vid], 0, 1);
    o.uv = uv[vid];
    return o;
}

float3 WorldPosFromDepth(float2 uv, float depth01)
{
    float2 ndc = uv * 2.0 - 1.0;
    ndc.y = -ndc.y;
    float4 clip = float4(ndc, depth01, 1.0);

    // あなたは C++ 側で Transpose して送ってる前提なので「行列×ベクトル」
    float4 view = mul(InvProjT, clip);
    view.xyz /= max(view.w, 1e-6);

    float4 w = mul(InvViewT, float4(view.xyz, 1.0));
    return w.xyz;
}

float4 PS_Lighting(float4 pos : SV_Position, float2 uv : TEXCOORD0) : SV_Target
{
    float4 al = GAlbedo.SampleLevel(Samp, uv, 0);
    float4 nr = GNormalR.SampleLevel(Samp, uv, 0);
    float4 em = GEmissive.SampleLevel(Samp, uv, 0);

    float depth01 = DepthTex.SampleLevel(Samp, uv, 0).r;

    float3 N = normalize(nr.xyz * 2.0 - 1.0);
    float rough = saturate(nr.w);

    float3 worldPos = WorldPosFromDepth(uv, depth01);
    float3 V = normalize(CameraWorldPos.xyz - worldPos);

    // directional
    float3 Ld = normalize(-Light.Direction.xyz); // ここは「光線方向」を想定
    float ndl = saturate(dot(N, Ld));
    float3 col = al.rgb * (Light.Ambient.rgb + Light.Diffuse.rgb * ndl);

    // spot lights
    [loop]
    for (int i = 0; i < SpotCount; i++)
    {
        SpotLightGPU s = SpotLights[i];
        if (s.Params2.x < 0.5)
            continue;

        float3 lp = s.Position.xyz;
        float3 sd = normalize(s.Direction.xyz);

        float3 toP = worldPos - lp;
        float dist = length(toP);
        float range = s.Params1.x;
        float innerCos = s.Params1.y;
        float outerCos = s.Params1.z;
        float intensity = s.Params1.w;
        float nearD = s.Params2.y;

        if (dist <= nearD)
            continue;
        if (dist > range)
            continue;

        float3 dirTo = toP / max(dist, 1e-6);
        float cosAng = dot(dirTo, sd);
        if (cosAng < outerCos)
            continue;

        float angle01 = saturate((cosAng - outerCos) / max(innerCos - outerCos, 1e-6));
        float dist01 = 1.0 - (dist - nearD) / max(range - nearD, 1e-6);
        dist01 = saturate(dist01);

        float atten = angle01 * dist01 * intensity;
        col += al.rgb * (s.Color.rgb * atten);
    }

    // emission add
    col += em.rgb;

    return float4(col, 1.0);
}
