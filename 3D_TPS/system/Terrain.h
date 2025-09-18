#pragma once
#include "system/CPlaneMesh.h"
#include "system/CTexture.h"
#include "system/CMeshRenderer.h"
#include "system/CMaterial.h"
#include "system/Framework/GameObject/GameObject.h"
#include "system/CShader.h"

/// <summary>
/// 3D地形クラス
/// </summary>
class Terrain : public GameObject
{
public:
	Terrain(uint64_t id, const std::string& name = "", const Tag& tag = Tag::None);
	~Terrain();

	void Init(int divx, int divy,
		float width, float height);

	void Init(void) override;
	void Update(uint64_t deltatime) override;
	void Draw(uint64_t deltatime) override;
	void Uninit(void) override;

	void SetImage(const std::filesystem::path& _filepath);

private:
	CPlaneMesh m_plane{};
	CMeshRenderer m_MeshRenderer{};
	CShader m_Shader;

	CTexture m_Texture{};
	std::unique_ptr<CMaterial> m_Material{};
};
