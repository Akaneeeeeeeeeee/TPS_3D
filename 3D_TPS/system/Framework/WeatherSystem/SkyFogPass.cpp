#include "SkyFogPass.h"
#include "renderer.h"
#include "Framework/AssetManager/AssetManager.h"
#include <vector>
#include <algorithm>
#include <stdexcept>

ID3D11Device* SkyFogPass::sDev = nullptr;
ID3D11DeviceContext* SkyFogPass::sCtx = nullptr;
int SkyFogPass::sW = 0;
int SkyFogPass::sH = 0;

float   SkyFogPass::sHours = 12.0f;
Vector3 SkyFogPass::sDirToSun = Vector3(0, 1, 0);
Color   SkyFogPass::sSunColor = Color(1, 1, 1, 1);
Vector3 SkyFogPass::sFogColor = Vector3(0.7f, 0.8f, 0.9f);
float   SkyFogPass::sFogDensity = 0.0f;

CShader* SkyFogPass::sShSky = nullptr;
CShader* SkyFogPass::sShFogLow = nullptr;
CShader* SkyFogPass::sShFogComposite = nullptr;

ComPtr<ID3D11Buffer> SkyFogPass::sCBSky;
ComPtr<ID3D11Buffer> SkyFogPass::sCBFog;

int SkyFogPass::sFogLW = 0;
int SkyFogPass::sFogLH = 0;
ComPtr<ID3D11Texture2D>          SkyFogPass::sFogLowTex;
ComPtr<ID3D11RenderTargetView>   SkyFogPass::sFogLowRTV;
ComPtr<ID3D11ShaderResourceView> SkyFogPass::sFogLowSRV;

ComPtr<ID3D11Texture3D>          SkyFogPass::sNoiseTex;
ComPtr<ID3D11ShaderResourceView> SkyFogPass::sNoiseSRV;

static void ThrowIfFailed(HRESULT hr, const char* msg) { if (FAILED(hr)) throw std::runtime_error(msg); }
static UINT Align16(UINT x) { return (x + 15u) & ~15u; }

SkyFogPass::SkyFogTuning SkyFogPass::sTuning{};

void SkyFogPass::SetTuning(const SkyFogTuning& t)
{
    sTuning = t;
}

struct D3D11StateBackup
{
    ID3D11InputLayout* inputLayout = nullptr;
    D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;

    ID3D11VertexShader* vs = nullptr;
    ID3D11PixelShader* ps = nullptr;
    ID3D11GeometryShader* gs = nullptr;
    ID3D11HullShader* hs = nullptr;
    ID3D11DomainShader* ds = nullptr;

    ID3D11RasterizerState* rs = nullptr;

    ID3D11BlendState* blend = nullptr;
    float blendFactor[4]{};
    UINT sampleMask = 0xffffffff;

    ID3D11DepthStencilState* dss = nullptr;
    UINT stencilRef = 0;

    ID3D11RenderTargetView* rtv = nullptr;
    ID3D11DepthStencilView* dsv = nullptr;

    UINT numViewports = 1;
    D3D11_VIEWPORT viewport{};

    void Capture(ID3D11DeviceContext* ctx)
    {
        ctx->IAGetInputLayout(&inputLayout);
        ctx->IAGetPrimitiveTopology(&topology);

        ctx->VSGetShader(&vs, nullptr, nullptr);
        ctx->PSGetShader(&ps, nullptr, nullptr);
        ctx->GSGetShader(&gs, nullptr, nullptr);
        ctx->HSGetShader(&hs, nullptr, nullptr);
        ctx->DSGetShader(&ds, nullptr, nullptr);

        ctx->RSGetState(&rs);

        ctx->OMGetBlendState(&blend, blendFactor, &sampleMask);
        ctx->OMGetDepthStencilState(&dss, &stencilRef);

        ctx->OMGetRenderTargets(1, &rtv, &dsv);

        ctx->RSGetViewports(&numViewports, &viewport);
    }

    void Restore(ID3D11DeviceContext* ctx)
    {
        ctx->IASetInputLayout(inputLayout);
        ctx->IASetPrimitiveTopology(topology);

        ctx->VSSetShader(vs, nullptr, 0);
        ctx->PSSetShader(ps, nullptr, 0);
        ctx->GSSetShader(gs, nullptr, 0);
        ctx->HSSetShader(hs, nullptr, 0);
        ctx->DSSetShader(ds, nullptr, 0);

        ctx->RSSetState(rs);

        ctx->OMSetBlendState(blend, blendFactor, sampleMask);
        ctx->OMSetDepthStencilState(dss, stencilRef);

        ctx->OMSetRenderTargets(1, &rtv, dsv);

        ctx->RSSetViewports(numViewports, &viewport);
    }

