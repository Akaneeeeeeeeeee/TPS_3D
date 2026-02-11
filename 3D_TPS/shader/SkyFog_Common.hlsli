SamplerState Samp : register(s0);

// resources
Texture3D NoiseSky : register(t0);

Texture2D DepthLowIn : register(t1);
Texture3D NoiseFogL : register(t2);

Texture2D FogLowTex : register(t3);
Texture2D DepthFull : register(t4);
Texture3D NoiseFogF : register(t5);

// Spot
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
    // 1) 画面UV(0..1) → 正規デバイス座標系(-1..1) に変換
    float2 ndc = uv * 2.0 - 1.0;

    // 2) 画面座標系の上下を合わせる（UVは上が0、NDCは上が+1 などの差を吸収）
    ndc.y = -ndc.y;

    // 3) 正規デバイス座標上の点を「クリップ空間」の位置として作る
    //    z=1 は遠側を指す値として使い、方向ベクトル復元に使う
    float4 clip = float4(ndc, 1.0, 1.0);

    // 4) 逆射影行列で、クリップ空間 → ビュー空間へ戻す
    //    これで「カメラから見た方向」に対応する位置(view)が得られる
    float4 view = mul(clip, gSkyInvProjT);

    // 5) 同次座標の補正（wで割る）
    //    逆変換後の位置を正しいビュー空間の座標に戻す
    view.xyz /= max(view.w, 1e-6);

    // 6) 逆ビュー行列で、ビュー空間 → ワールド空間へ
    //    w=0 を入れて「位置」ではなく「方向」として変換する（平行移動の影響を受けない）
    float3 wdir = mul(float4(view.xyz, 0.0), gSkyInvViewT).xyz;

    // 7) 長さを1に正規化して「ワールド空間の視線方向」を返す
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

