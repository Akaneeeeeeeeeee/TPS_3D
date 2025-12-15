#include "CharacterVirtualComponent.h"
#include "system/Framework/PhysicsSystem/Physics.h"
#include "system/Framework/PhysicsSystem/PhysicsManager.h"
#include "system/Framework/EngineContext/EngineContext.h"
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include "Framework/GameObject/GameObject.h"
#include <Jolt/Math/Vec3.h>
#include <DirectXMath.h>
#include "directxtk/include/SimpleMath.h"

namespace
{
	// キャラ専用の重力（ユニット指定）
	// 1ユニット ≒ 1cm と仮定するなら -980.0f で地球重力ぐらい
	constexpr float CHAR_GRAVITY_Y = -980.0f;

	// キャラが到達してほしいジャンプの高さ（ユニット）
	// 120なら 1.2m ぐらい
	constexpr float DESIRED_JUMP_HEIGHT = 120.0f;
}

// DirectX::SimpleMath::Vector3 から JPH::Vec3 への変換関数
static JPH::Vec3 ToJPH(const DirectX::SimpleMath::Vector3& v)
{
	return JPH::Vec3(v.x, v.y, v.z);
}


CharacterVirtualComponent::~CharacterVirtualComponent()
{
	Uninit();
}

void CharacterVirtualComponent::Attach(EngineContext& ctx)
{
	m_Physics = &ctx.joltPhysicsManager;
}

void CharacterVirtualComponent::Detach(void)
{
	m_Physics = nullptr;
}

bool CharacterVirtualComponent::IsOnGround(void) const
{
	if (!m_Character) { return false; }
	return m_Character->GetGroundState() == JPH::CharacterVirtual::EGroundState::OnGround;
}

/*
* @brief	水平速度の取得
* @return	水平速度の大きさ
*/
float CharacterVirtualComponent::GetHorizontalSpeed(void) const
{
	if (!m_Character) { return 0.0f; }

	using namespace JPH;

	Vec3 v = m_Character->GetLinearVelocity();
	Vec3 up = m_Character->GetUp();		// 通常 (0,1,0)
	Vec3 v_y = up * v.Dot(up);			// 縦成分
	Vec3 v_xz = v - v_y;				// 水平成分

	return v_xz.Length();
}

/*
* @brief	線形速度を取得する
* @return	線形速度ベクトル
*/
Vector3 CharacterVirtualComponent::GetLinearVelocity(void) const
{
	if (!m_Character) { return Vector3::Zero; }

	JPH::Vec3 velocity = m_Character->GetLinearVelocity();
	return Vector3(velocity.GetX(), velocity.GetY(), velocity.GetZ());
}

/*
* @brief	姿勢ごとの Shape を作成
*/
void CharacterVirtualComponent::BuildStanceShapes()
{
	using namespace JPH;

	// 立ち
	m_StandShape = new CapsuleShape(m_StandHalfHeight, m_Radius);
	// しゃがみ
	m_CrouchShape = new CapsuleShape(m_CrouchHalfHeight, m_Radius);
	// 伏せ
	m_ProneShape = new CapsuleShape(m_ProneHalfHeight, m_Radius);

	auto CalcFeetOffset = [](float half_height, float radius) -> Vector3
		{
			// 原点 = 足元 にしたいので、カプセル中心を (0, half_height + radius, 0) に置く
			float off_y = half_height + radius;
			return Vector3(0.0f, off_y, 0.0f);
		};

	m_StandOffset = CalcFeetOffset(m_StandHalfHeight, m_Radius);
	m_CrouchOffset = CalcFeetOffset(m_CrouchHalfHeight, m_Radius);
	m_ProneOffset = CalcFeetOffset(m_ProneHalfHeight, m_Radius);
}

void CharacterVirtualComponent::Stop(void)
{
	// 入力方向を消す
	m_MoveDir = Vector3::Zero;
	// 移動量も 0 にする
	m_MoveAmount = 0.0f;

	// 物理側の速度も 0 にする
	if (m_Character)
	{
		m_Character->SetLinearVelocity(JPH::Vec3::sZero());
	}

	// ジャンプ要求も消しておく
	m_WantsJump = false;
}

