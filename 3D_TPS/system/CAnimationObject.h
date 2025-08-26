#pragma once
#include	"commontypes.h"
#include	"BoneCombMatrix.h"
#include	"CAnimationMesh.h"

class CAnimationObject
{
	// �{�[���R���r�l�[�V�����s��p�萔�o�b�t�@���e
	BoneCombMatrix m_BoneCombMatrix{};							// 20240723

	// �A�j���[�V�������b�V��
	CAnimationMesh*	m_AnimMesh = nullptr;

	// ���݃t���[��
	float m_CurrentFrame = 0;

public:
	void Init();
	void Update(float dt);
	void Draw();
	void SetAnimationMesh(CAnimationMesh* animmesh) { m_AnimMesh = animmesh; }
};

