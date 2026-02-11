#include "EngineSystem.h"
#include "system/CDirectInput.h"
#include "commontypes.h"

void EngineSystems::Init()
{
    if (m_Inited) return;
    m_Inited = true;

    //m_Shader.Init();
    //m_Asset.Init();
	m_Asset.GetInstance().Init();

    m_Render.Init(&m_Graphics, &m_Light, &m_Weather);

    m_Physics.Init();

#ifdef _DEBUG
	m_Physics.SetCameraManager(&m_Camera);
#endif
    m_Weather.Init();

    // LightSystem が Physics を参照するならここで1回だけ配線
    m_Light.SetPhysics(&m_Physics);
    m_Light.SetOcclusionEnabled(false);

    // --- Sound ---
	m_Sound.Init(&m_Weather);
}

void EngineSystems::BeginFrame(float dt)
{   
	m_LastDt = dt;
    // 入力更新
    CDirectInput::GetInstance().Update();

    // サウンドイベントバッファをクリア
    m_Sound.BeginFrame(dt);
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

    // リスナー位置（カメラ）を毎フレーム渡す
    // listenerPos（カメラ）
    Vector3 listenerPos = Vector3::Zero;
    if (auto* cam = m_Camera.GetMain())
    {
        //listenerPos = cam->GetPosition();   // カメラの位置基準
		listenerPos = cam->GetCollisionPivot();    // 注視点基準（プレイヤー位置基準にしたい）
		//listenerPos = cam->GetLookAt();    // 注視点基準（プレイヤー位置基準にしたい）
    }

    m_Sound.UpdateFrame(dt, listenerPos);

    // ライト更新
    m_Light.UpdateCache();
    m_Light.UploadToGPU();
}

void EngineSystems::EndFrame()
{
    // Scene.UpdateでEmitが終わった後に呼ぶ想定
    m_Sound.EndFrame(m_LastDt);
}

void EngineSystems::Uninit()
{
	m_Weather.Uninit();
	m_Sound.Uninit();
    m_Physics.Uninit();
	m_Asset.GetInstance().Uninit();
    m_Render.Uninit();

    Renderer::Uninit();

    m_Inited = false;
}
