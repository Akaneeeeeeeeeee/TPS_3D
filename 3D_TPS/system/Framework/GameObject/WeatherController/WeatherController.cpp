#include "WeatherController.h"
#include "Framework/Component/Particle/ParticleComponent.h"

WeatherController::WeatherController(ComponentFactory* factory, const uint64_t id,
	const std::string& name, const Tag& tag,
	const Transform& transform)
	: GameObject(factory, id, name, tag, transform)
{
}

WeatherController::~WeatherController()
{
}

void WeatherController::Init(void)
{
	m_ParticleComp = AddComponent<ParticleComponent>("WeatherParticle");

    // 例えばカメラの少し上にオフセットを置く
	m_ParticleComp->SetLocalOffset(DirectX::XMFLOAT3(0.0f, 200.0f, 0.0f));
}

void WeatherController::Update(const float dt)
{
    GameObject::Update(dt);

    // カメラに追従させたいならここで位置を合わせる
}

void WeatherController::Uninit(void)
{
	GameObject::Uninit();
}