void CharacterVirtualComponent::Init(void)
{
	if (!m_Physics) { return; }

	using namespace JPH;

	// ---- キャラ専用の重力からジャンプ初速度を計算 ----
	float g_char = std::abs(CHAR_GRAVITY_Y);             // 980
	m_JumpSpeed = std::sqrt(2.0f * g_char * DESIRED_JUMP_HEIGHT);

	// 各姿勢の Shape を作成
	this->BuildStanceShapes();

	Ref<CharacterVirtualSettings> settings = new CharacterVirtualSettings();

	// 初期は立ち姿
	m_Stance = Stance::Stand;
	settings->mShape = m_StandShape;
	settings->mInnerBodyShape = m_StandShape;       // Inner Body も同じ形
	settings->mInnerBodyLayer = Layers::CHARACTER;

	// 姿勢ごとの offset を使う(初期は立ち)
	settings->mShapeOffset = ToJPH(m_StandOffset);

	// 上方向（Y軸）と SupportingVolume など
	settings->mUp = JPH::Vec3::sAxisY();
	settings->mSupportingVolume = JPH::Plane(settings->mUp, -m_Radius);

	// 開始位置
	JPH::RVec3 start_pos = ToJPH(m_pOwner->GetPosition());
	JPH::Quat  rot = JPH::Quat::sIdentity();
	JPH::PhysicsSystem* system = &m_Physics->GetSystem();

	// キャラクター作成
	m_Character = new JPH::CharacterVirtual(
		settings,
		start_pos,
		rot,
		system
	);

	// InnerBodyID を保存
	m_InnerBodyID = m_Character->GetInnerBodyID();

	// InnerBody にも GameObject* を UserData として設定
	{
		auto& bi = m_Physics->GetBodyInterface();
		bi.SetUserData(
			m_InnerBodyID,
			reinterpret_cast<JPH::uint64>(m_pOwner)
		);
	}

	// PhysicsManager が保持している CharacterContactListenerImpl を渡す
	m_Character->SetListener(
		m_Physics->GetCharacterContactListener()
	);

	// 姿勢に応じて移動速度を設定
	m_MoveSpeed = m_BaseMoveSpeed * GetMoveSpeedCoeff();
}

void CharacterVirtualComponent::Update(const float dt)
{
	if (!m_Physics || !m_Character) { return; }

	using namespace JPH;

	Vec3 up = m_Character->GetUp();
	Vec3 vel = m_Character->GetLinearVelocity();

	// 縦 / 横に分解
	Vec3 vertical = up * vel.Dot(up);
	// 水平速度は「常に一定の速さ」でいい
	Vec3 horizontal = Vec3::sZero();

	// Player から渡された移動方向（ワールド）
	Vec3 move_dir(m_MoveDir.x, m_MoveDir.y, m_MoveDir.z);

	// 加速・減速
	if (move_dir.LengthSq() > 0.0f)
	{
		// 念のため正規化
		move_dir = move_dir.Normalized();
		// 倒し具合で速度を変える
		const float targetSpeed = m_MoveSpeed * m_MoveAmount;
		horizontal = move_dir * targetSpeed;
	}
	else
	{
		// 入力が無いときその場で停止
		horizontal = Vec3::sZero();
	}

	// ジャンプ & 重力
	if (m_Character->GetGroundState() == CharacterVirtual::EGroundState::OnGround)
	{
		vertical = Vec3::sZero();

		if (m_WantsJump)
			vertical += up * m_JumpSpeed;
	}

	// ★ キャラ専用の重力を使う
	Vec3 charGravity(0.0f, CHAR_GRAVITY_Y, 0.0f);

	vertical += charGravity * dt;

	Vec3 new_vel = horizontal + vertical;
	m_Character->SetLinearVelocity(new_vel);

	CharacterVirtual::ExtendedUpdateSettings settings;

	auto bp_filter = m_Physics->GetSystem().GetDefaultBroadPhaseLayerFilter(Layers::CHARACTER);
	auto obj_filter = m_Physics->GetSystem().GetDefaultLayerFilter(Layers::CHARACTER);
	BodyFilter  body_filter;
	ShapeFilter shape_filter;

	m_Character->ExtendedUpdate(
		dt,
		charGravity,  // ← ここもキャラ専用
		settings,
		bp_filter,
		obj_filter,
		body_filter,
		shape_filter,
		*m_Physics->GetTempAllocator()
	);

	RVec3 pos = m_Character->GetPosition();
	m_pOwner->SetPosition(Vector3(
		(float)pos.GetX(),
		(float)pos.GetY(),
		(float)pos.GetZ()
	));

	m_WantsJump = false;
}

void CharacterVirtualComponent::Uninit()
{
	m_Character = nullptr;
}

