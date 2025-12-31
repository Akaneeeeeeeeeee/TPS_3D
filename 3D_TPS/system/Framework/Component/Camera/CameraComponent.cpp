#include "CameraComponent.h"
#include "Framework/EngineSystem/EngineSystem.h"
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

void CameraComponent::Attach(EngineServices& context)
{
	// CameraManager に登録する
	m_CameraManager = &context.camera;
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
    // m_Mode に応じて位置を決める（ゲーム側の都合は一切入れない）
    if (m_Mode == Mode::Orbit)
    {
        // 角度から forward を作る（座標系が合わなければ sin/cos の符号を調整）
        const float ce = std::cos(m_Elevation);
        const float se = std::sin(m_Elevation);
        const float ca = std::cos(m_Azimuth);
        const float sa = std::sin(m_Azimuth);

        // LookAt から見たカメラの“向き”
        Vector3 forward(-sa * ce, se, -ca * ce);
        if (forward.LengthSquared() > 1e-6f) forward.Normalize();

        Vector3 pos = m_LookAt - forward * m_Radius;

        // 肩オフセット：カメラの右方向へずらす（単なるオフセット機能）
        Vector3 right = forward.Cross(m_Up); // 逆なら m_Up.Cross(forward)
        if (right.LengthSquared() > 1e-6f) right.Normalize();
        pos += right * m_ShoulderOffset;

        m_Position = pos;
    }

    // ---- 2) ビュー ----
    m_View = DirectX::XMMatrixLookAtLH(m_Position, m_LookAt, m_Up);
    Renderer::SetViewMatrix(&m_View);

    // ---- 3) プロジェクション ----
    float aspectRatio =
        static_cast<float>(Window::GetInstance().GetWidth()) /
        static_cast<float>(Window::GetInstance().GetHeight());

    m_Proj = DirectX::XMMatrixPerspectiveFovLH(m_FovY, aspectRatio, m_Near, m_Far);
    Renderer::SetProjectionMatrix(&m_Proj);
}
