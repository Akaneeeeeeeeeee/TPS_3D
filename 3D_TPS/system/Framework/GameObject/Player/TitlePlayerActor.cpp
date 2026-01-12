#include "TitlePlayerActor.h"
#include "system/meshmanager.h"
#include "system/Framework/Component/Animator/SkinnedAnimatorComponent.h"
#include "system/Framework/AssetManager/AssetManager.h"
#include "system/Framework/Component/Physic/CharacterVirtualComponent.h"
#include "Framework/GameObject/Terrain/Terrain.h"
#include <cmath>
#include "Framework/Component/Physic/StaticMeshCollider.h"
#include "Framework/Component/Renderer/MeshRenderer/SkinnedMeshRendererComponent.h"

namespace
{
	constexpr float BLEND_SEC = 0.15f;
	constexpr Vector3 UP = Vector3(0, 1, 0);

	// タイトル用の見た目速度（ゲームプレイより演出優先で調整）
	constexpr float TITLE_WALK_SPEED = 120.0f;
	constexpr float TITLE_RUN_SPEED = 220.0f;

	constexpr float AMOUNT_TO_IDLE = 0.05f;
	constexpr float AMOUNT_TO_RUN = 0.90f;

	constexpr float PLAYER_CAPSULE_HALF_HEIGHT = 60.0f; // カプセル高さ(半分)
	constexpr float PLAYER_CAPSULE_RADIUS = 35.0f;      // カプセル半径

	// 注視点
	constexpr Vector3 CAMERA_LOOKAT_POSITION = Vector3(1920.0f, 275.0f, -3500.0f);

	// 姿勢ごとの移動速度係数（現状 CharacterVirtualComponent 側で係数管理しているなら未使用）
	constexpr float CROUCH_MOVE_SPEED_FACTOR = 0.5f;
	constexpr float PRONE_MOVE_SPEED_FACTOR = 0.25f;

}

void TitlePlayerActor::Awake(void)
{
	auto& am = AssetManager::GetInstance();

	// ---- 見た目（スキン） ----
	m_pAnimComp = AddComponent<SkinnedAnimationComponent>("TitleSkinnedAnim");

	// 1) コンポーネント追加
	m_pAnimComp = AddComponent<SkinnedAnimationComponent>("SkinnedAnim");

	SkinnedAnimSetup setup{};
	setup.meshName = "Akai";
	setup.shaderName = "animshader";
	setup.clips = {
		{ AnimType::Covered_Idle,       "Cover_Idle",         "Cover_Idle",         0, 1.0f },
		{ AnimType::StoneThrow,       "StoneThrow",          "StoneThrow",          0, 1.0f },
		{ AnimType::CrouchWalk, "Title_Sneaking", "Title_Sneaking", 0, 1.0f },
		{ AnimType::Check_OverWall, "checkOverWall",       "checkOverWall",       0, 1.0f },
		{ AnimType::Run,        "Akai_Run",         "Akai_Run",         0, 1.0f },
		{ AnimType::Crouch,     "Crouching_Idle",   "Crouching_Idle",   0, 1.0f },
	};

	// 2) アセット情報からセットアップ
	m_pAnimComp->SetupFromAssets(setup);

	// タイトル用は台本で動かすので、MoveSpeedは演出値に
	m_MoveSpeed = TITLE_WALK_SPEED;

	// カメラ追加
	{
		m_pCamera = this->AddComponent<CameraComponent>("TitleCameraComponent");
		Vector3 campos = this->GetPosition() - Vector3(0.0f, 100.0f, 550.0f);
		m_pCamera->SetPosition(campos);
		// 低い位置から、少し離れて見上げる
		m_pCamera->SetLookAt(CAMERA_LOOKAT_POSITION);
		m_pCamera->SetMode(CameraComponent::Mode::Direct);
	}

	// CharacterVirtual 必須（地形当たり判定のため）
	{
		m_pCharaVirtualComp = AddComponent<CharacterVirtualComponent>("TitleCharacterVirtualComponent");
		m_pCharaVirtualComp->SetCapsule(PLAYER_CAPSULE_HALF_HEIGHT, PLAYER_CAPSULE_RADIUS);
	}

	// 描画（RenderManagerに出す）
	{
		auto* r = AddComponent<SkinnedMeshRendererComponent>("SkinnedRenderer");
		r->SetMeshKey("Akai");
		r->SetShaderKey("animshader");
		r->SetAnimator(m_pAnimComp);    // アニメーターをセット
	}

	m_MoveDir = Vector3(0.5f, 0.0f, -0.75f);
	m_MoveAmount = 1.0f;
}

