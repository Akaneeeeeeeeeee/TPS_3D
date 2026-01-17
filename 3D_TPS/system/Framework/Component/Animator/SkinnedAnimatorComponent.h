#pragma once

#include "Framework/Component/IComponent/IComponent.h"
#include "system/CAnimationObject.h"
#include "system/CAnimationMesh.h"
#include "Animator.h"
#include "system/CShader.h"

// 前方宣言
class AssetManager;

// 将来増えてもいい
enum class AnimType
{
    Idle = 0,
    Covered_Idle,
	Crouch,
	CrouchWalk,
    Walk,
	LookAround,         // 周囲確認
	Surprise_RightTurn, // 右振り向き
	Surprise_LeftTurn,  // 左振り向き
	Check_OverWall,
	StoneThrow,
	GunShot,
    Run,
    Max,
};

struct AnimClipRef
{
    AnimType type{};
    std::string dataName;   // AnimationData の名前
    std::string clipName;   // その中のクリップ名
    int index = 0;
    float speed = 1.0f;
};

struct SkinnedAnimSetup
{
    std::string meshName;
    std::string shaderName;
    std::vector<AnimClipRef> clips;
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
	DECLARE_COMPONENT_TYPE(SkinnedAnimationComponent, IComponent)
    SkinnedAnimationComponent() = default;
    ~SkinnedAnimationComponent() override = default;

    void Attach(EngineServices& ctx) override;
    void Detach(void) override;

    void Init(void) override;
    void Update(const float dt) override;
    void Uninit(void) override;
    void Draw(void) const override;

    // ---- 設定系 ----

    // 共有メッシュを設定（AssetManager から取得したもの）
    void SetMesh(CAnimationMesh* mesh) { m_pMesh = mesh; }

    // シェーダ設定
    void SetShader(CShader* shader) { m_pShader = shader; }

    // アニメクリップのキャッシュ
    void SetClip(AnimType type, aiAnimation* clip, float speed = 1.0f);

    // アニメ遷移（Player / Enemy から呼ぶ用）
    void Play(AnimType type, float blendTimeSec, bool loop = true);
    bool IsPlaying(AnimType type) const;

    // 強制的に現在クリップを差し替え（ブレンドを切る）
    void ForceSet(AnimType type, float startTimeSec = 0.0f, bool loop = true);

    // 再生速度設定
    void SetPlaybackSpeed(float s) { m_PlaybackSpeed = std::max(0.0f, s); }

	// アセット情報からセットアップ
    void SetupFromAssets(const SkinnedAnimSetup& setup);

    // ===== 外から制御 =====
    bool  IsCurrentFinished() const { return m_Animator.IsFinished(); }
    float GetCurrentTimeSec() const { return m_Animator.GetCurrentTimeSec(); }
    float GetCurrentDurationSec() const { return m_Animator.GetCurrentDurationSec(); }
    float GetCurrentNormalizedTime() const { return m_Animator.GetCurrentNormalizedTime(); }

    void  SetCurrentTimeSec(float sec) { m_Animator.SetCurrentTimeSec(sec); }
    void  SetCurrentNormalizedTime(float t01) { m_Animator.SetCurrentNormalizedTime(t01); }

	aiAnimation* GetCurrentClip() const { return m_Animator.GetCurrentClip(); }
    aiAnimation* GetClipPtr(AnimType type) const;

    // 描画（GameObject::Draw から呼ぶ）
    void DebugImGui();

    BoneCombMatrix* GetBones() { return m_AnimObject ? &m_AnimObject->GetBoneCombMatrix() : nullptr; }

private:
    void ApplySetupIfPossible();
    void ApplyMeshToRuntimeIfReady(); // Init 後に Mesh が来た場合の救済
    float GetSpeedScaleForClip(aiAnimation* clip) const;

private:
    struct ClipInfo
    {
        aiAnimation* clip = nullptr;
        float        speed = 1.0f;
    };

    AssetManager* m_asset = nullptr;  // 非所有

	std::optional<SkinnedAnimSetup> m_PendingSetup; // 初期化前にセットされたセットアップ情報
	bool m_SetupApplied = false;                    // セットアップ適用済みか?

	// 再生速度倍率
    float m_PlaybackSpeed = 1.0f;

    // どの AnimType にどの aiAnimation* を割り当てるか
    ClipInfo m_Clips[static_cast<int>(AnimType::Max)]{};

    CAnimationMesh* m_pMesh = nullptr;  // 共有メッシュ
    std::unique_ptr<CAnimationObject> m_AnimObject;    // 1体専用
    Animator m_Animator;                // 1体専用
    CShader* m_pShader = nullptr;
};
