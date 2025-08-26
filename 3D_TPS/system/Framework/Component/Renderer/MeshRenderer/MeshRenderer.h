#pragma once
#include "Src/Framework/Component/Renderer/IRenderer/IRenderer.h"
#include "Mesh.h"

// ����
// GM�F���b�V���������A�j���[�V�����I�u�W�F�N�g����
// ���A�j���[�V�������b�V�����Z�b�g���A�j���[�V�����f�[�^��ǂݍ��݁��A�j���[�V�����f�[�^�Z�b�g

/// <summary>
/// ���b�V���`��p�̃����_���[
/// </summary>
class MeshRenderer final : public IRenderer
{
public:
	MeshRenderer();
	~MeshRenderer();

private:
	VertexBuffer<VERTEX_3D>	m_VertexBuffer;		// ���_�o�b�t�@
	IndexBuffer				m_IndexBuffer;		// �C���f�b�N�X�o�b�t�@
};

MeshRenderer::MeshRenderer()
{
}

MeshRenderer::~MeshRenderer()
{
}