// フォグ（距離フォグ＋体積フォグ＋ゴッドレイ）を 1ピクセルぶん計算して返す
// 戻り値: rgb=霧の色, a=霧の濃さ（合成用アルファ）
float4 FogCompute(float2 uv, Texture2D depthTex, Texture3D noiseTex, bool allowNear)
{
    // 深度（0..1）。これで「このピクセルが空か、物体か」を判定する
    float depth01 = depthTex.SampleLevel(Samp, uv, 0).r;

    // 霧の基本色
    float3 fogColor = gFogColor_Density.rgb;

    // 体積フォグの基本密度（天候などで変わる想定）
    float baseDensity = gFogColor_Density.a;

    // 近距離の体積フォグ（レイマーチ）の分割数
    float nearSteps = max(1.0, gFogParams.x);

    // この距離より遠い体積フォグは「簡易式」で済ませる切り替え境界
    float farSwitch = gFogParams.y;

    // 体積フォグを計算する最大距離（無駄な遠距離計算を切る）
    float fogMaxDist = gFogParams.z;

    // ノイズの効き（密度ムラの強さ）
    float noiseStr = gFogParams.w;

    // ゴッドレイ（ビーム）側の制御
    float beamMaxDist = max(gBeamParams.x, 1.0); // ビーム計算の最大距離
    float beamStepLenWanted = max(gBeamParams.y, 1.0); // 1ステップの目標長さ
    float kBeam = max(gBeamParams.z, 0.0); // ビームの濃さ係数（0で無効）
    float beamTint = saturate(gBeamParams.w); // ビーム色の寄せ（調整）

    // カメラのワールド座標
    float3 camW = gFogCameraWorldPos.xyz;

    // 深度がほぼ1なら「空」扱い（遠景。ジオメトリが無い）
    bool isSky = (depth01 >= 0.99999);

    // カメラからそのピクセル方向への「視線方向」と距離を用意する
    float3 dir; // カメラ→ピクセル方向（正規化）
    float distRaw; // カメラからそのピクセルまでの距離（空なら擬似距離）
    float distVolFog; // 体積フォグ計算に使う距離（fogMaxDistで打ち切る）
    float distVolBeam; // ビーム計算に使う距離（beamMaxDistで打ち切る）

    if (isSky)
    {
        // 空は「方向」だけ分かれば十分（遠景なので実際の交点がない）
        dir = FogWorldDirFromUV(uv);

        // 距離フォグは「終点距離」を擬似的に使って空も霧で馴染ませる
        distRaw = max(gFogDist.y, 1.0);

        // 体積フォグ／ビームは最大距離までを上限として扱う
        distVolFog = max(fogMaxDist, 1.0);
        distVolBeam = max(beamMaxDist, 1.0);
    }
    else
    {
        // 物体がある場合：深度からワールド座標の交点を復元して距離を得る
        float3 wp = FogWorldPosFromDepth(uv, depth01);
        float3 v = wp - camW;

        distRaw = length(v);
        if (distRaw <= 1e-6)
            return float4(0, 0, 0, 0);

        // 方向（カメラ→交点）を正規化
        dir = v / distRaw;

        // 計算の上限距離で打ち切り（遠距離で重くならないように）
        distVolFog = min(distRaw, fogMaxDist);
        distVolBeam = min(distRaw, beamMaxDist);
    }

    // -------------------------
    // 1) 距離フォグ（画面全体のフェード）
    // -------------------------
    // distRaw に応じて「霧の濃さ」を決める（0..1）
    float distA = FogFactorFromParams(distRaw, gFogDist);

    // 色は基本的に霧色で塗る（空気遠近）
    float3 distRgb = fogColor;

    // -------------------------
    // 2) 体積フォグ（近距離は高品質、遠距離は軽量化）
    // -------------------------
    // 近距離は薄く、中距離から本来の密度へ…などの距離フェード
    float volFade = FogFactorFromParams(distRaw, gFogVolDist);

    // 最終的な密度
    float density = baseDensity * volFade;

    float volA = 0.0; // 体積フォグのアルファ（濃さ）
    float3 volRgb = 0.0; // 体積フォグの色

    // 「近距離だけレイマーチ」：空はやらない / 近距離許可 / farSwitch内だけ
    bool useRaymarch = (!isSky) && allowNear && (distVolFog <= farSwitch);

    if (!useRaymarch)
    {
        // ---- 遠距離の軽量版 ----
        // 密度×距離から透過率を指数で近似（積分を式で済ませる）
        volA = 1.0 - exp(-density * distVolFog);

        // 光方向との角度で明るさを変える（前方散乱っぽい雰囲気）
        float phase = 0.5 + 0.5 * saturate(dot(normalize(gFogLightDir.xyz), -dir));
        volRgb = fogColor * phase;
    }
    else
    {
        // ---- 近距離の高品質版（レイマーチ）----
        int steps = (int) nearSteps;
        float stepLen = distVolFog / steps;

        float3 sumPremul = 0.0; // 色×濃さ（先に掛けたもの）を加算
        float trans = 1.0; // 透過率（1=何も吸われてない）

        [loop]
        for (int i = 0; i < steps; ++i)
        {
            // このステップのサンプル位置（区間の中央）
            float tt = (i + 0.5) * stepLen;
            float3 p = camW + dir * tt;

            // ノイズで密度ムラを作る（煙っぽさ）
            float n = NoiseFBM(p * 0.02, noiseTex);
            float d = density * lerp(1.0, n * 2.0, noiseStr);

            // この区間で増える濃さ（指数で「吸収」を表現）
            float aStep = 1.0 - exp(-d * stepLen);

            // 光方向との角度で明るさ（簡易の散乱）
            float phase = 0.5 + 0.5 * saturate(dot(normalize(gFogLightDir.xyz), -dir));
            float3 colStep = fogColor * phase;

            // まだ残っている光(trans)に対して、この区間の寄与を足す
            sumPremul += trans * colStep * aStep;

            // 透過率を減らす（霧が濃いほど先が見えなくなる）
            trans *= (1.0 - aStep);

            // ほぼ見えなくなったら打ち切り（高速化）
            if (trans < 0.02)
                break;
        }
        // 全体の濃さと色を確定
        volA = 1.0 - trans;
        volRgb = (volA > 1e-6) ? (sumPremul / volA) : 0.0;
    }

    // -------------------------
    // 3) ゴッドレイ（スポットライト由来のビーム）
    // -------------------------
    float beamA = 0.0;
    float3 beamRgb = 0.0;

    // 有効なライトがあり、距離があり、係数が0より大きい時だけ計算
    if (SpotCount > 0 && distVolBeam > 1e-3 && kBeam > 0.0)
    {
        // バンディング（縞）を減らすためのピクセルごとのズラし
        float jitter = noiseTex.SampleLevel(Samp, float3(uv * 64.0, 0.5), 0).r;

        // 目標ステップ長からステップ数を決め、範囲でクランプ
        int stepsB = (int) ceil(distVolBeam / beamStepLenWanted);
        stepsB = clamp(stepsB, 8, 512);
        float stepLenB = distVolBeam / stepsB;

        float beamEnergy = 0.0; // ビームの“当たり具合”の総量
        float3 beamColorPremul = 0.0; // 色×重みの総和

        [loop]
        for (int i = 0; i < stepsB; ++i)
        {
            // ジッター込みのサンプル位置
            float tt = (i + jitter) * stepLenB;
            float3 p = camW + dir * tt;

            // この点が各スポットライトの円錐内に入るかを調べる
            [loop]
            for (int si = 0; si < SpotCount; ++si)
            {
                SpotLightGPU s = SpotLights[si];
                if (s.Params2.x < 0.5) // 無効ライト
                    continue;

                float cone = SpotCone01(p, s);
                if (cone <= 0.0)
                    continue;

                // ライトの強さ
                float I = s.Params1.w;

                // このサンプル点の寄与（距離積分なので stepLenB を掛ける）
                float w = cone * I * stepLenB;

                beamEnergy += w;
                beamColorPremul += s.Color.rgb * w;
            }
        }

        // 総量からアルファ（濃さ）を指数で決める（増えるほど濃くなる）
        beamA = saturate(1.0 - exp(-kBeam * beamEnergy));

        // 色は寄与の重み平均。最後に色味調整
        if (beamEnergy > 1e-6)
            beamRgb = (beamColorPremul / beamEnergy) * beamTint;
    }

    // -------------------------
    // 4) 最終合成（距離フォグ＋体積フォグ＋ビーム）
    // -------------------------
    // 数値を0..1に揃える
    volA = saturate(volA);
    distA = saturate(distA);
    beamA = saturate(beamA);

    // 「複数の半透明を重ねる」合成：1 - (1-a)(1-b)(1-c)
    float outA = 1.0 - (1.0 - volA) * (1.0 - distA) * (1.0 - beamA);

    // 手前の要素から順に見える分だけ足す（premultiplyで安定）
    float3 premul =
        volRgb * volA +
        distRgb * distA * (1.0 - volA) +
        beamRgb * beamA * (1.0 - volA) * (1.0 - distA);

    // 最終rgb（premultiply→通常色に戻す）
    float3 outRgb = (outA > 1e-6) ? (premul / outA) : 0.0;
    return float4(outRgb, outA);
}


