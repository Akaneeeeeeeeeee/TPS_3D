#pragma once
#include "Framework/GameObject/GameObject.h"

class CStaticMesh;
class CStaticMeshRenderer;
class CShader;

/*
* @brief	ゴールクラス
* @detail	ゴールオブジェクトを表すクラス
* @auther	赤根　和樹
* @date		2025/12/10
*/
class Goal : public GameObject
{
public:
	Goal(ComponentFactory* factory,
		const uint64_t id,
		const std::string& name = "",
		const Tag tag = Tag::Goal,
		const Transform& transform = Transform::One())
		: GameObject(factory, id, name, tag, transform),
		m_meshrenderer(nullptr) {
	}
	~Goal() = default;

	void Awake(void) override;
	void Update(const float delta) override;
	void Draw(void) const override;
	void Uninit(void) override;
	bool IsReached(void) const { return m_Reached; }

	void OnCollisionCharacterEnter(GameObject& other) override;

	void DebugImGui(void);

private:
	CStaticMesh* m_mesh{};
	CStaticMeshRenderer* m_meshrenderer{};
	CShader* m_shader{};

	bool m_Reached = false;
};

