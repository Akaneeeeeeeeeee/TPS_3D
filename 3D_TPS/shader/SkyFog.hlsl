SamplerState Samp : register(s0);

// ---------- Resources (unique registers) ----------
Texture3D NoiseSky : register(t0); // sky noise

Texture2D DepthLowIn : register(t1); // fog low: depth SRV
Texture3D NoiseFogL : register(t2); // fog low: noise

Texture2D FogLowTex : register(t3); // fog composite: low-res fog
Texture2D DepthFull : register(t4); // fog composite: depth SRV
Texture3D NoiseFogF : register(t5); // fog composite: noise

// ---------- CB b7: Sky ----------
cbuffer CBSky : register(b7)
{
    float4 gSkyZenith;
    float4 gSkyHorizon;

    float4 gSkySunDir_Size; // xyz dirToSun, w size
    float4 gSkySunColor_Glow; // rgb, a glow

    float4 gSkyFogColor_Blend; // rgb, a cloud->fog blend
    float4 gSkyVignette; // x strength, y power

    float4 gSkyCloud; // x tiling, y opacity, z distort, w time
    float4 gSkyCloudSpeed; // x u, y v

    matrix gSkyInvViewT;
    matrix gSkyInvProjT;

    float4 gSkyScreen; // xy screen
};

// ---------- CB b8: Fog ----------
cbuffer CBFog : register(b8)
{
    float4 gFogColor_Density; // rgb, a density
    float4 gFogLightDir; // xyz (light direction), w unused
    float4 gFogParams; // x nearSteps, y farSwitchDist, z maxDist, w noiseStr
    float4 gFogCameraWorldPos; // xyz

    matrix gFogInvViewT;
    matrix gFogInvProjT;

    float4 gFogScreen; // xy screen
};

// ---------- Fullscreen triangle ----------
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

float NoiseFBM(float3 p, Texture3D tex)
{
    float n = 0.0;
    float a = 0.5;
    float f = 1.0;
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        n += a * tex.SampleLevel(Samp, frac(p * f), 0).r;
        f *= 2.0;
        a *= 0.5;
    }
    return n;
}

// ---------- Sky ----------
float3 SkyWorldDirFromUV(float2 uv)
{
    float2 ndc = uv * 2.0 - 1.0;
    ndc.y = -ndc.y;

    float4 clip = float4(ndc, 1.0, 1.0);
    float4 view = mul(clip, gSkyInvProjT);
    view.xyz /= max(view.w, 1e-6);

    float4 w = mul(float4(view.xyz, 0.0), gSkyInvViewT);
    return normalize(w.xyz);
}

float4 PS_Sky(float4 pos : SV_Position, float2 uv : TEXCOORD0) : SV_Target
{
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

    // clouds (simple)
    float2 suv = float2(atan2(wdir.z, wdir.x) * (1.0 / 6.2831853) + 0.5, wdir.y * 0.5 + 0.5);
    float2 scroll = gSkyCloudSpeed.xy * gSkyCloud.w;
    float n = NoiseFBM(float3(suv * gSkyCloud.x + scroll, gSkyCloud.w * 0.01), NoiseSky);
    float cloud = smoothstep(0.45, 0.65, n) * gSkyCloud.y;

    float3 cloudCol = lerp(float3(1, 1, 1), gSkyFogColor_Blend.rgb, gSkyFogColor_Blend.a);
    col = lerp(col, col * cloudCol, cloud);

    // sky-only vignette
    float2 c = uv - 0.5;
    float r = length(c);
    float vig = 1.0 - saturate(pow(r * 1.4142, gSkyVignette.y) * gSkyVignette.x);
    col *= vig;

    return float4(col, 1.0);
}

// ---------- Fog ----------
float3 FogWorldPosFromDepth(float2 uv, float depth01)
{
    float2 ndc = uv * 2.0 - 1.0;
    ndc.y = -ndc.y;

    float4 clip = float4(ndc, depth01, 1.0);
    float4 view = mul(gFogInvProjT, clip);
    view.xyz /= max(view.w, 1e-6);

    float4 w = mul(gFogInvViewT, float4(view.xyz, 1.0));
    return w.xyz;
}

float4 FogCompute(float2 uv, Texture2D depthTex, Texture3D noiseTex, bool allowNear)
{
    float depth01 = depthTex.SampleLevel(Samp, uv, 0).r;

    float3 camW = gFogCameraWorldPos.xyz;
    float3 wp = FogWorldPosFromDepth(uv, depth01);

    float3 v = wp - camW;
    float dist = length(v);

    float maxDist = gFogParams.z;
    float farSwitch = gFogParams.y;
    float density = gFogColor_Density.a;
    float noiseStr = gFogParams.w;

    dist = min(dist, maxDist);

    float3 dir = (dist > 1e-6) ? (v / dist) : float3(0, 0, 1);

    // far: analytic
    if (!allowNear || dist > farSwitch)
    {
        float a = 1.0 - exp(-density * dist);
        float phase = 0.5 + 0.5 * saturate(dot(normalize(gFogLightDir.xyz), -dir));
        //float phase = 1.0; // isotropic
        return float4(gFogColor_Density.rgb * phase, a);
    }

    // near: ray-march
    int steps = (int) max(1.0, gFogParams.x);
    float stepLen = dist / steps;

    float3 sum = 0.0;
    float trans = 1.0;

    [loop]
    for (int i = 0; i < steps; ++i)
    {
        float tt = (i + 0.5) * stepLen;
        float3 p = camW + dir * tt;

        float n = NoiseFBM(p * 0.02, noiseTex);
        float d = density * lerp(1.0, n * 2.0, noiseStr);

        float a = 1.0 - exp(-d * stepLen);

        float phase = 0.5 + 0.5 * saturate(dot(normalize(gFogLightDir.xyz), -dir));
        sum += trans * (gFogColor_Density.rgb * phase) * a;

        trans *= (1.0 - a);
        if (trans < 0.02)
            break;
    }

    return float4(sum, 1.0 - trans);
}

float4 PS_FogLow(float4 pos : SV_Position, float2 uv : TEXCOORD0) : SV_Target
{
    return FogCompute(uv, DepthLowIn, NoiseFogL, true);
}

float4 PS_FogComposite(float4 pos : SV_Position, float2 uv : TEXCOORD0) : SV_Target
{
    float4 low = FogLowTex.SampleLevel(Samp, uv, 0);

    // edge detect by depth (recompute only on edges)
    float d0 = DepthFull.SampleLevel(Samp, uv, 0).r;
    float d1 = DepthFull.SampleLevel(Samp, uv + float2(1.0 / gFogScreen.x, 0), 0).r;
    float edge = abs(d0 - d1);

    if (edge > 0.002)
        return FogCompute(uv, DepthFull, NoiseFogF, true);

    return low;
}