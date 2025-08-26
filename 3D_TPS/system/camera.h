#pragma once

#include	"commontypes.h"
#include	"renderer.h"
#include	"../application.h"
#include	"CPolar3D.h"

class Camera {
protected:
	Vector3	m_position = Vector3(0.0f, 0.0f, 0.0f);	// カメラ位置

	Vector3		m_lookat{};				// 注視点
	Matrix4x4	m_viewmtx{};			// ビュー行列
	Matrix4x4   m_projmtx{};			// プロジェクション行列

public:
	Camera() = default;

	void Init();
	void Dispose();
	void Update();
	void Draw();
	void SetPosition(const Vector3& position) { m_position = position; }
	void SetLookat(const Vector3& position) { m_lookat = position; }

	Matrix4x4 GetViewMatrix() const { return m_viewmtx; }
	Matrix4x4 GetProjMatrix() const { return m_projmtx; }
};

// / 自由視点カメラ
class FreeCamera : public Camera {
private:
	float   m_elevation = -90.0f * PI / 180.0f;		// 仰角
	float	m_azimuth = PI / 2.0f;					// 方位角	
	float   m_radius = 100.0f;						// 半径	

	Vector3 m_up = { 0.0f, 1.0f, 0.0f };			// 上方向ベクトル
public:
	void SetElevation(float elevation) {
		m_elevation = elevation;
	}

	void SetAzimuth(float azimuth) {
		m_azimuth = azimuth;
	}

	void SetRadius(float radius) 
{
		m_radius = radius;
	}

	void CalcCameraPosition() 
	{
		// 仰角と方位角からカメラの位置を計算
		CPolor3D polar(m_radius, m_elevation, m_azimuth);
		// 極座標からデカルト座標に変換
		m_position = polar.ToCartesian();

		CPolor3D polarup(m_radius, (m_elevation + (90.0f * PI / 180.0f)), m_azimuth);
		m_up = polarup.ToCartesian();
	}

	void CalcCameraPositionTranslate(Vector3 lookat)
	{
		// 注視点を中心にカメラの位置を計算
		CPolor3D polar(m_radius, m_elevation, m_azimuth);
		// 極座標からデカルト座標に変換
		m_position = polar.ToCartesian();
		m_position += lookat;	// 注視点を中心にカメラの位置を設定
		m_lookat = lookat;		// 注視点を設定
		CPolor3D polarup(m_radius, (m_elevation + (90.0f * PI / 180.0f)), m_azimuth);
		m_up = polarup.ToCartesian();
	}

	void Draw() {
		// ビュー変換後列作成
		m_viewmtx =
			DirectX::XMMatrixLookAtLH(
				m_position,
				m_lookat,
				m_up);				// 左手系にした　20230511 by suzuki.tomoki

		// DIRECTXTKのメソッドは右手系　20230511 by suzuki.tomoki
		// 右手系にすると３角形頂点が反時計回りになるので描画されなくなるので注意
		// このコードは確認テストのために残す
		//	m_ViewMatrix = m_ViewMatrix.CreateLookAt(m_Position, m_Target, up);					

		Renderer::SetViewMatrix(&m_viewmtx);

		//プロジェクション行列の生成
		constexpr float fieldOfView = DirectX::XMConvertToRadians(45.0f);    // 視野角

		float aspectRatio = static_cast<float>(Application::GetWidth()) / static_cast<float>(Application::GetHeight());	// アスペクト比	
		float nearPlane = 1.0f;			// ニアクリップ
		float farPlane = 3000.0f;		// ファークリップ

		//プロジェクション行列の生成
		Matrix4x4 projectionMatrix;
		projectionMatrix =
			DirectX::XMMatrixPerspectiveFovLH(
				fieldOfView,
				aspectRatio,
				nearPlane,
				farPlane);	// 左手系にした　20230511 by suzuki.tomoki

		// DIRECTXTKのメソッドは右手系　20230511 by suzuki.tomoki
		// 右手系にすると３角形頂点が反時計回りになるので描画されなくなるので注意
		// このコードは確認テストのために残す
	//	projectionMatrix = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(fieldOfView, aspectRatio, nearPlane, farPlane);

		Renderer::SetProjectionMatrix(&projectionMatrix);

	}
};

