#include "Animator.h"
#include "assimp/anim.h"
#include "CAnimationData.h"
#include "CAnimationMesh.h"

Animator::Animator(CAnimationMesh* mesh, CAnimationData* data, const std::string& defaultAnim)
    : m_TargetMesh(mesh)
    , m_AnimData(data)
    , m_DefaultAnimation(defaultAnim)
{

}

Animator::~Animator()
{

}


void Animator::Init(void)
{
    assert(m_TargetMesh);
    assert(m_AnimData);

    // デフォルトアニメーションを読み込む
    m_CurrentAnimation = m_AnimData->GetAnimation(m_DefaultAnimation.c_str(), 0);
    assert(m_CurrentAnimation);

    // ボーン行列定数バッファの生成
    m_BoneMatrix.Create();

    m_CurrentFrame = 0.0f;
}

void Animator::AdvanceFrame(float dt)
{
    if (!m_CurrentAnimation) return;

    const float duration = static_cast<float>(m_CurrentAnimation->mDuration);
    const float ticksPerSecond = (float)(m_CurrentAnimation->mTicksPerSecond != 0 ?
        m_CurrentAnimation->mTicksPerSecond : 25.0f);

    // 時間経過
    m_CurrentFrame += dt * ticksPerSecond * m_PlaySpeed;

    // ループ
    if (m_CurrentFrame >= duration)
    {
        m_CurrentFrame = fmod(m_CurrentFrame, duration);
    }
}

void Animator::Update(float dt)
{
    if (!m_CurrentAnimation || !m_TargetMesh) return;

    AdvanceFrame(dt);

    int frame = (int)m_CurrentFrame;

    // target mesh がボーン行列を構築してくれる
    m_TargetMesh->Update(m_BoneMatrix, frame);

    // 定数バッファ更新
    m_BoneMatrix.Update();
}