/*
* @brief	姿勢を設定する
* @param	s	姿勢
*/
void CharacterVirtualComponent::SetStance(Stance s)
{
	if (!m_Character || !m_Physics) { return; }
	if (m_Stance == s) { return; }

	using namespace JPH;

	const Shape* new_shape = nullptr;
	Vector3 new_offset = Vector3::Zero;

	// 姿勢に応じた Shape を取得
	switch (s)
	{
	case CharacterVirtualComponent::Stance::Stand:
		new_shape = m_StandShape;
		new_offset = m_StandOffset;
		break;
	case CharacterVirtualComponent::Stance::Crouch:
		new_shape = m_CrouchShape;
		new_offset = m_CrouchOffset;
		break;
	case CharacterVirtualComponent::Stance::Prone:
		new_shape = m_ProneShape;
		new_offset = m_ProneOffset;
		break;
	default:
		break;
	}

	if (!new_shape) { return; }

	// Shape を差し替え
	auto& system = m_Physics->GetSystem();
	auto bp_filter = system.GetDefaultBroadPhaseLayerFilter(Layers::CHARACTER);
	auto obj_filter = system.GetDefaultLayerFilter(Layers::CHARACTER);
	JPH::BodyFilter  body_filter;
	JPH::ShapeFilter shape_filter;

	// 許容するめり込み距離（40.0までだと失敗する）
	const float max_penetration = 40.1f;

	bool ok = m_Character->SetShape(
		new_shape,
		max_penetration,
		bp_filter, obj_filter,
		body_filter, shape_filter,
		*m_Physics->GetTempAllocator()
	);

	if (!ok)
	{
		// ここでログだけ出して、姿勢変更をキャンセルする、など
		return;
	}

	// Inner Body 側の Shape も合わせる
	m_Character->SetInnerBodyShape(new_shape);
	// 足元が動かないようオフセットも姿勢用に切り替える
	m_Character->SetShapeOffset(ToJPH(new_offset));
	// 現在の姿勢を更新
	m_Stance = s;
	// 係数に応じて最高速度を更新
	m_MoveSpeed = m_BaseMoveSpeed * GetMoveSpeedCoeff();
}

float CharacterVirtualComponent::GetCurrentHalfHeight(void) const
{
	switch (m_Stance)
	{
	case Stance::Stand:  return m_StandHalfHeight;
	case Stance::Crouch: return m_CrouchHalfHeight;
	case Stance::Prone:  return m_ProneHalfHeight;
	}
	return m_StandHalfHeight;
}

float CharacterVirtualComponent::GetMoveSpeedCoeff(void) const
{
	switch (m_Stance)
	{
	case Stance::Stand:  return StandCoeff.moveSpeed;
	case Stance::Crouch: return CrouchCoeff.moveSpeed;
	case Stance::Prone:  return ProneCoeff.moveSpeed;
	}
	return 1.0f;
}

float CharacterVirtualComponent::GetFootstepIntervalCoeff(void) const
{
	switch (m_Stance)
	{
	case Stance::Stand:  return StandCoeff.footstepInterval;
	case Stance::Crouch: return CrouchCoeff.footstepInterval;
	case Stance::Prone:  return ProneCoeff.footstepInterval;
	}
	return 1.0f;
}

float CharacterVirtualComponent::GetFootstepRadiusCoeff(void) const
{
	switch (m_Stance)
	{
	case Stance::Stand:  return StandCoeff.footstepRadius;
	case Stance::Crouch: return CrouchCoeff.footstepRadius;
	case Stance::Prone:  return ProneCoeff.footstepRadius;
	}
	return 1.0f;
}

float CharacterVirtualComponent::GetFootstepLoudnessCoeff(void) const
{
	switch (m_Stance)
	{
	case Stance::Stand:  return StandCoeff.footstepLoudness;
	case Stance::Crouch: return CrouchCoeff.footstepLoudness;
	case Stance::Prone:  return ProneCoeff.footstepLoudness;
	}
	return 1.0f;
}

void CharacterVirtualComponent::Teleport(const Vector3& worldPos)
{
	if(m_pOwner)
	{
		m_pOwner->SetPosition(worldPos);
	}

	if (!m_Character) { return; }

	// Jolt側のキャラ位置も強制的に変更
	m_Character->SetPosition(ToJPH(worldPos));

	// ついでに速度も止めておくと安全
	m_Character->SetLinearVelocity(JPH::Vec3::sZero());
	m_WantsJump = false;
}

const JPH::Shape* CharacterVirtualComponent::GetCurrentShape(void) const
{
	switch (m_Stance)
	{
	case CharacterVirtualComponent::Stance::Stand:
		return m_StandShape.GetPtr();
		break;
	case CharacterVirtualComponent::Stance::Crouch:
		return m_CrouchShape.GetPtr();
		break;
	case CharacterVirtualComponent::Stance::Prone:
		return m_ProneShape.GetPtr();
		break;
	default:
		break;
	}
	return nullptr;
}