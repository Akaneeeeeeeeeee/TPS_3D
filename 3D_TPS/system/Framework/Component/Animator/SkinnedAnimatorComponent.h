#pragma once

#include "Framework/Component/IComponent/IComponent.h"
#include "system/CAnimationObject.h"
#include "system/CAnimationMesh.h"
#include "Animator.h"
#include "system/CShader.h"

// 将来増えてもいい
enum class AnimType
{
    Idle = 0,
	Crouch,
	CrouchWalk,
    Walk,
	Surprise_RightTurn, // 右振り向き
	Surprise_LeftTurn,  // 左振り向き
    Run,
    Max,
};

/*
* @brief	スキンメッシュアニメーションコンポーネント
* @detail	スキンメッシュアニメーションを扱うコンポーネント
* @remark	共有メッシュと専用アニメーションオブジェクトを持ち、アニメーションの再生・遷移を管理する
* @auther	赤根 和樹
* @date     2025/11/23
*/
class SkinnedAnimationComponent : public IComponent
{
public:
    SkinnedAnimationComponent() = default;
    ~SkinnedAnimationComponent() override = default;

    void Attach(EngineContext& ctx) override {}
    void Detach(void) override {}

    void Init(void) override;
    void Update(const float dt) override;
    void Uninit(void) override;

    // ---- 設定系 ----

    // 共有メッシュを設定（AssetManager から取得したもの）
    void SetMesh(CAnimationMesh* mesh) { m_pMesh = mesh; }

    // シェーダ設定
    void SetShader(CShader* shader) { m_pShader = shader; }

    // アニメクリップのキャッシュ
    void SetClip(AnimType type, aiAnimation* clip, float speed = 1.0f);

    // アニメ遷移（Player / Enemy から呼ぶ用）
    void Play(AnimType type, float blendTimeSec);

    // 描画（GameObject::Draw から呼ぶ）
    void Draw() const;
    void DebugImGui();
private:
    struct ClipInfo
    {
        aiAnimation* clip = nullptr;
        float        speed = 1.0f;
    };

    // どの AnimType にどの aiAnimation* を割り当てるか
    ClipInfo m_Clips[static_cast<int>(AnimType::Max)]{};

    CAnimationMesh* m_pMesh = nullptr;  // 共有メッシュ
    std::unique_ptr<CAnimationObject> m_AnimObject;    // 1体専用
    Animator m_Animator;                // 1体専用
    CShader* m_pShader = nullptr;
	bool m_CurrentLoop = true;
};