    void Release()
    {
        if (inputLayout) inputLayout->Release();
        if (vs) vs->Release();
        if (ps) ps->Release();
        if (gs) gs->Release();
        if (hs) hs->Release();
        if (ds) ds->Release();
        if (rs) rs->Release();
        if (blend) blend->Release();
        if (dss) dss->Release();
        if (rtv) rtv->Release();
        if (dsv) dsv->Release();

        inputLayout = nullptr;
        vs = nullptr;
        ps = nullptr;
        gs = nullptr;
        hs = nullptr;
        ds = nullptr;
        rs = nullptr;
        blend = nullptr;
        dss = nullptr;
        rtv = nullptr;
        dsv = nullptr;
    }
};

// ---- CB（HLSLと一致させる）----
struct CBSky
{
    Vector4 Zenith;
    Vector4 Horizon;
    Vector4 SunDir_Size;     // xyz, w=size
    Vector4 SunColor_Glow;   // rgb, a=glow
    Vector4 FogColor_Blend;  // rgb, a=cloud->fog blend
    Vector4 Vignette;        // x=str, y=pow
    Vector4 Cloud;           // x=tiling, y=opacity, z=distort, w=time
    Vector4 CloudSpeed;      // x=u, y=v
    Matrix4x4 InvViewT;
    Matrix4x4 InvProjT;
    Vector4 Screen;          // xy
};

struct CBFog
{
    Vector4 FogColor_Density;   // rgb, a=density
    Vector4 LightDir;           // xyz（光が飛んでくる向き）
    Vector4 Params;             // x=nearSteps, y=farSwitchDist, z=maxDist, w=noiseStr
    Vector4 CameraWorldPos;     // xyz
    Matrix4x4 InvViewT;
    Matrix4x4 InvProjT;
    Vector4 Screen;             // xy full
    Vector4 FogDist;            // x=start, y=end, z=power, w=strength
	Vector4 FogVolDist;         // volumetric fog: x=start, y=end
	Vector4 BeamParams;
};

void SkyFogPass::Init(ID3D11Device* dev, ID3D11DeviceContext* ctx, int w, int h)
{
    sDev = dev; sCtx = ctx; sW = w; sH = h;
    CreateShaders();
    CreateCBs();
    CreateLowResFogTargets();
    CreateNoise3D();
}

void SkyFogPass::Resize(int w, int h)
{
    sW = w; sH = h;
    CreateLowResFogTargets();
}

void SkyFogPass::UpdateFromWeather(const WeatherSystem& ws)
{
    sHours = ws.GetSun().timeOfDayHours;

    sDirToSun = ws.GetSun().dirToSun;
    if (sDirToSun.LengthSquared() < 1e-6f) sDirToSun = Vector3(0, 1, 0);
    sDirToSun.Normalize();

    // 太陽色（強さ込み）
    sSunColor = ws.GetSun().lightColor * ws.GetSun().lightIntensity;

    sFogDensity = ws.GetFogDensity();
    sFogColor = ws.GetFogColor();
}

static Vector3 Lerp3(const Vector3& a, const Vector3& b, float t)
{
    return Vector3(
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t
    );
}

