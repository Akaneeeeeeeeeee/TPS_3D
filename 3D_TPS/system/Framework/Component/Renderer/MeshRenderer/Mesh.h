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
	std::vector<VERTEX_3D>	m_vertices;		// ���_���W�Q
	std::vector<uint32_t>	m_indices;		// �C���f�b�N�X�f�[�^�Q
public:
	// ���_�f�[�^�擾
	const std::vector<VERTEX_3D>& GetVertices() const {
		return m_vertices;
	}

	// �C���f�b�N�X�f�[�^�擾
	const std::vector<unsigned int>& GetIndices() const {
		return m_indices;
	}
};
