#pragma once
#include "system/CPlaneMesh.h"
#include "system/CTexture.h"
#include "system/CMeshRenderer.h"
#include "system/CMaterial.h"
#include "system/transform.h"
#include "system/CShader.h"

/// <summary>
/// 3D地形クラス
/// </summary>
class Terrain
{
public:
	Terrain();
	~Terrain();

	void Init(int divx, int divy,
		float width, float height);

	void Draw(void);

	void SetImage(const std::filesystem::path& _filepath);

private:
	SRT m_transform;
	CPlaneMesh m_plane{};
	CMeshRenderer m_MeshRenderer{};
	CShader m_Shader;

	CTexture m_Texture{};
	std::unique_ptr<CMaterial> m_Material{};
};
