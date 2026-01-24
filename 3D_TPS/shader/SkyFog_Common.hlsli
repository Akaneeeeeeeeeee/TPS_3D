SamplerState Samp : register(s0);

// resources
Texture3D NoiseSky : register(t0);

Texture2D DepthLowIn : register(t1);
Texture3D NoiseFogL : register(t2);

Texture2D FogLowTex : register(t3);
Texture2D DepthFull : register(t4);
Texture3D NoiseFogF : register(t5);

// Spot
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

// Sky CB b7
cbuffer CBSky : register(b7)
{
    float4 gSkyZenith;
    float4 gSkyHorizon;

    float4 gSkySunDir_Size;
    float4 gSkySunColor_Glow;

    float4 gSkyFogColor_Blend;
    float4 gSkyVignette;

    float4 gSkyCloud;
    float4 gSkyCloudSpeed;

    matrix gSkyInvViewT;
    matrix gSkyInvProjT;

    float4 gSkyScreen;
}

// Fog CB b8
cbuffer CBFog : register(b8)
{
    float4 gFogColor_Density; // rgb, a=density
    float4 gFogLightDir; // xyz
    float4 gFogParams; // x=nearSteps, y=farSwitchDist, z=maxDistVol, w=noiseStr
    float4 gFogCameraWorldPos; // xyz

    matrix gFogInvViewT;
    matrix gFogInvProjT;

    float4 gFogScreen;

    float4 gFogDist; // (start,end,power,strength)
    float4 gFogVolDist; // (start,end,power,strength)
    float4 gBeamParams; // (beamMaxDist, stepLenWanted, kBeam, beamTint)
};

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

// row-vector運用：mul(v, M)
float3 SkyWorldDirFromUV(float2 uv)
{
    float2 ndc = uv * 2.0 - 1.0;
    ndc.y = -ndc.y;

    float4 clip = float4(ndc, 1.0, 1.0);
    float4 view = mul(clip, gSkyInvProjT);
    view.xyz /= max(view.w, 1e-6);

    float3 wdir = mul(float4(view.xyz, 0.0), gSkyInvViewT).xyz;
    return normalize(wdir);
}

// Fog用：空でも使う
float3 FogWorldDirFromUV(float2 uv)
{
    float2 ndc = uv * 2.0 - 1.0;
    ndc.y = -ndc.y;

    float4 clip = float4(ndc, 1.0, 1.0);
    float4 view = mul(clip, gFogInvProjT);
    view.xyz /= max(view.w, 1e-6);

    float3 wdir = mul(float4(view.xyz, 0.0), gFogInvViewT).xyz;
    return normalize(wdir);
}

float3 FogWorldPosFromDepth(float2 uv, float depth01)
{
    float2 ndc = uv * 2.0 - 1.0;
    ndc.y = -ndc.y;

    float4 clip = float4(ndc, depth01, 1.0);
    float4 view = mul(clip, gFogInvProjT);
    view.xyz /= max(view.w, 1e-6);

    float3 wp = mul(float4(view.xyz, 1.0), gFogInvViewT).xyz;
    return wp;
}

float FogFactorFromParams(float dist, float4 p)
{
    float t = saturate((dist - p.x) / max(p.y - p.x, 1e-6));
    t = pow(t, max(p.z, 1e-3));
    return t * saturate(p.w);
}

float SpotCone01(float3 p, SpotLightGPU s)
{
    float3 lp = s.Position.xyz;
    float3 sd = normalize(s.Direction.xyz);

    float3 toP = p - lp;
    float dist = length(toP);

    float range = s.Params1.x;
    float innerCos = s.Params1.y;
    float outerCos = s.Params1.z;
    float nearD = s.Params2.y;

    if (dist <= nearD || dist > range)
        return 0.0;

    float3 dirTo = toP / max(dist, 1e-6);
    float cosAng = dot(dirTo, sd);
    if (cosAng < outerCos)
        return 0.0;

    float angle01 = saturate((cosAng - outerCos) / max(innerCos - outerCos, 1e-6));

    // 柱が途中で消えにくいように
    float dist01 = saturate(1.0 - (dist - nearD) / max(range - nearD, 1e-6));
    dist01 = sqrt(dist01);

    return angle01 * dist01;
}

