#pragma once
#include "system/CPlaneMesh.h"
#include "system/CTexture.h"
#include "system/CVertexBuffer.h"
#include "system/CIndexBuffer.h"
#include "system/CMaterial.h"

/// <summary>
/// 3D地形クラス
/// </summary>
class Terrain
{
public:
	Terrain();
	~Terrain();

	void Init(int divx, int divy,
		float width, float height,
		const std::string& texfilename,
		MATERIAL mtrl);

	void Draw();

private:
	CPlaneMesh m_plane{};
	CTexture m_texture{};
	std::unique_ptr<CMaterial> m_material{};
};

Terrain::Terrain()
{
}

Terrain::~Terrain()
{
}