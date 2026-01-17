#include "EngineSystem.h"

#include "system/CDirectInput.h"
// #include "system/Framework/SoundManager/SoundManager.h"  // 置くならここ

#include "commontypes.h"

void EngineSystems::Init()
{
    if (m_Inited) return;
    m_Inited = true;

    //m_Graphics.Init();

    m_Shader.Init();
    //m_Asset.Init();
	m_Asset.GetInstance().Init();

    m_Render.Init(&m_Graphics);

    m_Physics.Init();

#ifdef _DEBUG
	m_Physics.SetCameraManager(&m_Camera);
#endif
    m_Weather.Init();

    // LightSystem が Physics を参照するならここで1回だけ配線
    m_Light.SetPhysics(&m_Physics);
    m_Light.SetOcclusionEnabled(false);

    // 必要なら
    // m_camera.Init();
}

void EngineSystems::BeginFrame(float dt)
{
    (void)dt;

    // 入力更新
    CDirectInput::GetInstance().Update();

    // サウンド BeginFrame を統一したいならここへ寄せる
    // SoundManager::GetInstance().BeginFrame();
}

void EngineSystems::UpdateFrame(float dt)
{
	// 物理更新
    m_Physics.Update(dt);
    // 衝突イベントだけメインスレッドで発火
    m_Physics.DispatchCollisionEvents();

	// 天候更新
    m_Weather.Update(dt);

    // Weather に view/proj を渡す（あなたの現状の移植）
    {
        auto* cam = m_Camera.GetMain();
        Matrix4x4 view = Matrix4x4::Identity;
        Matrix4x4 proj = Matrix4x4::Identity;

        if (cam)
        {
            view = cam->GetViewMatrix();
            proj = cam->GetProjMatrix();
        }
        m_Weather.SetViewProjMatrices(view, proj);
    }

    m_Light.UpdateCache();
    m_Light.UploadToGPU();
}

void EngineSystems::EndFrame()
{
}

void EngineSystems::Uninit()
{
    m_Inited = false;
}