void TitlePlayerActor::Start(void)
{
	// 初期はしゃがみ状態に
	m_IsCrouching = true;

	if (m_pCharaVirtualComp)
	{
		m_pCharaVirtualComp->SetStance(CharacterVirtualComponent::Stance::Crouch);
		m_pCharaVirtualComp->SetMoveInput(Vector3::Zero, 0.0f);
	}

	//SetupFixedTitleCamera();	// ここで一度だけ確定
}

namespace
{
	static Vector3 NormalizeXZOr(Vector3 v, const Vector3& fallback)
	{
		v.y = 0.0f;
		if (v.LengthSquared() < 1e-6f) return fallback;
		v.Normalize();
		return v;
	}

	static float YawFromDirXZ(const Vector3& dir)
	{
		// 座標系に合わせた atan2（既存と同じ）
		return std::atan2(-dir.x, -dir.z);
	}
}

void TitlePlayerActor::Update(const float dt)
{
	// ---- 姿勢（しゃがみ等） ----
	if (m_pCharaVirtualComp)
	{
		m_pCharaVirtualComp->SetStance(
			m_IsCrouching ? CharacterVirtualComponent::Stance::Crouch
			: CharacterVirtualComponent::Stance::Stand
		);
	}

	if (m_UseTargetPose)
	{
		if (m_pCharaVirtualComp) m_pCharaVirtualComp->Teleport(m_TargetPos);
		m_Transform.SetRotation(m_TargetRot);
		if (m_pCharaVirtualComp) m_pCharaVirtualComp->SetMoveInput(Vector3::Zero, 0.0f);

		m_UseTargetPose = false;

		GameObject::Update(dt);
		ApplyAnimation(dt);
		return;
	}

	// =========================================================
	// A) 移動入力（MoveDir / MoveAmount）
	// =========================================================
	const float amount = std::clamp(m_MoveAmount, 0.0f, 1.0f);

	Vector3 move_dir = NormalizeXZOr(m_MoveDir, m_LastMoveDir);

	if (amount > AMOUNT_TO_IDLE)
	{
		m_LastMoveDir = move_dir; // 停止した瞬間にゼロ化して向きが壊れないよう保持
		if (m_pCharaVirtualComp) m_pCharaVirtualComp->SetMoveInput(move_dir, amount);
	}
	else
	{
		if (m_pCharaVirtualComp) m_pCharaVirtualComp->SetMoveInput(Vector3::Zero, 0.0f);
	}

	// =========================================================
	// B) 体の向き（FaceMode で決める）
	// =========================================================
	Vector3 face_dir = Vector3::Zero;

	switch (m_FaceMode)
	{
	case FaceMode::FaceMoveDir:
		// 「止まってる時は last を使う」でも良い
		face_dir = (amount > AMOUNT_TO_IDLE) ? move_dir : m_LastMoveDir;
		break;

	case FaceMode::FaceManualDir:
		face_dir = NormalizeXZOr(m_FaceDir, m_LastMoveDir);
		break;

	case FaceMode::FaceTargetPos:
		face_dir = NormalizeXZOr(m_FaceTargetPos - this->GetPosition(), m_LastMoveDir);
		break;
	}

	// Player と同じ：有効なときだけ回転を更新
	if (face_dir.LengthSquared() > 1e-6f)
	{
		// まずは「見たい向き(face_dir)」を向く
		float yaw = YawFromDirXZ(face_dir);

		// 横移動演出したいときだけ +90度（右向き補正）
		if (m_SidewaysRight)
			yaw += (PI * 0.5f);

		m_Transform.SetRotation(Quaternion::CreateFromAxisAngle(UP, yaw));
	}

	// カメラ更新
	if (m_pCamera)
	{
		m_pCamera->ApplyCamera();
	}

	// ---- アニメ ----
	ApplyAnimation(dt);
}

void TitlePlayerActor::Draw(void) const
{
}

void TitlePlayerActor::Uninit(void)
{
	Character::Uninit();
}

