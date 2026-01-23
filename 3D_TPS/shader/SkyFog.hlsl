SamplerState Samp : register(s0);

// ---------- Resources (unique registers) ----------
Texture3D NoiseSky : register(t0); // sky noise

Texture2D DepthLowIn : register(t1); // fog low: depth SRV
Texture3D NoiseFogL : register(t2); // fog low: noise

Texture2D FogLowTex : register(t3); // fog composite: low-res fog
Texture2D DepthFull : register(t4); // fog composite: depth SRV
Texture3D NoiseFogF : register(t5); // fog composite: noise

// ---------- CB b6: Light ----------
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
    float4 gFogColor_Density; // rgb, a=density(体積フォグの基本密度)
    float4 gFogLightDir; // xyz (光が飛んでくる向き)
    float4 gFogParams; // x=nearSteps, y=farSwitchDist, z=maxDistVol, w=noiseStr
    float4 gFogCameraWorldPos; // xyz

    matrix gFogInvViewT;
    matrix gFogInvProjT;

    float4 gFogScreen; // xy screen

    // 距離フォグ（画面全体のフェード）
    // x=startDist, y=endDist, z=power, w=strength(0..1)
    float4 gFogDist;

    // 体積フォグの「距離フェード」（近距離は薄く、遠距離で本来の密度へ）
    // x=startDist, y=endDist, z=power, w=strength(0..1)
    float4 gFogVolDist;
    
    float4 gBeamParams; // x beamMaxDist, y beamStepLenWanted, z kBeam, w beamTint
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
    float2 saturated_uv = saturate(uv);
    float3 wdir = SkyWorldDirFromUV(saturated_uv); // uvをこっちに渡す
    
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

// ---------- Spot Light Cone ----------
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
    // 空中は “二乗しない/平方根にする” などで途中の見えを確保
    float dist01 = saturate(1.0 - (dist - nearD) / max(range - nearD, 1e-6));
    dist01 = sqrt(dist01); // ここが肝（強すぎるなら消してOK）

    return angle01 * dist01;
}

// ---------- Fog ----------
float3 FogWorldPosFromDepth(float2 uv, float depth01)
{
    float2 ndc = uv * 2.0 - 1.0;
    ndc.y = -ndc.y;

    float4 clip = float4(ndc, depth01, 1.0);
    // Transpose 済み前提：row-vector
    float4 view = mul(clip, gFogInvProjT);
    view.xyz /= max(view.w, 1e-6);

    float4 w = mul(float4(view.xyz, 1.0), gFogInvViewT);
    return w.xyz;
}

float4 ApplyDistanceFog(float3 fogRgb, float fogA, float dist)
{
    // strength付きの距離フォグ係数
    float a = saturate((dist - gFogDist.x) / max(gFogDist.y - gFogDist.x, 1e-6));
    a = pow(a, max(gFogDist.z, 1e-3));
    a *= saturate(gFogDist.w);

    // fogRgb/fogA に “距離フォグ” を足す（濃い方が勝つ合成）
    // outA = 1 - (1-a0)(1-a1)
    float outA = 1.0 - (1.0 - fogA) * (1.0 - a);

    // RGBは「まだ残っている透過分(1-fogA)」に距離フォグ色を足す
    float3 outRgb = fogRgb + gFogColor_Density.rgb * a * (1.0 - fogA);

    return float4(outRgb, outA);
}


float DistanceFogAlpha(float dist)
{
    float a = saturate((dist - gFogDist.x) / max(gFogDist.y - gFogDist.x, 1e-6));
    a = pow(a, max(gFogDist.z, 1e-3));
    return a * saturate(gFogDist.w);
}

float FogFactorLinear(float dist)
{
    // x=start, y=end, z=power, w=strength
    float t = saturate((dist - gFogDist.x) / max(gFogDist.y - gFogDist.x, 1e-6));
    t = pow(t, max(gFogDist.z, 1e-3));
    return t * saturate(gFogDist.w);
}

float4 CombineFog(float3 volColor, float volA, float3 distColor, float distA)
{
    volA = saturate(volA);
    distA = saturate(distA);

    // 透過率の合成： (1-a0)(1-a1)
    float outA = 1.0 - (1.0 - volA) * (1.0 - distA);

    // 色は「残り(1-volA)」に距離フォグを乗せる
    float3 premul =
        volColor * volA +
        distColor * distA * (1.0 - volA);

    float3 outColor = (outA > 1e-6) ? (premul / outA) : 0.0;
    return float4(outColor, outA);
}


float FogFactorFromParams(float dist, float4 p)
{
    // p = (start, end, power, strength)
    float t = saturate((dist - p.x) / max(p.y - p.x, 1e-6));
    t = pow(t, max(p.z, 1e-3));
    return t * saturate(p.w);
}

