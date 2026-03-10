#include "Gate.h"
#include "Framework/ObjectManager/ObjectManager.h"
#include "Framework/Component/Renderer/MeshRenderer/StaticMeshRenderer.h"
#include "Framework/Component/Physic/BoxCollider.h"
#include "Framework/Component/Physic/Rigidbody.h"
#include "system/CStaticMesh.h"
#include "Framework/AssetManager/AssetManager.h"
#include "Framework/Component/Sound/SoundEmitterComponent.h"

#include <cmath>
#include <algorithm>

static float DegToRad(float deg) { return deg * (PI / 180.0f); }

static Quaternion MakeYawQuatDeg(float yawDeg)
{
    float r = DegToRad(yawDeg);
    float h = r * 0.5f;
    return Quaternion(0.0f, std::sin(h), 0.0f, std::cos(h));
}

// 子の見た目補正（FenceMeshが縦に刺さる対策）
static Quaternion MakePitchQuatDeg(float deg)
{
    float r = DegToRad(deg);
    float h = r * 0.5f;
    return Quaternion(std::sin(h), 0.0f, 0.0f, std::cos(h));
}

static Vector3 SafeNormalize(const Vector3& v)
{
    if (v.LengthSquared() < 1e-6f) return Vector3(1, 0, 0);
    Vector3 n = v;
    n.Normalize();
    return n;
}

static void ComputeLocalAABB(const CStaticMesh& mesh, Vector3& outMin, Vector3& outMax)
{
    const auto& verts = mesh.GetVertices();
    outMin = outMax = verts[0].Position;

    for (auto& v : verts)
    {
        outMin.x = std::min(outMin.x, v.Position.x);
        outMin.y = std::min(outMin.y, v.Position.y);
        outMin.z = std::min(outMin.z, v.Position.z);

        outMax.x = std::max(outMax.x, v.Position.x);
        outMax.y = std::max(outMax.y, v.Position.y);
        outMax.z = std::max(outMax.z, v.Position.z);
    }
}

void Gate::Awake(void)
{
    // Gate = 蝶番ピボット
    m_BaseRot = GetRotation();

    // Gateに入っているscaleは子へ移す。Gate自身は(1,1,1)で固定。
    m_DoorScale = GetScale();
    SetScale(Vector3::One);

	m_SoundEmitter = AddComponent<SoundEmitterComponent>("SoundEmitter");
}

void Gate::Start()
{
    BuildDoorChild();

    m_CurYawDeg = 0.0f;
    m_TargetYawDeg = 0.0f;
    ApplyPivotYaw(m_CurYawDeg);
}

void Gate::SetHingeLocalOffset(const Vector3& dirToCenterLocal)
{
    m_HingeDirLocal = SafeNormalize(dirToCenterLocal);
}

void Gate::BuildDoorChild()
{
    if (!m_pObjectManager) return;
    if (m_Door) return;

    // 子を生成して親子付け
    m_Door = m_pObjectManager->Instantiate<GameObject>(GetName() + "_Door", Tag::Object, Transform::One());

    // 親をGateに
    m_Door->TransformRef().SetParent(&this->TransformRef());

    // 子にスケールを移す（見た目も当たり判定もここで大きくする）
    m_Door->SetScale(m_DoorScale);

    // 見た目
    {
        auto* r = m_Door->AddComponent<StaticMeshRendererComponent>("DoorRenderer");
        r->SetMeshRendererKey("FenceMesh");
        r->SetShaderKey("unlightshader");
        r->SetTransparent(false);
    }

    // 当たり判定（メッシュAABBに合わせる）
    BoxCollider* box = m_Door->AddComponent<BoxCollider>("DoorBox");
    auto* mesh = AssetManager::GetInstance().GetMesh<CStaticMesh>("FenceMesh");
    if (mesh)
    {
        box->FitToMeshLocalAABB(*mesh, 1.f);

        Vector3 mn, mx;
        ComputeLocalAABB(*mesh, mn, mx);
        Vector3 ext = mx - mn;
        printf("[Gate] AABB ext x=%.3f y=%.3f z=%.3f\n", ext.x, ext.y, ext.z);
        Vector3 center = (mn + mx) * 0.5f;

        Vector3 dir = SafeNormalize(m_HingeDirLocal);

        // どの軸で「左右端」を決めるか（X主 / Z主）
        const bool useX = (std::abs(dir.x) >= std::abs(dir.z));

        Vector3 hingePointLocal = center;

        if (useX)
        {
            // dir.x > 0 なら「蝶番→中心が +X」なので蝶番は minX 側
            hingePointLocal.x = (dir.x >= 0.0f) ? mn.x : mx.x;
        }
        else
        {
            hingePointLocal.z = (dir.z >= 0.0f) ? mn.z : mx.z;
        }

        // 親(蝶番)原点に hingePointLocal が来るように、子を逆方向へ移動
        Vector3 sc = m_DoorScale; // 子のスケール
        Vector3 childLocalPos(
            -hingePointLocal.x * sc.x,
            -hingePointLocal.y * sc.y,
            -hingePointLocal.z * sc.z
        );

        m_Door->SetPosition(childLocalPos);
    }
    else
    {
        // メッシュ取れない場合の保険（とりあえず）
        m_Door->SetPosition(Vector3(0, 0, 0));
    }

    // Rigidbody（Kinematic）
    {
        auto* rb = m_Door->AddComponent<Rigidbody>("DoorRB", 1.0f);
        rb->SetBodyType(Rigidbody::Kinematic);
        rb->SetObjectLayer(Layers::MOVING);
    }

    // 見た目の向き補正は「子のローカル回転」に入れる（親はYawだけ回す）
    m_Door->SetRotation(MakePitchQuatDeg(90.0f));
}

