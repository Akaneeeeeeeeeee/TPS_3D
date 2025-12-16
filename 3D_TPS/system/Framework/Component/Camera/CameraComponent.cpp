#include "CameraComponent.h"
#include "Framework/EngineContext/EngineContext.h"
#include "system/CPolar3D.h"
#include "system/renderer.h"
#include "system/Framework/Window/Window.h"

using namespace DirectX;

CameraComponent::CameraComponent()
    : m_Position(0.0f, 10.0f, -100.0f)
    , m_LookAt(0.0f, 10.0f, 0.0f)
    , m_Elevation(-90.0f * PI / 180.0f)
    , m_Azimuth(PI / 2.0f)
    , m_Radius(100.0f)
    , m_FovY(DirectX::XMConvertToRadians(45.0f))
    , m_Near(1.0f)
    , m_Far(100000.0f)
{
    m_View = Matrix4x4::Identity;
    m_Proj = Matrix4x4::Identity;
}

void CameraComponent::Attach(EngineContext& context)
{
    // CameraManager に登録する
    m_CameraManager = &context.cameraManager;
    m_CameraManager->Register(this);
}

void CameraComponent::Detach()
{
     if (m_CameraManager)
     {
         m_CameraManager->UnRegister(this);
         m_CameraManager = nullptr;
     }
}

void CameraComponent::Init()
{
    // 現状は特に何もしなくてよい
}

void CameraComponent::Update(const float /*deltaTime*/)
{
    // 今回は Player 側が角度や注視点を更新し、
    // 描画前に ApplyCamera() を呼ぶ設計にしているので、ここでは何もしない。
}

void CameraComponent::Uninit()
{
}

void CameraComponent::SetPerspective(float fovYRad, float nearPlane, float farPlane)
{
    m_FovY = fovYRad;
    m_Near = nearPlane;
    m_Far = farPlane;
}

void CameraComponent::ApplyCamera()
{
    // ---- 1) 極座標からカメラ位置と上方向を計算 ----
    // 注視点 m_LookAt を中心に、半径 m_Radius の球上にカメラを置く
    CPolor3D polar(m_Radius, m_Elevation, m_Azimuth);
    Vector3 offset = polar.ToCartesian();
    m_Position = offset + m_LookAt;

    // 上方向ベクトル（仰角 + 90 度の位置から取得）
    CPolor3D polarUp(
        m_Radius,
        m_Elevation + (90.0f * PI / 180.0f),
        m_Azimuth
    );
    m_Up = polarUp.ToCartesian();

    // ---- 2) ビュー行列 ----
    m_View =
        DirectX::XMMatrixLookAtLH(
            m_Position,
            m_LookAt,
            m_Up
        );

    Renderer::SetViewMatrix(&m_View);

    // ---- 3) プロジェクション行列 ----
    float aspectRatio =
        static_cast<float>(Window::GetInstance().GetWidth()) /
        static_cast<float>(Window::GetInstance().GetHeight());

    m_Proj =
        DirectX::XMMatrixPerspectiveFovLH(
            m_FovY,
            aspectRatio,
            m_Near,
            m_Far
        );

    Renderer::SetProjectionMatrix(&m_Proj);
}