SamplerState Samp : register(s0);

SamplerComparisonState ShadowCmp : register(s1);

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

#define MAX_SPOT_LIGHT 128
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

// 深度を元にワールド座標を復元
float3 WorldPosFromDepth(float2 uv, float depth01)
{
    // 正規デバイス座標に変換
    float2 ndc = uv * 2.0 - 1.0;
    ndc.y = -ndc.y;
    float4 clip = float4(ndc, depth01, 1.0);

    // クリップ空間→ビュー空間
    float4 view = mul(clip, InvProjT);
    view.xyz /= max(view.w, 1e-6);

    // ビュー空間→ワールド空間
    float4 w = mul(float4(view.xyz, 1.0), InvViewT);
    return w.xyz;
}

// ShadowMap
Texture2D ShadowMapTex : register(t4);
Texture2D SpotAccumTex : register(t5);

// Shadow CB（b10）
cbuffer CBShadow : register(b10)
{
    matrix LightViewProjT; // row-vector運用（Transpose済み）
    float4 ShadowTexel; // x=1/width, y=1/height, z=width, w=height
    float4 ShadowParams; // x=bias, y=normalBias, z=pcfRadius(0..2), w=unused
}

// 光源計算して影の境界をぼかす
float ShadowPCF(float3 worldPos, float3 N)
{
    float3 wp = worldPos + N * ShadowParams.y;
    
    // 光空間へ
    float4 lc = mul(float4(wp, 1.0), LightViewProjT);

    // wが0に近いのは無視
    if (abs(lc.w) < 1e-6)
        return 1.0;

    float3 ndc = lc.xyz / lc.w;

    // NDC->UV（DXはz=0..1）
    float2 suv = ndc.xy * 0.5 + 0.5;
    suv.y = 1.0 - suv.y;
    
    // 範囲外は影なし扱い
    if (suv.x < 0 || suv.x > 1 || suv.y < 0 || suv.y > 1)
        return 1.0;

    float depth = ndc.z;
    if (depth < 0 || depth > 1)
        return 1.0;

    // バイアス（定数 + 法線で少し増やす）
    float bias = ShadowParams.x;
    float normalBias = ShadowParams.y;
    // 光の向き（光が飛んでくる向きが Light.Direction なので、面→光は -Light.Direction）
    float3 L = normalize(-Light.Direction.xyz);
    float ndl = saturate(dot(N, L));
    float b = bias + (1.0 - ndl) * normalBias;

    // PCF
    float2 texel = ShadowTexel.xy;
    int r = (int) ShadowParams.z; // 0..2 くらい

    float lit = 0.0;
    int count = 0;

    [loop]
    for (int y = -r; y <= r; ++y)
    {
        [loop]
        for (int x = -r; x <= r; ++x)
        {
            float2 uv = suv + float2(x, y) * texel;
            if (uv.x < 0 || uv.x > 1 || uv.y < 0 || uv.y > 1)
            {
                lit += 1.0;
                count++;
                continue;
            }

            // 比較サンプル：戻り値は 0..1（影=0、明るい=1）
            lit += ShadowMapTex.SampleCmpLevelZero(ShadowCmp, uv, depth - b);
            count++;
        }
    }

    return (count > 0) ? (lit / count) : 1.0;
}
