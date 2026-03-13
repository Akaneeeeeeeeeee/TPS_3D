#include "commonTypes.h"
#include "renderer.h"
#include "camera.h"

void Camera::Init()
{
	m_position = Vector3(0.0f, 10.0f, -100.0f);
	m_lookat = Vector3(0.0f, 10.0f, 0.0f);
}

void Camera::Dispose()
{

}


void Camera::Update()
{

}

void Camera::Draw(void)
{
	// ビュー変換後列作成
	Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
	m_viewmtx = 
		DirectX::XMMatrixLookAtLH(
			m_position, 
			m_lookat, 
			up);

	Renderer::SetViewMatrix(&m_viewmtx);

	//プロジェクション行列の生成
	constexpr float fieldOfView = DirectX::XMConvertToRadians(45.0f);    // 視野角
	
	float aspectRatio = static_cast<float>(Window::GetInstance().GetWidth()) / static_cast<float>(Window::GetInstance().GetHeight());	// アスペクト比	
	float nearPlane = 1.0f;       // ニアクリップ
	float farPlane = 100000.0f;      // ファークリップ

	//プロジェクション行列の生成
	m_projmtx =
		DirectX::XMMatrixPerspectiveFovLH(
			fieldOfView, 
			aspectRatio, 
			nearPlane, 
			farPlane);

	Renderer::SetProjectionMatrix(&m_projmtx);
}