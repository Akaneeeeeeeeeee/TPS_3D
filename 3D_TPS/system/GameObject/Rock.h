#pragma once
#include "system/Framework/GameObject/GameObject.h"

// 前方宣言
struct EngineContext;
class CStaticMesh;
class CStaticMeshRenderer;
class CShader;

/*
* @brief	Rockクラス
* @detail	岩オブジェクトクラス
* @auther	赤根和樹
* @date		2025/11/15
*/
class Rock final : public GameObject
{
public:
	Rock(EngineContext& context, const uint64_t id, 
		const std::string& name = "", const Tag& tag = Tag::Object, 
		const Transform& transform = Transform::One());
	~Rock();

	void Init(void) override;
	void Update(const float deltatime) override;
	void Draw(void) const override;
	void Uninit(void) override;

private:
	CStaticMesh* m_Mesh{};
	CStaticMeshRenderer* m_MeshRenderer{};
	CShader* m_Shader{};
};

