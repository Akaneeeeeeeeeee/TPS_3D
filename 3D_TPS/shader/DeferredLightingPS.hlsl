#include "DeferredLighting_Common.hlsli"

float4 main(VS_OUT pin) : SV_Target
{
    float2 uv = saturate(pin.uv);

    float4 al = GAlbedo.SampleLevel(Samp, uv, 0);
    float4 nr = GNormalR.SampleLevel(Samp, uv, 0);
    float4 em = GEmissive.SampleLevel(Samp, uv, 0);

    float depth01 = DepthTex.SampleLevel(Samp, uv, 0).r;

    // depthが1に近い＝何も描かれてない → α=0で下地（空）を残す
    if (depth01 >= 0.99999)
        return float4(0, 0, 0, 0);

    float3 N = normalize(nr.xyz * 2.0 - 1.0);

    float3 worldPos = WorldPosFromDepth(uv, depth01);

    // directional
    float3 Ld = normalize(-Light.Direction.xyz);
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
        dist01 *= dist01;

        float3 Ls = normalize(lp - worldPos);
        float ndlS = saturate(dot(N, Ls));
        float atten = angle01 * dist01 * intensity;

        ndlS = max(ndlS, 0.2); // あなたのテスト値
        atten *= ndlS;

        col += al.rgb * (s.Color.rgb * atten);
    }

    // emission add
    col += em.rgb;

    return float4(col, 1.0);
}