float4 FogCompute(float2 uv, Texture2D depthTex, Texture3D noiseTex, bool allowNear)
{
    float depth01 = depthTex.SampleLevel(Samp, uv, 0).r;

    float3 fogColor = gFogColor_Density.rgb;

    float baseDensity = gFogColor_Density.a;
    float nearSteps = max(1.0, gFogParams.x);
    float farSwitch = gFogParams.y;
    float fogMaxDist = gFogParams.z;
    float noiseStr = gFogParams.w;

    float beamMaxDist = max(gBeamParams.x, 1.0);
    float beamStepLenWanted = max(gBeamParams.y, 1.0);
    float kBeam = max(gBeamParams.z, 0.0);
    float beamTint = saturate(gBeamParams.w);

    float3 camW = gFogCameraWorldPos.xyz;

    bool isSky = (depth01 >= 0.99999);

    float3 dir;
    float distRaw;
    float distVolFog;
    float distVolBeam;

    if (isSky)
    {
        dir = FogWorldDirFromUV(uv);
        distRaw = max(gFogDist.y, 1.0);
        distVolFog = max(fogMaxDist, 1.0);
        distVolBeam = max(beamMaxDist, 1.0);
    }
    else
    {
        float3 wp = FogWorldPosFromDepth(uv, depth01);
        float3 v = wp - camW;

        distRaw = length(v);
        if (distRaw <= 1e-6)
            return float4(0, 0, 0, 0);

        dir = v / distRaw;
        distVolFog = min(distRaw, fogMaxDist);
        distVolBeam = min(distRaw, beamMaxDist);
    }

    // distance fog
    float distA = FogFactorFromParams(distRaw, gFogDist);
    float3 distRgb = fogColor;

    // volumetric fog distance fade
    float volFade = FogFactorFromParams(distRaw, gFogVolDist);
    float density = baseDensity * volFade;

    float volA = 0.0;
    float3 volRgb = 0.0;

    bool useRaymarch = (!isSky) && allowNear && (distVolFog <= farSwitch);

    if (!useRaymarch)
    {
        volA = 1.0 - exp(-density * distVolFog);
        float phase = 0.5 + 0.5 * saturate(dot(normalize(gFogLightDir.xyz), -dir));
        volRgb = fogColor * phase;
    }
    else
    {
        int steps = (int) nearSteps;
        float stepLen = distVolFog / steps;

        float3 sumPremul = 0.0;
        float trans = 1.0;

        [loop]
        for (int i = 0; i < steps; ++i)
        {
            float tt = (i + 0.5) * stepLen;
            float3 p = camW + dir * tt;

            float n = NoiseFBM(p * 0.02, noiseTex);
            float d = density * lerp(1.0, n * 2.0, noiseStr);

            float aStep = 1.0 - exp(-d * stepLen);

            float phase = 0.5 + 0.5 * saturate(dot(normalize(gFogLightDir.xyz), -dir));
            float3 colStep = fogColor * phase;

            sumPremul += trans * colStep * aStep;
            trans *= (1.0 - aStep);
            if (trans < 0.02)
                break;
        }

        volA = 1.0 - trans;
        volRgb = (volA > 1e-6) ? (sumPremul / volA) : 0.0;
    }

    // beam
    float beamA = 0.0;
    float3 beamRgb = 0.0;

    if (SpotCount > 0 && distVolBeam > 1e-3 && kBeam > 0.0)
    {
        float jitter = noiseTex.SampleLevel(Samp, float3(uv * 64.0, 0.5), 0).r;

        int stepsB = (int) ceil(distVolBeam / beamStepLenWanted);
        stepsB = clamp(stepsB, 8, 512);
        float stepLenB = distVolBeam / stepsB;

        float beamEnergy = 0.0;
        float3 beamColorPremul = 0.0;

        [loop]
        for (int i = 0; i < stepsB; ++i)
        {
            float tt = (i + jitter) * stepLenB;
            float3 p = camW + dir * tt;

            [loop]
            for (int si = 0; si < SpotCount; ++si)
            {
                SpotLightGPU s = SpotLights[si];
                if (s.Params2.x < 0.5)
                    continue;

                float cone = SpotCone01(p, s);
                if (cone <= 0.0)
                    continue;

                float I = s.Params1.w;

                float w = cone * I * stepLenB;
                beamEnergy += w;
                beamColorPremul += s.Color.rgb * w;
            }
        }

        beamA = saturate(1.0 - exp(-kBeam * beamEnergy));
        if (beamEnergy > 1e-6)
            beamRgb = (beamColorPremul / beamEnergy) * beamTint;
    }

    // final combine
    volA = saturate(volA);
    distA = saturate(distA);
    beamA = saturate(beamA);

    float outA = 1.0 - (1.0 - volA) * (1.0 - distA) * (1.0 - beamA);

    float3 premul =
        volRgb * volA +
        distRgb * distA * (1.0 - volA) +
        beamRgb * beamA * (1.0 - volA) * (1.0 - distA);

    float3 outRgb = (outA > 1e-6) ? (premul / outA) : 0.0;
    return float4(outRgb, outA);
}

