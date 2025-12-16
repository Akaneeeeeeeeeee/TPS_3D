#pragma once
#include "Framework/Component/Physic/PhysicsComponent.h"
#include "system/commontypes.h"
#include "system/Framework/PhysicsSystem/PhysicsLayer.h"

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
    Rigidbody(const float mass = 1.0f);
    ~Rigidbody() override = default;

    enum Type {
        Static,
        Dynamic,
        Kinematic
    };

    // Jolt の MotionType 変換関数を追加
    JPH::EMotionType ToJPHMotionType(Rigidbody::Type type)
    {
        switch (type)
        {
        case Rigidbody::Static:   return JPH::EMotionType::Static;
        case Rigidbody::Dynamic:  return JPH::EMotionType::Dynamic;
        case Rigidbody::Kinematic:return JPH::EMotionType::Kinematic;
        default:                  return JPH::EMotionType::Static;
        }
    }

    void CreateBody(JPH::BodyInterface& bi) override;
	void SetObjectLayer(JPH::ObjectLayer layer) { m_ObjectLayer = layer; }

    void Attach(EngineContext& context) override;
    void Detach(void) override;

    void Init(void) override;
    void Update(const float dt) override;
    void Uninit(void) override;

    // トリガー設定
    void SetAsTrigger(bool flg) { m_IsTrigger = flg; }
    bool IsTrigger(void) const { return m_IsTrigger; }

	void SetBodyType(const Type type) { m_BodyType = type; }

private:
	Type m_BodyType = Dynamic;
    float m_Mass = 1.0f;
    JPH::RefConst<JPH::Shape> mCompoundShape; // 最終Shapeの参照（単体でもCompoundでも）
    JPH::ObjectLayer m_ObjectLayer = Layers::MOVING; // デフォルトは MOVING
    bool m_IsTrigger = false;
};
