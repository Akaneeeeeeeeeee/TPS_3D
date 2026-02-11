#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include "CommonTypes.h"
#include "WeatherSystem.h"
#include "CShader.h"

using Microsoft::WRL::ComPtr;

class SkyFogPass
{
public:
    static void Init(ID3D11Device* dev, ID3D11DeviceContext* ctx, int w, int h);
	static void Uninit(void);
    static void Resize(int w, int h);

    static void UpdateFromWeather(const WeatherSystem& ws);

    static void SetBeamSRV(ID3D11ShaderResourceView* srv);
    static void DrawSky(); // スカイドーム不要
    static void DrawFog(); // 低解像度→アップスケール＋穴だけ再計算

    struct SkyFogTuning
    {
        bool  enableSky;
        bool  enableFog;
        float cloudOpacity, cloudTiling, cloudSpeedU, cloudSpeedV;
        float vignetteStrength, vignettePower;
        float cloudToFogBlend;
        float fogMaxDist, fogFarSwitchDist, fogNearSteps, fogNoiseStrength;
    };

    static void SetTuning(const SkyFogTuning& t);

private:
    static void CreateShaders();
    static void CreateCBs();
    static void CreateLowResFogTargets();
    static void CreateNoise3D();

    static void UploadSkyCB();
    static void UploadFogCB();

private:
    static ID3D11Device* sDev;
    static ID3D11DeviceContext* sCtx;
    static int sW, sH;

    // Weather から
    static float   sHours;
    static Vector3 sDirToSun;     // dirToSun（太陽方向）
    static Color   sSunColor;     // lightColor * intensity
    static Vector3 sFogColor;
    static float   sFogDensity;

    // shaders
    static CShader* sShSky;
    static CShader* sShFogLow;
    static CShader* sShFogComposite;

    // CB: b7,b8（既存 b0〜b6 は触らない）
    static ComPtr<ID3D11Buffer> sCBSky; // b7
    static ComPtr<ID3D11Buffer> sCBFog; // b8

    // low-res fog RT
    static int sFogLW, sFogLH;
    static ComPtr<ID3D11Texture2D>          sFogLowTex;
    static ComPtr<ID3D11RenderTargetView>   sFogLowRTV;
    static ComPtr<ID3D11ShaderResourceView> sFogLowSRV;

    // 3D noise（ファイル不要）
    static ComPtr<ID3D11Texture3D>          sNoiseTex;
    static ComPtr<ID3D11ShaderResourceView> sNoiseSRV;

    static ComPtr<ID3D11ShaderResourceView> sBeamSRV;

    static ComPtr<ID3D11SamplerState> sBeamSamp;

    static SkyFogTuning sTuning;
};
