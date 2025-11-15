#pragma once

#include	<memory>
#include	"system/CStaticMesh.h"
#include	"system/CStaticMeshRenderer.h"
#include	"system/CShader.h"
#include	"Framework/Scene/IScene.h"
#include	"Framework/GameObject/GameObject.h"

class obstacle : public GameObject {

public:
	obstacle(EngineContext& context,
		const uint64_t id,
		const std::string& name = "",
		const Tag tag = Tag::None,
		IScene* currentscene = nullptr,
		const Transform& transform = Transform::One())
		: GameObject(context, id, name, tag, transform),
		m_meshrenderer(nullptr),
		m_ownerscene(currentscene) {
	}

	void Update(const float delta) override;
	void Draw(void) const override;
	void Init(void) override;
	void Uninit(void) override;
	void DebugImGui(void);

private:
	CStaticMesh*			m_mesh{};
	CStaticMeshRenderer*	m_meshrenderer{};
	CShader*				m_shader{};

	// オーナーSCENE
	IScene* m_ownerscene = nullptr;
};