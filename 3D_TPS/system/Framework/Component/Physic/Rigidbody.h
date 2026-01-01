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

	void Attach(EngineServices& context) override;
	void Detach(void) override;

	void Init(void) override;
	void Update(const float dt) override;
	void Uninit(void) override;

	// トリガー設定
	void SetAsTrigger(bool flg) { m_IsTrigger = flg; }
	bool IsTrigger(void) const { return m_IsTrigger; }

	void SetBodyType(const Type type) { m_BodyType = type; }

	// “今すぐ”速度を入れたい（Bodyが無ければ何もしない）
	void SetLinearVelocity(const Vector3& v, bool activate = true);

	// 最大線形速度（上限の速さ）
	void SetMaxLinearVelocity(float maxVel, bool applyNow = true);
	float GetMaxLinearVelocity(void) const { return m_MaxLinearVelocity.value_or(1000.0f); }

	// “投げる”用途：Bodyが無ければ保留して、Body生成直後に適用
	void SetInitialVelocity(const Vector3& v);
	Vector3 GetLinearVelocity(void);
	void SetAngularVelocity(const Vector3& w, bool activate = true);
	void SetInitialAngularVelocity(const Vector3& w);

	void SetMass(float mass) { m_Mass = mass; }

	bool HasBody() const { return !m_BodyID.IsInvalid(); }

private:
	Type m_BodyType = Dynamic;
	float m_Mass = 1.0f;
	JPH::RefConst<JPH::Shape> mCompoundShape; // 最終Shapeの参照（単体でもCompoundでも）
	JPH::ObjectLayer m_ObjectLayer = Layers::MOVING; // デフォルトは MOVING
	bool m_IsTrigger = false;
	std::optional<Vector3> m_PendingLinearVelocity;
	std::optional<float> m_MaxLinearVelocity;
	std::optional<Vector3> m_PendingAngularVelocity;
};
