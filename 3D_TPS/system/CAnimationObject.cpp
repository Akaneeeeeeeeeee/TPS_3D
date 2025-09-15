#include	"CAnimationObject.h"

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
	m_AnimMesh->Update(m_BoneCombMatrix,frame);
	m_CurrentFrame += dt;
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


//void CAnimationObject::SetAnimation(aiAnimation* anim) {
//    m_CurrentAnimation = anim;
//    m_CurrentTime = 0.0f;
//}
//
//void CAnimationObject::Update(float dt) {
//    if (!m_AnimMesh || !m_CurrentAnimation) return;
//
//    m_CurrentTime += dt * m_PlaySpeed;
//
//    float duration = static_cast<float>(m_CurrentAnimation->mDuration);
//    float ticksPerSecond = static_cast<float>(
//        m_CurrentAnimation->mTicksPerSecond != 0 ?
//        m_CurrentAnimation->mTicksPerSecond : 25.0f);
//
//    float timeInTicks = fmod(m_CurrentTime * ticksPerSecond, duration);
//
//    // メッシュに計算してもらう
//    m_AnimMesh->UpdateBoneMatrix(
//        m_AnimMesh->GetRootNode(),
//        DirectX::SimpleMath::Matrix::Identity,
//        m_BoneCombMatrix,
//        timeInTicks,
//        m_CurrentAnimation);
//}

//void CAnimationObject::Draw() {
//    if (m_AnimMesh) {
//        m_AnimMesh->Draw();
//    }
//}
