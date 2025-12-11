#pragma once
#include "system/Framework/Component/IComponent/IComponent.h"
#include "Framework/PhysicsSystem/Physics.h"
#include "renderer.h"

// 前方宣言
class PhysicsManager; 

/*
* @brief	物理コンポーネント基底クラス
* @detail	物理演算機能を提供するコンポーネントの基底クラス
* @remark	Rigidbodyや各種コライダーコンポーネントはこのクラスを継承して実装する
* @auther	赤根 和樹
* @date     2025/11/11
*/
class PhysicsComponent : public IComponent
{
public:
    virtual ~PhysicsComponent() noexcept override = default;

    virtual void Init(void) override = 0;
    virtual void Update(const float deltatime) override = 0;
    virtual void Uninit(void) override;

	// 当たり判定系かどうかの判定
    virtual bool IsCollider() const noexcept { return false; }
    virtual JPH::RefConst<JPH::Shape> GetShape() const { return nullptr; }
    virtual JPH::RMat44 GetLocalPose() const { return JPH::RMat44::sTranslation(JPH::Vec3(m_Offset.x, m_Offset.y, m_Offset.z)); }
	virtual void SetOffset(const Vector3& offset) { m_Offset = offset; }
	virtual Vector3 GetOffset(void) const { return m_Offset; }

    virtual void CreateBody(JPH::BodyInterface& bi) = 0;
    virtual void DestroyBody(JPH::BodyInterface& bi);   

	virtual void Attach(EngineContext& context) override;
    virtual void Detach(void) override;

    virtual void SetMesh(const std::vector<VERTEX_3D>& vertices, const std::vector<uint32_t>& indices) {};

protected:
    PhysicsComponent() = default;
    JPH::BodyID m_BodyID;
    PhysicsManager* m_Physics = nullptr;
	Vector3 m_Offset = Vector3::Zero;
};
