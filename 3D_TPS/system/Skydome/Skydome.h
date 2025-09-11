#pragma once
#include "system/CSphereMesh.h"
#include "system/CTexture.h"
#include "system/CShader.h"
#include "system/CMaterial.h"
#include "system/camera.h"
#include "system/transform.h"

class Skydome
{
public:
	Skydome();
	~Skydome();

	void Init(void);
	void Draw(void);

	void SetTexture(const std::filesystem::path& filepath);

	void InvertNormal(void);
	void InvertNormalAndIndices(void);

private:
	SRT m_Transform;
	CSphereMesh m_SphereMesh;
	CMeshRenderer m_MeshRenderer;

	CTexture m_Texture;
	CMaterial m_Material;
	CShader m_Shader;
};

