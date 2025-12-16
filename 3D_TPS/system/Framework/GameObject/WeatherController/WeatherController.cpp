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

void WeatherController::Awake(void)
{
	m_ParticleComp = AddComponent<ParticleComponent>("WeatherParticle");
}

void WeatherController::Update(const float dt)
{
    GameObject::Update(dt);

    // ÉJÉÅÉâÇ…í«è]Ç≥ÇπÇΩÇ¢Ç»ÇÁÇ±Ç±Ç≈à íuÇçáÇÌÇπÇÈ
}

void WeatherController::Uninit(void)
{
	GameObject::Uninit();
}