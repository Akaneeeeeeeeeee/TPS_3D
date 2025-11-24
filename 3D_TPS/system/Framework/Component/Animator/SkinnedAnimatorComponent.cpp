#include "SkinnedAnimatorComponent.h"
#include "Animator.h"
#include "Framework/GameObject/GameObject.h"
#include "system/Renderer.h"   // あなたの環境に合わせる

void SkinnedAnimationComponent::SetClip(AnimType type,
    aiAnimation* clip,
    float speed)
{
    if (!clip) return;
    int idx = static_cast<int>(type);
    m_Clips[idx].clip = clip;
    m_Clips[idx].speed = speed;

    // 最初に Idle が設定されたタイミングで Animator を初期化してもいい
    if (type == AnimType::Idle && !m_Animator.GetCurrentClip())
    {
        m_Animator.SetInitialClip(clip, 0.0f);
    }
}

void SkinnedAnimationComponent::Play(AnimType type, float blendTimeSec)
{
    int idx = static_cast<int>(type);
    const auto& info = m_Clips[idx];
    if (!info.clip) return;

    // 速度はとりあえず timeScale 側でまとめて制御するなら
    // Animator 側はそのまま RequestTransition だけでOK
    m_Animator.RequestTransition(info.clip, blendTimeSec);
}


void SkinnedAnimationComponent::Init(void)
{
    m_AnimObject = std::make_unique<CAnimationObject>();
    m_AnimObject->Init();

    if (m_pMesh)
    {
        m_AnimObject->SetAnimationMesh(m_pMesh);
    }
}

void SkinnedAnimationComponent::Update(const float dt)
{
    // アニメ時間を進める
    m_Animator.Update(dt, m_TimeScale);

    // ボーン行列計算
    if (m_AnimObject)
    {
        m_AnimObject->UpdateFromAnimator(m_Animator);
    }
}

void SkinnedAnimationComponent::Uninit(void)
{
    m_AnimObject.reset();
}

void SkinnedAnimationComponent::Draw() const
{
    if (!m_AnimObject) return;

    // シェーダセット
    m_pShader->SetGPU();

    // オーナーのワールド行列をセット
    if (m_pOwner)
    {
        Matrix4x4 world = m_pOwner->GetWorldMatrix();
        Renderer::SetWorldMatrix(&world);
    }

    // メッシュ描画（ボーン行列は Update で仕込んだ状態）
    m_AnimObject->Draw();
}