void SkyFogPass::UploadSkyCB()
{
    CBSky cb{};

    // 4段階プリセット（値は調整前提）
    float h = sHours;
    while (h < 0) h += 24.0f;
    while (h >= 24) h -= 24.0f;

    int seg = (int)(h / 6.0f);
    int next = (seg + 1) & 3;
    float t = (h - seg * 6.0f) / 6.0f;

    Vector3 zen[4] = {
        {0.02f,0.03f,0.06f}, // night
        {0.20f,0.15f,0.25f}, // dawn
        {0.25f,0.45f,0.85f}, // day
        {0.25f,0.10f,0.18f}, // evening
    };
    Vector3 hor[4] = {
        {0.05f,0.05f,0.08f},
        {0.80f,0.35f,0.20f},
        {0.75f,0.85f,0.95f},
        {0.95f,0.25f,0.15f},
    };

    Vector3 zenith = Lerp3(zen[seg], zen[next], t);
    Vector3 horizon = Lerp3(hor[seg], hor[next], t);

    cb.Zenith = Vector4(zenith.x, zenith.y, zenith.z, 1);
    cb.Horizon = Vector4(horizon.x, horizon.y, horizon.z, 1);

    cb.SunDir_Size = Vector4(sDirToSun.x, sDirToSun.y, sDirToSun.z, 0.02f);
    cb.SunColor_Glow = Vector4(sSunColor.R(), sSunColor.G(), sSunColor.B(), 1.0f);

    // 雲色を fog 色に寄せて馴染ませる
    cb.FogColor_Blend = Vector4(sFogColor.x, sFogColor.y, sFogColor.z, sTuning.cloudToFogBlend);
    // 空だけビネット
    cb.Vignette = Vector4(sTuning.vignetteStrength, sTuning.vignettePower, 0, 0);

    float timeSec = (h / 24.0f) * 86400.0f;
    cb.Cloud = Vector4(sTuning.cloudTiling, sTuning.cloudOpacity, 0.25f, timeSec);
    cb.CloudSpeed = Vector4(sTuning.cloudSpeedU, sTuning.cloudSpeedV, 0, 0);

    // inv(View/Proj)
    Matrix4x4 view = Renderer::GetViewMatrix();
    Matrix4x4 proj = Renderer::GetProjectionMatrix();
    Matrix4x4 invView = view.Invert();
    Matrix4x4 invProj = proj.Invert();

    cb.InvViewT = invView.Transpose();
    cb.InvProjT = invProj.Transpose();
    cb.Screen = Vector4((float)sW, (float)sH, 0, 0);

    sCtx->UpdateSubresource(sCBSky.Get(), 0, nullptr, &cb, 0, 0);
}


void SkyFogPass::UploadFogCB()
{
    CBFog cb{};

    // 体積フォグの基本密度（天候）
    cb.FogColor_Density = Vector4(sFogColor.x, sFogColor.y, sFogColor.z, sFogDensity);

    LIGHT L = Renderer::GetLight();
    cb.LightDir = Vector4(L.Direction.x, L.Direction.y, L.Direction.z, 0);

	// -------------------------
	// 体積フォグパラメータ
	// -------------------------
    cb.Params = Vector4(
        sTuning.fogNearSteps,        // nearSteps
        sTuning.fogFarSwitchDist,    // farSwitchDist
        sTuning.fogMaxDist,          // maxDistVol
        sTuning.fogNoiseStrength     // noiseStr
    );

    cb.BeamParams = Vector4(
        12000.0f,  // beamMaxDist ← 柱はここまで
        20.0f,     // beamStepLenWanted
        0.0008f,   // kBeam
        0.6f       // beamTint
    );

    // -------------------------
    // 距離フォグ（画面全体のフェード）※晴れでも少しだけ欲しいなら base を入れる
    // -------------------------
    cb.FogDist = Vector4(
        /*start*/  600.0f,   // ←ここが「霧がかかり始める距離」
        /*end*/    6000.0f,
        /*power*/  1.2f,
        /*strength*/ 0.15f   // 晴れ基準（0.05～0.25）
    );

    // 天候で足す（LightRain/HeavyRain を濃くしたいなら係数を上げる）
    const float weatherAdd = 0.25f * std::clamp(sFogDensity / 0.02f, 0.0f, 1.0f);
    cb.FogDist.w = std::clamp(cb.FogDist.w + weatherAdd, 0.0f, 1.0f);

    // -------------------------
    // 体積フォグ距離フェード（近距離を薄く）
    // 例：キャラ付近は薄い→中距離から本来の密度へ
    // -------------------------
    cb.FogVolDist = Vector4(
        /*start*/  400.0f,   // ←体積フォグが効き始める距離
        /*end*/    1400.0f,  // ←ここで100%まで到達
        /*power*/  1.0f,
        /*strength*/ 1.0f
    );

    Matrix4x4 view = Renderer::GetViewMatrix();
    Matrix4x4 proj = Renderer::GetProjectionMatrix();
    Matrix4x4 invView = view.Invert();
    Matrix4x4 invProj = proj.Invert();

    Vector3 camPos = invView.Translation();
    cb.CameraWorldPos = Vector4(camPos.x, camPos.y, camPos.z, 0);

    cb.InvViewT = invView.Transpose();
    cb.InvProjT = invProj.Transpose();
    cb.Screen = Vector4((float)sW, (float)sH, 0, 0);

    sCtx->UpdateSubresource(sCBFog.Get(), 0, nullptr, &cb, 0, 0);
}


