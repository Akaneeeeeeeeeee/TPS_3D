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

// 姿勢ごとの Shape を作成
void CharacterVirtualComponent::BuildStanceShapes()
{
    using namespace JPH;

    // 立ち
    m_StandShape = new CapsuleShape(m_StandHalfHeight, m_Radius);
    // しゃがみ
    m_CrouchShape = new CapsuleShape(m_CrouchHalfHeight, m_Radius);
    // 伏せ
    m_ProneShape = new CapsuleShape(m_ProneHalfHeight, m_Radius);
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

    // ローカルオフセットをセット
    settings->mShapeOffset = ToJPH(m_Offset);

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

    // InnerBodyID を控える
    m_InnerBodyID = m_Character->GetInnerBodyID();
}

void CharacterVirtualComponent::Update(const float dt)
{
    if (!m_Physics || !m_Character) return;

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
        horizontal = move_dir * m_MoveSpeed;   // ★ 新パラメータ（最高速度）
    }
    else
    {
        // 入力が無いときはピタっと止める
        // （慣性を残したいなら: horizontal = (vel - vertical) * 0.8f; みたいにしてもOK）
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

Vector3 CharacterVirtualComponent::GetLinearVelocity(void) const
{
    if (!m_Character) { return Vector3::Zero; }

    JPH::Vec3 velocity = m_Character->GetLinearVelocity();
    return Vector3(velocity.GetX(), velocity.GetY(), velocity.GetZ());
}

void CharacterVirtualComponent::SetStance(Stance s)
{
    if (!m_Character || !m_Physics) { return; }
    if (m_Stance == s) { return; }

    using namespace JPH;

    const Shape* new_shape = nullptr;

	// 姿勢に応じた Shape を取得
    switch (s)
    {
    case CharacterVirtualComponent::Stance::Stand:
        new_shape = m_StandShape;
        break;
    case CharacterVirtualComponent::Stance::Crouch:
        new_shape = m_CrouchShape;
        break;
    case CharacterVirtualComponent::Stance::Prone:
        new_shape = m_ProneShape;
        break;
    default:
        break;
    }

    if (!new_shape) { return; }

    auto& system = m_Physics->GetSystem();
    auto bp_filter = system.GetDefaultBroadPhaseLayerFilter(Layers::CHARACTER);
    auto obj_filter = system.GetDefaultLayerFilter(Layers::CHARACTER);
    JPH::BodyFilter  body_filter;
    JPH::ShapeFilter shape_filter;

    // 許容するめり込み距離（小さすぎると失敗しやすい）
    const float max_penetration = 0.1f;

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

    m_Stance = s;
}