// Fog用：uvから「ワールド方向レイ」を作る（depth=1の空でも使う）
float3 FogWorldDirFromUV(float2 uv)
{
    float2 ndc = uv * 2.0 - 1.0;
    ndc.y = -ndc.y;

    float4 clip = float4(ndc, 1.0, 1.0);

    // row-vector運用（Transpose済み）に合わせる
    float4 view = mul(clip, gFogInvProjT);
    view.xyz /= max(view.w, 1e-6);

    float3 wdir = mul(float4(view.xyz, 0.0), gFogInvViewT).xyz;
    return normalize(wdir);
}

float4 FogCompute(float2 uv, Texture2D depthTex, Texture3D noiseTex, bool allowNear)
{
    float depth01 = depthTex.SampleLevel(Samp, uv, 0).r;

    // -------------------------
    // 0) CBからパラメータ取得
    // -------------------------
    float3 fogColor = gFogColor_Density.rgb;

    float baseDensity = gFogColor_Density.a; // 体積フォグの基準密度
    float nearSteps = max(1.0, gFogParams.x);
    float farSwitch = gFogParams.y;
    float fogMaxDist = gFogParams.z; // 体積フォグの最大距離（霧）
    float noiseStr = gFogParams.w;

    float beamMaxDist = max(gBeamParams.x, 1.0);
    float beamStepLenWanted = max(gBeamParams.y, 1.0);
    float kBeam = max(gBeamParams.z, 0.0);
    float beamTint = saturate(gBeamParams.w);

    float3 camW = gFogCameraWorldPos.xyz;

    // -------------------------
    // 1) レイ方向と距離（空でも成立）
    // -------------------------
    bool isSky = (depth01 >= 0.99999);

    float3 dir;
    float distRaw; // 距離フォグ用（長めでもOK）
    float distVolFog; // 体積フォグ用（霧の上限）
    float distVolBeam; // 柱用（柱だけの上限）

    if (isSky)
    {
        dir = FogWorldDirFromUV(uv);

        // 距離フォグは endDist まで届かせる
        distRaw = max(gFogDist.y, 1.0);

        // 霧は fogMaxDist まで
        distVolFog = max(fogMaxDist, 1.0);

        // 柱は beamMaxDist まで
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

    // -------------------------
    // 2) 距離フォグ（画面全体のフェード）
    // -------------------------
    float distA = FogFactorFromParams(distRaw, gFogDist);
    float3 distRgb = fogColor;

    // -------------------------
    // 3) 体積フォグ（霧）
    //    体積フォグは「距離フェード」を掛けて、近距離を薄くする
    // -------------------------
    float volFade = FogFactorFromParams(distRaw, gFogVolDist); // 0..1
    float density = baseDensity * volFade; // ここで距離フェード

    float volA = 0.0;
    float3 volRgb = 0.0;

    // 空は解析式に寄せる（コスト/安定）
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

    // -------------------------
    // 4) 柱（スポットのボリューム光）
    //    霧の密度(density)には掛けない：霧が薄くても柱は見せたい
    // -------------------------
    float beamA = 0.0;
    float3 beamRgb = 0.0;

    if (SpotCount > 0 && distVolBeam > 1e-3 && kBeam > 0.0)
    {
        // 1ピクセル内で一定のジッター（輪切り軽減）
        float jitter = noiseTex.SampleLevel(Samp, float3(uv * 64.0, 0.5), 0).r; // 0..1

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

                float cone = SpotCone01(p, s); // 0..1
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

    // -------------------------
    // 5) 最終合成（vol / dist / beam）
    // -------------------------
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

float4 PS_FogLow(float4 pos : SV_Position, float2 uvs : TEXCOORD0) : SV_Target
{
    float2 uv = saturate(uvs);
    return FogCompute(uv, DepthLowIn, NoiseFogL, /*allowNear=*/true);
}

float4 PS_FogComposite(float4 pos : SV_Position, float2 uvs : TEXCOORD0) : SV_Target
{
    float2 uv = saturate(uvs);

    float4 low = FogLowTex.SampleLevel(Samp, uv, 0);

    float2 px = float2(1.0 / gFogScreen.x, 1.0 / gFogScreen.y);
    float d0 = DepthFull.SampleLevel(Samp, uv, 0).r;
    float dx = DepthFull.SampleLevel(Samp, uv + float2(px.x, 0), 0).r;
    float dy = DepthFull.SampleLevel(Samp, uv + float2(0, px.y), 0).r;
    float edge = max(abs(d0 - dx), abs(d0 - dy));

    if (edge > 0.002)
        return FogCompute(uv, DepthFull, NoiseFogF, /*allowNear=*/true);

    return low;
}
