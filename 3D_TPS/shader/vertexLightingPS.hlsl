#include "common.hlsl"

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

float3 ApplySpotLights(float3 worldPos, float3 normalW)
{
    float3 N = normalize(normalW);
    float3 sum = 0.0f;

    [loop]
    for (int i = 0; i < SpotCount; ++i)
    {
        SpotLightGPU s = SpotLights[i];
        if (s.Params2.x < 0.5f)
            continue;

        float3 P = s.Position.xyz;
        float3 D = normalize(s.Direction.xyz);

        float range = s.Params1.x;
        float innerCos = s.Params1.y;
        float outerCos = s.Params1.z;
        float intensity = s.Params1.w;

        // ライト→点
        float3 Lvec = worldPos - P;
        float dist = length(Lvec);
        if (dist > range)
            continue;

        float3 L = Lvec / max(dist, 1e-6);

        // 角度：ライトの向き D と、ライト→点 L が一致するほど1
        float cosAng = dot(L, D);

        // outer以下0、inner以上1
        float spot = saturate((cosAng - outerCos) / max(innerCos - outerCos, 1e-6));

        // 距離減衰（簡易）
        float att = saturate(1.0f - dist / range);

        // 拡散反射：L はライト→点なので、点→ライトは -L
        float ndotl = saturate(dot(N, -L));

        sum += s.Color.rgb * (ndotl * spot * att * intensity);
    }

    return sum;
}


float4 main(in PS_IN In) : SV_Target
{
    // ----------------------------
    // 1) 基本色（テクスチャ or 頂点色）
    // ----------------------------
    float4 base;
    if (Material.TextureEnable)
    {
        base = g_Texture.Sample(g_SamplerState, In.TexCoord) * In.Diffuse;
    }
    else
    {
        base = In.Diffuse;
    }

    // ----------------------------
    // 2) 平行光（既存と同じ考え方）
    // ----------------------------
    float3 N = normalize(In.NormalW);

    float3 lightTerm = 1.0f; // Light.Enable=false のとき用
    if (Light.Enable)
    {
        float ndl = saturate(-dot(Light.Direction.xyz, N)); // 既存に合わせて -dot
        lightTerm = Light.Ambient.rgb + ndl * Light.Diffuse.rgb;
    }

    // ----------------------------
    // 3) スポットライト加算
    // ----------------------------
    float3 spot = ApplySpotLights(In.WorldPos, N);

    // ----------------------------
    // 4) 合成
    // base に光を掛け、発光は最後に足す
    // ----------------------------
    float3 rgb = base.rgb * (lightTerm + spot) + Material.Emission.rgb;

    return float4(rgb, base.a);
}