void Gate::Toggle(void)
{
    SetOpen(!m_IsOpen);
}

void Gate::SetOpen(bool open)
{
    m_IsOpen = open;
    m_TargetYawDeg = open ? m_OpenYawDeg : 0.0f;

    // 動かないなら何もしない
    if (std::abs(m_TargetYawDeg - m_CurYawDeg) <= 0.01f) return;

    // すでに動作中なら、開始処理を二重にしない
    if (!m_IsMoving)
    {
        m_IsMoving = true;

        // 1) 人間に聞かせる（ループ開始）
        StartMoveLoopSound();

        // 2) 敵AIにだけ「鳴り始め」を1回だけ知らせる
        if (!m_SentStartHear && m_SoundEmitter)
        {
            WorldSoundEvent ev{};
            ev.Position = GetPosition();
            ev.Type = SoundType::Custom;                  // 既存の範囲でOK
            ev.Emitter = SoundEmitterKind::Other;
            ev.Volume = m_MoveSoundVolume;
            ev.Loudness = m_MoveSoundLoudness;
            ev.Radius = m_MoveSoundRadius;

            // 再生側に拾わせない
            ev.PlayLabel = SOUND_LABEL_MAX;

            m_SoundEmitter->EmitSound(ev);
            m_SentStartHear = true;
        }
    }
}

void Gate::Update(float dt)
{
    if (!m_IsMoving) return;

    float diff = m_TargetYawDeg - m_CurYawDeg;
    if (std::abs(diff) < 0.01f)
    {
        m_CurYawDeg = m_TargetYawDeg;
        ApplyPivotYaw(m_CurYawDeg);

        m_IsMoving = false;
        StopMoveLoopSound();

        m_SentStartHear = false; // 次回の開閉でまた1回だけEmitできるように
        return;
    }

    float step = m_SpeedDegPerSec * dt;
    if (std::abs(diff) <= step) m_CurYawDeg = m_TargetYawDeg;
    else m_CurYawDeg += (diff > 0.0f ? step : -step);

    ApplyPivotYaw(m_CurYawDeg);
}

void Gate::ApplyPivotYaw(float yawDeg)
{
    // 親（蝶番）だけYaw回転。子は親子関係で勝手に回る。
    Quaternion yaw = MakeYawQuatDeg(yawDeg);

    //「設置時の回転 + yaw」
    SetRotation(m_BaseRot * yaw);
}


void Gate::StartMoveLoopSound(void)
{
    if (m_LoopPlaying) return;
    if (!m_SoundEmitter) return;

    m_SoundEmitter->StartLoop(SE_GATESOUND, m_MoveSoundVolume);
    m_LoopPlaying = true;
}

void Gate::StopMoveLoopSound(void)
{
    if (!m_LoopPlaying) return;
    if (!m_SoundEmitter) return;

    m_SoundEmitter->StopLoop(SE_GATESOUND);
    m_LoopPlaying = false;
}
