#include "Player.h"
#include "system/CDirectInput.h"

Player::Player(uint64_t id, const std::string& name, const Tag& tag)
	: Character(id, name, tag)
{
}

Player::~Player()
{
}

void Player::Init(void)
{
	// アニメーションオブジェクトを生成
	this->m_pAnimationObject = std::make_unique<CAnimationObject>();
	this->m_pAnimationObject->Init();

	// メッシュを取得
	this->m_pAnimationMesh = AssetManager::GetInstance().GetAnimationMesh("Akai");
	// シェーダーの初期化
	this->m_Shader.Create("shader/vertexLightingOneSkinVS.hlsl", "shader/vertexLightingPS.hlsl");
	// アニメーションデータ取得
	this->m_pAnimationData = AssetManager::GetInstance().GetAnimationData("Akai_Idle");
	// 現在のアニメーションをセット
	aiAnimation* animation = m_pAnimationData->GetAnimation("Akai_Idle", 0);
	this->m_pCurrentAnimation = animation;
	m_pAnimationMesh->SetCurentAnimation(animation);
	// アニメーションメッシュをセット
	this->m_pAnimationObject->SetAnimationMesh(m_pAnimationMesh);

	// ステータスを設定
	this->m_MoveSpeed = 10.0f;
	this->m_AnimationSpeed = 1.0f;
}

void Player::Update(uint64_t deltatime)
{
	// 入力処理
	CDirectInput& input = CDirectInput::GetInstance();

    Vector3 move(0, 0, 0);
	Vector3 pos = m_Transform.GetPosition();

    if (input.CheckKeyBuffer(DIK_W)) { move.z += 1.0f; }
    if (input.CheckKeyBuffer(DIK_S)) { move.z -= 1.0f; }
    if (input.CheckKeyBuffer(DIK_A)) { move.x -= 1.0f; }
    if (input.CheckKeyBuffer(DIK_D)) { move.x += 1.0f; }

	aiAnimation* animdata;

	// 移動ベクトルが0でなければ正規化して移動
    if (move.Length() > 0.0f) {
		// 移動ベクトルを正規化
        move.Normalize();
		// 入力に基づいてキャラクターの向きを更新
        float targetYaw = std::atan2(-move.x, -move.z);
		
		// Y軸回転をクォータニオンで適用
		Quaternion q = Quaternion::CreateFromAxisAngle(Vector3(0, 1, 0), targetYaw);

		// 前方向ベクトルを計算して移動
        Matrix4x4 rotY = Matrix4x4::CreateFromQuaternion(q);
        Vector3 forward = Vector3::TransformNormal(Vector3(0, 0, -1), rotY);

		// 移動
        pos += forward * m_MoveSpeed;

		// Transform更新
		m_Transform.SetPosition(pos);
		m_Transform.SetRotation(q);

		// 入力があれば移動アニメーションを再生
		// アニメーションデータ取得
		// アニメーションを Run に切り替え
		animdata = AssetManager::GetInstance().GetAnimationData("Akai_Run")->GetAnimation("Akai_Run", 0);
		if (m_pCurrentAnimation != animdata) {
			m_pCurrentAnimation = animdata;
			m_pAnimationMesh->SetCurentAnimation(m_pCurrentAnimation);
		}
	}
	else {
        // 入力がない場合はアニメーションをアイドル状態に変更
		// アニメーションデータ取得
		// アニメーションを Idle に切り替え
		animdata = AssetManager::GetInstance().GetAnimationData("Akai_Idle")->GetAnimation("Akai_Idle", 0);
		if (m_pCurrentAnimation != animdata) {
			m_pCurrentAnimation = animdata;
			m_pAnimationMesh->SetCurentAnimation(m_pCurrentAnimation);
		}
    }

    // ワールド行列をセット
	Matrix4x4 world = this->GetWorldMatrix();
    Renderer::SetWorldMatrix(&world);

    // カメラ位置更新
	// --- カメラ回転 ---
	static float azimuth = m_pCamera->GetAzimuth();
	static float elevation = m_pCamera->GetElevation();

	// 右ボタンが押されているとき
	if (input.GetMouseRButtonCheck()) {
		LONG dx = input.GetMouseStateData().lX;
		LONG dy = input.GetMouseStateData().lY;

		// マウス感度
		float sensitivity = 0.005f;

		// マウスの移動量に応じてカメラの角度を更新
		azimuth += dx * sensitivity;
		elevation -= dy * sensitivity;

		// 仰角の制限(-89°～89°の範囲に制限)
		// TODO: クォータニオンを使った方法に変更し、ジンバルロックを防止・障害物に当たったら壁ずりしてキャラに近づく処理を入れたい
		const float limit = (PI / 2.0f) - 0.01f;
		if (elevation > limit) elevation = limit;
		if (elevation < -limit) elevation = -limit;
	}

	// カメラ位置更新
	// TPSなのでカメラはプレイヤーから一定距離離れる
	m_pCamera->SetRadius(800.0f);
	m_pCamera->SetAzimuth(azimuth);
	m_pCamera->SetElevation(elevation);
	pos.y += 100.0f;	// 注視点を少し上にずらす
	m_pCamera->SetLookat(pos);
	m_pCamera->CalcCameraPositionTranslate(pos);

    m_pAnimationObject->Update(m_AnimationSpeed);
}

void Player::Draw(uint64_t deltatime)
{
	this->m_pCamera->Draw();
	// シェーダーをセット
	m_Shader.SetGPU();

	// ワールド行列をセット
	Matrix4x4 worldMatrix = this->GetWorldMatrix();
	Renderer::SetWorldMatrix(&worldMatrix);

	m_pAnimationObject->Draw();
}

void Player::Uninit(void)
{
}