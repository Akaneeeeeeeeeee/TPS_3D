#pragma once
#include	"commontypes.h"
#include	"BoneCombMatrix.h"
#include	"CAnimationMesh.h"

// 前方宣言
class Animator;

class CAnimationObject
{
public:
	void Init();
	void Update(float dt);
	void Draw();
	void SetAnimationMesh(CAnimationMesh* animmesh) { m_AnimMesh = animmesh; }

	void SetCurrentAnimation(aiAnimation* anim)
	{
		if (m_pCurrentAnimation != anim)
		{
			m_pCurrentAnimation = anim;
			//m_CurrentFrame = 0.0f;   // 切り替え時にリセットするならここで
		}
	}

	// Animator の結果を使ってボーン計算を行う
	void UpdateFromAnimator(const Animator& animator);

	BoneCombMatrix& GetBoneCombMatrix() { return m_BoneCombMatrix; }
	
private:
	// ボーンコンビネーション行列用定数バッファ内容
	BoneCombMatrix m_BoneCombMatrix{};							// 20240723

	// アニメーションメッシュ
	CAnimationMesh* m_AnimMesh = nullptr;

	// 再生中のアニメーションデータ
	aiAnimation* m_pCurrentAnimation = nullptr;

	// 現在フレーム
	float m_CurrentFrame = 0;
};

