#pragma once
#include "Framework/Component/Physic/PhysicsComponent.h"
#include "system/commontypes.h"


/*
* @brief	Rigidbodyコンポーネント
* @detail	物理演算機能を提供するコンポーネント
* @remark	必要であれば各コライダーに情報を提供する
* @auther	赤根 和樹
* @date     2025/11/11
*/
class Rigidbody final : public PhysicsComponent
{
public:
    Rigidbody(const float mass);
    ~Rigidbody() override = default;

    enum Type {
        Static,
        Dynamic,
        Kinematic
    };

    void CreateBody(JPH::BodyInterface& bi) override;

    void Attach(EngineContext& context) override;
    void Detach(EngineContext& context) override;

    void Init(void) override;
    void Update(const float dt) override;
    void Uninit(void) override;

	void SetBodyType(const Type type) { m_BodyType = type; }

private:
	Type m_BodyType = Dynamic;
    float m_Mass = 1.0f;
    Vector3 m_Size = Vector3(1.0f, 1.0f, 1.0f);
};
