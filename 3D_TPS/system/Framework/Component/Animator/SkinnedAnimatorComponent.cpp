#include "SkinnedAnimatorComponent.h"
#include "Animator.h"
#include "Framework/GameObject/GameObject.h"
#include "system/Renderer.h"

void SkinnedAnimationComponent::Attach(EngineServices& ctx)
{
    m_asset = &ctx.asset;
    ApplySetupIfPossible();
}

void SkinnedAnimationComponent::Detach()
{
    m_asset = nullptr;
}

void SkinnedAnimationComponent::SetupFromAssets(const SkinnedAnimSetup& setup)
{
    m_PendingSetup = setup;
    m_SetupApplied = false;
    ApplySetupIfPossible();
}


void SkinnedAnimationComponent::SetClip(AnimType type, aiAnimation* clip, float speed)
{
    if (!clip) return;
    int idx = static_cast<int>(type);
    m_Clips[idx].clip = clip;
    m_Clips[idx].speed = speed;

    if (type == AnimType::Idle && !m_Animator.GetCurrentClip())
    {
        m_Animator.SetInitialClip(clip, 0.0f, true);
    }
}

aiAnimation* SkinnedAnimationComponent::GetClipPtr(AnimType type) const
{
    int idx = static_cast<int>(type);
    return m_Clips[idx].clip;
}

float SkinnedAnimationComponent::GetSpeedScaleForClip(aiAnimation* clip) const
{
    if (!clip) return 1.0f;
    for (int i = 0; i < static_cast<int>(AnimType::Max); ++i)
    {
        if (m_Clips[i].clip == clip)
            return m_Clips[i].speed;
    }
    return 1.0f;
}

void SkinnedAnimationComponent::ApplySetupIfPossible()
{
    if (m_SetupApplied) { return; }
    if (!m_asset) { return; }
    if (!m_PendingSetup.has_value()) { return; }

    const auto& s = *m_PendingSetup;

    // Mesh
    if (!s.meshName.empty())
    {
        auto* mesh = m_asset->GetMesh<CAnimationMesh>(s.meshName);
        SetMesh(mesh);
        ApplyMeshToRuntimeIfReady(); // Init後でも反映できるように
    }

    // Shader
    if (!s.shaderName.empty())
    {
        auto* shader = m_asset->GetShader<CShader>(s.shaderName);
        SetShader(shader);
    }

    // Clips
    for (const auto& c : s.clips)
    {
        auto* data = m_asset->GetAnimationData<CAnimationData>(c.dataName);
        if (!data) continue;

        auto* clip = data->GetAnimation(c.clipName.c_str(), c.index);
        if (!clip) continue;

        SetClip(c.type, clip, c.speed);
    }

    m_SetupApplied = true;
}

void SkinnedAnimationComponent::Play(AnimType type, float blendTimeSec, bool loop)
{
    int idx = static_cast<int>(type);
    const auto& info = m_Clips[idx];
    if (!info.clip) return;

    // 既定はループ（必要であれば ForceSet(loop=false) する）
    m_Animator.RequestTransition(info.clip, blendTimeSec, loop);
}


bool SkinnedAnimationComponent::IsPlaying(AnimType type) const
{
    return m_Animator.GetCurrentClip() == GetClipPtr(type);
}

void SkinnedAnimationComponent::ForceSet(AnimType type, float startTimeSec, bool loop)
{
    int idx = static_cast<int>(type);
    const auto& info = m_Clips[idx];
    if (!info.clip) return;

    m_Animator.ForceSetClip(info.clip, startTimeSec, loop);
}

void SkinnedAnimationComponent::ApplyMeshToRuntimeIfReady()
{
    // Init後なら m_AnimObject が存在するので、その場で反映
    if (m_AnimObject && m_pMesh)
    {
        m_AnimObject->SetAnimationMesh(m_pMesh);
    }
}

void SkinnedAnimationComponent::Init()
{
    m_AnimObject = std::make_unique<CAnimationObject>();
    m_AnimObject->Init();

    // Setup が先に済んでればここで反映される
    if (m_pMesh)
    {
        m_AnimObject->SetAnimationMesh(m_pMesh);
    }

    // Attach 済みで Setup が未適用だった場合もあるので一応
    ApplySetupIfPossible();
}

void SkinnedAnimationComponent::Update(const float dt)
{
    // m_PlaybackSpeed と clip の speed を反映
    aiAnimation* cur = m_Animator.GetCurrentClip();
    float clipScale = GetSpeedScaleForClip(cur);

    const float scaledDt = dt * m_PlaybackSpeed * clipScale;

    m_Animator.Update(scaledDt);

    // ボーン行列計算
    if (m_AnimObject)
    {
        // CPU側の BoneCombMatrix.ConstantBufferMemory を更新
        m_AnimObject->UpdateFromAnimator(m_Animator);

        // GPU側の定数バッファへ反映（Map/Unmap）
        m_AnimObject->GetBoneCombMatrix().Update();
    }
}

void SkinnedAnimationComponent::Uninit(void)
{
    m_AnimObject.reset();
}

void SkinnedAnimationComponent::Draw() const
{
}