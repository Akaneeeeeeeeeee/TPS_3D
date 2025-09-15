#pragma once
#include	"commontypes.h"
#include	"BoneCombMatrix.h"
#include	"CAnimationMesh.h"

//class CAnimationObject {
//    BoneCombMatrix m_BoneCombMatrix{};
//    CAnimationMesh* m_AnimMesh = nullptr;
//
//    aiAnimation* m_CurrentAnimation = nullptr;
//    float m_CurrentTime = 0.0f;
//    float m_PlaySpeed = 1.0f;
//
//public:
//	void Init() { m_BoneCombMatrix.Create(); }
//    void SetAnimationMesh(CAnimationMesh* animmesh) { m_AnimMesh = animmesh; }
//    void SetAnimation(aiAnimation* anim);
//    void Update(float dt);
//    void Draw();
//};

class CAnimationObject
{
	// ボーンコンビネーション行列用定数バッファ内容
	BoneCombMatrix m_BoneCombMatrix{};							// 20240723

	// アニメーションメッシュ
	CAnimationMesh*	m_AnimMesh = nullptr;

	// 現在フレーム
	float m_CurrentFrame = 0;

public:
	void Init();
	void Update(float dt);
	void Draw();
	void SetAnimationMesh(CAnimationMesh* animmesh) { m_AnimMesh = animmesh; }
};