void TitlePlayerActor::ApplyMovement(float dt)
{
	// 1) 台本が姿勢を直接指定するモード
	if (m_UseTargetPose)
	{
		m_Transform.SetPosition(m_TargetPos);
		m_Transform.SetRotation(m_TargetRot);
		return;
	}

	// 2) dir/amount で動かすモード
	if (m_MoveAmount < AMOUNT_TO_IDLE) return;

	Vector3 dir = m_MoveDir;
	if (dir.LengthSquared() < 1e-6f) return;
	dir.Normalize();

	// amountで walk/run を切替
	const bool isRun = (m_MoveAmount >= AMOUNT_TO_RUN);
	const float speed = isRun ? TITLE_RUN_SPEED : TITLE_WALK_SPEED;

	Vector3 pos = m_Transform.GetPosition();
	pos += dir * (speed * m_MoveAmount * dt);
	m_Transform.SetPosition(pos);

	// 向きを進行方向へ
	//if (m_FaceMoveDir)
	//{
	//	// あなたの座標系（前が -Z など）に合わせて符号調整
	//	const float yaw = std::atan2(-dir.x, -dir.z);
	//	m_Transform.SetRotation(Quaternion::CreateFromAxisAngle(UP, yaw));
	//}
}

void TitlePlayerActor::ApplyAnimation(float dt)
{
	if (!m_pAnimComp) { return; }

	TitleAnim anim = m_TargetAnim;

	// 入力から自動決定したい場合はここで上書き（必要なら）
	// 今は台本が SetTargetAnim する前提なら、この自動判定は消してもOK

	switch (anim)
	{
	case TitleAnim::Idle:
		m_pAnimComp->Play(AnimType::Covered_Idle, BLEND_SEC);
		break;

	case TitleAnim::Walk:
		// Walk ではなく CrouchWalk を再生
		m_pAnimComp->Play(AnimType::CrouchWalk, BLEND_SEC);
		break;

	case TitleAnim::Run:
		m_pAnimComp->Play(AnimType::Run, BLEND_SEC);
		break;

	case TitleAnim::CheckOverWall:
		m_pAnimComp->Play(AnimType::Check_OverWall, BLEND_SEC);
		break;

	case TitleAnim::ThrowStone:
		m_pAnimComp->Play(AnimType::StoneThrow, BLEND_SEC);
		break;
	}

	m_pAnimComp->SetPlaybackSpeed(1.0f);
}

namespace
{
	static Vector3 NormalizeXZ(Vector3 v, const Vector3& fallback)
	{
		v.y = 0.0f;
		if (v.LengthSquared() < 1e-6f) return fallback;
		v.Normalize();
		return v;
	}

	static float PlayerBodyHeight()
	{
		return (PLAYER_CAPSULE_HALF_HEIGHT * 2.0f) + (PLAYER_CAPSULE_RADIUS * 2.0f);
	}
}

void TitlePlayerActor::SetupFixedTitleCamera()
{
	if (!m_pCamera) return;

	const float bodyH = PlayerBodyHeight();

	// プレイヤーの向き（無効なら +Z を仮採用）
	Vector3 forward = NormalizeXZ(this->GetForward(), Vector3(0, 0, 1));
	Vector3 up = Vector3(0, 1, 0);

	// 右方向
	Vector3 right = up.Cross(forward);
	right = NormalizeXZ(right, Vector3(1, 0, 0));

	// オフセット（全部 “体格比” なのでマジックナンバーが消える）
	const float frontDist = bodyH * 3.5f;   // 手前(前方)に置く距離
	const float sideDist = bodyH * 0.8f;   // 斜め成分（0にすると正面）
	const float eyeHeight = bodyH * 0.15f;  // 地上付近（好みで 0.1～0.25）
	const float lookHeight = bodyH * 0.25f;  // 見る位置（胸～頭）

	Vector3 playerPos = this->GetPosition();
	Vector3 lookAt = playerPos + up * lookHeight;

	// カメラのXZ（前方＋少し横）
	Vector3 camXZ = playerPos + forward * frontDist + right * sideDist;

	// 地面Yを Terrain から取得（失敗したらプレイヤー足元付近を使う）
	float groundY = playerPos.y - PLAYER_CAPSULE_HALF_HEIGHT;

	if (m_Terrain)
	{
		if (auto* col = m_Terrain->GetComponent<StaticMeshCollider>())
		{
			float y;
			if (col->SampleHeight(camXZ.x, camXZ.z, y))
			{
				groundY = y;
			}
		}
	}

	Vector3 camPos(camXZ.x, groundY + eyeHeight, camXZ.z);

	// 固定カメラ：位置と注視点だけセット（Radius/Elevation/Azimuth は使わない）
	m_pCamera->SetPosition(camPos);
	m_pCamera->SetLookAt(lookAt);
}
