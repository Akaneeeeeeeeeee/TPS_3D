#include	"CAnimationObject.h"
#include    "Framework/Component/Animator/Animator.h"

using namespace DirectX::SimpleMath;

void CAnimationObject::Init()
{
	// ボーンコンビネーション行列初期化
	m_BoneCombMatrix.Create();							// 20240723 
}

void CAnimationObject::Update(float dt) 
{
	int frame = static_cast<int>(m_CurrentFrame);
	// アニメーションメッシュ更新
	m_AnimMesh->Update(m_BoneCombMatrix, m_pCurrentAnimation ,frame);
	m_CurrentFrame += dt;
}

void CAnimationObject::UpdateFromAnimator(const Animator& animator)
{
    if (!m_AnimMesh) return;

    aiAnimation* current = animator.GetCurrentClip();
    aiAnimation* next = animator.GetNextClip();

    if (!current) return;

    float t0Sec = animator.GetCurrentTimeSec();
    float t1Sec = animator.GetNextTimeSec();
    float alpha = animator.GetBlendAlpha();

    if (!animator.IsBlending() || !next)
    {
        // 単一アニメ
        m_AnimMesh->Update(m_BoneCombMatrix, current, t0Sec);
    }
    else
    {
        // ブレンド
		// 秒単位で渡す版
        //m_AnimMesh->UpdateBlended(m_BoneCombMatrix, current, t0Sec, next, t1Sec, alpha);

		// フレーム番号に変換して渡す版
        int frame0 = static_cast<int>(t0Sec * (current->mTicksPerSecond > 0.0 ? current->mTicksPerSecond : 1.0));
        int frame1 = static_cast<int>(t1Sec * (next->mTicksPerSecond > 0.0 ? next->mTicksPerSecond : 1.0));
        m_AnimMesh->UpdateBlended(m_BoneCombMatrix, current, frame0, next, frame1, alpha);
    }
}

void CAnimationObject::Draw()
{
	// ボーンコンビネーション行列用定数バッファ更新
	m_BoneCombMatrix.Update();

	// 定数バッファGPUへセット
	m_BoneCombMatrix.SetGPU();

	// メッシュ描画
	m_AnimMesh->Draw();
}
