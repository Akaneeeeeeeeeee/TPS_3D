#pragma once
#include "PhysicsComponent.h"
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>

/*
* @brief	カプセルコライダーコンポーネント
* @detail	カプセル形状のコライダーを提供するコンポーネント
* @remark	物理演算を行うためには Rigidbody コンポーネントと組み合わせて使用する必要がある
* @auther	赤根 和樹
* @date     2025/11/11
*/
class CapsuleCollider : public PhysicsComponent
{
public:
    CapsuleCollider()
        : PhysicsComponent(), m_HalfHeight(0.0f), m_Radius(0.0f) {
    }
    ~CapsuleCollider() = default;

    void Init(void) override;
    void Update(const float deltaTime) override {};
    void Uninit(void) override;

	// 形状を取得
    JPH::RefConst<JPH::Shape> GetShape(void) const override { return JPH::RefConst<JPH::Shape>(m_Shape); }
    bool IsCollider() const noexcept override { return true; }

	void SetHalfHeight(const float halfHeight) { m_HalfHeight = halfHeight; }
	void SetRadius(const float radius) { m_Radius = radius; }
    void SetSize(const float halfHeight, const float radius) {
        m_HalfHeight = halfHeight;
        m_Radius = radius;
	}

protected:
    void Attach(EngineContext& context) override;
    void Detach(EngineContext& context) override;

	void CreateBody(JPH::BodyInterface& bi) override;

private:
    float m_HalfHeight;
    float m_Radius;
    JPH::RefConst<JPH::CapsuleShape> m_Shape = nullptr;
};
