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

void CharacterVirtualComponent::Init(void)
{
    if (!m_Physics) { return; }

    using namespace JPH;

    // ---- キャラ専用の重力からジャンプ初速度を計算 ----
    float g_char = std::abs(CHAR_GRAVITY_Y);             // ≒ 980
    m_JumpSpeed = std::sqrt(2.0f * g_char * DESIRED_JUMP_HEIGHT);

    // Capsule shape 作成（立ち姿）
    RefConst<Shape> shape = new CapsuleShape(m_HalfHeight, m_Radius);

    Ref<CharacterVirtualSettings> settings = new CharacterVirtualSettings();
    settings->mShape = shape;

    // カプセル Shape
    settings->mShape = shape;                           // CapsuleShape とか
    settings->mInnerBodyLayer = Layers::CHARACTER;      // レイヤーはここで指定する

    // ローカルオフセットをセット
    settings->mShapeOffset = Vec3(m_Offset.x, m_Offset.y, m_Offset.z);

    // 上方向（Y軸）と SupportingVolume など
    JPH::Vec3 up = JPH::Vec3::sAxisY();
    settings->mUp = up;
    settings->mSupportingVolume = JPH::Plane(up, -m_Radius);

    JPH::RVec3 start_pos = ToJPH(m_pOwner->GetPosition());
    JPH::Quat  rot = JPH::Quat::sIdentity();

    // PhysicsSystem は参照なので & を付けてポインタにする
    JPH::PhysicsSystem* system = &m_Physics->GetSystem();

    m_Character = new JPH::CharacterVirtual(
        settings,
        start_pos,
        rot,
        system
    );
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