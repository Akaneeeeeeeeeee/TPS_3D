#include	"CAnimationObject.h"

using namespace DirectX::SimpleMath;

void CAnimationObject::Init()
{
	// �{�[���R���r�l�[�V�����s�񏉊���
	m_BoneCombMatrix.Create();							// 20240723 
}

void CAnimationObject::Update(float dt) 
{
	int frame = static_cast<int>(m_CurrentFrame);
	// �A�j���[�V�������b�V���X�V
	m_AnimMesh->Update(m_BoneCombMatrix,frame);
	m_CurrentFrame+=dt;
}

void CAnimationObject::Draw()
{
	// �{�[���R���r�l�[�V�����s��p�萔�o�b�t�@�X�V
	m_BoneCombMatrix.Update();

	// �萔�o�b�t�@GPU�փZ�b�g
	m_BoneCombMatrix.SetGPU();

	// ���b�V���`��
	m_AnimMesh->Draw();
}

