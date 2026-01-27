#include "SkyFog_Common.hlsli"

struct PS_IN
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

float4 main(PS_IN pin) : SV_Target
{
    float2 uv = saturate(pin.uv);
    float3 wdir = SkyWorldDirFromUV(uv);

    float t = saturate(wdir.y * 0.5 + 0.5);
    t = pow(t, 1.2);
    float3 col = lerp(gSkyHorizon.rgb, gSkyZenith.rgb, t);

    // sun
    float3 sdir = normalize(gSkySunDir_Size.xyz);
    float sd = saturate(dot(wdir, sdir));
    float sunDisk = smoothstep(1.0 - gSkySunDir_Size.w, 1.0, sd);
    float sunGlow = pow(sd, 32.0) * gSkySunColor_Glow.a;
    col += gSkySunColor_Glow.rgb * (sunDisk + sunGlow);

    // clouds
    float2 suv = float2(atan2(wdir.z, wdir.x) * (1.0 / 6.2831853) + 0.5, wdir.y * 0.5 + 0.5);
    float2 scroll = gSkyCloudSpeed.xy * gSkyCloud.w;
    float n = NoiseFBM(float3(suv * gSkyCloud.x + scroll, gSkyCloud.w * 0.01), NoiseSky);
    float cloud = smoothstep(0.45, 0.65, n) * gSkyCloud.y;

    float3 cloudCol = lerp(float3(1, 1, 1), gSkyFogColor_Blend.rgb, gSkyFogColor_Blend.a);
    col = lerp(col, col * cloudCol, cloud);

    // vignette
    float2 c = uv - 0.5;
    float r = length(c);
    float vig = 1.0 - saturate(pow(r * 1.4142, gSkyVignette.y) * gSkyVignette.x);
    col *= vig;

    return float4(col, 1.0);
}
