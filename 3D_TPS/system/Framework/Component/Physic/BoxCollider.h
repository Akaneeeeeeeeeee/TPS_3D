#pragma once
#include "PhysicsComponent.h"

//#include <Jolt/Physics/Body/BodyInterface.h>
//#include <Jolt/Physics/Body/BodyCreationSettings.h>
//#include <Jolt/Physics/Collision/Shape/BoxShape.h>
//#include <Jolt/Math/Vec3.h>
#include "Framework/PhysicsSystem/Physics.h"
#include "commontypes.h"

// 前方宣言
class PhysicsManager;

/*
* @brief	ボックスコライダーコンポーネント
* @detail	ボックス形状のコライダーを提供するコンポーネント
* @remark	物理演算を行うためには Rigidbody コンポーネントと組み合わせて使用する必要がある
* @auther	赤根 和樹
* @date     2025/11/11
*/
class BoxCollider : public PhysicsComponent
{
public:
    BoxCollider();
    ~BoxCollider() noexcept override = default;

    void CreateBody(JPH::BodyInterface& bi) override;

    void Init(void) override;
    void Update(const float deltaTime) override;
    void Uninit(void) override;

    // 形状を取得
    JPH::RefConst<JPH::Shape> GetShape(void) const override { return JPH::RefConst<JPH::Shape>(m_Shape); }
    bool IsCollider() const noexcept override { return true; }
	void SetHalfSize(const DirectX::XMFLOAT3& halfSize) { m_HalfSize = halfSize; }

protected:
    void Attach(EngineContext& context) override;
    void Detach(EngineContext& context) override;

public:
    JPH::RefConst<JPH::Shape> GetShape() { return JPH::RefConst<JPH::Shape>(m_Shape); }

private:
    DirectX::XMFLOAT3 m_HalfSize;

    JPH::Ref<JPH::BoxShape> m_Shape = nullptr;
};
