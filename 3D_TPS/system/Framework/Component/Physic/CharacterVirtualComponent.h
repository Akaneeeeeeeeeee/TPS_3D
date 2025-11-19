#pragma once

#include "Framework/Component/IComponent/IComponent.h"
#include "Framework/PhysicsSystem/PhysicsManager.h"

#include <Jolt/Physics/Character/CharacterVirtual.h>

class PhysicsManager;

/*
* @brief	バーチャルキャラクターコンポーネント
* @detail	キャラクターの物理演算を提供するコンポーネント
* @remark   当たり判定のため、カプセルの形状を持つ
* @remark   他の物理系コンポーネントと違い、JPH::CharacterVirtualを使用してキャラクターの移動を制御する
* @remark   主にプレイヤーキャラクターやNPCの移動に使用する（Joltのリファレンス参照）
* @auther	赤根 和樹
* @date     2025/11/16
*/
class CharacterVirtualComponent : public IComponent
{
public:
    CharacterVirtualComponent() = default;
    ~CharacterVirtualComponent() noexcept override; // デストラクタの例外指定を親クラスと一致させるため、noexceptを追加

	void Init(void) override;
	void Update(const float deltaTime) override;
	void Uninit(void) override;

	void Attach(EngineContext& context) override;
	void Detach(EngineContext& context) override;
   
    // --- 入力 API ---
    void SetMoveDir(const Vector3& dir) { m_MoveDir = dir; }
    void RequestJump(void) { m_WantsJump = true; }
	void SetOffset(const Vector3& offset) { m_Offset = offset; }

	Vector3 GetLinearVelocity(void) const;

	// カプセル形状
	void SetCapsule(float halfHeight, float radius)
	{
        m_HalfHeight = halfHeight;
		m_Radius = radius;
	}

private:
    PhysicsManager* m_Physics = nullptr;
    JPH::CharacterVirtual* m_Character = nullptr;

	// カプセル形状パラメータ
    float   m_HalfHeight = 60.0f;
    float   m_Radius = 35.0f;
	Vector3 m_Offset = Vector3::Zero;

	// 移動制御用パラメータ
    Vector3 m_MoveDir = Vector3::Zero;
    bool    m_WantsJump = false;

    // チューニング用
    //float   m_Acceleration = 800.0f;
    //float   m_Friction = 1.0f;
    float   m_MoveSpeed = 500.0f;   // 最高速度
    float   m_JumpSpeed = 400.0f;
};
