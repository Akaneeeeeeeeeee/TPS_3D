#include "CameraComponent.h"
#include "Framework/EngineSystem/EngineSystem.h"
#include "system/CPolar3D.h"
#include "system/renderer.h"
#include "system/Framework/Window/Window.h"
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>

using namespace DirectX;

static JPH::Vec3 ToJolt(const Vector3& v) { return JPH::Vec3(v.x, v.y, v.z); }
static Vector3   FromJolt(const JPH::Vec3& v) { return Vector3(v.GetX(), v.GetY(), v.GetZ()); }

Vector3 ResolveCameraCollision_Jolt(
	JPH::PhysicsSystem* physics,
	const Vector3& pivot,
	const Vector3& desiredPos,
	const JPH::BodyID& ignoreBody)
{
	if (!physics) return desiredPos;

	Vector3 dir = desiredPos - pivot;
	const float dist = dir.Length();
	if (dist <= 1e-4f) return desiredPos;

	dir /= dist;

	// Jolt は「方向ベクトルの長さ = 最大距離」
	JPH::RRayCast ray(ToJolt(pivot), ToJolt(dir * dist));
	JPH::RayCastResult hit;

	// 自分を無視（プレイヤーのBodyID）
	JPH::IgnoreSingleBodyFilter body_filter(ignoreBody);

	const auto& npq = physics->GetNarrowPhaseQuery();

	JPH::BroadPhaseLayerFilter broadphase_filter;
	JPH::ObjectLayerFilter object_filter;
	const bool isHit = npq.CastRay(ray, hit, broadphase_filter, object_filter, body_filter);

	if (!isHit) return desiredPos;

	// 壁に少しめり込まないように手前へ
	constexpr float PADDING = 8.0f;

	const float hitDist = dist * hit.mFraction;
	const float fixedDist = std::max(0.0f, hitDist - PADDING);

	return pivot + dir * fixedDist;
}

CameraComponent::CameraComponent()
	: m_Mode(Mode::Orbit)             
	, m_Position(0.0f, 10.0f, -100.0f)
	, m_LookAt(0.0f, 10.0f, 0.0f)
	, m_Up(0.0f, 1.0f, 0.0f)          
	, m_CollisionPivot(Vector3::Zero) 
	, m_ShoulderOffset(0.0f)          
	, m_IgnoreBody(JPH::BodyID())     
	, m_Elevation(DirectX::XMConvertToRadians(-15.0f))
	, m_Azimuth(0.0f)
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
	// PhysicsManager も取得しておく
	m_Physics = &context.physics.GetSystem();
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
	Vector3 desiredPos = m_Position;
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

		desiredPos = m_LookAt - forward * m_Radius;

		// 肩オフセット：カメラの右方向へずらす（単なるオフセット機能）
		Vector3 right = forward.Cross(m_Up); // 逆なら m_Up.Cross(forward)
		if (right.LengthSquared() > 1e-6f) right.Normalize();
		desiredPos += right * m_ShoulderOffset;
	}

	// ---- 2) ビュー ----
	// 衝突判定の起点
	// Orbitなら基本は m_LookAt でOK。Direct(構え)は Player が SetCollisionPivot で渡す。
	const Vector3 pivot = (m_CollisionPivot.LengthSquared() > 1e-6f) ? m_CollisionPivot : m_LookAt;

	// 距離ゼロのときは衝突解決しても意味がないのでスキップ
	if ((desiredPos - pivot).LengthSquared() > 1e-8f)
	{
		if (m_Mode == Mode::Direct)
		{
			// タイトル固定/演出用：カメラを勝手に動かさない
			m_Position = desiredPos;
		}
		else
		{
			m_Position = ResolveCameraCollision_Jolt(m_Physics, pivot, desiredPos, m_IgnoreBody);
		}
	}
	else
	{
		m_Position = desiredPos;
	}

	// Eye と Focus が同じになるのを防ぐ
	Vector3 eyeDir = m_LookAt - m_Position;

	constexpr float MIN_DIST = 1.0f;
	constexpr float MIN_DIST_SQ = MIN_DIST * MIN_DIST;

	if (eyeDir.LengthSquared() < MIN_DIST_SQ)
	{
		// 「本来いたい方向（desiredPos）」から離す向きを作る
		Vector3 away = desiredPos - m_LookAt; // lookAt -> desired camera
		away.y = 0.0f;

		if (away.LengthSquared() < 1e-6f)
		{
			// 最悪の保険（固定方向）
			away = Vector3(0.0f, 0.0f, -1.0f);
		}
		away.Normalize();

		m_Position = m_LookAt + away * MIN_DIST;
	}

	m_View = DirectX::XMMatrixLookAtLH(m_Position, m_LookAt, m_Up);
	Renderer::SetViewMatrix(&m_View);

	// ---- 3) プロジェクション ----
	float aspectRatio =
		static_cast<float>(Window::GetInstance().GetWidth()) /
		static_cast<float>(Window::GetInstance().GetHeight());

	m_Proj = DirectX::XMMatrixPerspectiveFovLH(m_FovY, aspectRatio, m_Near, m_Far);
	Renderer::SetProjectionMatrix(&m_Proj);
}
