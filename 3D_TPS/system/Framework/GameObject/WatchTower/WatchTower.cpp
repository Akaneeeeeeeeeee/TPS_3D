#include "WatchTower.h"

#include "Framework/Component/Light/SpotLightComponent.h"
#include "Framework/Component/SearchLightController/SearchLightControllerComponent.h"
#include "Framework/Component/Physic/StaticMeshCollider.h"
#include "Framework/Component/Renderer/MeshRenderer/StaticMeshRenderer.h"
#include "Framework/Component/Physic/Rigidbody.h"

static float DegToCos(float deg)
{
    return std::cos(deg * (PI / 180.0f));
}

void WatchTower::Awake()
{
    auto& am = AssetManager::GetInstance();

    // 1) メッシュ取得（登録済み）
    CStaticMesh* mesh = am.GetMesh<CStaticMesh>("WatchTower");

    // 2) 描画コンポーネント（登録済み MeshRendererKey を指定）
    auto* render = AddComponent<StaticMeshRendererComponent>("WatchTowerRenderer");
    render->SetMeshRendererKey("WatchTower");
    render->SetShaderKey("unlightshader");   // 使ってるシェーダに合わせて
    render->SetTransparent(false);

    // 3) コライダー（必要なら）
    if (mesh)
    {
        auto* collider = AddComponent<StaticMeshCollider>("WatchTowerCollider");
        collider->SetMesh(*mesh);
    }

    // 4) Rigidbody（静的）
    {
        auto* rb = AddComponent<Rigidbody>("Rigidbody", 1.0f);
        rb->SetBodyType(Rigidbody::Static);
        rb->SetObjectLayer(Layers::NON_MOVING);
    }

    // 5) SpotLight（同じWatchTowerに付ける）
    auto* spot = AddComponent<SpotLightComponent>("SearchSpot");
    spot->SetEnabled(true);

    // ライト位置：塔の上に置く（LocalOffset は (0,高さ,0) 推奨）
    spot->SetLocalOffset(Vector3(0.0f, 320.0f, 0.0f));

    spot->SetColor(Color(1, 0.95f, 0.8f, 1));
    spot->SetIntensity(10.0f);
    spot->SetRange(2200.0f);
    spot->SetAnglesDeg(12.0f, 18.0f);
    spot->SetTopRadius(8.0f);
    spot->SetNear(80.0f);

    // 6) Controller（首振り）
    auto* ctrl = AddComponent<SearchLightControllerComponent>("SearchCtrl");
    ctrl->SetSpot(spot);
    ctrl->sweepMinDeg = -80.0f;
    ctrl->sweepMaxDeg = 80.0f;
    ctrl->yawSpeedDeg = 40.0f;
    ctrl->pitchDeg = -10.0f;
}

void WatchTower::Update(float dt)
{
}

void WatchTower::Uninit()
{
}