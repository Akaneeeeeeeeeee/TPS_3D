#include "Goal.h"
#include "system/meshmanager.h"
#include "system/CStaticMeshRenderer.h"
#include "system/CShader.h"
#include "Framework/Component/Physic/StaticMeshCollider.h"
#include "Framework/Component/Physic/Rigidbody.h"
#include "Framework/Component/Renderer/MeshRenderer/StaticMeshRenderer.h"
#include "Framework/GameObject/Player/Player.h"
#include "Framework/ObjectManager/ObjectManager.h"
#include "Framework/Component/Light/SpotLightComponent.h"

void Goal::Awake(void)
{
	auto& am = AssetManager::GetInstance();
	m_mesh = am.GetMesh<CStaticMesh>("goalmesh");

	// 描画コンポーネント
	m_RenderComp = AddComponent<StaticMeshRendererComponent>("GoalRenderer");
	m_RenderComp->SetMeshRendererKey("goalmesh");
	m_RenderComp->SetShaderKey("unlightshader");
	m_RenderComp->SetTransparent(false);

	// コライダー
	{
		auto collider = AddComponent<StaticMeshCollider>("GoalMeshCollider");
		collider->SetMesh(*m_mesh);
	}

	// Rigidbody
	{
		auto rb = AddComponent<Rigidbody>("Rigidbody", 1.0f);
		rb->SetBodyType(Rigidbody::Static);
		rb->SetObjectLayer(Layers::NON_MOVING);
	}

    // BeamA / BeamB を生成（※あなたの生成APIに合わせて）
    m_BeamA = m_pObjectManager->Instantiate<GameObject>("BeamA", Tag::Light, this->TransformRef());
    m_BeamB = m_pObjectManager->Instantiate<GameObject>("BeamB", Tag::Light, this->TransformRef());

    // 位置は Goal の上
    const Vector3 top = GetPosition() + Vector3(0, m_BeamHeight, 0);
    m_BeamA->SetPosition(top);
    m_BeamB->SetPosition(top);

    // SpotLight 付与
    m_SpotA = m_BeamA->AddComponent<SpotLightComponent>("SpotA");
    m_SpotB = m_BeamB->AddComponent<SpotLightComponent>("SpotB");

    // 向きはオーナーの Forward を使う
    //m_SpotA->SetAimOwnerForward();
    //m_SpotB->SetAimOwnerForward();

    //// 見た目・強さ（例）
    //m_SpotA->SetColor(Color(1, 0.95f, 0.8f, 1));
    //m_SpotB->SetColor(Color(1, 0.95f, 0.8f, 1));
    //m_SpotA->SetIntensity(6.0f);
    //m_SpotB->SetIntensity(6.0f);

    //// 「地面の円」を狙う設定を自動で作る（数値はゲームに合わせて）
    //const float groundY = 0.0f;
    //const float groundRadius = 200.0f;   // 地面に映る円の半径
    //const float topRadiusMin = 10.0f;    // ライト根元の太さ
    //const float innerRatio = 0.6f;       // 内側の強い部分の割合
    //const float nearMinAxis = 10.0f;
    //const float rangeExtraAxis = 50.0f;

    //m_SpotA->FitToGroundCircle(groundY, groundRadius, topRadiusMin, innerRatio, nearMinAxis, rangeExtraAxis);
    //m_SpotB->FitToGroundCircle(groundY, groundRadius, topRadiusMin, innerRatio, nearMinAxis, rangeExtraAxis);

    for (auto* s : { m_SpotA, m_SpotB })
    {
        if (!s) continue;

        s->SetAimOwnerForward();
        s->SetColor(Color(1, 0.95f, 0.8f, 1));
        s->SetIntensity(10.0f);

        s->SetRange(1200.0f);        // 長く
        s->SetAnglesDeg(3.0f, 30.0f);  // outerを小さくして細く
        s->SetTopRadius(8.0f);        // 根元も細くしたいなら
        s->SetNear(20.0f);            // 近すぎる所を消したいなら
    }

    m_DayNight = AddComponent<DayNightObserverComponent>("DayNight");

    if (m_DayNight)
        m_DayNight->SetReceiver(this);

}

void Goal::Update(const float deltatime)
{
    // 例：回転速度（ラジアン/秒）
    const float yawSpeed = PI * 0.5f; // 90度/秒

    m_Yaw += yawSpeed * deltatime;

    // 値が増え続けないように折り返し
    if (m_Yaw >= PI * 2.0f) m_Yaw -= PI * 2.0f;

    // --- BeamA ---
    {
        const float half = m_Yaw * 0.5f;
        const float sy = std::sin(half);
        const float cy = std::cos(half);

        // Y軸回転クォータニオン（x,y,z,w の並び前提）
        const Quaternion qYaw(0.0f, sy, 0.0f, cy);

        m_BeamA->SetRotation(qYaw);
    }

    // --- BeamB（180度反対）---
    {
        const float yawB = m_Yaw + PI;
        const float half = yawB * 0.5f;
        const float sy = std::sin(half);
        const float cy = std::cos(half);

        const Quaternion qYaw(0.0f, sy, 0.0f, cy);
        m_BeamB->SetRotation(qYaw);
    }
}

void Goal::Draw(void) const
{
}

void Goal::Uninit(void)
{
    if (m_DayNight) m_DayNight->SetReceiver(nullptr);

    m_DayNight = nullptr;
    m_SpotA = nullptr;
    m_SpotB = nullptr;
}

void Goal::OnCollisionCharacterEnter(GameObject& other)
{
	// プレイヤーがゴールに触れたらクリア
	if (other.GetTag() == Tag::Player)
	{
		m_Reached = true;
	}
}

void Goal::OnDayNightChanged(bool isNight)
{
    m_IsNight = isNight;
    RefreshLighting();
}

void Goal::RefreshLighting()
{
    bool wantLit = m_LightEnabled;
    if (m_NightOnly)
        wantLit = wantLit && m_IsNight;

    if (wantLit == m_RuntimeLit) return;
    m_RuntimeLit = wantLit;

    if (m_SpotA) m_SpotA->SetEnabled(wantLit);
    if (m_SpotB) m_SpotB->SetEnabled(wantLit);
}