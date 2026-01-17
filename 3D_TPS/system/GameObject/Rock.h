#pragma once
#include "system/Framework/GameObject/GameObject.h"

// 前方宣言
class EngineServices;
class CStaticMesh;
class CStaticMeshRenderer;
class CShader;
class SphereCollider;
class Rigidbody;
class StaticMeshRendererComponent;

/*
* @brief	Rockクラス
* @detail	岩オブジェクトクラス
* @auther	赤根和樹
* @date		2025/12/28
*/
class Rock final : public GameObject
{
public:
	Rock(ComponentFactory* factory, const uint64_t id,
		const std::string& name = "", const Tag& tag = Tag::Object, 
		const Transform& transform = Transform::One());
	~Rock();

	void Awake(void) override;
	void Start(void) override;
	void Update(const float deltatime) override;
	void Draw(void) const override;
	void Uninit(void) override;

	// ThrowAction から呼ぶ
	void SetInitialVelocity(const Vector3& v);
	void SetInitialAngularVelocity(const Vector3& av);

	void OnCollisionEnter(GameObject& other) override;

private:
	CStaticMesh* m_Mesh{};
	CStaticMeshRenderer* m_MeshRenderer{};
	CShader* m_Shader{};

	// 物理
	SphereCollider* m_Sphere = nullptr;
	Rigidbody* m_RB = nullptr;

	StaticMeshRendererComponent* m_pRenderComp = nullptr;

	std::optional<Vector3> m_PendingVel;	// 速度
	std::optional<Vector3> m_PendingAngVel;	// 角速度

	bool  m_HitOnce = false;     // 1回だけ鳴らす
	float m_DespawnTimer = -1.0f; // 0以上なら消滅カウント中
};

