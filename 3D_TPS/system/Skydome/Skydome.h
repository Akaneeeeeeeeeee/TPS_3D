#pragma once
#include "system/CSphereMesh.h"
#include "system/CTexture.h"
#include "system/CShader.h"
#include "system/CMaterial.h"
#include "system/camera.h"
#include "system/Framework/GameObject/GameObject.h"

class Skydome : public GameObject
{
public:
	Skydome(EngineContext& context, const uint64_t id, 
		const std::string& name = "", const Tag& tag = Tag::None,
		const Transform& transform = Transform::One());
	~Skydome();

	void Init(void) override;
	void Update(const uint64_t deltatime) override;
	void Draw(void) const override;
	void Uninit(void) override;

	void SetTexture(const std::filesystem::path& filepath);

	void InvertNormal(void);

private:
	CSphereMesh m_SphereMesh;
	CMeshRenderer m_MeshRenderer;

	CTexture m_Texture;
	CMaterial m_Material;
	CShader m_Shader;
};

