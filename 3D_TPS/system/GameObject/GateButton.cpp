#include "GateButton.h"
#include "Framework/Component/Physic/Rigidbody.h"
#include "Framework/Component/Physic/BoxCollider.h"
#include "Framework/Component/Renderer/MeshRenderer/StaticMeshRenderer.h"
#include "Framework/ObjectManager/ObjectManager.h"
#include "Framework/GameObject/Player/Player.h"
#include "Gate.h"
#include "system/CDirectInput.h"
#include "Framework/Component/Sound/SoundEmitterComponent.h"

#include <algorithm>


void GateButton::Awake()
{
    // 見た目用スケールを退避して、親は1に戻す
    m_ModelScale = GetScale();
    SetScale(Vector3::One);

    // 近接判定（トリガー）は「親」に付ける（固定サイズ）
    //m_TriggerBox = AddComponent<BoxCollider>("ButtonTrigger");
    //m_TriggerBox->SetHalfSize(DirectX::XMFLOAT3(80.0f, 80.0f, 80.0f));

    //m_RB = AddComponent<Rigidbody>("ButtonRB", 1.0f);
    //m_RB->SetBodyType(Rigidbody::Kinematic);
    //m_RB->SetAsTrigger(true);
    //m_RB->SetObjectLayer(Layers::TRIGGER);

	//m_SoundEmitter = AddComponent<SoundEmitterComponent>("ButtonSound");
}

void GateButton::Start()
{
    // 見た目メッシュは「子」に付けてスケールを掛ける
    if (!m_Model && m_pObjectManager)
    {
        m_Model = m_pObjectManager->Instantiate<GameObject>(GetName() + "_Model", Tag::Object, Transform::One());
        m_Model->TransformRef().SetParent(&this->TransformRef());
        m_Model->SetScale(m_ModelScale);

        auto* r = m_Model->AddComponent<StaticMeshRendererComponent>("ButtonRenderer");
        r->SetMeshRendererKey("GateButton");
        r->SetShaderKey("unlightshader");
        r->SetTransparent(false);
		m_Render = r;
    }

    if (m_TargetDoor) m_SyncedOpen = m_TargetDoor->IsOpen();
}

bool GateButton::CanUseByPlayer() const
{
    auto* player = m_pObjectManager ? m_pObjectManager->GetObjectByTag<Player>(Tag::Player) : nullptr;
    if (!player) return false;

    Vector3 d = player->GetPosition() - GetPosition();
    return d.LengthSquared() <= (m_UseRadius * m_UseRadius);
}

//void GateButton::Update(float dt)
//{
//    m_Cooldown = std::max(0.0f, m_Cooldown - dt);
//    if (!m_TargetDoor) { return; }
//
//    // ここで完全ロック（音も入力も遮断）
//    if (m_TargetDoor->IsMoving()) return;
//
//    if (m_Cooldown > 0.0f) return;
//    if (!m_PlayerInRange)  return;
//
//    if (CDirectInput::GetInstance().GetButtonTrigger(XINPUT_GAMEPAD_X))
//    {
//        // ここに来た時だけ「カチ」
//        //m_SoundEmitter->PlayUIOneShot(SOUND_LABEL_GATE_BUTTON_CLICK, 1.0f);
//
//        m_TargetDoor->Toggle();
//        m_Cooldown = m_CooldownSec;
//    }
//}

void GateButton::Update(float dt)
{
    m_Cooldown = std::max(0.0f, m_Cooldown - dt);
    if (!m_TargetDoor) return;

    // ドア動作中は完全ロック（入力も音も無し）
    if (m_TargetDoor->IsMoving()) return;

    if (m_Cooldown > 0.0f) return;
    if (!CanUseByPlayer()) return;

    if (CDirectInput::GetInstance().GetButtonTrigger(XINPUT_GAMEPAD_X))
    {
        m_TargetDoor->Toggle();
        m_Cooldown = m_CooldownSec;
    }
}

void GateButton::OnCollisionCharacterEnter(GameObject& other)
{
    //if (other.GetTag() == Tag::Player) m_PlayerInRange = true;
}

void GateButton::OnCollisionCharacterExit(GameObject& other)
{
    //if (other.GetTag() == Tag::Player) m_PlayerInRange = false;
}