void SkyFogPass::DrawSky()
{
    D3D11StateBackup bk;
    bk.Capture(sCtx);

    UploadSkyCB();

    // Sky は GS などを一時的に外して描く
    sCtx->GSSetShader(nullptr, nullptr, 0);
    sCtx->HSSetShader(nullptr, nullptr, 0);
    sCtx->DSSetShader(nullptr, nullptr, 0);

    Renderer::SetDepthEnable(false);
    Renderer::SetBlendState(BS_NONE);

    sCtx->IASetInputLayout(nullptr);
    sCtx->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
    sCtx->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
    sCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	sShSky->SetGPU();

    ID3D11Buffer* cb = sCBSky.Get();
    sCtx->VSSetConstantBuffers(7, 1, &cb);
    sCtx->PSSetConstantBuffers(7, 1, &cb);

    ID3D11ShaderResourceView* n = sNoiseSRV.Get();
    sCtx->PSSetShaderResources(0, 1, &n);

    sCtx->Draw(3, 0);

    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    sCtx->PSSetShaderResources(0, 1, nullSRV);

    // ここで元の状態に戻す（地形が出なくなるのを防ぐ）
    bk.Restore(sCtx);
    bk.Release();
}

void SkyFogPass::DrawFog()
{
    UploadFogCB();

    // 現在のRTV/DSVを退避（後で戻す）
    ID3D11RenderTargetView* savedRTV = nullptr;
    ID3D11DepthStencilView* savedDSV = nullptr;
    sCtx->OMGetRenderTargets(1, &savedRTV, &savedDSV);

    // ---- 低解像度 fog 作成（DepthSRV を読むので DSV は必ず外す）----
    {
        ID3D11RenderTargetView* rtv = sFogLowRTV.Get();
        sCtx->OMSetRenderTargets(1, &rtv, nullptr);

        D3D11_VIEWPORT vp{};
        vp.Width = (FLOAT)sFogLW;
        vp.Height = (FLOAT)sFogLH;
        vp.MinDepth = 0;
        vp.MaxDepth = 1;
        sCtx->RSSetViewports(1, &vp);

        float clear[4] = { 0,0,0,0 };
        sCtx->ClearRenderTargetView(sFogLowRTV.Get(), clear);

        sCtx->IASetInputLayout(nullptr);
        sCtx->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
        sCtx->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
        sCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        sCtx->GSSetShader(nullptr, nullptr, 0);
        sCtx->HSSetShader(nullptr, nullptr, 0);
        sCtx->DSSetShader(nullptr, nullptr, 0);

		sShFogLow->SetGPU();

        ID3D11Buffer* cb = sCBFog.Get();
        sCtx->VSSetConstantBuffers(8, 1, &cb);
        sCtx->PSSetConstantBuffers(8, 1, &cb);

        ID3D11ShaderResourceView* depth = Renderer::GetDepthSRV(); // t0
        ID3D11ShaderResourceView* noise = sNoiseSRV.Get();         // t1
        sCtx->PSSetShaderResources(1, 1, &depth);
        sCtx->PSSetShaderResources(2, 1, &noise);

        sCtx->Draw(3, 0);

        ID3D11ShaderResourceView* nulls[2] = { nullptr, nullptr };
        sCtx->PSSetShaderResources(1, 2, nulls);
    }

    // ---- 合成（savedRTV に戻して、fog をアルファブレンド）----
    {
        // ここも DepthSRV を読むので DSV は外す
        sCtx->OMSetRenderTargets(1, &savedRTV, nullptr);

        D3D11_VIEWPORT vp{};
        vp.Width = (FLOAT)sW;
        vp.Height = (FLOAT)sH;
        vp.MinDepth = 0;
        vp.MaxDepth = 1;
        sCtx->RSSetViewports(1, &vp);

        sCtx->IASetInputLayout(nullptr);
        sCtx->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
        sCtx->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
        sCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		sShFogComposite->SetGPU();

        ID3D11Buffer* cb = sCBFog.Get();
        sCtx->VSSetConstantBuffers(8, 1, &cb);
        sCtx->PSSetConstantBuffers(8, 1, &cb);

        ID3D11ShaderResourceView* fogLow = sFogLowSRV.Get();       // t0
        ID3D11ShaderResourceView* depth = Renderer::GetDepthSRV();// t1
        ID3D11ShaderResourceView* noise = sNoiseSRV.Get();        // t2
        sCtx->PSSetShaderResources(3, 1, &fogLow);
        sCtx->PSSetShaderResources(4, 1, &depth);
        sCtx->PSSetShaderResources(5, 1, &noise);

        Renderer::SetDepthEnable(false);
        Renderer::SetBlendState(BS_ALPHABLEND);

        sCtx->Draw(3, 0);

        Renderer::SetBlendState(BS_NONE);
        Renderer::SetDepthEnable(true);

        ID3D11ShaderResourceView* nulls[3] = { nullptr, nullptr, nullptr };
        sCtx->PSSetShaderResources(3, 3, nulls);
    }

    // 退避していたRTV/DSVを戻す（この後のUI/デバッグが壊れない）
    sCtx->OMSetRenderTargets(1, &savedRTV, savedDSV);

    if (savedRTV) savedRTV->Release();
    if (savedDSV) savedDSV->Release();
}

