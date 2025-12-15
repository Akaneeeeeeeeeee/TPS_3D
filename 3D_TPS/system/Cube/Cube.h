#pragma once
#include "system/Framework/GameObject/GameObject.h"
#include "system/CBoxMesh.h"
#include "system/Framework/Material/MyMaterial.h"
#include "system/CMeshRenderer.h"

class Cube : public GameObject
{
public:
	Cube(ComponentFactory* factory, const uint64_t id,
		const std::string& name = "", const Tag& tag = Tag::None,
		const Transform& transform = Transform::One());
	~Cube();

	void Awake(void) override;
	void Update(const float deltatime) override;
	void Draw(void) const override;
	void Uninit(void) override;

private:
	CBoxMesh m_BoxMesh;
	MyMaterial m_Material;
	CMeshRenderer m_MeshRenderer;
};

