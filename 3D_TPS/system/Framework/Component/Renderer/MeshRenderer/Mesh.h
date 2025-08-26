#pragma once
#include <vector>
#include "Src/Framework/Buffer/VertexBuffer.h"
#include "Src/Framework/Buffer/IndexBuffer.h"
#include "Src/Framework/Texture/LowLevel/Texture.h"

enum class MeshType
{
	Primitive
	3DModel,
};


class Mesh {
protected:
	std::vector<VERTEX_3D>	m_vertices;		// 頂点座標群
	std::vector<uint32_t>	m_indices;		// インデックスデータ群
public:
	// 頂点データ取得
	const std::vector<VERTEX_3D>& GetVertices() const {
		return m_vertices;
	}

	// インデックスデータ取得
	const std::vector<unsigned int>& GetIndices() const {
		return m_indices;
	}
};