void SkyFogPass::CreateCBs()
{
    D3D11_BUFFER_DESC bd{};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    bd.ByteWidth = Align16(sizeof(CBSky));
    ThrowIfFailed(sDev->CreateBuffer(&bd, nullptr, sCBSky.GetAddressOf()), "Create CBSky failed.");

    bd.ByteWidth = Align16(sizeof(CBFog));
    ThrowIfFailed(sDev->CreateBuffer(&bd, nullptr, sCBFog.GetAddressOf()), "Create CBFog failed.");
}

void SkyFogPass::CreateLowResFogTargets()
{
    sFogLW = std::max(1, sW / 2);
    sFogLH = std::max(1, sH / 2);

    sFogLowTex.Reset();
    sFogLowRTV.Reset();
    sFogLowSRV.Reset();

    D3D11_TEXTURE2D_DESC td{};
    td.Width = sFogLW;
    td.Height = sFogLH;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    ThrowIfFailed(sDev->CreateTexture2D(&td, nullptr, sFogLowTex.GetAddressOf()), "Create fog low tex failed.");
    ThrowIfFailed(sDev->CreateRenderTargetView(sFogLowTex.Get(), nullptr, sFogLowRTV.GetAddressOf()), "Create fog low rtv failed.");
    ThrowIfFailed(sDev->CreateShaderResourceView(sFogLowTex.Get(), nullptr, sFogLowSRV.GetAddressOf()), "Create fog low srv failed.");
}

void SkyFogPass::CreateNoise3D()
{
    const int N = 32;
    std::vector<unsigned char> data(N * N * N);

    unsigned int seed = 1337;
    auto rnd = [&]() { seed = seed * 1664525u + 1013904223u; return (unsigned char)(seed >> 24); };
    for (auto& v : data) v = rnd();

    auto idx = [&](int x, int y, int z) { return z * N * N + y * N + x; };

    for (int pass = 0; pass < 2; ++pass)
    {
        auto tmp = data;
        for (int z = 0; z < N; ++z) for (int y = 0; y < N; ++y) for (int x = 0; x < N; ++x)
        {
            int sum = 0, cnt = 0;
            for (int dz = -1; dz <= 1; ++dz) for (int dy = -1; dy <= 1; ++dy) for (int dx = -1; dx <= 1; ++dx)
            {
                int xx = (x + dx + N) % N;
                int yy = (y + dy + N) % N;
                int zz = (z + dz + N) % N;
                sum += tmp[idx(xx, yy, zz)];
                cnt++;
            }
            data[idx(x, y, z)] = (unsigned char)(sum / cnt);
        }
    }

    D3D11_TEXTURE3D_DESC td{};
    td.Width = N;
    td.Height = N;
    td.Depth = N;
    td.MipLevels = 1;
    td.Format = DXGI_FORMAT_R8_UNORM;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA sd{};
    sd.pSysMem = data.data();
    sd.SysMemPitch = N;
    sd.SysMemSlicePitch = N * N;

    ThrowIfFailed(sDev->CreateTexture3D(&td, &sd, sNoiseTex.GetAddressOf()), "Create noise3D tex failed.");
    ThrowIfFailed(sDev->CreateShaderResourceView(sNoiseTex.Get(), nullptr, sNoiseSRV.GetAddressOf()), "Create noise3D srv failed.");
}

void SkyFogPass::CreateShaders()
{
    auto& am = AssetManager::GetInstance();

    // AssetManager::Init() で登録したキーと一致させる
    sShSky = am.GetShader<CShader>("skyfog_sky");
    sShFogLow = am.GetShader<CShader>("skyfog_foglow");
    sShFogComposite = am.GetShader<CShader>("skyfog_fogcomp");

    if (!sShSky || !sShFogLow || !sShFogComposite)
        throw std::runtime_error("SkyFogPass::CreateShaders: skyfog shaders not found.");
}
